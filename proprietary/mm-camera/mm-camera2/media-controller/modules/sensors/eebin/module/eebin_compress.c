/* eeprom_compress.c
 *
 * Copyright (c) 2014 Qualcomm Technologies, Inc. All Rights Reserved.
 * Qualcomm Technologies Proprietary and Confidential.
 */

#include <stdlib.h>
#include <zlib.h>
#include "eebin_compress.h"
#include "sensor_common.h"


int32_t un_compress(char* in, uint32_t in_len, char** out, uint32_t* out_len)
{
  int32_t rc = 0;
  z_stream stream;

  // Init structs
  stream.zalloc = Z_NULL;
  stream.zfree = Z_NULL;
  stream.opaque = Z_NULL;
  stream.next_in  = (Byte *)in;
  stream.avail_in = (uInt)in_len;

  rc = inflateInit(&stream);
  if (rc < 0) {
    SERR("failed: rc  = %d", rc);
  }

  // Estimate output size length
  uint32_t read_chunk_size = 128;
  Byte temp_buffer[read_chunk_size];

  while (rc == Z_OK) {
    stream.next_out = temp_buffer;
    stream.avail_out = (uInt)read_chunk_size;
    rc = (inflate(&stream, Z_NO_FLUSH));
    if (rc < 0) {
      SERR("failed: rc = %d", rc);
    }
  }

  uint32_t estimated_out_len = stream.total_out;
  rc = inflateEnd(&stream);
  if (rc < 0) {
    SERR("failed: rc = %d", rc);
  }

  if (rc == Z_OK && estimated_out_len > 0) {
    // Alloc output memory with estimated output length
    *out = (char *)malloc(estimated_out_len);

    if (*out == NULL) {
      SERR("failed!");
      return -1;
    }

    stream.next_in  = (Byte *)in;
    stream.avail_in = (uInt)in_len;
    stream.next_out = (unsigned char *)*out;
    stream.avail_out = (uInt)estimated_out_len;

    // Init inflate
    rc = inflateInit(&stream);
    if (rc < 0) {
      SERR("failed: rc = %d", rc);
    }
    rc = inflate(&stream, Z_FINISH); // Do once
    if (rc < 0) {
      SERR("failed: rc = %d", rc);
    }
    rc = inflateEnd(&stream);
    if (rc < 0) {
      SERR("failed: rc = %d", rc);
    }

    *out_len = estimated_out_len;

    if (rc != Z_OK) {
      free(*out);
      *out = NULL;
      return -1;
    }
    return 0;
  } else {
    return -1;
  }
}

