ENTRY(main)
SECTIONS
{
 . = 0x0;
 .text : { *(.text .text.*) }
 . = ALIGN(0x1000); 
 .rodata : { *(.rodata .rodata.*) }
 . = ALIGN(0x1000); 
 /* .eh_frame : { *(.eh_frame) *(.eh_frame.*) } */
}
