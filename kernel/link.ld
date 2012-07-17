OUTPUT_FORMAT(elf32-i386)
ENTRY(start)

SECTIONS
{
  . = 0xC0100000;
  .text : AT(ADDR(.text) - 0xC0000000)
  {
	  mboot = .;
	  code = .;
	  *(.__mbootheader)
	  *(.text);
	  . = ALIGN(0x1000);
  }

  .data : AT(ADDR(.data) - 0xC0000000)
  {
	  data = .;
      *(.data);
	  *(.rodata*);
	  . = ALIGN(0x1000);
  }

  .bss : AT(ADDR(.bss) - 0xC0000000)
  {
	  bss = .;
	  *(COMMON);
      *(.bss);
	  . = ALIGN(0x1000);
  }
  end = ALIGN(0x1000);
}

