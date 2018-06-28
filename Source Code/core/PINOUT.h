#ifndef __PINOUT_STRUCT_H__
#define __PINOUT_STRUCT_H__

typedef struct POTAG
{
  uint32_t address;
  uint32_t data;
  bool rw;          // Write = 1
  bool fiq;         // __NOT_USED__
  bool irq;         // __NOT_USED__
  bool benable;     // __NOT_USED__
  uint32_t bw;      // (2 bit) 0 = word, 1 = byte, 2 = half word, 3 = UNDEF
  uint32_t burst;   // (5 bit) Burst size
  uint8_t id;
  
  //constructor - init values
  POTAG()
  {
    address = 0xDEADBEEF;
    data = 0xDEADBEEF;
    rw = false;
    fiq = false;
    irq = false;
    benable = true;
    bw = 0;
    burst = 0; 
    id = 0xFF; 
  };
  
  inline bool operator == (const POTAG& rhs) const {
    return ( rhs.address == address && rhs.data == data && rhs.fiq == fiq &&
             rhs.irq == irq  && rhs.rw == rw && rhs.benable == benable &&
             rhs.bw == bw && rhs.burst == burst && rhs.id == id );
  }
  
  inline friend ostream & operator << ( ostream & os, const POTAG & v )
  {
    return os;
  }
  
} PINOUT;

inline void sc_trace(sc_trace_file *tf, const POTAG& v, const std::string& NAME) {
  //Not all signals are usefull for tracing 
  //uncomment those you want to be traced

  //sc_trace(tf,v.nreset, NAME + ".nreset");
  sc_trace(tf,v.id, NAME + ".id");
  sc_trace(tf,v.address, NAME + ".address");
  sc_trace(tf,v.data, NAME + ".data");
  sc_trace(tf,v.rw, NAME + ".rw");
  //sc_trace(tf,v.fiq, NAME + ".fiq");
  //sc_trace(tf,v.irq, NAME + ".irq");
  //sc_trace(tf,v.benable, NAME + ".benable");
  sc_trace(tf,v.bw, NAME + ".bw");
  sc_trace(tf,v.burst, NAME + ".burst");
  //sc_trace(tf,v.id, NAME + ".id");
}

typedef PINOUT ICACHE_LINE[4];

#endif

