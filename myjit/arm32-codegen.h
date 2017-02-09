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

#define is_pow2(x) ((((x) & (((x) - 1))) == 0) && ((x) != (1 << 31)))
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
	ARMREG_D0 = 0,
	ARMREG_D1,
	ARMREG_D2,
	ARMREG_D3,
	ARMREG_D4,
	ARMREG_D5,
	ARMREG_D6,
	ARMREG_D7,
	ARMREG_D8,
	ARMREG_D9,
	ARMREG_D10,
	ARMREG_D11,
	ARMREG_D12,
	ARMREG_D13,
	ARMREG_D14,
	ARMREG_D15
} ARMVreg;


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

#define arm32_bl_imm(ins, _imm) arm32_emit_al(ins, \
	  B(24, 0xb) \
	| B(0, (_imm) & 0xffffff))


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
	do { \
		int __imm = ENSURE_INT(_imm); \
		if (arm32_imm_rotate(ENSURE_INT(__imm)) != -1) { \
			arm32_cond_alu_reg_imm(ins, _cond, ARMOP_MOV, _rd, 0, __imm);\
		} else if (arm32_imm_rotate(~__imm) != -1) {\
			arm32_cond_alu_reg_imm(ins, _cond, ARMOP_MVN, _rd, 0, ~__imm);\
		} else { \
			arm32_cond_movw_reg_imm16(ins, _cond, _rd, __imm & 0xffff); \
			arm32_cond_movt_reg_imm16(ins, _cond, _rd, __imm >> 16 & 0xffff); \
		} \
	} while (0)

#define arm32_mov_reg_imm32(ins, _rd, _imm) \
	arm32_cond_mov_reg_imm32(ins, ARMCOND_AL, _rd, _imm)

#define arm32_mov_reg_imm32x(ins, _rd, _imm) do { \
		int __imm = ENSURE_INT(_imm); \
		arm32_cond_movw_reg_imm16(ins, ARMCOND_AL, _rd, __imm & 0xffff); \
		arm32_cond_movt_reg_imm16(ins, ARMCOND_AL, _rd, __imm >> 16 & 0xffff); \
	} while (0)
	
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

#define arm32_push_reg(ins, reg) arm32_emit_al(ins, \
	  B(16, 0x52d) \
	| B(12, reg) \
	| B(0,  0x4))

#define arm32_pop_reg(ins, reg) arm32_emit_al(ins, \
	  B(16, 0x49d) \
	| B(12, reg) \
	| B(0,  0x4))

#define arm32_push_regs(ins, mask) do { \
	if (mask == 0) {} \
	else if (is_pow2(mask)) arm32_push_reg(ins, ffs(mask) - 1); \
	else arm32_emit_al(ins, B(16, 0x92d) | B(0,  mask)); \
} while (0)

#define arm32_pop_regs(ins, mask) do {\
	if (mask == 0) {} \
	else if (is_pow2(mask)) arm32_pop_reg(ins, ffs(mask) - 1); \
	else arm32_emit_al(ins, B(16, 0x8bd) | B(0,  mask)); \
} while (0)

#define arm32_pushall(ins) arm32_push_regs(ins, 0x4fff)
#define arm32_pushall_but_r0(ins) arm32_push_regs(ins, 0x6ffe)
#define arm32_pushall_but_r0123(ins) arm32_push_regs(ins, 0x6ff0)


#define arm32_popall(ins) arm32_pop_regs(ins, 0x4fff)
#define arm32_popall_but_r0(ins) arm32_pop_regs(ins, 0x6ffe)
#define arm32_popall_but_r0123(ins) arm32_pop_regs(ins, 0x6ff0)

#define arm32_single_data_transfer(ins, cond, load, regs, byte, rd, rn, op2) \
	do { \
		unsigned int op = \
		  B(28, cond) \
		| B(26, 0x1)  \
		| B(25, regs) \
		| B(24, 0x1) /* post/pre index*/ \
		| B(22, byte) \
		| B(21, 0x0) /* write */ \
		| B(20, load) \
		| B(16, rn) \
		| B(12, rd); \
		if (regs) { \
			op |= B(23, 0x1); /* rn + rm */ \
			op |= B(0, op2); \
		} else { \
			int _imm = ENSURE_INT(op2); \
			if (arm32_imm_rotate(_imm) >= 0) { \
				op |= B(23, 0x1); /* rn + rm */ \
				op |= arm32_encode_imm(op2); \
			} else if (arm32_imm_rotate(-_imm) >= 0) { \
				op |= B(23, 0x0); /* rn - imm */ \
				op |= arm32_encode_imm(-_imm); \
			} else { abort(); } \
		} \
		arm32_emit(ins, op);\
	} while (0) 

#define arm32_ld_reg(ins, rd, rn, rm) \
	arm32_single_data_transfer(ins, ARMCOND_AL, 1, 1, 0, rd, rn, rm)

#define arm32_ldub_reg(ins, rd, rn, rm) \
	arm32_single_data_transfer(ins, ARMCOND_AL, 1, 1, 1, rd, rn, rm)

#define arm32_ld_imm(ins, rd, rn, imm) \
	arm32_single_data_transfer(ins, ARMCOND_AL, 1, 0, 0, rd, rn, imm)

#define arm32_ldub_imm(ins, rd, rn, imm) \
	arm32_single_data_transfer(ins, ARMCOND_AL, 1, 0, 1, rd, rn, imm)

#define arm32_cond_st_reg(ins, cond, rd, rn, rm) \
	arm32_single_data_transfer(ins, cond, 0, 1, 0, rd, rn, rm)

#define arm32_cond_stb_reg(ins, cond, rd, rn, rm) \
	arm32_single_data_transfer(ins, cond, 0, 1, 1, rd, rn, rm)

#define arm32_cond_st_imm(ins, cond, rd, rn, imm) \
	arm32_single_data_transfer(ins, cond, 0, 0, 0, rd, rn, imm)

#define arm32_cond_stb_imm(ins, cond, rd, rn, imm) \
	arm32_single_data_transfer(ins, cond, 0, 0, 1, rd, rn, imm)

#define arm32_st_reg(ins, rd, rn, rm)   arm32_cond_st_reg(ins, ARMCOND_AL, rd, rn, rm)
#define arm32_stb_reg(ins, rd, rn, rm)  arm32_cond_stb_reg(ins, ARMCOND_AL, rd, rn, rm)
#define arm32_st_imm(ins, rd, rn, imm)  arm32_cond_st_imm(ins, ARMCOND_AL, rd, rn, imm)
#define arm32_stb_imm(ins, rd, rn, imm) arm32_cond_stb_imm(ins, ARMCOND_AL, rd, rn, imm)


#define arm32_signed_and_half_data_transfer_reg(ins, cond, load, signed_value, halfword, rd, rn, rm) \
arm32_emit(ins, \
	  B(28, cond) \
	| B(24, 0x1) \
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

#define arm32_signed_and_half_data_transfer_imm(ins, cond, load, signed_value, halfword, rd, rn, imm) \
	do { \
		int __val = imm; \
		int __absval = (__val < 0 ? -__val : __val); \
		arm32_emit_al(ins,\
			  B(28, cond) \
			| B(24, 0x1) \
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
	arm32_signed_and_half_data_transfer_reg(ins, ARMCOND_AL, 1, 1, 0, rd, rn, rm)

#define arm32_ldsh_reg(ins, rd, rn, rm) \
	arm32_signed_and_half_data_transfer_reg(ins, ARMCOND_AL, 1, 1, 1, rd, rn, rm)

#define arm32_lduh_reg(ins, rd, rn, rm) \
	arm32_signed_and_half_data_transfer_reg(ins, ARMCOND_AL, 1, 0, 1, rd, rn, rm)

#define arm32_ldsb_imm(ins, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, ARMCOND_AL, 1, 1, 0, rd, rn, imm)

#define arm32_ldsh_imm(ins, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, ARMCOND_AL, 1, 1, 1, rd, rn, imm)

#define arm32_lduh_imm(ins, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, ARMCOND_AL, 1, 0, 1, rd, rn, imm)

#define arm32_cond_sth_reg(ins, cond, rd, rn, rm) \
	arm32_signed_and_half_data_transfer_reg(ins, cond, 0, 0, 1, rd, rn, rm)

#define arm32_cond_sth_imm(ins, cond, rd, rn, imm) \
	arm32_signed_and_half_data_transfer_imm(ins, cond, 0, 0, 1, rd, rn, imm)

#define arm32_sth_reg(ins, rd, rn, rm) arm32_cond_sth_reg(ins, ARMCOND_AL, rd, rn, rm)
#define arm32_sth_imm(ins, rd, rn, imm) arm32_cond_sth_imm(ins, ARMCOND_AL, rd, rn, imm)


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
	arm32_alu_reg_imm(ins, _opcode, ARMREG_SP, ARMREG_SP, _imm);
	
#define arm32_add_sp_imm(ins, imm)\
	arm32_stack_op(ins, ARMOP_ADD, (imm))

#define arm32_sub_sp_imm(ins, imm)\
	arm32_stack_op(ins, ARMOP_SUB, (imm))

#define arm32_ld_fp_imm(ins, dest, _imm) \
	do { \
		int __disp = _imm; \
		if ((arm32_imm_rotate(__disp) >= 0) || (arm32_imm_rotate(-__disp) >= 0))  { \
			arm32_ld_imm(ins, dest, ARMREG_FP, __disp); \
		} else { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, __disp); \
			arm32_ld_reg(ins, dest, ARMREG_FP, ARMREG_R12); \
		} \
	} while (0)

#define arm32_st_fp_imm(ins, dest, _imm) \
	do { \
		int __disp = _imm; \
		if ((arm32_imm_rotate(__disp) >= 0) || (arm32_imm_rotate(-__disp) >= 0))  { \
			arm32_st_imm(ins, dest, ARMREG_FP, __disp); \
		} else { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, __disp); \
			arm32_st_reg(ins, dest, ARMREG_FP, ARMREG_R12); \
		} \
	} while (0)


#define arm32_sub_reg_reg(ins, rd, rn, rm) \
	arm32_alu_reg_reg(ins, ARMOP_SUB, rd, rn, rm);

#define arm32_and_reg_imm(ins, rd, rn, rm) \
	arm32_alu_reg_imm(jit->ip, ARMOP_AND, rd, rn, rm)

//
//
// VFP instructions
//
//

#define arm32_vcvt(ins, size, vd, vm) arm32_emit_al(ins, \
	  B(16, 0xeb7) \
	| B(12, vd) \
	| B(9, 0x5) \
	| B(8, size) \
	| B(4, 0xc) \
	| B(0, vm))


#define arm32_vcvt_dtos(ins, vd, vm) arm32_vcvt(ins, 1, vd, vm)
#define arm32_vcvt_stod(ins, vd, vm) arm32_vcvt(ins, 0, vd, vm)


#define arm32_vcvt_dtos_vsreg(ins, vd, vm)  arm32_emit_al(ins, \
	  B(16, 0xeb7) \
	| B(12, ((vd) >> 1) & 0xf) \
	| B(22, ((vd) & 0x1)) \
	| B(4, 0xbc) \
	| B(0, vm))

#define arm32_vcvt_sint_double(ins, vd, vm) arm32_emit_al(ins, \
	  B(16, 0xeb8) \
	| B(12, vd) \
	| B(8,  0xb) \
	| B(4,  0xc) \
	| B(0,  vm))

#define arm32_vcvt_double_sint(ins, vd, vm, round) arm32_emit_al(ins, \
	  B(16, 0xebd) \
	| B(12, vd) \
	| B(8,  0xb) \
	| B(7, (!round) & 0x1) \
	| B(4,  0x4) \
	| B(0,  vm))


#define arm32_vfp_data_transfer(ins, cond, load, flt, vd, rn, imm) \
	do { \
		int __val = (imm) / 4; \
		int __absval = (__val < 0 ? -__val : __val); \
		arm32_emit_al(ins,\
			  B(28, cond) \
			| B(24, 0xd) \
			| B(23, (__val >= 0) & 0x1) /* UP => rn + rm */ \
			| B(22, 0) /* double */ \
			| B(20, (load) & 0x1) \
			| B(16, (rn) & 0xf) \
			| B(12, (vd) & 0xf) \
			| B(9,  0x5) \
			| B(8,  ((flt) ? 0x0: 0x1)) \
			| B(0,  __absval & 0xff)); \
		if (load && flt) arm32_vcvt_stod(ins, vd, vd); \
	} while (0)

#define arm32_vfp_float_data_transfer(ins, cond, load, flt, vd, rn, imm) \
	do { \
		int __val = (imm) / 4; \
		int __absval = (__val < 0 ? -__val : __val); \
		arm32_emit_al(ins,\
			  B(28, cond) \
			| B(24, 0xd) \
			| B(23, __val >= 0) /* UP => rn + rm */ \
			| B(22, (vd) & 0x1) /* double */ \
			| B(20, load) \
			| B(16, rn) \
			| B(12, (vd) >> 1) \
			| B(9,  0x5) \
			| B(8,  ((flt) ? 0x0: 0x1)) \
			| B(0,  __absval & 0xff)); \
		if (load && flt) arm32_vcvt_stod(ins, vd, vd); \
	} while (0)


#define arm32_vldr_size(ins, vd, rn, imm, size) \
	arm32_vfp_data_transfer(ins, ARMCOND_AL, 1, (size) == sizeof(float), vd, rn, imm)

#define arm32_vstr_size(ins, vd, rn, imm, size) \
	arm32_vfp_data_transfer(ins, ARMCOND_AL, 0, (size) == sizeof(float), vd, rn, imm)

#define arm32_vldr_fp_imm(ins, dest, _imm, _size) \
	do { \
		int __disp = _imm; \
		if ((__disp >= -4095) && (__disp <= 4095))  { \
			arm32_vldr_size(ins, dest, ARMREG_FP, __disp, _size); \
		} else { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, __disp); \
			arm32_vldr_size(ins, dest, ARMREG_FP, ARMREG_R12, _size); \
		} \
	} while (0)

#define arm32_vstr_fp_imm(ins, dest, _imm, _size) \
	do { \
		int __disp = _imm; \
		if ((__disp >= -4095) && (__disp <= 4095))  { \
			arm32_vstr_size(ins, dest, ARMREG_FP, __disp, _size); \
		} else { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, __disp); \
			arm32_vstr_size(ins, dest, ARMREG_FP, ARMREG_R12, _size); \
		} \
	} while (0)


#define arm32_vstr_float_fp_imm(ins, dest, _imm) \
	do { \
		int __disp = _imm; \
		if ((__disp >= -4095) && (__disp <= 4095))  { \
			arm32_vfp_float_data_transfer(ins, ARMCOND_AL, 0, 1, dest, ARMREG_FP, __disp); \
		} else { \
			arm32_mov_reg_imm32(ins, ARMREG_R12, __disp); \
			arm32_vfp_data_transfer(ins, ARMCOND_AL, 0, 1, dest, ARMREG_FP, ARMREG_R12); \
		} \
	} while (0)


#define arm32_vmov_vreg_vreg_double(ins, vd, vn) arm32_emit_al(ins, \
	  B(16, 0xeb0) \
	| B(12, vd) \
	| B(4,  0xb4) \
	| B(0,  vn))

#define arm32_vmov_vreg_reg_float(ins, rd, vn) arm32_emit_al(ins, \
	  B(20, 0xe1) \
	| B(16, vn) \
	| B(12, rd) \
	| B(4,  0xa1))

#define arm32_vmov_reg_vreg_float(ins, vd, rn) arm32_emit_al(ins, \
	  B(20, 0xe0) \
	| B(16, vd) \
	| B(12, rn) \
	| B(0,  0xb10))

#define arm32_vmov_vsreg_reg_float(ins, vn, rd) arm32_emit_al(ins, \
	  B(20, 0xe0) \
	| B(16, (vn & 0x1f) >> 1) \
	| B(12, rd) \
	| B(8, 0xa) \
	| B(7, (vn & 0x1)) \
	| B(0,  0x10))

#define arm32_vmov_reg_vsreg_float(ins, rt, vn) arm32_emit_al(ins, \
	  B(20, 0xe1) \
	| B(16, (vn & 0x1f) >> 1) \
	| B(12, rt) \
	| B(8, 0xa) \
	| B(7, (vn & 0x1)) \
	| B(0,  0x10))

#define arm32_vop_double(ins, op1, op2, vd, vn, vm) arm32_emit_al(ins, \
	  B(24, 0xe) \
	| B(20, op1) \
	| B(16, vn) \
	| B(12, vd) \
	| B(8,  0xb) \
	| B(4,  op2) \
	| B(0, vm))

#define arm32_vadd_double(ins, vd, vn, vm) arm32_vop_double(ins, 0x3, 0x0, vd, vn, vm)
#define arm32_vsub_double(ins, vd, vn, vm) arm32_vop_double(ins, 0x3, 0x4, vd, vn, vm)
#define arm32_vmul_double(ins, vd, vn, vm) arm32_vop_double(ins, 0x2, 0x0, vd, vn, vm)
#define arm32_vdiv_double(ins, vd, vn, vm) arm32_vop_double(ins, 0x8, 0x0, vd, vn, vm)

#define arm32_vneg_double(ins, vd, vm) arm32_emit_al(ins, \
	  B(16, 0xeb1) \
	| B(12, vd) \
	| B(4, 0xb4) \
	| B(0, vm))

#define arm32_vabs(ins, vd, vm) arm32_emit_al(ins, \
	  B(16, 0xeb0) \
	| B(12, vd) \
	| B(4,  0xbc) \
	| B(0,  vm))

#define arm32_vfp_cmp_double(ins, vd, vm, zero) arm32_emit_al(ins, \
	  B(20, 0xeb) \
	| B(16, (zero) ? 0x5 : 0x4) \
	| B(12, vd) \
	| B(4, 0xbc) \
	| B(0, vm))

#define arm32_vcmp_double(ins, vd, vm) arm32_vfp_cmp_double(ins, vd, vm, 0)
#define arm32_vcmp0_double(ins, vd) arm32_vfp_cmp_double(ins, vd, 0, 1)

#define arm32_vpush(ins, vd) arm32_emit_al(ins, \
	  B(16, 0xd2d) \
	| B(12, vd) \
	| B(8,  0xb) \
	| B(0, 2))

#define arm32_vpop(ins, vd) arm32_emit_al(ins, \
	  B(16, 0xcbd) \
	| B(12, vd) \
	| B(8,  0xb) \
	| B(0, 2))

#define arm32_vfp_fpscr(ins, read, r) arm32_emit_al(ins, \
	  B(21, 0x77) \
	| B(20, read) \
	| B(16, 0x1) \
	| B(12, r) \
	| B(0,  0xa10))

#define arm32_vmsr(ins, rn) arm32_vfp_fpscr(ins, 0, rn)
#define arm32_vmrs(ins, rd) arm32_vfp_fpscr(ins, 1, rd)
#define arm32_vmrs_flags(ins) arm32_vfp_fpscr(ins, 1, 0xf)
