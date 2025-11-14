ENTRY(main)
SECTIONS
{
 . = 0x0;
 .text : { *(.text .text.*) }
 .rodata : { *(.rodata .rodata.*) }
 .eh_frame : { *(.eh_frame) *(.eh_frame.*) }
}
