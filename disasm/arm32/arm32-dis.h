/*
 * MyJIT Disassembler 
 *
 * Copyright (C) 2017 Petr Krajca, <petr.krajca@upol.cz>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 3 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 * 
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 */

#include <stdint.h>

typedef struct arm32d_t {
	unsigned long pc;
	uint32_t *ibuf;
	unsigned int ibuf_index;
	unsigned int ibuf_size;
	char out[1024];
} arm32d_t;

void arm32d_init(arm32d_t *);
void arm32d_set_input_buffer(arm32d_t *, unsigned char *, int);
int arm32d_disassemble(arm32d_t *);
char *arm32d_insn_asm(arm32d_t *);
void arm32d_set_pc(arm32d_t *, unsigned long);
