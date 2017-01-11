/*
 * MyJIT arm-codegen.h
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
 *
 * Constants used in this file taken from the Mono Project.
 *
 * Copyright (c) 2002-2003 Sergey Chaban <serge@wildwestsoftware.com>
 * Copyright 2005-2011 Novell Inc
 * Copyright 2011 Xamarin Inc
 */

#include <stdint.h>

#define is_imm8(x) (((x) & ~0xff) == 0)
#define ror(x, shift) ((((unsigned int) (x)) >> (shift)) | ((unsigned int) (x) << (32 - (shift))))
#define rol(x, shift) ((((unsigned int) (x)) << (shift)) | ((unsigned int) (x) >> (32 - (shift))))

#define B(x, y) ((y) << x)

typedef enum {
	ARMREG_R0 = 0,
	ARMREG_R1,
	ARMREG_R2,
	ARMREG_R3,
	ARMREG_R4,
	ARMREG_R5,
	ARMREG_R6,
	ARMREG_R7,
	ARMREG_R8,
	ARMREG_R9,
	ARMREG_R10,
	ARMREG_R11,
	ARMREG_R12,
	ARMREG_R13,
	ARMREG_R14,
	ARMREG_R15,


	/* aliases */
	/* args */
	ARMREG_A1 = ARMREG_R0,
	ARMREG_A2 = ARMREG_R1,
	ARMREG_A3 = ARMREG_R2,
	ARMREG_A4 = ARMREG_R3,

	/* local vars */
	ARMREG_V1 = ARMREG_R4,
	ARMREG_V2 = ARMREG_R5,
	ARMREG_V3 = ARMREG_R6,
	ARMREG_V4 = ARMREG_R7,
	ARMREG_V5 = ARMREG_R8,
	ARMREG_V6 = ARMREG_R9,
	ARMREG_V7 = ARMREG_R10,

	ARMREG_FP = ARMREG_R11,
	ARMREG_IP = ARMREG_R12,
	ARMREG_SP = ARMREG_R13,
	ARMREG_LR = ARMREG_R14,
	ARMREG_PC = ARMREG_R15,

	/* co-processor */
	ARMREG_CR0 = 0,
	ARMREG_CR1,
	ARMREG_CR2,
	ARMREG_CR3,
	ARMREG_CR4,
	ARMREG_CR5,
	ARMREG_CR6,
	ARMREG_CR7,
	ARMREG_CR8,
	ARMREG_CR9,
	ARMREG_CR10,
	ARMREG_CR11,
	ARMREG_CR12,
	ARMREG_CR13,
	ARMREG_CR14,
	ARMREG_CR15,

	/* XScale: acc0 on CP0 */
	ARMREG_ACC0 = ARMREG_CR0,

	ARMREG_MAX = ARMREG_R15
} ARMReg;

typedef enum {
	ARMCOND_EQ = 0x0,          /* Equal; Z = 1 */
	ARMCOND_NE = 0x1,          /* Not equal, or unordered; Z = 0 */
	ARMCOND_CS = 0x2,          /* Carry set; C = 1 */
	ARMCOND_HS = ARMCOND_CS,   /* Unsigned higher or same; */
	ARMCOND_CC = 0x3,          /* Carry clear; C = 0 */
	ARMCOND_LO = ARMCOND_CC,   /* Unsigned lower */
	ARMCOND_MI = 0x4,          /* Negative; N = 1 */
	ARMCOND_PL = 0x5,          /* Positive or zero; N = 0 */
	ARMCOND_VS = 0x6,          /* Overflow; V = 1 */
	ARMCOND_VC = 0x7,          /* No overflow; V = 0 */
	ARMCOND_HI = 0x8,          /* Unsigned higher; C = 1 && Z = 0 */
	ARMCOND_LS = 0x9,          /* Unsigned lower or same; C = 0 || Z = 1 */
	ARMCOND_GE = 0xA,          /* Signed greater than or equal; N = V */
	ARMCOND_LT = 0xB,          /* Signed less than; N != V */
	ARMCOND_GT = 0xC,          /* Signed greater than; Z = 0 && N = V */
	ARMCOND_LE = 0xD,          /* Signed less than or equal; Z = 1 && N != V */
	ARMCOND_AL = 0xE,          /* Always */
	ARMCOND_NV = 0xF,          /* Never */

	ARMCOND_SHIFT = 28
} ARMCond;

typedef enum {
	ARMSHIFT_LSL = 0,
	ARMSHIFT_LSR = 1,
	ARMSHIFT_ASR = 2,
	ARMSHIFT_ROR = 3,

	ARMSHIFT_ASL = ARMSHIFT_LSL
	/* rrx = (ror, 1) */
} ARMShiftType;

/*
typedef struct {
	armword_t PSR_c : 8;
	armword_t PSR_x : 8;
	armword_t PSR_s : 8;
	armword_t PSR_f : 8;
} ARMPSR;
*/

typedef enum {
	ARMOP_AND = 0x0,
	ARMOP_EOR = 0x1,
	ARMOP_SUB = 0x2,
	ARMOP_RSB = 0x3,
	ARMOP_ADD = 0x4,
	ARMOP_ADC = 0x5,
	ARMOP_SBC = 0x6,
	ARMOP_RSC = 0x7,
	ARMOP_TST = 0x8,
	ARMOP_TEQ = 0x9,
	ARMOP_CMP = 0xa,
	ARMOP_CMN = 0xb,
	ARMOP_ORR = 0xc,
	ARMOP_MOV = 0xd,
	ARMOP_BIC = 0xe,
	ARMOP_MVN = 0xf,


	/* not really opcodes */

	ARMOP_STR = 0x0,
	ARMOP_LDR = 0x1,

	/* ARM2+ */
	ARMOP_MUL   = 0x0, /* Rd := Rm*Rs */
	ARMOP_MLA   = 0x1, /* Rd := (Rm*Rs)+Rn */

	/* ARM3M+ */
	ARMOP_UMULL = 0x4,
	ARMOP_UMLAL = 0x5,
	ARMOP_SMULL = 0x6,
	ARMOP_SMLAL = 0x7,

	/* for data transfers with register offset */
	ARM_UP   = 1,
	ARM_DOWN = 0
} ARMOpcode;

/**
 * Returns even number of bits necessary to rotate to the left
 * to obtain a imm8 value. If no such value exists, returns -1.
 * This value can be used along with the ROR-feature of data operations.
 */
static inline int arm32_imm_rotate(int x)
{
	if (is_imm8(x)) return 0;
	for (int i = 2; i < 32; i += 2) {
		x = rol(x, 2);
		if (is_imm8(x)) return i;
	}
	return -1;
}

static inline int arm32_encode_imm(int x)
{
	int r = arm32_imm_rotate(x);
	if (r == -1) abort();
	return ((r / 2) << 8) | (rol(x, r) & 0xff);
}

#define ENSURE_INT(_i) ((intptr_t)(_i))

#define arm32_emit(ins, op) \
	do { \
		*((unsigned int *)(ins)) = op; \
		(ins) = (unsigned char *)((unsigned int*)(ins) + 1); \
	} while (0)


#define arm32_emit_al(ins, op) arm32_emit(ins, B(28, ARMCOND_AL) | (op))


#define arm32_encode_dataop(ins, _cond, _imm, _opcode, _s, _rd, _rn, _op2) arm32_emit(ins, \
	  B(28, _cond)     \
	| B(25, (_imm))    \
	| B(21, (_opcode)) \
	| B(20, _s) \
	| B(16, _rn)\
	| B(12, _rd)\
	| B(0,  ((_op2) & 0xfff)))
	
#define arm32_encode_branch(ins, _cond, _link, _offset) arm32_emit(ins, \
	  B(28, _cond) \
	| B(25, 0x5)   \
	| B(24, _link) \
	| B(0,  (_offset) & 0xffffff))


#define arm32_branch(ins, _cond, _offset) \
	arm32_encode_branch(ins, _cond, 0, _offset - 2)

#define arm32_bx(ins, _cond, _rn) arm32_emit(ins, \
	  B(28, _cond) \
	| B(4,  0x12fff1) \
	| B(0, _rn))

#define arm32_blx_reg(ins, _rn) arm32_emit_al(ins, \
	  B(4, 0x12fff3) \
	| B(0, _rn))

#define arm32_patch(target, pos) \
	do { \
		long __p =  ((long)(pos)) >> 2; \
		long __t =  ((long)(target)) >> 2; \
		long __location = (__p - __t - 2); \
			*(int *)(target) &= ~(0xffffff); /* 24 bits */\
			*(int *)(target) |= (0xffffff & __location); /* 24 bits */\
	} while (0)

#define arm32_cond_movw_reg_imm16(ins, _cond, _rd, _imm) \
	do { \
		unsigned __val = _imm; \
		arm32_emit(ins, \
			  B(28, _cond) \
			| B(20, 0x30)  \
			| B(12, _rd)   \
			| B(4,  (__val & 0xf000)) \
			| B(0,  (__val & 0xfff))); \
	} while (0)

#define arm32_cond_movt_reg_imm16(ins, _cond, _rd, _imm) \
	do { \
		unsigned __val = _imm; \
		arm32_emit(ins, \
			  B(28, _cond) \
			| B(20, 0x34)  \
			| B(12, _rd)   \
			| B(4,  (__val & 0xf000)) \
			| B(0,  (__val & 0xfff))); \
	} while (0)

#define arm32_cond_mov_reg_imm32(ins, _cond, _rd, _imm) \
	/* FIXME: multiple access to _imm */ \
	do { \
		if (arm32_imm_rotate(ENSURE_INT(_imm)) != -1) { \
			arm32_cond_alu_reg_imm(ins, _cond, ARMOP_MOV, _rd, 0, ENSURE_INT(_imm));\
		} else if (arm32_imm_rotate(~ENSURE_INT(_imm)) != -1) {\
			arm32_cond_alu_reg_imm(ins, _cond, ARMOP_MVN, _rd, 0, ~ENSURE_INT(_imm));\
		} else { \
			arm32_cond_movw_reg_imm16(ins, _cond, _rd, ENSURE_INT(_imm) & 0xffff); \
			arm32_cond_movt_reg_imm16(ins, _cond, _rd, ENSURE_INT(_imm) >> 16 & 0xffff); \
		} \
	} while (0)

#define arm32_mov_reg_imm32(ins, _rd, _imm) \
	arm32_cond_mov_reg_imm32(ins, ARMCOND_AL, _rd, _imm)
	
#define arm32_alu_reg_reg(ins, _opcode, _rd, _rn, _rm) \
	arm32_encode_dataop(ins, ARMCOND_AL, 0, _opcode, 0, _rd, _rn, _rm)

#define arm32_alu_reg_imm(ins, _opcode, _rd, _rn, _imm) \
	arm32_encode_dataop(ins, ARMCOND_AL, 1, _opcode, 0, _rd, _rn, arm32_encode_imm(ENSURE_INT(_imm)))

#define arm32_cond_alu_reg_imm(ins, _cond, _opcode, _rd, _rn, _imm) \
	arm32_encode_dataop(ins, _cond, 1, _opcode, 0, _rd, _rn, arm32_encode_imm(ENSURE_INT(_imm)))

#define arm32_alucc_reg_reg(ins, _opcode, _cc, _rd, _rn, _rm) \
	arm32_encode_dataop(ins, ARMCOND_AL, 0, _opcode, _cc, _rd, _rn, _rm)

#define arm32_alucc_reg_imm(ins, _opcode, _cc, _rd, _rn, _imm) \
	arm32_encode_dataop(ins, ARMCOND_AL, 1, _opcode, _cc, _rd, _rn, arm32_encode_imm(_imm))

#define arm32_mov_reg_reg(ins, _rd, _rm) \
	arm32_alu_reg_reg(ins, ARMOP_MOV, _rd, 0, _rm)

#define arm32_cmp_reg_reg(ins, _rd, _rm) \
	arm32_alucc_reg_reg(ins, ARMOP_CMP, 1, 0, _rd, _rm)

#define arm32_cmp_reg_imm(ins, _rd, _imm) \
	arm32_alucc_reg_imm(ins, ARMOP_CMP, 1, 0, _rd, _imm)

#define arm32_tst_reg_reg(ins, _rd, _rm) \
	arm32_alucc_reg_reg(ins, ARMOP_TST, 1, 0, _rd, _rm)

#define arm32_tst_reg_imm(ins, _rd, _imm) \
	arm32_alucc_reg_imm(ins, ARMOP_TST, 1, 0, _rd, _imm)

#define arm32_nop(ins) arm32_emit_al(ins, 0x320f000)

#define arm32_mul(ins, rd, rm, rn) arm32_emit_al(ins, \
	  B(16, rd) \
	| B(8,  rm) \
	| B(4,  0x9) \
	| B(0,  rn))

#define arm32_hmul(ins, rd, rm, rn) arm32_emit_al(ins, \
	  B(20, 0x75) \
	| B(16, rd)   \
	| B(12, 0xf)  \
	| B(8, rm)    \
	| B(4, 0x1)   \
	| B(0, rn))

#define arm32_xdiv(ins, tag, rd, rn, rm) arm32_emit_al(ins, \
	  B(20, tag) \
	| B(16, rd)  \
	| B(12, 0xf) \
	| B(8,  rm)  \
	| B(4,  0x1) \
	| B(0,  rn))

#define arm32_sdiv(ins, rd, rn, rm) \
	arm32_xdiv(ins, 0x71, rd, rn, rm);


#define arm32_udiv(ins, rd, rn, rm) \
	arm32_xdiv(ins, 0x73, rd, rn, rm);

#define arm32_pushall(ins) arm32_emit_al(ins, \
	  B(16, 0x92d) \
	| B(0,  0x4fff)) 

#define arm32_pushall_but_r0(ins) arm32_emit_al(ins, \
	  B(16, 0x92d) \
	| B(0,  0x6ffe)) 

#define arm32_popall(ins) arm32_emit_al(ins, \
	  B(16, 0x8bd) \
	| B(0,  0x4fff))

#define arm32_popall_but_r0(ins) arm32_emit_al(ins, \
	  B(16, 0x8bd) \
	| B(0,  0x6ffe))

#define arm32_single_data_transfer(ins, load, regs, byte, rd, rn, op2) \
	/* FIXME: multiple access to _imm */ \
	do { \
		unsigned int op = 0; \
		op |= ARMCOND_AL << 28; \
		op |= 0x1 << 26; \
		op |= regs << 25; /* imm */ \
		op |= 0x1 << 24; /* post/pref index */ \
		op |= byte << 22; \
		op |= 0x0 << 21; /* write */ \
		op |= load << 20; /* load */ \
		op |= rn << 16; \
		op |= rd << 12; \
		if (regs) { \
			op |= 0x1 << 23; /* rn + rm */ \
			op |= op2; \
		} else { \
			int _imm = ENSURE_INT(op2); \
			if (arm32_imm_rotate(_imm) >= 0) { \
				op |= arm32_encode_imm(op2); \
				op |= 0x1 << 23; /* rn + imm */ \
			} else if (arm32_imm_rotate(-_imm) >= 0) { \
				op |= arm32_encode_imm(-_imm); \
				op |= 0x0 << 23; /* rn - imm */ \
			} else { printf("aaa\n"); abort(); } \
		} \
		arm32_emit(ins, op);\
	} while (0) \

#define arm32_ld_reg(ins, rd, rn, rm) \
	arm32_single_data_transfer(ins, 1, 1, 0, rd, rn, rm)

#define arm32_ldub_reg(ins, rd, rn, rm) \
	arm32_single_data_transfer(ins, 1, 1, 1, rd, rn, rm)

#define arm32_ld_imm(ins, rd, rn, imm) \
	arm32_single_data_transfer(ins, 1, 0, 0, rd, rn, imm)

#define arm32_ldub_imm(ins, rd, rn, imm) \
	arm32_single_data_transfer(ins, 1, 0, 1, rd, rn, imm)

#define arm32_st_reg(ins, rd, rn, rm) \
	arm32_single_data_transfer(ins, 0, 1, 0, rd, rn, rm)

#define arm32_stb_reg(ins, rd, rn, rm) \
	arm32_single_data_transfer(ins, 0, 1, 1, rd, rn, rm)

#define arm32_st_imm(ins, rd, rn, imm) \
	arm32_single_data_transfer(ins, 0, 0, 0, rd, rn, imm)

#define arm32_stb_imm(ins, rd, rn, imm) \
	arm32_single_data_transfer(ins, 0, 0, 1, rd, rn, imm)

#define arm32_signed_and_half_data_transfer_reg(ins, load, signed_value, halfword, rd, rn, rm) \
arm32_emit_al(ins, \
	  B(24, 0x1) \
	| B(23, 0x1) /* UP => rn + rm */ \
	| B(22, 0) \
	| B(21, 0) /* write */ \
	| B(20, load) \
	| B(16, rn) \
	| B(12, rd) \
	| B(7,  0x1) \
	| B(6,  signed_value) \
	| B(5,  halfword) \
	| B(4,  0x1) \
	| B(0,  rm)) \

#define arm32_signed_and_half_data_transfer_imm(ins, load, signed_value, halfword, rd, rn, imm) \
	do { \
		int __val = imm; \
		int __absval = (__val < 0 ? -__val : __val); \
		arm32_emit_al(ins,\
			  B(24, 0x1) \
			| B(23, __val >= 0) /* UP => rn + rm */ \
			| B(22, 0x1) \
			| B(21, 0x0) /* write */ \
			| B(20, load) \
			| B(16, rn) \
			| B(12, rd) \
			| B(8,  (__absval >> 4) & 0xf) \
			| B(7,  0x1) \
			| B(6,  signed_value) \
			| B(5,  halfword) \
			| B(4,  0x1) \
			| B(0,  __absval & 0xf)); \
	} while (0)

#define arm32_ldsb_reg(ins, rd, rn, rm) \
	arm32_signed_and_half_data_transfer_reg(ins, 1, 1, 0, rd, rn, rm)

#define arm32_ldsh_reg(ins, rd, rn, rm) \
	arm32_signed_and_half_data_transfer_reg(ins, 1, 1, 1, rd, rn, rm)

#define arm32_lduh_reg(ins, rd, rn, rm) \
	arm32_signed_and_half_data_transfer_reg(ins, 1, 0, 1, rd, rn, rm)

#define arm32_ldsb_imm(ins, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, 1, 1, 0, rd, rn, imm)

#define arm32_ldsh_imm(ins, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, 1, 1, 1, rd, rn, imm)

#define arm32_lduh_imm(ins, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, 1, 0, 1, rd, rn, imm)

#define arm32_sth_reg(ins, rd, rn, rm) \
	arm32_signed_and_half_data_transfer_reg(ins, 0, 0, 1, rd, rn, rm)

#define arm32_sth_imm(ins, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, 0, 0, 1, rd, rn, imm)

#define arm32_shift_reg(ins, type, rd, rn, rs) arm32_emit_al(ins, \
	  B(21, 0xd) \
	| B(20, 0) /* s */ \
	| B(12, rd) \
	| B(5,  type) \
	| B(4,  1) /* reg */ \
	| B(8,  rs) \
	| B(0,  rn))

#define arm32_lsh_reg(ins, rd, rn, rs) \
	arm32_shift_reg(ins, ARMSHIFT_LSL, rd, rn, rs)

#define arm32_rsh_reg(ins, rd, rn, rs) \
	arm32_shift_reg(ins, ARMSHIFT_LSR, rd, rn, rs)

#define arm32_rsa_reg(ins, rd, rn, rs) \
	arm32_shift_reg(ins, ARMSHIFT_ASR, rd, rn, rs)

#define arm32_shift_imm(ins, type, rd, rn, imm) arm32_emit_al(ins, \
	  B(21, 0xd) \
	| B(20, 0) /* s */ \
	| B(12, rd) \
	| B(5,  type) \
	| B(4,  0) /* imm */ \
	| B(7,  (imm) & 0x1f) \
	| B(0,  rn))

#define arm32_lsh_imm(ins, rd, rn, imm) \
	arm32_shift_imm(ins, ARMSHIFT_LSL, rd, rn, imm)

#define arm32_rsh_imm(ins, rd, rn, imm) \
	arm32_shift_imm(ins, ARMSHIFT_LSR, rd, rn, imm)

#define arm32_rsa_imm(ins, rd, rn, imm) \
	arm32_shift_imm(ins, ARMSHIFT_ASR, rd, rn, imm)


#define arm32_stack_op(ins, _opcode, _imm) \
	/* FIXME: multiple access to _imm */ \
	do { \
		if (arm32_imm_rotate(_imm) == -1) { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, _imm); \
			arm32_alu_reg_reg(ins, _opcode, ARMREG_SP, ARMREG_SP, ARMREG_R12); \
		} else { \
			arm32_alu_reg_imm(ins, _opcode, ARMREG_SP, ARMREG_SP, arm32_encode_imm(_imm)); \
		} \
	} while (0)

	
#define arm32_add_sp_imm(ins, imm)\
	arm32_stack_op(ins, ARMOP_ADD, (imm))

#define arm32_sub_sp_imm(ins, imm)\
	arm32_stack_op(ins, ARMOP_SUB, (imm))

#define arm32_ld_fp_imm(ins, dest, _imm) \
	/* FIXME: multiple access to _imm */ \
	do { \
		if (arm32_imm_rotate(_imm) == -1) { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, _imm); \
			arm32_ld_reg(ins, dest, ARMREG_FP, ARMREG_R12); \
		} else { \
			arm32_ld_imm(ins, dest, ARMREG_FP, _imm); \
		} \
	} while (0)

#define arm32_st_fp_imm(ins, dest, _imm) \
	/* FIXME: multiple access to _imm */ \
	do { \
		if (arm32_imm_rotate(_imm) == -1) { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, _imm); \
			arm32_st_reg(ins, dest, ARMREG_FP, ARMREG_R12); \
		} else { \
			arm32_st_imm(ins, dest, ARMREG_FP, _imm); \
		} \
	} while (0)


#define arm32_sub_reg_reg(ins, rd, rn, rm) \
	arm32_alu_reg_reg(ins, ARMOP_SUB, rd, rn, rm);

#define arm32_and_reg_imm(ins, rd, rn, rm) \
	arm32_alu_reg_imm(jit->ip, ARMOP_AND, rd, rn, rm)
