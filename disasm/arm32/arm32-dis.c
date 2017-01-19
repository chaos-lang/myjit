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

#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>

#include "arm32-dis.h"

#define ror(x, shift) ((((unsigned int) (x)) >> (shift)) | ((unsigned int) (x) << (32 - (shift))))
#define ABS(x)  ((x) < 0 ? - (x) : x)

#define B(x, index, mask_length) ((((unsigned long) x) >> index) & ((1UL << mask_length) - 1))

static char* shift_type[] = { "lsl", "lsr", "asr", "ror" };

enum op_type {
	ARITHM_OP,
	COMP_OP,
	ASSIGNMENT_OP
};

struct data_op {
	char *name;
	enum op_type type;
};

static struct data_op data_ops[] = {
	{ "and", ARITHM_OP },
	{ "eor", ARITHM_OP },
	{ "sub", ARITHM_OP },
	{ "rsb", ARITHM_OP },

	{ "add", ARITHM_OP },
	{ "adc", ARITHM_OP },
	{ "sbc", ARITHM_OP },
	{ "rsc", ARITHM_OP },

	{ "tst", COMP_OP },
	{ "teq", COMP_OP },
	{ "cmp", COMP_OP },
	{ "cmn", COMP_OP },

	{ "orr", ARITHM_OP },
	{ "mov", ASSIGNMENT_OP },
	{ "bic", ARITHM_OP },
	{ "mvn", ASSIGNMENT_OP }
};

#define IMM(imm) (ror(B(imm, 0, 8), B(imm, 8, 4) * 2))

static inline int sign_ext(int bits, int value)
{
        if (value & (1 << (bits - 1))) return value | (-1 << (bits - 1));
        return value;
}

/*
 *
 * Output functions
 *
 */
static int arm32d_out_printf(char *buf, const char *format, ...) {
        va_list ap;
        va_start(ap, format);
        int result = vsprintf(buf + strlen(buf), format, ap);
        va_end(ap);
        return result;
}


static void arm32d_out_name_s(arm32d_t *dis, uint32_t insn, char *opname, char *s)
{
	static char* cond[] = { "eq", "ne", "cs", "cc", "mi",  "pl", "vs", "vc", "hi", "ls", "ge",  "lt",  "gt", "le", "", "nv"};

	memcpy(dis->out, opname, strlen(opname) + 1);
	strcat(dis->out, cond[B(insn, 28, 4)]);
	strcat(dis->out, s);
	strcat(dis->out, " ");
}

static void arm32d_out_name(arm32d_t *dis, uint32_t insn, char *opname)
{
	arm32d_out_name_s(dis, insn, opname, "");
}

static void arm32d_out_addr(arm32d_t *dis, int addr)
{
	arm32d_out_printf(dis->out, "%x <pc %s %i>", 
			dis->pc + addr,
			addr < 0 ? "-" : "+", 
			ABS(addr));
}

static void arm32d_out_comma(arm32d_t *dis)
{
	strcat(dis->out, ", ");
}

static void arm32d_out_reg(arm32d_t *dis, int reg)
{
	static char *regs[] = {
		"r0", "r1", "r2", "r3", "r4", "r5", "r6", "r7",
		"r8", "r9", "r10", "fp", "r12", "sp", "lr", "pc" };

	strcat(dis->out, regs[reg]);
}

static void arm32d_out_shift(arm32d_t *dis, int shift)
{
	if (B(shift, 0, 1) || (B(shift, 3, 5))) {
		arm32d_out_comma(dis);
		arm32d_out_printf(dis->out, "%s ", shift_type[B(shift, 1, 2)]);
		if (B(shift, 0, 1)) arm32d_out_reg(dis, B(shift, 4, 4));
		else arm32d_out_printf(dis->out, "%i", B(shift, 3, 5));
	}
}

static void arm32d_out_imm(arm32d_t *dis, int imm)
{
	arm32d_out_printf(dis->out, "0x%x (%i)", imm, imm);
}

/*
 *
 * Decoding
 *
 */
static void decode_dataop_imm(arm32d_t *dis, uint32_t insn)
{
	struct data_op op = data_ops[B(insn, 21, 4)];

	arm32d_out_name_s(dis, insn, op.name, B(insn, 20, 1) && (op.type != COMP_OP) ? "s" : "");
	switch (op.type) {
		case ARITHM_OP:
			arm32d_out_reg(dis, B(insn, 12, 4));
			arm32d_out_comma(dis);
			arm32d_out_reg(dis, B(insn, 16, 4));
			arm32d_out_comma(dis);
			arm32d_out_imm(dis, IMM(B(insn, 0, 12)));
			break;

		case COMP_OP:
			arm32d_out_reg(dis, B(insn, 16, 4));
			arm32d_out_comma(dis);
			arm32d_out_imm(dis, IMM(B(insn, 0, 12)));
			break;

		case ASSIGNMENT_OP:
			arm32d_out_reg(dis, B(insn, 12, 4));
			arm32d_out_comma(dis);
			arm32d_out_imm(dis, IMM(B(insn, 0, 12)));
			break;
	}
}

static void decode_dataop_reg(arm32d_t *dis, uint32_t insn)
{
	if ((B(insn, 21, 4) == 13) && B(insn, 4, 8)) {
		arm32d_out_name(dis, insn, shift_type[B(insn, 5, 2)]);
		arm32d_out_reg(dis, B(insn, 12, 4));
		arm32d_out_comma(dis);

		arm32d_out_reg(dis, B(insn, 16, 4));
		arm32d_out_comma(dis);

		if (B(insn, 4, 1)) arm32d_out_reg(dis, B(insn, 8, 4));
		else arm32d_out_printf(dis->out, "%i", B(insn, 7, 4));
		return;
	}


	struct data_op op = data_ops[B(insn, 21, 4)];
	arm32d_out_name_s(dis, insn, op.name, B(insn, 20, 1) && (op.type != COMP_OP) ? "s" : "");

	switch (op.type) {
		case ARITHM_OP:
			arm32d_out_reg(dis, B(insn, 12, 4));
			arm32d_out_comma(dis);
			arm32d_out_reg(dis, B(insn, 16, 4));
			arm32d_out_comma(dis);
			arm32d_out_imm(dis, B(insn, 0, 12));
			arm32d_out_shift(dis, B(insn, 4, 8));
			break;

		case COMP_OP:
			arm32d_out_reg(dis, B(insn, 16, 4));
			arm32d_out_comma(dis);
			arm32d_out_reg(dis, B(insn, 0, 4));
			arm32d_out_shift(dis, B(insn, 4, 8));
			break;

		case ASSIGNMENT_OP:
			arm32d_out_reg(dis, B(insn, 12, 4));
			arm32d_out_comma(dis);
			arm32d_out_reg(dis, B(insn, 0, 4));
			arm32d_out_shift(dis, B(insn, 4, 8));
			break;
	}
}

static void decode_bx(arm32d_t *dis, uint32_t insn, int link)
{
	if (link) arm32d_out_name(dis, insn, "blx");
	else arm32d_out_name(dis, insn, "bx");
	arm32d_out_reg(dis, B(insn, 0, 4));
} 

static void decode_branch(arm32d_t *dis, uint32_t insn)
{
	if (B(insn, 24, 1)) arm32d_out_name(dis, insn, "bl");
	else arm32d_out_name(dis, insn, "b");
	int offset = sign_ext(24, B(insn, 0, 24)) * 4 + 8;
	arm32d_out_addr(dis, offset);
}

static void decode_single_data_transfer(arm32d_t *dis, uint32_t insn)
{
	int imm      = !B(insn, 25, 1);
	int preindex = B(insn, 24, 1);
	int up       = B(insn, 23, 1);
	int byte     = B(insn, 22, 1);
	int wback    = B(insn, 21, 1);
	int load     = B(insn, 20, 1);
	int rd       = B(insn, 12, 4);
	int rn       = B(insn, 16, 4);
	int rm       = B(insn, 0, 4);
	int immval   = B(insn, 0, 12);
	int shift    = B(insn, 4, 8);

	if (!load && wback && (rn == 13) && preindex && !up && (immval == 4)) {
		arm32d_out_name(dis, insn, "push");
		arm32d_out_reg(dis, rd);
		return;
	}

	if (load && !wback && (rn == 13) && up && !preindex && (immval == 4)) {
		arm32d_out_name(dis, insn, "pop");
		arm32d_out_reg(dis, rd);
		return;
	}

	if (load && byte) arm32d_out_name(dis, insn, "ldrb");
	if (load && !byte) arm32d_out_name(dis, insn, "ldr");
	if (!load && byte) arm32d_out_name(dis, insn, "strb");
	if (!load && !byte) arm32d_out_name(dis, insn, "str");

	arm32d_out_reg(dis, rd);
	arm32d_out_comma(dis);
	arm32d_out_printf(dis->out, "[");
	arm32d_out_reg(dis, rn);
	if (!preindex) arm32d_out_printf(dis->out, "]");
	arm32d_out_printf(dis->out, " %s ", up ? "+" : "-");
	if (imm) {
		arm32d_out_printf(dis->out, "0x%x", IMM(immval));
	} else {
		arm32d_out_reg(dis, rm);
		arm32d_out_shift(dis, shift);
	}
	if (preindex) arm32d_out_printf(dis->out, "]");

	if (preindex && wback) arm32d_out_printf(dis->out, "!");
}

static void decode_half_word_transfer(arm32d_t *dis, uint32_t insn)
{
	int preindex = B(insn, 24, 1);
	int up       = B(insn, 23, 1);
	int imm      = B(insn, 22, 1);
	int wback    = B(insn, 21, 1);
	int load     = B(insn, 20, 1);
	int rn       = B(insn, 16, 4);
	int rd       = B(insn, 12, 4);
	int hoffset  = B(insn, 8, 4);
	int loffset  = B(insn, 0, 4);
	int rm       = B(insn, 0, 4);
	int sign     = B(insn, 6, 1);
	int halfword = B(insn, 5, 1);

	int offset = sign_ext(8, (hoffset << 4 | loffset));

	if (load && halfword && sign) arm32d_out_name(dis, insn, "ldrsh");
	if (load && halfword && !sign) arm32d_out_name(dis, insn, "ldrh");
	if (load && !halfword && sign) arm32d_out_name(dis, insn, "ldrsb");
	if (load && !halfword && !sign) arm32d_out_name(dis, insn, "ldrb");
	if (!load && !halfword) arm32d_out_name(dis, insn, "strb");
	if (!load && halfword) arm32d_out_name(dis, insn, "strh");

	arm32d_out_reg(dis, rd);
	arm32d_out_comma(dis);
	arm32d_out_printf(dis->out, "[");
	arm32d_out_reg(dis, rn);

	if (!preindex) arm32d_out_printf(dis->out, "]");
	arm32d_out_printf(dis->out, " %s ", up ? "+" : "-");
	if (imm) {
		arm32d_out_printf(dis->out, "0x%x", IMM(offset));
	} else {
		arm32d_out_reg(dis, rm);
	}
	if (preindex) arm32d_out_printf(dis->out, "]");

	if (preindex && wback) arm32d_out_printf(dis->out, "!");
}

static void decode_mov16(arm32d_t *dis, uint32_t insn, char* name)
{
	arm32d_out_name(dis, insn, name);
	arm32d_out_reg(dis, B(insn, 12, 4));
	arm32d_out_comma(dis);
	arm32d_out_imm(dis, (B(insn, 16, 4) << 12) | B(insn, 0, 12));
}

static void decode_misc(arm32d_t *dis, uint32_t insn)
{
	if ((B(insn, 20, 8) == 0) && (B(insn, 4, 4) == 0x9)) {
		arm32d_out_name_s(dis, insn, "mul", B(insn, 20, 1) ? "s" : "");
		arm32d_out_reg(dis, B(insn, 16, 4));
		arm32d_out_comma(dis);
		arm32d_out_reg(dis, B(insn, 0, 4));
		arm32d_out_comma(dis);
		arm32d_out_reg(dis, B(insn, 8, 4));
		return;
	}

	if (B(insn, 5, 2)) decode_half_word_transfer(dis, insn);
}

static void decode_stack_op(arm32d_t *dis, uint32_t insn, char *name)
{
	int regs = B(insn, 0, 15);
	arm32d_out_name(dis, insn, name);
	arm32d_out_printf(dis->out, "{");
	for (int i = 0; i < 15; i++) {
		int index = 1 << i;
		if (regs & index) {
			arm32d_out_reg(dis, i);
			regs ^= index;
			if (regs) arm32d_out_comma(dis);
		}
	}
		
	arm32d_out_printf(dis->out, "}");
}

static void decode_divmul(arm32d_t *dis, uint32_t insn, char *name)
{
	arm32d_out_name(dis, insn, name);
	arm32d_out_reg(dis, B(insn, 16, 4));
	arm32d_out_comma(dis);
	arm32d_out_reg(dis, B(insn, 0, 4));
	arm32d_out_comma(dis);
	arm32d_out_reg(dis, B(insn, 8, 4));
}

static void decode(arm32d_t *dis, uint32_t insn)
{
	if (B(insn, 20, 8) == 0x30) decode_mov16(dis, insn, "movw");
	else if (B(insn, 20, 8) == 0x71) decode_divmul(dis, insn, "sdiv");
	else if (B(insn, 20, 8) == 0x73) decode_divmul(dis, insn, "udiv");
	else if (B(insn, 20, 8) == 0x75) decode_divmul(dis, insn, "smmul");
	else if (B(insn, 16, 12) == 0x92d) decode_stack_op(dis, insn, "push");
	else if (B(insn, 16, 12) == 0x8bd) decode_stack_op(dis, insn, "pop");
	else if ((B(insn, 25, 3) == 0) && B(insn, 7, 1) && B(insn, 4, 1)) decode_misc(dis, insn);
	else if (B(insn, 20, 8) == 0x34) decode_mov16(dis, insn, "movt");
	else if (B(insn, 25, 3) == 0x1) decode_dataop_imm(dis, insn);
	else if (B(insn, 25, 3) == 0x5) decode_branch(dis, insn);
	else if (B(insn, 4, 24) == 0x12fff1) decode_bx(dis, insn, 0);
	else if (B(insn, 4, 24) == 0x12fff3) decode_bx(dis, insn, 1);
	else if (B(insn, 26, 2) == 0x1) decode_single_data_transfer(dis, insn);
	else if (B(insn, 25, 3) == 0x0) decode_dataop_reg(dis, insn);
	else arm32d_out_printf(dis->out, "");
}

/*
 *
 * Public API
 *
 */
void arm32d_init(arm32d_t *dis)
{
	dis->ibuf = NULL;
	dis->ibuf_index = 0;
	dis->ibuf_size = 0;
	dis->pc = 0;
}

void arm32d_set_input_buffer(arm32d_t *dis, unsigned char *buf, int len)
{
	dis->ibuf = (uint32_t *) buf;
	dis->ibuf_index = 0;
	dis->ibuf_size = len / 4;
}

int arm32d_disassemble(arm32d_t *dis)
{
	arm32d_out_name(dis, 0xe0000000, "???");
	if (dis->ibuf_index >= dis->ibuf_size) return 0;
	decode(dis, dis->ibuf[dis->ibuf_index]);
	dis->ibuf_index++;
	dis->pc += 4;
	return 1;
}

char *arm32d_insn_asm(arm32d_t *dis)
{
	return dis->out;
}

void arm32d_set_pc(arm32d_t *dis, unsigned long pc)
{
	dis->pc = pc;
}
