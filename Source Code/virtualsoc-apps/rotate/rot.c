#include "appsupport.h"

#include "Rose256.h"	/* Input image */

char output[33823] /*LOCAL_SHARED*/;	/* Output image */
int input_height /*LOCAL_SHARED*/ = 149;	/* Image height */

#if (CROP_IMAGE == 1)
int input_width /*LOCAL_SHARED*/ = 224;		/* Image width */
#else
int input_width /*LOCAL_SHARED*/ = 227;		/* Image width */
#endif

int i /*LOCAL_SHARED*/;
int offset /*LOCAL_SHARED*/;
int row_offset /*LOCAL_SHARED*/;
int sstart /*LOCAL_SHARED*/;
int send /*LOCAL_SHARED*/;

#define CHECKSUM

#ifdef CHECKSUM
  #define EXPECTED_CRC 0xBA8
#endif

#ifndef ORDERED
  #define ORDERED	ordered
#endif

#ifndef PRIVATIZE_WORK_COUNTER
  #define PRIVATIZE_WORK_COUNTER	1
#endif

#ifndef CHUNK
  #define CHUNK 10
#endif

short Calc_crc8(char, short);
short th_crcbuffer (const void *, int, short);

static void rotate_slice(int num_rotates, int slice_start, int slice_end)
{
  int input_row, input_col, j;
  char * input;
  char image_in;

  switch (num_rotates % 4) {
    case 1:	/* Rotate 90 degrees */
      i = slice_start + input_height * (input_width-1);
      row_offset = 1;
      offset = -input_height;
      break;
    case 2:	/* Rotate 180 degrees */
      i = (input_height-slice_start) * (input_width) - 1;
      row_offset = -input_width;
      offset = -1;
      break;
    case 3:	/* Rotate 270 degrees */
      i = input_height-slice_start-1;
      row_offset = -1;
      offset = input_height;
      break;
    default:
      i = input_width*slice_start;
      row_offset = input_width;
      offset = 1;
      break;
  }

  sstart = slice_start;
  send = slice_end;

  #pragma omp parallel private(input_row, input_col, j, input, image_in) shared(input_image, output)
  {
/*  #pragma omp for schedule(dynamic, CHUNK)*/
  #pragma omp for schedule(static, CHUNK)
//  #pragma omp transfor schedule(dynamic, CHUNK) ORDERED
  for (input_row = sstart; input_row < send; ++input_row)
  {
    //_printdecn ("Rotating ROW", input_row);
    input = input_image + input_row * input_width;

#if (PRIVATIZE_WORK_COUNTER == 1)
    j = i + row_offset * input_row;
#else
    j = i;
    i += row_offset;
#endif

    for (input_col = 0; input_col < input_width; input_col ++)
    {
      image_in = *input++;
      *(output+j) = image_in;
      j += offset;
    }
  }
  }

  return;
}

int main ()
{
#ifdef CHECKSUM
  short crc;
#endif

  /* Rotate image by 180 degrees */
  rotate_slice (2, 0, input_height);

#ifdef CHECKSUM
  _printstrn ("Computing CHECKSUM..");
  crc = th_crcbuffer (output, 33823, 0);
  if (crc != EXPECTED_CRC)
    {_printhex ("ROTATE FAIL: CRC =", crc);_printhexn ("Expected", EXPECTED_CRC);}
  else
    _printstrn ("ROTATE SUCCESS!");
#endif

  return 0;
}


short th_crcbuffer (const void *inbuf, int size, short inputCRC)
{
  short CRC = inputCRC;
  char *buf = (void *)inbuf;
  int i;

  // Allow the AND case if found to be necessary.
  // Allow AND case being a NULL inbuf, and 0 size buffer where the inputCRC is returned.

  if (!buf && !size)
    return 0;
  else if (!buf || !size)
  {
    //_printstrn ("Failure: Attempt to CRC an empty buffer");
    return 0;
  }

  for (i=0; i<size; i++)
  {
    CRC = Calc_crc8(*buf,CRC);
    buf++;
  }

  return CRC;
}


short Calc_crc8(char data, short crc)
{
  char i, x16, carry;

  i = x16 = carry = 0;

  for (i = 0; i < 8; i++)
  {
    x16 = (char)((data & 1) ^ ((char)crc & 1));
    data >>= 1;

    if (x16 == 1)
    {
      crc ^= 0x4002;
      carry = 1;
    }
    else
      carry = 0;

    crc >>= 1;

    if (carry)
      crc |= 0x8000;
    else
      crc &= 0x7fff;
  }

  return crc;
}

