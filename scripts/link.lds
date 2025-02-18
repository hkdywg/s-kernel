/*                                                                                                                                                                     
 *  link.lds
 *
 *  brif
 *      kernel link scripts
 *  
 *  (C) 2025.01.03 <hkdywg@163.com>
 *
 *  This program is free software; you can redistribute it and/r modify
 *  it under the terms of the GNU General Public License version 2 as
 *  published by the Free Software Foundation.
 * */

SECTIONS
{
	. = 0x40008000;
	. = ALIGN(4096);
	.text :
	{
		KEEP(*(.text.entrypoint))		/* The entry point */
		*(.vectors)						/* vectors */
		*(.text)						/* remaining code */
		*(.text.*)						/* remaining code */

		*(.rodata)						/* read-only data (constants) */
		*(.rodata*)

		/* section information for shell */
		. = ALIGN(16);
		__tsymtab_tart = .;
		KEEP(*(Tsymtab))
		__tsymtab_end = .;

		/* section information for initial */
		. = ALIGN(16);
		__init_start = .;
		KEEP(*(SORT(.skernel_fn*)))
		__init_end = .;
		. = ALIGN(16);

		. = ALIGN(16);
		_etext = .;
	}

	. = ALIGN(16);
	.data :
	{
		*(.data)
		*(.data.*)

		*(.data1)
		*(.data1.*)

		. = ALIGN(16);
		_gp = ABSOLUTE(.);		/* Base of small data */

		*(.sdata)
		*(.sdata.*)
	}

	. = ALIGN(16);
	.bss :
	{
		PROVIDE(__bss_start = .);
		*(.bss)
		*(.bss.*)
		*(.dynbss)
		*(COMMON)
		PROVIDE(__bss_end = .);
	}
	_end = .;
}

__bss_size = (__bss_end - __bss_start) >> 3;

