/*
 * Copyright (c) 2013,2016 The Linux Foundation. All rights reserved.
 */


#ifndef _BTCONFIG_H_
#define _BTCONFIG_H_
#define VERSION     "2.14"
//#define DUMP_DEBUG 1
//#define DEBUG 1 // @@ boyi @@
#ifdef ANDROID
#define FW_PATH_AR "/etc/firmware/ar3k/"
#else
#define FW_PATH_AR "/lib/firmware/ar3k/"
#endif
#define BDADDR_FILE "ar3kbdaddr.pst"
#define PS_ASIC_FILE                    "PS_ASIC.pst"
#define PS_FPGA_FILE                    "PS_FPGA.pst"
#define PATCH_FILE        "RamPatch.txt"
#define ROM_DEV_TYPE      0xdeadc0de
#define FPGA_ROM_VERSION  0x99999999

#define PS_RAM_SIZE 2048
#define TRUE    1
#define FALSE   0
#define LINE_SIZE_MAX       3000
#define PS_RESET        2

#define PS_WRITE        1
#define HCI_PS_CMD_HDR_LEN 7

#define PS_RESET_PARAM_LEN 6
#define HCI_MAX_CMD_SIZE   260
#define PS_RESET_CMD_LEN   (HCI_PS_CMD_HDR_LEN + PS_RESET_PARAM_LEN)

#define PS_ID_MASK         0xFF

#define HCI_VENDOR_CMD_OGF    0x3F
#define HCI_PS_CMD_OCF        0x0B
#define HCI_CHG_BAUD_CMD_OCF  0x0C
#define HCI_CMD_OGF_HOST_CTL  0x03
#define HCI_CMD_OGF_INFO_PARAM 0x04

#define HCI_CMD_OCF_RESET     0x0003
#define HCI_CMD_OCF_READ_BD_ADDR  0x0009

#define WRITE_BDADDR_CMD_LEN 14
#define WRITE_BAUD_CMD_LEN   6
#define MAX_CMD_LEN          WRITE_BDADDR_CMD_LEN

#define BD_ADDR_SIZE        6
#define BD_ADDR_PSTAG       1
#define PS_READ         0
#define PS_READ_RAW     3

#define WRITE_PATCH     8
#define ENABLE_PATCH        11


#define PS_WRITE_RAW        4
#define PS_GET_LENGTH       5
#define PS_SET_ACCESS_MODE  6
#define PS_SET_ACCESS_PRIORITY  7
#define PS_WRITE        1
#define PS_DYNMEM_OVERRIDE  10
#define PS_VERIFY_CRC       9
#define CHANGE_BDADDR       15
#define PS_COMMAND_HEADER   4
#define HCI_EVENT_SIZE      7
#define PS_RETRY_COUNT      3
#define RAM_PS_REGION       (1<<0)
#define RAM_PATCH_REGION    (1<<1)
#define RAM_DYN_MEM_REGION  (1<<2)
#define RAMPS_MAX_PS_DATA_PER_TAG       244
#define RAMPS_MAX_PS_TAGS_PER_FILE      50
#define PSTAG_RF_TEST_BLOCK_START      (300)
#define PSTAG_SYSTEM_BLOCK_START    (1)
#define BT_SOC_INIT_TOOL_START_MAGIC_WORD 0xB1B1
#define PSTAG_RF_PARAM_TABLE0        (PSTAG_RF_TEST_BLOCK_START+0)
#define PSTAG_SYSCFG_PARAM_TABLE0    (PSTAG_SYSTEM_BLOCK_START+18)
#define PATCH_MAX_LEN                     20000
#define DYN_MEM_MAX_LEN                   40
#define SKIP_BLANKS(str) while (*str == ' ') str++
#define MAX_RADIO_CFG_TABLE_SIZE    1000
#define MAX_BYTE_LENGTH    244
#define DEBUG_EVENT_TYPE_PS     0x02
#define DEBUG_EVENT_TYPE_MEMBLK     0x03
#define HCI_EVENT_HEADER_SIZE       0x03
#define HI_MAGIC_NUMBER ((const unsigned short int) 0xFADE)
#define HI_VERSION  (0x0300)  //Version 3.0
#define EEPROM_CONFIG   0x00020C00
#define FPGA_REGISTER   0x4FFC
#define MAX_EVENT_SIZE  260

// Vendor specific command OCF
#define OCF_PS  0x000B
#define OCF_MEMOP   0x0014
#define OGF_TEST_CMD    0x06
#define OCF_HOST_INTEREST   0x000A
#define OCF_CONT_TX_TESTER  0x0023
#define OCF_TX_TESTER       0x001B
#define OCF_SLEEP_MODE      0x0004
#define OCF_READ_MEMORY     0x0005
#define OCF_WRITE_MEMORY    0x0006
#define OCF_DISABLE_TX      0x002D
#define OCF_TEST_MODE_SEQN_TRACKING 0x0018
#define OCF_READ_VERSION    0x001E
#define OCF_AUDIO_CMD       0x0013
#define OCF_GET_BERTYPE     0x005C
#define OCF_RX_TESTER       0x005B

#define UCHAR unsigned char
#define BOOL unsigned short
#define UINT16 unsigned short int
#define UINT32 unsigned int
#define SINT16 signed short int
#define UINT8 unsigned char
#define SINT8 signed char

#if __BYTE_ORDER == __LITTLE_ENDIAN
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define htobs(d) (d)
#define htobl(d) (d)
#define btohs(d) (d)
#define btohl(d) (d)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#elif __BYTE_ORDER == __BIG_ENDIAN
#define htobs(d) bswap_16(d)
#define htobl(d) bswap_32(d)
#define btohs(d) bswap_16(d)
/* WARNING: DO NOT EDIT, AUTO-GENERATED CODE - SEE TOP FOR INSTRUCTIONS */
#define btohl(d) bswap_32(d)
#else
#error "Unknown byte order"
#endif
#define MAX_TAGS              50
#define PS_HDR_LEN            4
#define HCI_VENDOR_CMD_OGF    0x3F
#define HCI_PS_CMD_OCF        0x0B
#define PS_EVENT_LEN 100
#define HCI_EV_SUCCESS        0x00

#define BT_PORT 2398

/* For ROME QCA61x4 chipset */
#define EVT_VENDOR          (0xFF)
#define EVT_CMD_COMPLETE        (0x0E)
#define HCI_COMMAND_PKT         (0x01)
#define HCI_MAX_EVENT_SIZE      (260)

#define MAX_SIZE_PER_TLV_SEGMENT    (243)
#define EDL_PATCH_CMD_LEN       (1)

#define EDL_PATCH_VER_REQ_CMD       (0x19)
#define EDL_PATCH_TLV_REQ_CMD       (0x1E)

#define EDL_CMD_REQ_RES_EVT     (0x00)
#define EDL_PATCH_VER_RES_EVT       (0x19)
#define EDL_APP_VER_RES_EVT     (0x02)
#define EDL_TVL_DNLD_RES_EVT        (0x04)
#define EDL_CMD_EXE_STATUS_EVT      (0x00)
#define EDL_SET_BAUDRATE_RSP_EVT    (0x92)
#define EDL_NVM_ACCESS_CODE_EVT     (0x0B)

#define HCI_PATCH_CMD_OCF       (0)

#define FW_PATH_ROME            "/lib/firmware/qca"
#define FLOW_CTL            (0x0001)
#define BAUDRATE_CHANGE_SUCCESS     (0x01)
#define CC_MIN_SIZE             (0x7)
#define HCI_CMD_SUCCESS         (0x0)

enum qca_bardrate_type {
    QCA_BAUDRATE_115200     = 0,
    QCA_BAUDRATE_57600,
    QCA_BAUDRATE_38400,
    QCA_BAUDRATE_19200,
    QCA_BAUDRATE_9600,
    QCA_BAUDRATE_230400,
    QCA_BAUDRATE_250000,
    QCA_BAUDRATE_460800,
    QCA_BAUDRATE_500000,
    QCA_BAUDRATE_720000,
    QCA_BAUDRATE_921600,
    QCA_BAUDRATE_1000000,
    QCA_BAUDRATE_1250000,
    QCA_BAUDRATE_2000000,
    QCA_BAUDRATE_3000000,
    QCA_BAUDRATE_4000000,
    QCA_BAUDRATE_1600000,
    QCA_BAUDRATE_3200000,
    QCA_BAUDRATE_3500000,
    QCA_BAUDRATE_AUTO   = 0xFE,
    QCA_BAUDRATE_RESERVED
};

enum qca_tlv_type {
    TLV_TYPE_PATCH = 1,
    TLV_TYPE_NVM
};

struct patch_data {
    int8_t type;
    int64_t len;
    uint8_t *data;
};
#ifndef ANDROID
size_t
strlcpy(char *dst, const char *src, size_t size)
{
    const char *s = src;
    size_t n = size;

    if (n && --n)
        do {
            if (!(*dst++ = *src++))
                break;
        } while (--n);

    if (!n) {
        if (size)
            *dst = '\0';
        while (*src++);
    }
    return src - s - 1;
}
#endif //strlcpy is present on the string.h on android but not on the x86. hence use this instead.
typedef struct tPsTagEntry
{
    int   TagId;
    UCHAR   TagLen;
    UCHAR    TagData[RAMPS_MAX_PS_DATA_PER_TAG];
} tPsTagEntry, *tpPsTagEntry;

typedef struct tRamPatch
{
    int   Len;
    UCHAR    Data[PATCH_MAX_LEN];
} tRamPatch, *ptRamPatch;

typedef struct tRamDynMemOverride
{
    int   Len;
    UCHAR    Data[DYN_MEM_MAX_LEN];
} tRamDynMemOverride, *ptRamDynMemOverride;

tPsTagEntry PsTagEntry[RAMPS_MAX_PS_TAGS_PER_FILE];
tRamPatch   RamPatch[50];
tRamDynMemOverride RamDynMemOverride;

enum MB_FILEFORMAT {
    MB_FILEFORMAT_PS,
    MB_FILEFORMAT_PATCH,
    MB_FILEFORMAT_DY,
};
enum RamPsSection
{
    RAM_PS_SECTION,
    RAM_PATCH_SECTION,
    RAM_DYN_MEM_SECTION
};

enum eType {
    eHex,
    edecimal,
};

struct ST_PS_DATA_FORMAT {
    enum eType   eDataType;
    BOOL    bIsArray;
};
#define CONV_DEC_DIGIT_TO_VALUE(c) ((c) - '0')
#define IS_HEX(c)   (IS_BETWEEN((c), '0', '9') || IS_BETWEEN((c), 'a', 'f') || IS_BETWEEN((c), 'A', 'F'))
#define IS_BETWEEN(x, lower, upper) (((lower) <= (x)) && ((x) <= (upper)))
#define IS_DIGIT(c) (IS_BETWEEN((c), '0', '9'))
#define CONV_HEX_DIGIT_TO_VALUE(c) (IS_DIGIT(c) ? ((c) - '0') : (IS_BETWEEN((c), 'A', 'Z') ? ((c) - 'A' + 10) : ((c) - 'a' + 10)))
#define BYTES_OF_PS_DATA_PER_LINE    16
struct ST_READ_STATUS {
    unsigned uTagID;
    unsigned uSection;
    unsigned uLineCount;
    unsigned uCharCount;
    unsigned uByteCount;
};

//DUT MODE related
#define MC_BCAM_COMPARE_ADDRESS           0x00008080
#define HCI_3_PATCH_SPACE_LENGTH_1            (0x80)
#define HCI_3_PATCH_SPACE_LENGTH_2            (0x279C)
#define MEM_BLK_DATA_MAX                (244)
#define MC_BCAM_VALID_ADDRESS                    0x00008100

//Audio stat

typedef struct tAudio_Stat {
    UINT16      RxSilenceInsert;
    UINT16      RxAirPktDump;
    UINT16      RxCmplt;
    UINT16      TxCmplt;
    UINT16      MaxPLCGenInterval;
    UINT16      RxAirPktStatusGood;
    UINT16      RxAirPktStatusError;
    UINT16      RxAirPktStatusLost;
    UINT16      RxAirPktStatusPartial;
    SINT16      SampleMin;
    SINT16      SampleMax;
    UINT16      SampleCounter;
    UINT16      SampleStatEnable;
} tAudioStat;

//DMA stats

typedef struct tBRM_Stats {
  // DMA Stats
  UINT32 DmaIntrs;

  // Voice Stats
  UINT16 VoiceTxDmaIntrs;
  UINT16 VoiceTxErrorIntrs;
  UINT16 VoiceTxDmaErrorIntrs;
  UINT16 VoiceTxPktAvail;
  UINT16 VoiceTxPktDumped;
  UINT16 VoiceTxDmaSilenceInserts;

  UINT16 VoiceRxDmaIntrs;
  UINT16 VoiceRxErrorIntrs;
  UINT16 VoiceRxGoodPkts;
  UINT16 VoiceRxErrCrc;
  UINT16 VoiceRxErrUnderOverFlow;
  UINT16 VoiceRxPktDumped;

  UINT16 VoiceTxReapOnError;
  UINT16 VoiceRxReapOnError;
  UINT16 VoiceSchedulingError;
  UINT16 SchedOnVoiceError;

  UINT16 Temp1;
  UINT16 Temp2;

  // Control Stats
  UINT16 ErrWrongLlid;
  UINT16 ErrL2CapLen;
  UINT16 ErrUnderOverFlow;
  UINT16 RxBufferDumped;
  UINT16 ErrWrongLmpPktType;
  UINT16 ErrWrongL2CapPktType;
  UINT16 HecFailPkts;
  UINT16 IgnoredPkts;
  UINT16 CrcFailPkts;
  UINT16 HwErrRxOverflow;

  UINT16 CtrlErrNoLmpBufs;

  // ACL Stats
  UINT16 DataTxBuffers;
  UINT16 DataRxBuffers;
  UINT16 DataRxErrCrc;
  UINT16 DataRxPktDumped;
  UINT16 LmpTxBuffers;
  UINT16 LmpRxBuffers;
  UINT16 ForceOverQosJob;

  // Sniff Stats
  UINT16 SniffSchedulingError;
  UINT16 SniffIntervalNoCorr;

  // Test Mode Stats
  UINT16 TestModeDroppedTxPkts;
  UINT16 TestModeDroppedLmps;

  // Error Stats
  UINT16 TimePassedIntrs;
  UINT16 NoCommandIntrs;

} tBRM_Stats;

typedef struct tSYSUTIL_ChipId {
  char *pName;
  UINT32 HwRev;
} tSYSUTIL_ChipId;

typedef struct tSU_RevInfo {
  tSYSUTIL_ChipId *pChipId;
  tSYSUTIL_ChipId *pChipRadioId;
  UINT32 ChipRadioId;
  UINT32 SubRadioId;
  UINT32 RomVersion;
  UINT32 RomBuildNumber;
  UINT32 BuildVersion;
  UINT32 BuildNumber;
  UINT16 RadioFormat;
  UINT16 RadioContent;
  UINT16 SysCfgFormat;
  UINT16 SysCfgContent;
  UINT8 ProductId;
} tSU_RevInfo;

typedef struct {
    UINT8 b[6];
} __attribute__((packed)) bdaddr_t;

typedef struct {
    UINT16 opcode;      /* OCF & OGF */
    UINT8   plen;
} __attribute__((packed)) hci_command_hdr;

typedef struct {
    UINT8   evt;
    UINT8   plen;
} __attribute__((packed)) hci_event_hdr;

typedef struct {
    UINT8   ncmd;
    UINT16  opcode;
} __attribute__ ((packed)) evt_cmd_complete;

#define HCI_COMMAND_HEADER_SIZE 3
#define HCI_OPCODE_PACK(ogf, ocf) (UINT16)((ocf & 0x03ff)|(ogf << 10))

#endif

