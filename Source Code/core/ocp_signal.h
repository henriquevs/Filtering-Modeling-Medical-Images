#ifndef __OCP_SIGNAL_H__
#define __OCP_SIGNAL_H__

// OCP bus widths
//FIXME should not be global
#define MCMDWD            3
#define MBURSTWD          3         //FIXME away
#define MADDRWD           32
#define MADDRSPACEWD      8
#define MDATAWD           32
#define MBURSTPRECISEWD   1
#define MCONNIDWD         4
#define MBURSTLENGTHWD    8
#define MTHREADIDWD       16
#define MREQINFOWD        2
#define MFLAGWD           1
#define MATOMICLENGTHWD   16
#define MBURSTSEQWD       3
#define MREQLASTWD        1
#define MBURSTSINGLEREQWD 1
#define SRESPWD           2
#define SDATAWD           32
#define STHREADIDWD       16
#define SRESPLASTWD       1
#define SFLAGWD           16
#define SINTERRUPTWD      1
#define MBYTEENWD         (SDATAWD/8)
#define MDATABYTEENWD     (MDATAWD/8)

// TG width parameters
#define MCMD_WDTH        MCMDWD
#define addr_wdth        MADDRWD
#define data_wdth        MDATAWD
#define reqinfo_wdth     MREQINFOWD
#define connid_wdth      MCONNIDWD
#define burstlength_wdth MBURSTLENGTHWD
#define threads          4
#define thread_wdth      MTHREADIDWD
#define byten_wdth       MBYTEENWD
#define mflag_wdth       MFLAGWD
#define sflag_wdth       SFLAGWD

// OCP MCmd values
#define OCPCMDIDLE   0
#define OCPCMDWRITE  1
#define OCPCMDREAD   2
#define OCPCMDREADEX 3
#define OCPCMDRDL    4
#define OCPCMDWRNP   5
#define OCPCMDWRC    6
#define OCPCMDBCST   7

// OCP MBurst values
//FIXME away
#define OCPBURLAST   0
#define OCPBURCUSP   1
#define OCPBURINC2   2
#define OCPBURCUSN   3
#define OCPBURINC4   4
#define OCPBURSTRE   5
#define OCPBURINC8   6
#define OCPBURINCR   7

// OCP MBurstSeq values
#define OCPMBSINCR   0
#define OCPMBSDFLT1  1
#define OCPMBSWRAP   2
#define OCPMBSDFLT2  3
#define OCPMBSXOR    4
#define OCPMBSSTRM   5
#define OCPMBSUNKN   6

// OCP SResp values
#define OCPSRESNULL  0
#define OCPSRESDVA   1
#define OCPSRESFAIL  2
#define OCPSRESERR   3

// OCP MByteEn values
//FIXME: may be the other way around?; assumes 32-bit; add more
#define OCPMBYENULL  0x0
#define OCPMBYEBYTE  0x1
#define OCPMBYEHWRD  0x3
#define OCPMBYEWORD  0xf

#endif // __OCP_SIGNAL_H__
