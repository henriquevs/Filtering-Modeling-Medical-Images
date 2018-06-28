#include "hws_support.h"

#include "appsupport.h"
#include "config.h"
#include <stdlib.h>

// #define ASM_API

extern unsigned int timers[10] LOCAL_SHARED;

#define HWS_PRG_PORT_BASE_ADDR HWS_BASE
#define HWS_SLV_PORT_BASE_ADDR (HWS_BASE + (0x4*(N_HWS_PRG_PORTS)))


// #define USE_ASSERT  // dont use it if you need to measure the HWS benefits

inline unsigned int func_port_prg(int ID) {
  #ifdef USE_ASSERT
  assert((ID>=0)&&(ID<n_cores));
  #endif

  unsigned int port_id = ID & (N_HWS_PRG_PORTS-1);

  #ifdef USE_ASSERT
  assert(port_id<256);
  #endif

  return port_id;
}

inline unsigned int func_port_slv(int ID) {
  #ifdef USE_ASSERT
  assert((ID>=0)&&(ID<n_cores));
  #endif
  
  unsigned int port_id = ID & (N_HWS_SLV_PORTS-1);

  #ifdef USE_ASSERT
  assert(port_id<256);
  #endif
  
  return port_id;
}

inline void hws_request_event_mem_single(unsigned int ID, unsigned int NSL, unsigned int PORT) {
  #ifdef USE_ASSERT
  assert(ID<=0x7F);
  assert((unsigned int)(HWS_PRG_PORT_BASE_ADDR + (0x4*PORT))<(unsigned int)(HWS_SLV_PORT_BASE_ADDR));
  #endif

		
  *((unsigned int volatile *) (HWS_PRG_PORT_BASE_ADDR + (0x4 * PORT))) =
		((1 & 0xFF) << 24) | ((1 & 0xFF) << 16) | ((ID & 0x7F) << 8) | (NSL & 0xFF);
}

inline void hws_request_event_no_mem_single(unsigned int ID, unsigned int NSL, unsigned int PORT) {
  
  #ifdef USE_ASSERT
  assert(ID<=0x7F);
  assert(((unsigned int)HWS_PRG_PORT_BASE_ADDR + PORT)<(unsigned int)(HWS_SLV_PORT_BASE_ADDR));
  #endif

  unsigned int value = ((1 & 0xFF) << 24) | ((0 & 0xFF) << 16)
			| ((ID & 0x7F) << 8) | (NSL & 0xFF);

  *((unsigned int volatile *) HWS_PRG_PORT_BASE_ADDR + PORT) = value;
}

inline int hws_wait_event(unsigned int ID, unsigned char PORT) {
  #ifdef USE_ASSERT
  assert((unsigned int)(HWS_PRG_PORT_BASE_ADDR + (0x4*PORT))<(unsigned int)(HWS_SLV_PORT_BASE_ADDR));
  #endif

  unsigned int ret = *((unsigned int volatile *) (HWS_PRG_PORT_BASE_ADDR
		  + (0x4 * PORT)));

  if (((ret & 0xFF00) >> 8) == (ID | 0x80)) {
	  return ret & 0xFF;
  } else {
    #ifdef USE_ASSERT
    assert(((ret & 0xFF00) >> 8) == (ID & 0xFF));
    #endif
  }

  return -1;
}

//event-programming API
inline int maybe_get_event(unsigned int NSL) {

//   unsigned int timer;
  int EVT_ID;
//   timer = opt_get_cycle();

  unsigned int ID = get_proc_id() - 1;

#ifndef ASM_API
  unsigned int PORT = func_port_prg(ID);

  volatile unsigned int * address = (unsigned int *) HWS_PRG_PORT_BASE_ADDR
		  + PORT;
  *((volatile unsigned int *) address) = (((ID & 0xFF) << 8) | 0x0);

  hws_request_event_no_mem_single(ID, NSL, PORT);
  EVT_ID = hws_wait_event(ID, PORT);
#else
asm(" mov %%r1, %2\n \
        sub %%r1,%%r1,#1\n \
        and %%r1, %%r1, %1 /*r1: programming port*/ \n  \
        mov %%r2, %3\n \
        add %%r1, %%r2, %%r1, lsl #2 /*r1: programming port address*/ \n \
        mov %%r2, %1\n \
        mov %%r3,#0\n \
        orr %%r2, %%r3,%%r2, lsl #8 /*r2: value to be written*/ \n \
        str %%r2,[%%r1] /*1st write*/ \n \
        mov %%r2, #0x01000000\n \
        mov %%r3, %1 \n \
        orr %%r3,%4,%%r3,lsl #8\n \
        add %%r2,%%r2,%%r3\n \
        str %%r2,[%%r1] /*2nd write*/ \n \
        ldr %%r2,[%%r1] \n \
        and %%r3,%%r2,#0xFF00 \n \
        mov %0, #0xFF \n \
        lsl %0, %0, #8 \n \
        add %0, %0, #0xFF \n \
        lsl %0, %0, #16 \n \
        add %0, %0, #0xFF \n \
        lsl %0, %0, #24 \n \
        add %0, %0, #0xFF \n \
        mov %%r3,%%r3,lsr #8 \n \
        and %%r4, %1, #0x80 \n \
        cmp %%r3, %%r4 \n \
        bne no \n \
        and %0,%%r2,#0xFF \n \
        no: "\
        :"=r"(EVT_ID)\
        :"r"(ID),"i"(N_HWS_PRG_PORTS),"i"(HWS_BASE), "r"(NSL)\
        :"%r1","%r2","%r3","%r4");	
#endif



//   timer = opt_get_cycle() - timer;
//   _printdecp("maybe_get_event, cycles: ", timer);

  return EVT_ID;
}

int get_event(unsigned int NSL) {
  unsigned int ID = get_proc_id() - 1;
  unsigned int PORT = func_port_prg(ID);
//   unsigned int timer;
// 
//   timer = opt_get_cycle();
  volatile unsigned int * address = (unsigned int *) HWS_PRG_PORT_BASE_ADDR
		  + PORT;
  *((volatile unsigned int *) address) = (((ID & 0xFF) << 8) | 0x0);

  hws_request_event_mem_single(ID, NSL, PORT);
  int EVT_ID = hws_wait_event(ID, PORT);
  while (EVT_ID < 0)
	  EVT_ID = hws_wait_event(ID, PORT);
//   timer = opt_get_cycle() - timer;
// 
//   _printdecp("get_event: ",timer);

  return EVT_ID;
}

void free_event(int EVT_ID) {
  unsigned int ID = get_proc_id() - 1;
  unsigned int PORT = func_port_prg(ID);
  
  #ifdef USE_ASSERT  
  assert(ID<=0x7F);
  assert((unsigned int)(HWS_PRG_PORT_BASE_ADDR + (0x4*PORT))<(unsigned int)(HWS_SLV_PORT_BASE_ADDR));
  #endif


  unsigned int volatile value = (((ID & 0xFF) << 8) | 0x1);
  *((unsigned int volatile *) (HWS_PRG_PORT_BASE_ADDR + (0x4 * PORT))) = value;			

//   pr("free1   ", EVT_ID, PR_CPU_ID | PR_HEX | PR_STRING | PR_TSTAMP | PR_NEWL);

  value = (((ID & 0xFF) << 8) | (((unsigned int) EVT_ID) & 0xFF));
  *((unsigned int volatile *) (HWS_PRG_PORT_BASE_ADDR + (0x4 * PORT))) = value;

//   pr("free2   ", value, PR_CPU_ID | PR_HEX | PR_STRING | PR_TSTAMP | PR_NEWL);

  value =	*((unsigned int volatile *) (HWS_PRG_PORT_BASE_ADDR + (0x4 * PORT)));

//   pr("free3   ", value, PR_CPU_ID | PR_HEX | PR_STRING | PR_TSTAMP | PR_NEWL);
}

inline void release(int EVT_ID, unsigned int BMSK) {
//   unsigned int timer;
//   timer = opt_get_cycle();


  unsigned int ID = get_proc_id() - 1, time;
  unsigned int PORT = func_port_prg(ID);

  #ifdef USE_ASSERT  
  assert(ID<=0x7F);
  assert((unsigned int)(HWS_PRG_PORT_BASE_ADDR + (0x4*PORT))<(unsigned int)(HWS_SLV_PORT_BASE_ADDR));
  #endif

  unsigned int value = (((ID & 0xFF) << 8) | 0x1);
  unsigned int address = HWS_PRG_PORT_BASE_ADDR + (0x4 * PORT);

  *((unsigned int volatile*) address) = value;

  value = (((ID & 0xFF) << 8) | (((unsigned int) EVT_ID) & 0xFF));


  *((unsigned int volatile*) address) = value;

  *((unsigned int volatile*) address) = BMSK;


//   timer =  opt_get_cycle() - timer;
//   _printdecp("release, cycles: ", timer);
}


//slave API
inline void notify(int EVT_ID) {
  
//   unsigned int timer;
//   timer = opt_get_cycle();
  unsigned int ID = get_proc_id() - 1;
		
#ifndef ASM_API

  unsigned int PORT = func_port_slv(ID);

  //_printdecp("PORT: ", PORT);

  #ifdef USE_ASSERT 
  assert(ID<=0x7F);
  #endif

  // _printdecp("Notify arrival, event: ", EVT_ID);

  unsigned int volatile * address = (unsigned int *) HWS_SLV_PORT_BASE_ADDR + PORT;
  unsigned int value = (unsigned int) (((ID & 0xFF) << 8) | (EVT_ID & 0xFF));

//   timer =  opt_get_cycle() - timer;
//   _printdecp("notify, cycles: ", timer);


  #ifdef SYNC
  sync_cores();
  #endif

  *address = value;

#else
  asm(" mov %%r1, %1\n \
	sub %%r1,%%r1,#1\n \
	and %%r1, %%r1, %0\n \
	mov %%r2, %2\n \
	mov %%r3,%4\n \
	add %%r2, %%r2, %%r3, lsl #2\n \
	add %%r1, %%r2, %%r1, lsl #2\n \
	mov %%r2, %3\n \
	and %%r2, %%r2, #0xFF\n \
	mov %%r3,%0\n \
	and %%r3,%%r3, #0xFF\n \
	orr %%r2, %%r2, %%r3, lsl #8\n \
	str %%r2,[%%r1] \n"
	:\
	:"r"(ID),"i"(N_HWS_SLV_PORTS),"i"(HWS_BASE),"r"(EVT_ID), "i"(N_HWS_PRG_PORTS) \
	:"%r1","%r2","%r3","%r4");
#endif

}

inline void notify_and_set(int EVT_ID) {

//   unsigned int timer;
//   timer = opt_get_cycle();

  unsigned int ID = get_proc_id() - 1;
  
#ifndef ASM_API
  unsigned int PORT = func_port_slv(ID);
  
  #ifdef USE_ASSERT 
  assert(ID<=0x7F);
  #endif

  unsigned int volatile * address = (unsigned int *) HWS_SLV_PORT_BASE_ADDR + PORT;
  unsigned int value = (unsigned int) (((1 & 0xFF) << 16) | ((ID & 0xFF) << 8) | (EVT_ID & 0xFF));

  *address = value;
#else
  asm(" mov %%r1, %1\n \
	sub %%r1,%%r1,#1\n \
	and %%r1, %%r1, %0\n \
	mov %%r2, %2\n \
	mov %%r3,%4\n \
	add %%r2, %%r2, %%r3, lsl #2\n \
	add %%r1, %%r2, %%r1, lsl #2\n \
	mov %%r2, %3\n \
	and %%r2, %%r2, #0xFF\n \
	mov %%r3,%0\n \
	and %%r3,%%r3, #0xFF\n \
	orr %%r2, %%r2, %%r3, lsl #8\n \
	mov %%r3,#1\n \
	orr %%r2, %%r2, %%r3, lsl #16\n \
	str %%r2,[%%r1]\n"
	:\
	:"r"(ID),"i"(N_HWS_SLV_PORTS),"i"(HWS_BASE),"r"(EVT_ID), "i"(N_HWS_PRG_PORTS) \
	:"%r1","%r2","%r3");
#endif
//   timer =  opt_get_cycle() - timer;
//   _printdecp("notify_and_set, cycles: ", timer);
}

