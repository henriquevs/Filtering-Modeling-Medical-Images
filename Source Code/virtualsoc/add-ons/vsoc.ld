/* ld script to make ARM Linux kernel
* taken from the i386 version by Russell King
* Written by Martin Mares <mj@atrey.karlin.mff.cuni.cz>
*/

STACK_TOP = 0x0805FFFC; /*0x0803EFFC;*/

MEMORY
{
    LOCAL_SHARED (wI) : ORIGIN = 0x08060000, LENGTH = 128K /*was 0x0803F000*/
    GLOBAL_SHARED (wx) : ORIGIN = 0x0, LENGTH = 256M
}

OUTPUT_ARCH(arm)
/*ENTRY(stext)*/
SECTIONS
{
    . = 0x0;
    .vectors1 : {
            *(.resetvector1)
    }>GLOBAL_SHARED
    . = 0x18;
    .irq_vector : {
            *(.irqvector)
    }>GLOBAL_SHARED
    . = 0x8000;
    .init : {			/* Init code and data		*/
/*		_stext = .;       */
/*		__init_begin = .;*/
            *(.text1.init)
            *(.text2.init)
/*		__init_end = .;  */
    }>GLOBAL_SHARED

    /DISCARD/ : {			/* Exit code and data		*/
        *(.text.exit)
        *(.data.exit)
        *(.exitcall.exit)
    }>GLOBAL_SHARED

    .text : {			/* Real text segment		*/
        _text = .;		/* Text and read-only data	*/
            *(.text)
        _etext = .;		/* End of text section		*/
    }>GLOBAL_SHARED

    . = ALIGN(8192);
    .data : {
        /*
        * first, the init task union, aligned
        * to an 8192 byte boundary.
        */
        *(.init.task)

        /*
        * then the cacheline aligned data
        */
        . = ALIGN(32);
        *(.data.cacheline_aligned)

        /*
        * and the usual data section
        */
        *(.rodata)
        *(.rodata.*)
        *(.data)
        CONSTRUCTORS

        _edata = .;
    }>GLOBAL_SHARED

    .bss : {
        __bss_start = . ;	/* BSS	*/
        *(.bss)
        *(COMMON)
        __bss_end = . ;
    }>GLOBAL_SHARED
        
    .stack1 : {
            __stack_start1 = STACK_TOP ;
    }
    . = 0x800009;
    .stack2 : {
        __stack_start2 = . ;
    }>GLOBAL_SHARED
        
    . = ALIGN(8192);
    _end_kernel = . ;

    .vectors2 : {
            *(.resetvector2)
    }>GLOBAL_SHARED

    .local_shared : {
            *(.local_shared)
    }>LOCAL_SHARED

    .global_shared : {
            *(.global_shared)
    }>GLOBAL_SHARED
 
   /* Stabs debugging sections.	*/

/*	.stab 0 : { *(.stab) }
    .stabstr 0 : { *(.stabstr) }
    .stab.excl 0 : { *(.stab.excl) }
    .stab.exclstr 0 : { *(.stab.exclstr) }
    .stab.index 0 : { *(.stab.index) }
    .stab.indexstr 0 : { *(.stab.indexstr) }
    .comment 0 : { *(.comment) }
    .debug_abbrev 0 : { *(.debug_abbrev) }
    .debug_info 0 : { *(.debug_info) }
    .debug_line 0 : { *(.debug_line) }
    .debug_pubnames 0 : { *(.debug_pubnames) }
    .debug_aranges 0 : { *(.debug_aranges) } */
}
