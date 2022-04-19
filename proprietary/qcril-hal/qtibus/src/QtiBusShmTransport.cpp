/*
 * Copyright (c) 2019 Qualcomm Technologies, Inc.
 * All Rights Reserved.
 * Confidential and Proprietary - Qualcomm Technologies, Inc.
 */

#include <QtiMutex.h>
#include "QtiBusShmTransport.h"
#include <fcntl.h>
#include <errno.h>
#include <sys/stat.h>
#include <sys/file.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>
#include <condition_variable>
#include <atomic>
#include <chrono>
#include <iostream>
#include <queue>
#include <iomanip>
#include <ctype.h>
#include "framework/Util.h"

#define TAG "QtiBus"

#define QTIBUS_MAX_CLIENTS 10

#define NEW_STATE(pid, flags, rfcnt) (({ \
            uint64_t __pid = pid; \
            uint16_t __flags = flags; \
            uint16_t __rfcnt = rfcnt; \
            (__pid << 32) | ((__rfcnt & 0xffff) << 16) | (__flags & 0xffff); \
            }))

#define PID_FROM_STATE(state) (({ \
            uint64_t _s = state; \
            (pid_t)((_s >> 32) & 0xffffffff); \
            }))
#define FLAGS_FROM_STATE(state) (({ \
            uint64_t _s = state; \
            (uint16_t)((_s & 0xffff)); \
            }))
#define RFCNT_FROM_STATE(state) (({ \
            uint64_t _s = state; \
            (uint16_t)((_s >> 16) & 0xffff); \
            }))

#define SEGMENT_PAYLOAD_SIZE 16
#define HEADER_SLOTS (4096 / sizeof(struct MessageHdr))

#define STATE_RESERVED (uint16_t)(1 << ((uint16_t)QtiBusShmTransportHdr::StateBits::RESERVED))
#define STATE_READY    (uint16_t)(1 << ((uint16_t)QtiBusShmTransportHdr::StateBits::READY))
#define STATE_DELETED  (uint16_t)(1 << ((uint16_t)QtiBusShmTransportHdr::StateBits::DELETED))

#define ALLOCATE_RETRIES 100
/*
 * QtiBusShmTransport - A QtiBusTransport implented using shared memory.
 *
 * This transport mmaps a specific 2-memory-page-sized file with two segments.
 * The first segment (QtiBusShmTransportHdr) contains the command exchange area (.command member),
 * a client registration area (.clients member), and a list of delivered buffers
 *
 * The list of delivered buffers is stored as a linked list, with the pointers (both next and data)
 * being indices. next is an index within .clients itself, whereas data is an index into the
 * mem segment.
 *
 * The second (mem) segment, is arranged as a series of slots
 */

class dumpHex
{
    private:
        const char *data;
        size_t length;
        static const size_t chars_per_row = 16;
        static const size_t chars_per_block = 8;

    public:
        dumpHex(const char *data, size_t length):
            data(data),
            length(length)
        { }

        template <class S>
        friend S& operator <<(S& os, const dumpHex&obj) {
            std::stringstream o;
            o << std::endl;
            size_t rows = (obj.length / chars_per_row) + ((obj.length % chars_per_row)?1:0);
            for (size_t i = 0; i < rows; i++) {
                o << std::hex;
                o << std::setfill('0');
                o << std::setw(8);
                o << i * chars_per_row << "  ";
                o << std::setw(2) << std::setfill('0');
                size_t j = 0;
                for (j = 0; j < chars_per_row && (i*chars_per_row + j) < obj.length; j++) {
                    o << std::setw(2) << std::setfill('0')
                        <<  ((unsigned)obj.data[i*chars_per_row + j] & 0xff) << " ";
                    if ( ((j + 1) % chars_per_block) == 0) {
                        o << " ";
                    }
                }
                if (j > 0) {
                    for (; j < chars_per_row; j++) {
                        o << "   ";
                        if ( ((j + 1) % chars_per_block) == 0) {
                            o << " ";
                        }
                    }
                }
                o << " ";

                for (j = 0; (j < chars_per_row) && ((i*chars_per_row + j) < obj.length); j++) {
                    const char c = obj.data[i*chars_per_row + j];
                    o << (isprint(c) ? c : '.');
                    if ( ((j + 1) % chars_per_block) == 0) {
                        o << " ";
                    }
                }
                o << std::endl;
            }
            os << o.str();
            return os;
        }
};

#define MAX(a,b) (({ \
            auto __a = a; \
            auto __b = b; \
            __a > __b ? __a : __b; \
            }))
#define MIN(a,b) (({ \
            auto __a = a; \
            auto __b = b; \
            (__a < __b) ? __a : __b; \
            }))
struct Callbacks {
    QtiBusShmTransport::OnClientConnectedCb onClientConnected;
    QtiBusShmTransport::OnClientDeadCb onClientDead;
    QtiBusShmTransport::OnNewMessageCb onNewMessage;
};
/*
 * QtiBusShmTransportHdr. This is a segment in shared memory. It will be seen by all clients using
 * the QtiBus Shared-memory transport implementation (QtiBusShmTransport).
 * Since it resides in shared memory, it is important not to place any pointers in it, that could
 * be dereferenced by remote clients, since the virtual memory addresses will be different between
 * processes. This applies to the internals of objects placed in here as well (e.g. std::string
 * cannot be stored here because it will allocate the actual string value and store a char * in the
 * object. Also, normal mutexes and condition variables will not work (Which is why
 * qtimutex::QtiShmMutex and qtimutex::QtiShmConditionVariable have been provided
 */
struct QtiBusShmTransportHdr {
    enum class StateBits: uint16_t {
        RESERVED = 0,
        READY = 1,
        DELETED = 2,
    };
    volatile std::atomic<unsigned> clientCount{0};

    struct Command {
        pid_t sender;
        CommandId id;
        unsigned arg1, arg2;
        Command() {}
        Command(pid_t sender, CommandId id, unsigned arg1, unsigned arg2):
            sender(sender),
            id(id),
            arg1(arg1),
            arg2(arg2) {}
    };
    /*
     * Command Header structure. This is where commands are actually exchanged.
     * A mutex in shared memory protects access to this structure. A condition variable informs
     * other clients when a new command becomes available.
     * Once a client is aware of a new command having been delivered, it will ack by setting the
     * clientAck bit corresponding to its own registration index (See ClientEntry below). This
     * has to be completed within a specific amount of time (CLIENT_ACK_TIMEOUT). If the timeout
     * expires and some clients didn't ack, Those clients will be considered dead, and their
     * clien entries will be cleared, allowing new clients to connect.
     *
     * Note that only one command can be processed at a time, so receive processing should happen
     * as quickly as possible to prevent throughput degradation.
     */
    struct CommandHdr {
        qtimutex::QtiShmMutex mutex;
        qtimutex::QtiShmConditionVariable cv;
        volatile struct Command cmd;
        volatile unsigned nackCnt;
        volatile std::atomic<unsigned> waiters;
        volatile bool newCommand;
        volatile bool clientAck[QTIBUS_MAX_CLIENTS];
    }command;
#define CLIENT_ACK_TIMEOUT std::chrono::seconds(3)

    void writeCommand(CommandId cmd, unsigned arg1, unsigned arg2) {
        command.cmd.id = cmd;
        command.cmd.arg1 = arg1;
        command.cmd.arg2 = arg2;
    }
    void setCommand(CommandId cmd, unsigned arg1, unsigned arg2) {
        DEBUG_LOG(<< __func__ << " cmd: " << cmd
            << " arg1: " << arg1
            << " arg2: " << arg2
            << " clientCount: " << clientCount << std::endl);

        if (clientCount == 0) { return; }
        std::unique_lock<qtimutex::QtiShmMutex> lock(command.mutex);

        DEBUG_LOG( << "Current command.nackCnt: " << command.nackCnt << std::endl);
        // Wait for the previous command to be delivered
        command.cv.wait(lock,
                [this]() {
                    DEBUG_LOG( << "command.newCommand: " << command.newCommand
                        << "command.waiters: " << command.waiters
                        << " current nackCnt: " << command.nackCnt);
                    return (!command.waiters) && (!command.newCommand) && command.nackCnt == 0;
                });
        DEBUG_LOG(<<"Woke up. command.newCommand: " << command.newCommand
                << "command.waiters: " << command.waiters
                << " Current nackCnt: " << command.nackCnt);

        command.newCommand = true;
        command.cmd.sender = getpid();
        writeCommand(cmd, arg1, arg2);
        command.cv.notify_all();

        DEBUG_LOG(<< __func__ << "< cmd: " << cmd
            << " arg1: " << arg1
            << " arg2: " << arg2
            << " clientCount: " << clientCount
            << " nackCnt: " << command.nackCnt << std::endl);
    }

    /*
     * ClientEntry is a lock-free structure that uses atomic variables to prevent concurrent
     * acquisition. As such, the order of operations is important. Be careful while updating
     * this code.
     *
     * Clients first try to reserve an entry, by atomically checking that the
     * reserved flag is false and setting the value to true. If the flag was not false (meaning
     * that the entry was not free) another entry is attempted.
     *
     * Once an entry is reserved, it should be ignored by other clients for reservation. A reserved
     * (but not enabled) entry will also be ignored by command senders.
     *
     * When the entry finally becomes enabled (after all its attributes have been set by the
     * reserving cient), it is reported to other clients with the NEW_CLIENT command, and it is
     * treated as an active client and will participate in the ack/nack protocol.
     *
     */
    struct ClientEntry {
        public:
            // Reserved: a client is setting up the entry
            std::atomic<uint64_t> state {0};

            /** tryToReserve
             * @ret true if reservation is successful
             */
            bool tryToReserve() volatile {
                uint64_t expected = 0;
                uint64_t newval = NEW_STATE(getpid(),
                        STATE_RESERVED,
                        1);
                bool ret = false;

                if (enabled()) {
                    return ret;
                }

                ret = state.compare_exchange_strong(expected, newval);
                if (ret) {
                    ret = true;
                }
                return ret;
            }
            pid_t clientId() volatile {
                return PID_FROM_STATE(state);
            }

            bool reserved() volatile {
                return (FLAGS_FROM_STATE(state) & STATE_RESERVED) != 0;
            }

            bool enabled() volatile {
                return (FLAGS_FROM_STATE(state) & STATE_READY) != 0;
            }

            bool activate() volatile {
                uint64_t expected = NEW_STATE(getpid(),
                        STATE_RESERVED,
                        1);
                uint64_t newstate = NEW_STATE(getpid(),
                        STATE_RESERVED|STATE_READY,
                        1);
                bool rc = state.compare_exchange_strong(expected, newstate);
                return rc;
            }
            bool release(pid_t pid) volatile {
                uint64_t expected = state;
                if (PID_FROM_STATE(expected) != pid) {
                    return false;
                }
                bool rc = state.compare_exchange_strong(expected, 0);

                return rc;
            }
    } volatile clients[QTIBUS_MAX_CLIENTS];

    void clearAcks(bool local) {
        for (unsigned i = 0; i < QTIBUS_MAX_CLIENTS; i++) {
            if(clients[i].enabled() && (local || (!local && clients[i].clientId() != getpid()))) {
                command.clientAck[i] = false;
            } else {
                command.clientAck[i] = true;
            }
        }
    }

    std::vector<pid_t> resetDeadClient() {
        std::vector<pid_t> ret;
        unsigned initialCount = clientCount;
        unsigned currentCount = initialCount;
        DEBUG_LOG( << "Initial Client Count: " << initialCount << std::endl);
        for (unsigned i = 0; i < QTIBUS_MAX_CLIENTS; i++) {
            if (!command.clientAck[i]) {
                pid_t pid = clients[i].clientId();
                if (clients[i].release(pid) ) {
                    ret.push_back(pid);
                    bool rc = false;
                    if (currentCount > 0) {
                        rc = clientCount.compare_exchange_strong( currentCount, currentCount - 1);
                        if (rc) {
                            DEBUG_LOG( << "Failed to set clientCount to " << currentCount - 1);
                        }
                        currentCount = clientCount;
                        if (rc) {
                            DEBUG_LOG( << "new currentCount " << initialCount );
                        }
                    }

                    command.clientAck[i] = true;
                    freeMessagesForClient(pid);
                }
            }
        }
        command.nackCnt = 0;
        command.cv.notify_all();
        DEBUG_LOG( << "Final Client Count: " << clientCount << std::endl);
        return ret;

    }
    enum class ClientRegistrationResult
    {
        SUCCESS,
        TABLE_FULL,
        UNKNOWN,
    };
#define MAX_REGISTRATION_ATTEMPTS 10
    ClientRegistrationResult registerLocalClient(size_t &idx) {
        DEBUG_LOG( << "Initial Client Count: " << clientCount << std::endl);
        ClientRegistrationResult ret = ClientRegistrationResult::UNKNOWN;
        // Find and reserve pid slot
        bool cont = true;
        size_t i;
        for (size_t attempt = 0; cont && attempt < MAX_REGISTRATION_ATTEMPTS; attempt ++) {
            for (i = 1; i < QTIBUS_MAX_CLIENTS && cont; i++) {
                cont = !clients[i].tryToReserve();
                if (!cont) break;
            }
            if (!cont && i < QTIBUS_MAX_CLIENTS) {
                clients[i].activate();
                clientCount ++;
                ret = ClientRegistrationResult::SUCCESS;
                idx = i;
            } else {
                ret = ClientRegistrationResult::TABLE_FULL;
                sleep(1);
            }
        }
        DEBUG_LOG( << "Final Client Count: " << clientCount << std::endl);
        return ret;
    }
    void deregisterLocalClient(size_t idx) {
        unsigned currentCount = clientCount;
        DEBUG_LOG( << "Current pid: " << getpid()
            << ". Registered pid: " << clients[idx].clientId() << std::endl);
        DEBUG_LOG( << "Client Count before: " << currentCount << std::endl);
        if (clients[idx].release(getpid())) {
            DEBUG_LOG( << "Released client" << clientCount << std::endl);
        }
        DEBUG_LOG( << "Client Count after: " << clientCount << std::endl);
    }

    std::atomic<size_t> tail{0};

    struct MessageHdr {
        int f = -1;
        std::atomic<uint64_t> state;
        size_t size;
        size_t data;

        pid_t owner() volatile {
            return PID_FROM_STATE(state);
        }
        uint16_t flags() volatile {
            return FLAGS_FROM_STATE(state);
        }

        uint16_t refCnt() volatile {
            return RFCNT_FROM_STATE(state);
        }

        bool tryToReserve(size_t size) volatile {
            uint64_t expected = 0;

            bool rc = state.compare_exchange_strong(expected,
                    (((uint64_t)getpid()) << 32) |
                    STATE_RESERVED);

            if (rc) {
                this->size = size;
            }

            return rc;
        }
        bool activate(uint16_t refcnt) volatile {
            bool ret = false;

            uint64_t expected = (((uint64_t)getpid()) << 32) |
                STATE_RESERVED;
            uint64_t news = (expected & 0xffffffff0000ffff) | (refcnt << 16)
                | STATE_RESERVED | STATE_READY;
            DEBUG_LOG( << "Current State: 0x" << std::hex << state <<
                    " Expected: 0x" << expected <<
                    " Setting state to 0x" << news << std::dec << std::endl);
            ret = state.compare_exchange_strong(expected,
                    news);
            DEBUG_LOG( << (ret ? "Success" : "Failure")
                    << " State after: 0x" << std::hex << state << std::dec << std::endl);
            return ret;
        }

        bool incr() volatile {
            uint64_t expected = state;
            bool ret = false;
            bool cont = true;
            uint16_t rfcnt = 0;

            while(cont) {
                rfcnt = RFCNT_FROM_STATE(expected);
                if (rfcnt > 0 || rfcnt < USHRT_MAX) {
                    uint64_t news = (expected & 0xffffffff0000ffff) |
                        ((rfcnt + 1) << 16);
                    ret = state.compare_exchange_strong(expected,
                            news);
                    cont = !ret;
                    DEBUG_LOG( << (ret? "Success": "Failure"));
                } else {
                    cont = false;
                }
            }

            return ret;
        }

        uint16_t decr() volatile {
            uint64_t expected = state;
            bool rc = false;
            bool cont = true;
            uint16_t rfcnt = 0;
            uint16_t ret = rfcnt;

            DEBUG_LOG(<< "Expected: 0x" << std::hex << expected << std::dec
                    << " State before: 0x" << std::hex << state << std::dec);
            while (cont) {
                rfcnt = RFCNT_FROM_STATE(expected);
                if (rfcnt > 0) {
                    rfcnt --;
                    uint64_t news = (expected & 0xffffffff0000ffff) |
                        (rfcnt << 16);
                    if (rfcnt > 0) {
                        rc = state.compare_exchange_strong(expected,
                                news);
                        if (rc) {
                            ret = rfcnt;
                        } else {
                            ret = RFCNT_FROM_STATE(expected);
                        }
                    } else {
                        ret = 0;
                        rc = release();
                    }
                    cont = !rc;
                    DEBUG_LOG( << (rc? "Success": "Failure"));
                } else {
                    cont = false;
                }
            }
            DEBUG_LOG(<< "State after: 0x" << std::hex << state << std::dec
                    << " ret: " << ret);
            return ret;
        }

        bool release() volatile {
            state = 0;
            data = 0;
            return true;
        }
        bool deleted() volatile {
            uint16_t flags = FLAGS_FROM_STATE(state);
            return (flags & STATE_DELETED) != 0;
        }
    };
    struct MessageHdr volatile entry [HEADER_SLOTS] __attribute__((aligned(0x1000)));

    struct QtiBusShmTransportMem {
        std::atomic<uint64_t> state;
        size_t size;
        size_t sequence_id;
        size_t next;
        char data[SEGMENT_PAYLOAD_SIZE];

        pid_t owner() volatile {
            return PID_FROM_STATE(state);
        }
        uint16_t flags() volatile {
            return FLAGS_FROM_STATE(state);
        }

        uint16_t refCnt() volatile {
            return RFCNT_FROM_STATE(state);
        }

        bool tryToReserve() volatile {
            uint64_t expected = 0;

            bool rc = state.compare_exchange_strong(expected,
                    ((uint64_t)getpid()) << 32 |
                    (uint16_t)STATE_RESERVED);

            return rc;
        }
        bool release() volatile {
            bool ret = true;

            size = 0;
            sequence_id = 0;
            next = 0;
            state = 0;

            return ret;
        }
    };

#define MEM_SLOTS (4096 / sizeof(struct QtiBusShmTransportMem))
#define MAX_MSG_LEN          (SEGMENT_PAYLOAD_SIZE * MEM_SLOTS)
    QtiBusShmTransportMem mem[MEM_SLOTS] __attribute__((aligned(0x1000)));
    void freeMessagesForClient(pid_t pid) {

        for (size_t idx = 0; idx < HEADER_SLOTS; idx++) {
            uint64_t state = entry[idx].state;
            if ((PID_FROM_STATE(state) == pid) &&
                    (FLAGS_FROM_STATE(state) & STATE_RESERVED)) {

                uint64_t desired = 0;
                bool rc = false;
                uint16_t refcnt = entry[idx].refCnt();

                DEBUG_LOG(<< "refCnt: " << refcnt);
                if (refcnt > 0) {
                    desired = state | STATE_DELETED;
                    while (!rc) {
                        rc = entry[idx].state.compare_exchange_strong(state,
                                desired);
                        if (!rc && entry[idx].owner() != pid) {
                            DEBUG_LOG(<< "Owner already changed");
                            break;
                        }
                        desired = state | STATE_DELETED;
                    }
                } else {
                    while (!rc) {
                        rc = entry[idx].state.compare_exchange_strong(state,
                                desired);
                        if (!rc && (entry[idx].owner() != pid || entry[idx].refCnt() != 0) ) {
                            break;
                        }
                    }
                }

                if (rc) {
                    DEBUG_LOG(<< "Freeing buffers for idx " << idx);
                    size_t data = entry[idx].data;
                    freeDataBuffers(data);
                } else {
                    DEBUG_LOG(<< "Not freeing buffers for idx " << idx);
                }
                DEBUG_LOG(<< "Final state: 0x" << std::hex << entry[idx].state << std::dec);
            }
        }
    }

    void freeDataBuffers(size_t idx) {
        size_t data = idx;
        while (data) {
            DEBUG_LOG(<< "Freeing buffer " << data);
            size_t next = mem[data].next;
            mem[data].release();
            data = next;
        }
    }

    /*
     * This should only be applied to the first entry in the list, because it will remove the entry
     * from the list, which can only be done safely when the entry is at the head of the list.
     */
    uint16_t decrRefCnt(size_t idx) {
        uint16_t rfcnt = 0;

        size_t data = entry[idx].data;
        if (!entry[idx].decr()) {
            freeDataBuffers(data);
        }

        return rfcnt;
    }

    size_t allocateHdr(size_t size) {
        bool found = false;
        size_t i = 1;
        size_t ret = 0;
        for (; i < HEADER_SLOTS; i++) {
            found = entry[i].tryToReserve(size);
            if (found) break;
        }
        if (found && i < HEADER_SLOTS) {
            ret = i;
        }
        return ret;

    }
    size_t allocateBuffer() {
        bool found = true;
        size_t i = 1;
        size_t ret = 0;

        for (; i < MEM_SLOTS; i++) {
            found = mem[i].tryToReserve();
            if (found) break;
        }
        if (found && i < MEM_SLOTS) {
            ret = i;
        }

        return ret;
    }

    bool activateHdr(size_t idx) {
        DEBUG_LOG( << "Trying to activate idx " << idx);
        bool ret = entry[idx].activate(clientCount);
        if (!ret) {
            DEBUG_LOG( << "Failure activating entry at idx " << idx);
        }
        return ret;
    }
    bool setupBuffer(std::string payload, size_t &out) {
        size_t hdr = 0;
        for (int i = 0; hdr == 0 && i < ALLOCATE_RETRIES; i++) {
            hdr = allocateHdr(payload.size());
        }

        if (!hdr) return false;
        DEBUG_LOG( << "Allocated hdr " << hdr);

        size_t prev = 0;
        size_t remainder = payload.size();
        for (size_t i = 1; remainder > 0; i ++) {
            size_t seg = 0;
            for (int j = 0; seg == 0 && j < ALLOCATE_RETRIES; j++) {
                seg = allocateBuffer();
                DEBUG_LOG(<< "New buffer allocated: " << seg);
            }
            if (seg == 0) {
                DEBUG_LOG(<< "Releasing hdr " << hdr);
                entry[hdr].release();
                return false;
            }
            if (seg) {
                if (i == 1) {
                    entry[hdr].data = seg;
                }
                if (prev) {
                    mem[prev].next = seg;
                }
                size_t ssize = MIN(remainder,SEGMENT_PAYLOAD_SIZE);
                mem[seg].sequence_id = i;
                mem[seg].size = ssize;
                payload.copy(mem[seg].data, ssize, payload.size() -  remainder);
                remainder -= ssize;
                prev = seg;
            }
        }
        out = hdr;
        return true;
    }
    bool write(std::string buffer) {
        size_t hdr = 0;
        if (setupBuffer(buffer, hdr)) {
            DEBUG_LOG(<< "Activating header " << hdr);
            activateHdr(hdr);
            setCommand(CommandId::NEW_MESSAGE, hdr, 0);
        }
        return true;
    }
    friend class CommandProcessor;
    class CommandProcessor {
        private:
            QtiBusShmTransportHdr &owner;
            QtiBusShmTransport::OnClientConnectedCb onClientConnected;
            QtiBusShmTransport::OnClientDeadCb onClientDead;
            QtiBusShmTransport::OnNewMessageCb onNewMessage;
            std::queue<Command> incoming;
            std::queue<Command> outgoing;
            qtimutex::QtiSharedMutex mutex;
            std::condition_variable_any cv;
            std::thread th;
            bool finish = false;
            bool started = false;

            void processor() {
                setThreadName("QtiBusShm-PROC");
                std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
                started = true;
                cv.notify_all();
                while (!finish) {
                    std::string buffer;
                    pid_t deadClientId;
                    pid_t newClientId;
                    while (!incoming.empty()) {
                        Command command = incoming.front();
                        incoming.pop();
                        DEBUG_LOG(<< "Unlocking before processing");
                        lock.unlock();
                        DEBUG_LOG(<< "Unlocked before processing");
                        CommandId cmd = command.id;
                        DEBUG_LOG(<< "Processing incoming command " << cmd
                                << " sender: " << command.sender
                                << " arg1: " << command.arg1);
                        switch (cmd) {
                            case CommandId::NEW_CLIENT:
                                if (command.sender != getpid()) {
                                    newClientId = command.sender;
                                    if (onClientConnected) {
                                        DEBUG_LOG(<< "Calling onClientConnected");
                                        onClientConnected(newClientId);
                                    } else {
                                        DEBUG_LOG(<< "onClientConnected is null");
                                    }
                                }
                                break;
                            case CommandId::NEW_MESSAGE:
                                {
                                    buffer = "";
                                    DEBUG_LOG(<< "NEW_MESSAGE");
                                    unsigned hdr = command.arg1;
                                    DEBUG_LOG( << "hdr: " << hdr);
                                    DEBUG_LOG( << "Owner: " << owner.entry[hdr].owner());
                                    DEBUG_LOG( << "Deleted: " << owner.entry[hdr].deleted());
                                    DEBUG_LOG( << "Size: " << owner.entry[hdr].size);

                                    if(owner.entry[hdr].deleted()) {
                                        DEBUG_LOG( << "Ignoring deleted entry");
                                    } else {
                                        if (command.sender != getpid()) {
                                            DEBUG_LOG(<< "Pid matches");
                                            assert(owner.entry[hdr].owner() == command.sender);
                                            if (hdr > 0) {
                                                DEBUG_LOG(<< "hdr is valid");
                                                uint64_t state = owner.entry[hdr].state;
                                                uint16_t refcnt = RFCNT_FROM_STATE(state);
                                                uint16_t flags = FLAGS_FROM_STATE(state);
                                                DEBUG_LOG( << "refcnt: " << refcnt
                                                        << " flags: " << flags );
                                                if (refcnt > 0 &&
                                                    flags & STATE_READY &&
                                                    !(flags & STATE_DELETED)) {
                                                    DEBUG_LOG(<< "Entry " << hdr
                                                            << " is processable");

                                                    unsigned data = owner.entry[hdr].data;
                                                    for(unsigned i = 0 ;
                                                            data ;
                                                            data = owner.mem[data].next, i++) {
                                                        buffer += std::string(owner.mem[data].data,
                                                                owner.mem[data].size);
                                                        DEBUG_LOG(<< "Buffer " << data
                                                                << " "
                                                                << dumpHex(owner.mem[data].data,
                                                                    sizeof(owner.mem[data].size)));
                                                        assert(i == owner.mem[data].sequence_id);
                                                    }
                                                    DEBUG_LOG(<< "Buffer for entry " << hdr
                                                            << " is: <<" <<
                                                            dumpHex(buffer.c_str(),buffer.size()) << ">>");
                                                    assert(owner.entry[hdr].size == buffer.size());
                                                    DEBUG_LOG(<< "onNewMessage: "
                                                            << (onNewMessage ? "true" : "false"));
                                                    if (onNewMessage) {
                                                        DEBUG_LOG(<< "Calling callback");
                                                        onNewMessage(buffer);
                                                    }
                                                }
                                            }
                                        }
                                    }
                                    DEBUG_LOG(<< "Decrementing refcount for hdr " << hdr);
                                    owner.decrRefCnt(hdr);
                                }
                                break;
                            case CommandId::CLIENT_DEAD:
                                if (command.sender != getpid()) {
                                    deadClientId = command.arg1;
                                    onClientDead(deadClientId);
                                }
                                break;
                            case CommandId::REQUEST_ACK:
                                break;
                            case CommandId::LOCAL_DISCONNECT:
                                break;
                        }
                        DEBUG_LOG(<< "Locking after processing");
                        lock.lock();
                        DEBUG_LOG(<< "Locked after processing");
                    }
                    while (!outgoing.empty()) {
                        Command command = outgoing.front();
                        outgoing.pop();
                        lock.unlock();
                        DEBUG_LOG(<<"Sending Outgoing Command " << command.id);
                        owner.setCommand(command.id, command.arg1, command.arg2);
                        lock.lock();
                    }
                    DEBUG_LOG(<< "Waiting on condvar");
                    cv.wait(lock, [this]() { return finish || !incoming.empty() || !outgoing.empty(); });
                    DEBUG_LOG(<< "Woke up");
                }
                DEBUG_LOG(<<"Finishing");
            }
        public:
            CommandProcessor(QtiBusShmTransportHdr &owner,
                    QtiBusShmTransport::OnClientConnectedCb onClientConnected,
                    QtiBusShmTransport::OnClientDeadCb onClientDead,
                    QtiBusShmTransport::OnNewMessageCb onNewMessage):
                owner(owner),
                onClientConnected(onClientConnected),
                onClientDead(onClientDead),
                onNewMessage(onNewMessage)
            {
            }
            void sendCommand(CommandId cmd, unsigned arg1, unsigned arg2) {
                std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
                DEBUG_LOG( << "Inserting as outgoing " << cmd);
                outgoing.emplace(gettid(), cmd, arg1, arg2);
                cv.notify_all();
            }
            void commandReceived(pid_t sender, CommandId cmd, unsigned arg1, unsigned arg2) {
                std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
                DEBUG_LOG( << "Inserting as incoming " << cmd);
                incoming.emplace(sender, cmd, arg1, arg2);
                cv.notify_all();
            }
            void stop() {
                {
                    std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
                    finish = true;
                    cv.notify_all();
                }
                th.join();
            }
            void start() {
                std::unique_lock<qtimutex::QtiSharedMutex> lock(mutex);
                th = std::thread(&CommandProcessor::processor, this);
                cv.wait(lock, [this] () { return started;});
                DEBUG_LOG(<< "Processor Thread started");
            }
    };
    using SetStartCb = std::function<void(bool)>;
    void monitorCmd(
            QtiBusShmTransport::OnClientConnectedCb onClientConnected,
            QtiBusShmTransport::OnClientDeadCb onClientDead,
            QtiBusShmTransport::OnNewMessageCb onNewMessage,
            SetStartCb setStarted) {
        setThreadName("QtiBusShm-MON");
        bool finish = false;
        DEBUG_LOG( << "registering local client" << std::endl);
        size_t idx = 0;
        ClientRegistrationResult rc = registerLocalClient(idx);
        if (rc != ClientRegistrationResult::SUCCESS) {
            DEBUG_LOG( << "Failed local client registration Aborting" );
            abort();
        }
        DEBUG_LOG( << "local client registered as idx " << idx << std::endl);

        CommandProcessor processor(*this,
                onClientConnected,
                onClientDead,
                onNewMessage);
        processor.start();
        processor.sendCommand(CommandId::NEW_CLIENT, getpid(), 0);

        setStarted(true);

        {
            std::unique_lock<qtimutex::QtiShmMutex> lock(command.mutex);
            while (!finish) {

                if (command.nackCnt == 0 && !command.newCommand) {
                    command.cv.wait(lock,
                            [this]() { return command.newCommand || command.nackCnt > 0;});
                }

                if (command.newCommand && command.nackCnt == 0) {
                    DEBUG_LOG( << "Updating nackCnt");
                    command.nackCnt = clientCount;
                    clearAcks(true);
                    command.newCommand = false;
                    DEBUG_LOG( << "newCommand: " << command.newCommand
                            << " nackCnt: " << command.nackCnt);
                    lock.unlock();
                    command.cv.notify_all();
                    lock.lock();
                }

                if (command.cmd.sender == getpid()) {
                    DEBUG_LOG( << "Received command "
                            << command.cmd.id << " From local client" << std::endl);
                } else {
                    DEBUG_LOG( << "Received command "
                            << command.cmd.id << " From remote client "
                            << command.cmd.sender << std::endl);
                }

                DEBUG_LOG(<< "Before processing command command.nackCnt: " << command.nackCnt);
                switch (command.cmd.id) {
                    case CommandId::LOCAL_DISCONNECT:
                        if (command.cmd.sender == getpid()) {
                            finish = true;
                            unsigned currentCount = clientCount;
                            DEBUG_LOG( << "Client Count before: " << currentCount << std::endl);
                            bool rc = false;
                            while (!rc && clientCount > 0) {
                                DEBUG_LOG(<< "Decrementing client count");
                                rc = clientCount.compare_exchange_strong(currentCount, currentCount - 1);
                            }
                            freeMessagesForClient(getpid());
                        }
                        break;
                    default:
                        processor.commandReceived(command.cmd.sender,
                                command.cmd.id,
                                command.cmd.arg1,
                                command.cmd.arg2);
                        break;
                }

                if (clients[idx].clientId() != getpid() ) {
                    DEBUG_LOG( << "(" << getpid()
                            << ") Got kicked out of previous slot "
                            << idx << ". Registering again" << std::endl);
                    Callbacks cb { onClientConnected, onClientDead, onNewMessage };
                    QtiBusShmTransportHdr::ClientRegistrationResult rc = registerLocalClient(idx);
                    if (rc != ClientRegistrationResult::SUCCESS) {
                        DEBUG_LOG( << "Failed local client registration Aborting" );
                        abort();
                    }
                    DEBUG_LOG( << "(" << getpid() << ") New idx: " << idx << std::endl);
                    continue;
                }

                if (command.nackCnt > 0 && !command.clientAck[idx]) {
                    command.clientAck[idx] = true;
                    (command.nackCnt)--;

                    if (command.nackCnt == 0) {
                        DEBUG_LOG(<<"command.nackCnt is 0. Notifying clients");
                        command.cv.notify_all();
                    } else {
                        DEBUG_LOG(<<"Updated nackCnt: " << command.nackCnt <<
                                " clientAck[idx]: " << command.clientAck[idx]);
                    }
                }
                DEBUG_LOG(<< "After processing command command.nackCnt: " << command.nackCnt);

                command.waiters ++;
                qtimutex::QtiShmCVStatus rc = command.cv.wait_until(lock,
                        std::chrono::steady_clock::now() + CLIENT_ACK_TIMEOUT,
                        [this]() {
                            DEBUG_LOG( << "command.nackCnt: " << command.nackCnt);
                            DEBUG_LOG( << "command.newCommand: " << command.newCommand);
                            DEBUG_LOG( << "command.waiters: " << command.waiters);
                            bool ret = command.nackCnt == 0;
                            return ret ;});

                command.waiters --;
                DEBUG_LOG( << "After wait: command.nackCnt: " << command.nackCnt);
                DEBUG_LOG( << "After wait: command.newCommand: " << command.newCommand);
                DEBUG_LOG( << "After wait: command.waiters: " << command.waiters);
                if (rc == qtimutex::QtiShmCVStatus::timeout) {
                    DEBUG_LOG(<< "Timeout");
                    std::vector<pid_t> dead = resetDeadClient();
                    for(pid_t pid: dead) {
                        processor.sendCommand(CommandId::CLIENT_DEAD, pid, 0);
                    }
                    DEBUG_LOG(<< "After reset dead: command.nackCnt: " << command.nackCnt);
                } else {
                    DEBUG_LOG(<< "No timeout");
                    if (!command.waiters) {
                        command.cv.notify_all();
                    }
                }

            }
        }

        deregisterLocalClient(idx);
        setStarted(false);
        DEBUG_LOG(<< "Stopping processor");
        processor.stop();
        DEBUG_LOG(<< "Stopped processor");
    }
    void dumpData(size_t idx, size_t depth) volatile {
        std::string indent;
        if (idx >= MEM_SLOTS) {
            return;
        }
        for (size_t i = 0; i < depth; i++) {
            indent += "    ";
        }
        std::stringstream o;
        for (size_t i = 0; i < SEGMENT_PAYLOAD_SIZE / 16; i++) {
            o << std::hex;
            o << std::setfill('0');
            o << std::setw(8);
            o << i * 16 << "  " << std::setw(2);
            size_t j = 0;
            for (j = 0; j < 16 && (i*16 + j) < mem[idx].size; j++) {
                o << (unsigned) mem[idx].data[i*16 + j];
                if ( ((j + 1) % 8) == 0) {
                    o << " ";
                }
            }
            if (j > 0) {
                for (; j < 16; j++) {
                    o << "   ";
                    if ( ((j + 1) % 8) == 0) {
                        o << " ";
                    }
                }
            }
            o << " ";

            for (j = 0; j < 16 && (i*16 + j) < mem[idx].size; j++) {
                const char c = mem[idx].data[i*16 + j];
                o << ((c > 32 && c < 127) ? c : '.');
            }
            o << std::endl;
        }
        DEBUG_LOG(<< indent << o.str());
    }
    void dump() volatile {
        DEBUG_LOG(<< "mmapped at: " << (void *) this);
        DEBUG_LOG(<< "Command Info");
        if (command.nackCnt > 0) {
            DEBUG_LOG(<< "nackCnt: " << command.nackCnt);
            DEBUG_LOG(<< "sender: " << command.cmd.sender);
            DEBUG_LOG(<< "cmd: " << command.cmd.id);
            DEBUG_LOG(<< "arg1: " << command.cmd.arg1);
            DEBUG_LOG(<< "arg2: " << command.cmd.arg2);
            for (unsigned i = 0; i < QTIBUS_MAX_CLIENTS; i++) {
                DEBUG_LOG(<< "    client" << i + 1 << ": " << command.clientAck[i]);
            }
        }
        DEBUG_LOG(<< "Registered client Info. Client Slots: " << QTIBUS_MAX_CLIENTS);
        for (unsigned i = 0; i < QTIBUS_MAX_CLIENTS; i++) {
            DEBUG_LOG(<< "    Client Entry " << i);
            DEBUG_LOG(<< "        reserved: " << clients[i].reserved());
            DEBUG_LOG(<< "        enabled: " << clients[i].enabled());
            DEBUG_LOG(<< "        clientId: " << clients[i].clientId());
        }
        DEBUG_LOG(<< "Msg Hdr Info. HEADER_SLOTS: " << HEADER_SLOTS);
        for (size_t i = 0; i < HEADER_SLOTS; i++) {
            DEBUG_LOG(<< "    Msg Hdr " << i);
            DEBUG_LOG(<< "        state: 0x" << std::hex << entry[i].state);
            DEBUG_LOG(<< "            owner: " << std::dec << entry[i].owner());
            DEBUG_LOG(<< "            flags: 0x" << std::hex << entry[i].flags());
            DEBUG_LOG(<< "                ["
                    << (entry[i].flags() & STATE_RESERVED ? "RESERVED " : "")
                    << (entry[i].flags() & STATE_READY ? "READY " : "")
                    << (entry[i].flags() & STATE_DELETED ? "DELETED": "")
                    << "]");
            DEBUG_LOG(<< "            refCnt: " << std::dec << entry[i].refCnt());
        }
        DEBUG_LOG(<< "Buffer Info. MEM_SLOTS: " << MEM_SLOTS);
        for (size_t i = 0; i < MEM_SLOTS; i++ ) {
            DEBUG_LOG(<< "    Buffer " << i);
            DEBUG_LOG(<< "        state: 0x" << std::hex << mem[i].state);
            DEBUG_LOG(<< "            owner: " << std::dec << mem[i].owner());
            DEBUG_LOG(<< "            flags: 0x" << std::hex << mem[i].flags());
            DEBUG_LOG(<< "                ["
                    << (mem[i].flags() & STATE_RESERVED ? "RESERVED " : "")
                    << (mem[i].flags() & STATE_READY ? "READY " : "")
                    << (mem[i].flags() & STATE_DELETED ? "DELETED": "")
                    << "]");
            DEBUG_LOG(<< "            refCnt: " << std::dec << mem[i].refCnt());
            DEBUG_LOG(<< "        size: " << mem[i].size);
            DEBUG_LOG(<< "        sequence_id: " << mem[i].sequence_id);
            DEBUG_LOG(<< "        next: " << mem[i].next);
            DEBUG_LOG(<< "        data: ");
            dumpData(i, 3);
        }
    }
};

void *QtiBusShmTransport::mmapfd(int fd, bool init) {
    size_t pgsize = sysconf(_SC_PAGE_SIZE);
    void * ret = MAP_FAILED;

    ret = mmap(nullptr, 3 * pgsize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);

    if (ret != MAP_FAILED) {
        this->ptr = ret;
        if (init) {
            header = new(ret) QtiBusShmTransportHdr();
        } else {
            header = static_cast<QtiBusShmTransportHdr *>(ret);
        }
        header_end = static_cast<char *>(ret) + 3 * pgsize;
    }


    return ret;
}

int QtiBusShmTransport::openTransport(const char *fname) {

    if(getStarted()) {
        return -1;
    }

    // Open file. Use the O_EXCL flag to detect if other
    // process already created it.
    mode_t mode = S_IRUSR | S_IWUSR;
    errno = 0;
    int fd = open(fname, O_CREAT|O_EXCL|O_RDWR, mode);
    int rc = 0;

    if (fd < 0) {
        if (errno == EEXIST) {
            // The file already exists, which probably means
            // it is already initialized.
            //
            // Wait 1 sec to give the other process a chance to lock and
            // initialize before attempting to acquire share lock.
            sleep(1);

            // Proceed to lock as shared
            fd = open(fname, O_RDWR, mode);
            goto lock_shared;
        }
    }

    // Lock the file exclusively
    do {
        errno = 0;
        rc = flock(fd, LOCK_EX);
    } while (rc < 0 && errno == EINTR);
    if (rc == 0) {
        // If exclusive locking succeeds, assume uninitalized.
        //    Proceed to shared memory initialization
        unsigned buf = 0;
        ssize_t count = ::write(fd, &buf, sizeof(buf));
        void *ptr = mmapfd(fd, true);
        if (count == sizeof(buf) && ptr != MAP_FAILED) {
            // Remove exclusive lock
            rc = flock(fd, LOCK_UN);
        } else {
            abort();
        }

    }

    //    Unlock exclusive and lock shared.
    // If exclusive locking fails, lock as shared
    // If shared locking fails, wait and try again.
    // If shared locking succeeds, assume already initialized
lock_shared:
    do {
        errno = 0;
        rc = flock(fd,LOCK_SH);
    } while (rc < 0 && errno == EINTR);

    if (rc == 0) {
        if (ptr == nullptr) {
            ptr = mmapfd(fd, false);
            if (ptr == MAP_FAILED) {
                abort();
            }
        }
        if (header) {
            header->dump();
        }

        DEBUG_LOG( << "Starting monitorCmd thread" << std::endl);
        commandReceiver =
            std::thread{&QtiBusShmTransportHdr::monitorCmd,
                header,
                onClientConnected,
                onClientDead,
                onNewMessage,
                [this](bool started) {setStarted(started);}};

        waitUntilStarted();
        sendCommand(CommandId::REQUEST_ACK, 0, 0);
    }

    return fd;

}

void QtiBusShmTransport::sendCommand(CommandId cmd, unsigned arg1, unsigned arg2) {
    DEBUG_LOG( << "> " << __func__ << std::endl);
    if (header) {
        Callbacks cb{onClientConnected, onClientDead, onNewMessage};
        header->setCommand(cmd, arg1, arg2);
    }
    DEBUG_LOG( << "< " << __func__ << std::endl);
}

bool QtiBusShmTransport::registerLocalClient(size_t &idx) {
    bool ret = false;
    if (header) {
        QtiBusShmTransportHdr::ClientRegistrationResult rc =
            QtiBusShmTransportHdr::ClientRegistrationResult::UNKNOWN;

        Callbacks cb { onClientConnected, onClientDead, onNewMessage };
        rc = header->registerLocalClient(idx);
        sendCommand(CommandId::NEW_CLIENT, getpid(), 0);
        if(rc == QtiBusShmTransportHdr::ClientRegistrationResult::SUCCESS) {
            ret = true;
        }
    }
    return ret;
}

bool QtiBusShmTransport::write( const std::string &buffer)
{
    bool ret = false;
    if (getStarted() && header) {
        ret = header->write(buffer);
    }
    return ret;
}

QtiBusShmTransport::QtiBusShmTransport(std::string filename):
    filename(filename)
{
}

void QtiBusShmTransport::start() {
    int fd = openTransport(filename.c_str());
    if (fd < 0) {
        // Unable to open file
        abort();
    }
    this->fd = fd;
}
void QtiBusShmTransport::stop() {
    if (getStarted() && header) {
        sendCommand(CommandId::LOCAL_DISCONNECT, 0, 0);
        commandReceiver.join();
        header->dump();
    }
}

std::string QtiBusShmTransport::receive()
{
    std::string ret;
    return ret;
}
