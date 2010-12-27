/*
//*****************************************************************
// Command file for GNU linker
//
// Generates appropriate mappings for objects in memory
//
// Target mcu: AT91SAM7 (Atmel)
// Code location: Internal flash
//
//
// $Revision: 202 $
// $Date:: 2008-09-12 #$
//
//*****************************************************************
*/

/* ENTRY(_start) */
STACK_SIZE = 0x400;

/* memory layout */
MEMORY
{
  ROM (rx) : ORIGIN = 0x00000000, LENGTH = 120K
  RAM (rw) : ORIGIN = 0x40000000, LENGTH = 16K  
}

SECTIONS
{
  /*
  .vectors :
  {
    KEEP(*(.vectors))
  } > ROM
  */

  . = 0;
  startup : { *(.startup)} >ROM

  /* first section is .text which is used for code */
  .text :
  {
    *crt0.o (.text) /* Startup code */
    *(.text*) /* code */

    __ctors_start = .;
    *(.init_array)
    *(.ctors)
     __ctors_end = .;
     __dtors_start = .;
     *(.dtors)
     __dtors_end = .;
    KEEP(SORT(*)(.ctors))
    KEEP(SORT(*)(.dtors))
    KEEP(SORT(*)(.init_array))

    . = ALIGN(4);

    *(.rodata*) /* read-only data (constants) */

    *(.glue_7)
    *(.glue_7t)
  } > ROM

  . = ALIGN(4);
  _etext = .;

  /* .data section which is used for initialized data */
  .data :
  {
    __data_beg__ = .;
    __data_beg_src__ = _etext;

    _data = .;
    *(.ramfunc*)
    *(.data*)

    _edata = . ;
    __data_end__ = . ;
    _data_image = LOADADDR(.data) ;
    PROVIDE (edata = .);
  } > RAM AT > ROM

  . = ALIGN(4);

  /* .bss section which is used for uninitialized data */
  .bss: (NOLOAD) :
  {
    __bss_start = . ;
    __bss_start__ = . ;
    __bss_beg__ = . ;
    *(.bss*)
    *(COMMON)
    . = ALIGN(4);
__bss_end__ = . ;
  } > RAM

  . = ALIGN(4);
  PROVIDE (__bss_end = .);

  .noinit (NOLOAD) :
  {
     PROVIDE (__noinit_start = .) ;
    *(.noinit*)
     PROVIDE (__noinit_end = .) ;
     _end = . ;
     PROVIDE (__heap_start = .) ;
  } > RAM

  .stack ALIGN(256) :
  {
    . += STACK_SIZE;
    PROVIDE (_stack = .);
  } > RAM

  _end = . ;
  PROVIDE (end = .);

  __stack_end__ = 0x40000000 + 16K - 4;
} 
