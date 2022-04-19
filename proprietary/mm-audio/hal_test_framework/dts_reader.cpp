/**
* Copyright (c) 2017 Qualcomm Technologies, Inc.
* All Rights Reserved.
* Confidential and Proprietary - Qualcomm Technologies, Inc
*/

#include "dts_reader.h"

namespace qti_hal_test
{
DTSReader::DTSReader() :
    file_stream_{ nullptr }
{
    LOG_ENTRY;
}

DTSReader::DTSReader(std::string const & file_name) :
    file_stream_{ nullptr }
{
    LOG_ENTRY;

    Open(file_name.c_str());

    ParseFileHeader();
}

DTSReader::~DTSReader()
{
    LOG_ENTRY;

    try
    {
        Close();
    }
    catch (std::exception const & e)
    {
        std::cout << "Could not close DTSReader" << std::endl;
    }
}

void DTSReader::Open(std::string const & file_name)
{
    LOG_ENTRY;

    //read_stream_.Open(file_name);

    file_stream_ = fopen(file_name.c_str(), "r");
    if (file_stream_ == nullptr)
    {
        std::ostringstream error_msg;
        error_msg << std::endl << "Failed to open file:\"" << file_name << "\"";
        throw std::ios::failure{ error_msg.str() }; // Note that we're using ios exception even though
    }                                               // we're implementing with a C-stream.
}                                                   // This is for consistency with other readers.

void DTSReader::Read(char* const sample_bytes,
                     uint64_t num_bytes_to_read,
                     uint64_t & num_bytes_read,
                     bool & eof)
{
    //read_stream_.Read(sample_bytes, num_bytes_to_read, num_bytes_read, eof);

    const size_t element_size = 1;
    num_bytes_read = fread(sample_bytes, element_size, static_cast<size_t>(num_bytes_to_read), file_stream_);

    if (num_bytes_read != num_bytes_to_read) // This comparison depends on element_size == 1
    {
        if (feof(file_stream_))
        {
            eof = true;
        }
        else if (ferror(file_stream_))
        {
            throw std::ios::failure{ "Failed to read file" };
        }
    }
}

void DTSReader::Close()
{
    LOG_ENTRY;

    //read_stream_.Close();

    fclose(file_stream_);
    file_stream_ = nullptr;
}

void DTSReader::ParseFileHeader()
{
#define DTSHD_CHUNK_HEADER_KEYWORD "DTSHDHDR"
#define DTSHD_CHUNK_STREAM_KEYWORD "STRMDATA"
#define DTSHD_META_KEYWORD_SIZE 8 /*in bytes */

#ifndef ssize_t
#define ssize_t int64_t
#endif

    char keyword[DTSHD_META_KEYWORD_SIZE + 1];
    bool is_dtshd_stream = false;
    uint64_t read_chunk_size = 0;
    uint64_t chunk_size = 0;
    ssize_t file_read_size = -1;
    ssize_t header_read_size = -1;
    long int pos;
    int ret = 0;

    //first locate the ASCII header "DTSHDHDR"identifier
    while (!feof(file_stream_) &&
           (header_read_size < 1024) && // <-- different from original -- was "&"
           (fread(&keyword, sizeof(char), DTSHD_META_KEYWORD_SIZE, file_stream_) == DTSHD_META_KEYWORD_SIZE))
    {
        //update the number of bytes was read for identifying the header
        header_read_size = ftell(file_stream_);

        if (strncmp(keyword, DTSHD_CHUNK_HEADER_KEYWORD, DTSHD_META_KEYWORD_SIZE) == 0)
        {
            // read the 8-byte size field
            if (fread(&read_chunk_size, sizeof(char),
                DTSHD_META_KEYWORD_SIZE, file_stream_) == DTSHD_META_KEYWORD_SIZE)
            {
                is_dtshd_stream = true;
                chunk_size = convert_BE_to_LE(read_chunk_size);
                pos = ftell(file_stream_);
                fseek(file_stream_, static_cast<long>(chunk_size), SEEK_CUR);
                fprintf(stdout, "DTS header chunk offset:%lu and chunk_size:%llu \n",
                        pos, chunk_size);
                break;
            }
            else
            {
                throw std::ios::failure{ "Failed to parse DTS header" };
            } //end reading chunk size
        }
    }

    if (!is_dtshd_stream)
    {
        //fprintf(stdout, "raw dts hd stream");
        fseek(file_stream_, 0, SEEK_SET);
        //return file_read_size;
        return;
    }

    /* parsing each chunk data */
    while (!feof(file_stream_) &&
           fread(&keyword, sizeof(uint8_t), DTSHD_META_KEYWORD_SIZE, file_stream_) == DTSHD_META_KEYWORD_SIZE)
    {
        /* check for the stream audio data */
        ret = strncmp(keyword, DTSHD_CHUNK_STREAM_KEYWORD, DTSHD_META_KEYWORD_SIZE);
        if (!ret)
        {
            ret = fread(&read_chunk_size, 1, DTSHD_META_KEYWORD_SIZE, file_stream_);
            chunk_size = convert_BE_to_LE(read_chunk_size);

            if (ret != DTSHD_META_KEYWORD_SIZE)
            {
                //fprintf(stderr, "%s %d file read error ret %d\n", __func__, __LINE__, ret);
                throw std::ios::failure{ "Failed to parse DTS chunk" };
            }

            file_read_size = chunk_size;
            fprintf(stderr, "DTS read_chunk_size %llu and file_read_size: %zd\n",
                    chunk_size,
                    static_cast<unsigned int>(file_read_size));
            break;
        }
        else
        {
            fprintf(stdout, "Identified chunk of %c %c %c %c %c %c %c %c \n",
                    keyword[0], keyword[1], keyword[2], keyword[3],
                    keyword[4], keyword[5], keyword[6], keyword[7]);

            ret = fread(&read_chunk_size, 1, DTSHD_META_KEYWORD_SIZE, file_stream_);
            pos = ftell(file_stream_);

            chunk_size = convert_BE_to_LE(read_chunk_size);
            fseek(file_stream_, static_cast<long>(chunk_size), SEEK_CUR);
        }
    }
}

/* convert big-endian to little-endian */
uint64_t DTSReader::convert_BE_to_LE(uint64_t in) const
{
    uint64_t out;
    char *p_in = (char *)&in;
    char *p_out = (char *)&out;
    p_out[0] = p_in[7];
    p_out[1] = p_in[6];
    p_out[2] = p_in[5];
    p_out[3] = p_in[4];
    p_out[4] = p_in[3];
    p_out[5] = p_in[2];
    p_out[6] = p_in[1];
    p_out[7] = p_in[0];
    return out;
}
}
