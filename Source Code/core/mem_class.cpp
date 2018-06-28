#include "mem_class.h"
#include "address.h"
#include <stdlib.h>
#include <fcntl.h>
#ifdef WIN32
#include <IO.h>
#else
#include <unistd.h>
#endif
#include <iostream>
#include <sys/stat.h>

///////////////////////////////////////////////////////////////////////////////
// hexstringtonumber - Translates a string representing a hex into a number.
long Mem_class::hexstringtonumber(char *str, int start, int len)
{
  long iValue;
  char sValue[256];
  int iCur;

  for(iCur = 0; iCur < len; iCur++)
    sValue[iCur] = str[start+iCur];
  sValue[len] = '\0';
  iValue = strtol(sValue, NULL, 16);
  //printf("String: %s Value: %d\n", sValue, iValue);
  return iValue;
}

///////////////////////////////////////////////////////////////////////////////
// load_tcdm_init_data - loads initialized tcdm data onto proper tcdm bank (ID).
int Mem_class::load_tcdm_init_data(char * image_filename, int i, unsigned int start_addr, unsigned int offset)
{
  int fd;
  struct stat s;

  if (image_filename == NULL)
    return EXIT_FAILURE;

  fd = open(image_filename, O_RDONLY);

  if (fd == -1)
  {
    cerr << "Error Uploading Program Binary: " << image_filename << endl;
    return EXIT_FAILURE;
  }

  int res;

  fstat(fd, &s);
    
  if(s.st_size <= start_addr) // no tcdm initialized data in binary
  {
    cerr << "WARNING: trying to copy initialized data in TCDM but there's no data (skipping...)" << endl;
    return EXIT_FAILURE;
  }
    
  lseek(fd, start_addr + offset + i*CL_WORD, SEEK_SET);
  //FIXME more error checking
  res = read(fd, myMemory + get_local_bank_addr(offset + i*CL_WORD,log2(N_CL_BANKS)+2),CL_WORD);
                            
  close(fd);

  return EXIT_SUCCESS;
}


///////////////////////////////////////////////////////////////////////////////
// load_program - Tries to load a binary image onto the simulated memory.
int Mem_class::load_program(char * image_filename)
{
  int fd;
//   struct stat s;

  if (image_filename == NULL)
    return EXIT_FAILURE;

  fd = open(image_filename, O_RDONLY);

  if (fd == -1)
  {
    cerr << "Error Uploading Program Binary: " << image_filename << endl;
    return EXIT_FAILURE;
  }

  int res;

//  fstat(fd, &s);
//   res = read(fd, myMemory, s.st_size); //OLD version
  res = read(fd, myMemory, CL_L3_SIZE);
  close(fd);
  //cout << "Uploaded Program Binary: " << image_filename << " (only " << CL_L3_SIZE/1024 << " KB)" <<  endl;
  return EXIT_SUCCESS;
}

#if 0
///////////////////////////////////////////////////////////////////////////////
// load_srec_program - Tries to load an s-record onto the simulated memory.
//                     supports only 32bit addresses and transfer:
//                     S3   Data record with 32 bit load address
//                     S7   Termination record with 32 bit transfer address
//
//                     Stnnaaaaaaaa[dddd...dddd]cc
//                     t     record type field (0,1,2,3,6,7,8,9).
//                     nn    record length field, number of bytes in record
//                           excluding record type and record length.
//                     a...a load address field, can be 16, 24 or 32 bit address
//                           for data to be loaded.
//                     d...d data field, actual data to load, each byte is
//                           encoded in 2 characters.
//                     cc    checksum field, 1's complement of the sum of all
//                           bytes in the record length, load address and data
//                           fields.
int Mem_class::load_srec_program(char * srec_filename)
{
  FILE *fd;
  char str[4096];
  int CountBytes, iCur, iValue;
  unsigned long Address, MinAddress, MaxAddress;

  if (srec_filename == NULL)
    {
      cerr << "Note: No Program SRec to Upload" << endl;
      return EXIT_SUCCESS;
    }
  MinAddress = 0xffffffff;
  MaxAddress = 0;

  fd = fopen(srec_filename, "r");
  if (fd == NULL)
  {
    cerr << "Error Uploading Program SRec: " << srec_filename << endl;
    return EXIT_FAILURE;
  }

  while(fgets(str,4096,fd) != NULL)
    {
      if(str[0] != 'S')
	{
	  fprintf(stderr, "Error: Not a SRec line\n");
	  continue;
	}
      switch(str[1])
	{
	case '2':
	  CountBytes = hexstringtonumber(str,2,2);
	  Address = hexstringtonumber(str,4,6);
	  if(MinAddress > Address) MinAddress = Address;
	  if(MaxAddress < Address) MaxAddress = Address;
	  for(iCur = 10; iCur < (CountBytes-4)*2+10; iCur+=2)
	    {
	      iValue = hexstringtonumber(str,iCur,2);
	      myMemory[Address++] = iValue;
	      //printf("Just read: %x is %x\n", pMemory[Address-1], iValue);
	    }
	  break;
	case '3':
	  CountBytes = hexstringtonumber(str,2,2);
	  Address = hexstringtonumber(str,4,8);
	  for(iCur = 12; iCur < (CountBytes-5)*2+12; iCur+=2)
	    {
	      iValue = hexstringtonumber(str,iCur,2);
	      myMemory[Address++] = iValue;
	      //printf("Just read: %x is %x\n", pMemory[Address-1], iValue);
	    }
	  break;
	case '7':
	  break;
	}
    }
  fclose(fd);
  printf("Uploaded Program SRec %s between addresses[hex]: %lx to %lx\n", srec_filename, MinAddress, MaxAddress);
  return EXIT_SUCCESS;
}
#endif
