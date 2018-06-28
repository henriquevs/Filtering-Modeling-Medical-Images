#define __nop { \
  asm("mov r0,r0"); \
}

#define _10_nop_block  { \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  asm("mov r0,r0"); \
  }

#define _50_nop_block  { \
  _10_nop_block \
  _10_nop_block \
  _10_nop_block \
  _10_nop_block \
  _10_nop_block \
  }

#define _100_nop_block  { \
  _50_nop_block \
  _50_nop_block \
  }

#define _200_nop_block  { \
  _100_nop_block \
  _100_nop_block \
  }

#define _500_nop_block  { \
  _200_nop_block \
  _200_nop_block \
  _100_nop_block \
  }

#define _1000_nop_block  { \
  _500_nop_block \
  _500_nop_block \
  }

#define _2000_nop_block  { \
  _1000_nop_block \
  _1000_nop_block \
  }

#define _5000_nop_block  { \
  _2000_nop_block \
  _2000_nop_block \
  _1000_nop_block \
  }

#define _10000_nop_block  { \
  _5000_nop_block \
  _5000_nop_block \
  }

#define _20000_nop_block  { \
  _10000_nop_block \
  _10000_nop_block \
  }

#define _50000_nop_block  { \
  _10000_nop_block \
  _20000_nop_block \
  _20000_nop_block \
  }

#define _100000_nop_block  { \
  _50000_nop_block \
  _50000_nop_block \
  }

