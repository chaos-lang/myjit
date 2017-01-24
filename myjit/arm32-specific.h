/*
 * MyJIT 
 * Copyright (C) 2016, 2017 Petr Krajca, <petr.krajca@upol.cz>
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

#include "arm32-codegen.h"
#include "x86-common-stuff.c"

/* Stack frame organization:
 *
 *          +-----------------+
 *          | remaining args. |
 *          +-----------------+
 *          | saved regs.     |
 * FP       +-----------------+
 *          | shadow space    |
 *          | for arg. regs   |
 * FP -  n  +-----------------+ n = 4 * 4
 *          | GP registers    |
 * FP  - m  +-----------------+
 *          | FP registers    |
 * FP  - k  +-----------------+
 *          | allocai mem     |
 * SP       +-----------------+
 */


#define SHADOW_REG_SPACE (16)
#define JIT_GET_ADDR(jit, imm) (!jit_is_label(jit, (void *)(imm)) ? (imm) :  \
		((((long)jit->buf + (long)((jit_label *)(imm))->pos - (long)jit->ip)) / 4))

//#define GET_GPREG_POS(jit, r) (- ((JIT_REG_ID(r) + 1) * REG_SIZE) - SHADOW_REG_SPACE)
//#define GET_GPREG_POS(jit, r) (- ((JIT_REG_ID(r) + 1) * REG_SIZE) - SHADOW_REG_SPACE)
/*
#define GET_FPREG_POS(jit, r) (- jit_current_func_info(jit)->gp_reg_count * REG_SIZE - (JIT_REG_ID(r) + 1) * sizeof(jit_float) - jit_current_func_info(jit)->allocai_mem)

*/
//#define GET_ARG_SPILL_POS(jit, info, arg) ((- (arg + info->gp_reg_count + info->fp_reg_count) * REG_SIZE) - jit_current_func_info(jit)->allocai_mem)
#define GET_ARG_SPILL_POS(jit, info, arg) ((- (arg + info->gp_reg_count) * REG_SIZE) - jit_current_func_info(jit)->allocai_mem)

//#define EXTRA_SPACE (0)

inline jit_hw_reg * rmap_get(jit_rmap * rmap, jit_value reg);

static inline int GET_REG_POS(struct jit * jit, int r)
{
	//struct jit_func_info * info = jit_current_func_info(jit);
	if (JIT_REG_SPEC(r) == JIT_RTYPE_REG) {
		if (JIT_REG_TYPE(r) == JIT_RTYPE_INT) {
			return - (/*info->allocai_mem + */ SHADOW_REG_SPACE + (JIT_REG_ID(r) - 1) * REG_SIZE /*+ REG_SIZE*/);
		} else {
			assert(0);
//			return - (info->allocai_mem + EXTRA_SPACE + info->gp_reg_count * REG_SIZE + JIT_REG_ID(r) * sizeof(double) + sizeof(double));
		}
	}
	if (JIT_REG_SPEC(r) == JIT_RTYPE_ARG) {
		int arg_id = JIT_REG_ID(r);
		struct jit_inp_arg * a = &(jit_current_func_info(jit)->args[arg_id]);
		return a->spill_pos;
	}
	assert(0);
}

int jit_allocai(struct jit * jit, int size)
{
	int real_size = jit_value_align(size, 8);
	jit_add_op(jit, JIT_ALLOCA | IMM, SPEC(IMM, NO, NO), (long)real_size, 0, 0, 0, NULL);

	int stack_offset = jit_current_func_info(jit)->allocai_mem;
	jit_current_func_info(jit)->allocai_mem += real_size;	
	return stack_offset;
}

void jit_init_arg_params(struct jit *jit, struct jit_func_info *info, int p, int *phys_reg)
{
	struct jit_inp_arg * a = &(info->args[p]);
	if (a->type != JIT_FLOAT_NUM) { // normal argument
		int pos = a->gp_pos;
		if (pos < jit->reg_al->gp_arg_reg_cnt) {
			a->passed_by_reg = 1;
			a->location.reg = jit->reg_al->gp_arg_regs[pos]->id;
			a->spill_pos = GET_ARG_SPILL_POS(jit, info, p);
		} else {
//			int stack_pos = (pos - jit->reg_al->gp_arg_reg_cnt) + MAX(0, (a->fp_pos - jit->reg_al->fp_arg_reg_cnt));
			a->location.stack_pos = (10 + (pos - jit->reg_al->gp_arg_reg_cnt)) * 4;
			a->spill_pos = a->location.stack_pos;
			a->passed_by_reg = 0;
		}
		a->overflow = 0;
		return;
	}

	// FP argument
	abort();
/*
	int pos = a->fp_pos;
	if (pos < jit->reg_al->fp_arg_reg_cnt) {
		a->passed_by_reg = 1;
		a->location.reg = jit->reg_al->fp_arg_regs[pos]->id;
		a->spill_pos = GET_ARG_SPILL_POS(jit, info, p);
	} else {

		int stack_pos = (pos - jit->reg_al->fp_arg_reg_cnt) + MAX(0, (a->gp_pos - jit->reg_al->gp_arg_reg_cnt));

		a->location.stack_pos = 16 + stack_pos * 8;
		a->spill_pos = 16 + stack_pos * 8;
		a->passed_by_reg = 0;
	}
	a->overflow = 0;
*/
}

static inline void emit_cond_op(struct jit *jit, struct jit_op *op, int cond, int imm)
{
	if (imm) {
		arm32_cmp_reg_imm(jit->ip, op->r_arg[1], op->r_arg[2]);
	} else {
		arm32_cmp_reg_reg(jit->ip, op->r_arg[1], op->r_arg[2]);
	}

	arm32_mov_reg_imm32(jit->ip, op->r_arg[0], 0);
	arm32_cond_mov_reg_imm32(jit->ip, cond, op->r_arg[0], 1);
}

static inline void emit_branch_op(struct jit * jit, struct jit_op * op, int cond, int imm)
{
	if (imm) {
		arm32_cmp_reg_imm(jit->ip, op->r_arg[1], op->r_arg[2]);
	} else {
		arm32_cmp_reg_reg(jit->ip, op->r_arg[1], op->r_arg[2]);
	}

	op->patch_addr = JIT_BUFFER_OFFSET(jit);
	arm32_branch (jit->ip, cond, JIT_GET_ADDR(jit, op->r_arg[0]));
}


static inline void emit_branch_mask_op(struct jit * jit, struct jit_op * op, int cond, int imm)
{
	if (imm) {
		arm32_tst_reg_imm(jit->ip, op->r_arg[1], op->r_arg[2]);
	} else {
		arm32_tst_reg_reg(jit->ip, op->r_arg[1], op->r_arg[2]);
	}

	op->patch_addr = JIT_BUFFER_OFFSET(jit);
	arm32_branch (jit->ip, cond, JIT_GET_ADDR(jit, op->r_arg[0]));
}

static inline void emit_op_and_overflow_branch(struct jit * jit, struct jit_op * op, int alu_op, int imm, int negation)
{
	long a1 = op->r_arg[0];
	long a2 = op->r_arg[1];
	long a3 = op->r_arg[2];
	if (imm) {
		switch (alu_op) {
			case JIT_ADD: arm32_alucc_reg_imm(jit->ip, ARMOP_ADD, 1, a2, a2, a3); break;
			case JIT_SUB: arm32_alucc_reg_imm(jit->ip, ARMOP_SUB, 1, a2, a2, a3); break;
			default: assert(0);
		}
	} else {
		switch (alu_op) {
			case JIT_ADD: arm32_alucc_reg_reg(jit->ip, ARMOP_ADD, 1, a2, a2, a3); break;
			case JIT_SUB: arm32_alucc_reg_reg(jit->ip, ARMOP_SUB, 1, a2, a2, a3); break;
			default: assert(0);
		}
	}
	op->patch_addr = JIT_BUFFER_OFFSET(jit);
	if (!negation) arm32_branch (jit->ip, ARMCOND_VS, JIT_GET_ADDR(jit, a1));
	else arm32_branch (jit->ip, ARMCOND_VC, JIT_GET_ADDR(jit, a1));
}

static inline void emit_fpbranch_op(struct jit * jit, struct jit_op * op, int cond, int arg1, int arg2)
{
	if (IS_IMM(op)) {
		arm32_vcmp0_double(jit->ip, op->r_arg[1]);
	} else {
		arm32_vcmp_double(jit->ip, op->r_arg[1], op->r_arg[2]);
	}
	arm32_vmrs(jit->ip);

	op->patch_addr = JIT_BUFFER_OFFSET(jit);
	arm32_branch (jit->ip, cond, JIT_GET_ADDR(jit, op->r_arg[0]));
}

/* determines whether the argument value was spilled out or not,
 * if the register is associated with the hardware register
 * it is returned through the reg argument
 */
// FIXME: more general, should use information from the reg. allocator
static inline int is_spilled(int arg_id, jit_op * prepare_op, int * reg)
{
	jit_hw_reg * hreg = rmap_get(prepare_op->regmap, arg_id);
	if (hreg) {
		*reg = hreg->id;
		return 0;
	}
	return 1;
}

/**
 * Assigns integer value to register which is used to pass the argument
 */
static inline void emit_set_arg(struct jit * jit, struct jit_out_arg * arg)
{
        int sreg;
        int reg = jit->reg_al->gp_arg_regs[arg->argpos]->id;
        jit_value value = arg->value.generic;
        if (arg->isreg) {
                if (is_spilled(value, jit->prepared_args.op, &sreg)) {
                        arm32_ld_fp_imm(jit->ip, reg, GET_REG_POS(jit, value));
                } else {
                        if (reg != sreg) arm32_mov_reg_reg(jit->ip, reg, sreg);
                }
        } else arm32_mov_reg_imm32(jit->ip, reg, value);
}


/**
 * Pushes integer value on the stack
 */
static inline void emit_push_arg(struct jit * jit, struct jit_out_arg * arg)
{
        int sreg;
        if (arg->isreg) {
                if (is_spilled(arg->value.generic, jit->prepared_args.op, &sreg)) {
			arm32_ld_fp_imm(jit->ip, ARMREG_R12, GET_REG_POS(jit, arg->value.generic));
                        arm32_push_reg(jit->ip, ARMREG_R12);
		} else {
			arm32_push_reg(jit->ip, sreg);
		}
        } else {
		arm32_mov_reg_imm32(jit->ip, ARMREG_R12, arg->value.generic);
		arm32_push_reg(jit->ip, ARMREG_R12);
        }
}


static inline int emit_arguments(struct jit * jit)
{
	struct jit_out_arg * args = jit->prepared_args.args;

	int gp_pushed = MAX(jit->prepared_args.gp_args - jit->reg_al->gp_arg_reg_cnt, 0);

        for (int x = jit->prepared_args.count - 1; x >= 0; x --) {
                struct jit_out_arg * arg = &(args[x]);
                if (!arg->isfp) {
                        if (arg->argpos < jit->reg_al->gp_arg_reg_cnt) emit_set_arg(jit, arg);
                        else emit_push_arg(jit, arg);
                } else {
  //                      if (arg->argpos < jit->reg_al->fp_arg_reg_cnt) emit_set_fparg(jit, arg);
//                        else emit_fppush_arg(jit, arg);
                }
        }
	return 0;
}

static inline void emit_funcall(struct jit * jit, struct jit_op * op, int imm)
{
	// XXX: caller saved registers?
	int stack_correction = emit_arguments(jit);
	if (!imm) {
		jit_hw_reg *hreg = rmap_get(op->regmap, op->arg[0]);
                if (hreg) arm32_blx_reg(jit->ip, hreg->id);
                else {
			arm32_ld_fp_imm(jit->ip, ARMREG_R12, GET_REG_POS(jit, op->arg[0]));
			arm32_blx_reg(jit->ip, ARMREG_R12);
		}
	} else {
		if (op->r_arg[0] == (long)JIT_FORWARD) {
			arm32_bl_imm(jit->ip, 0); 
		} else if (jit_is_label(jit, (void *)op->r_arg[0])) {
			arm32_bl_imm(jit->ip, (((long)jit->buf - (long)jit->ip) + (long)((jit_label *)(op->r_arg[0]))->pos - 8) / 4); 
		} else {
			arm32_mov_reg_imm32(jit->ip, ARMREG_R12, (long) op->r_arg[0]);
			arm32_blx_reg(jit->ip, ARMREG_R12);
		}
	}
	stack_correction += jit->prepared_args.stack_size;
	if (stack_correction)
		arm32_add_sp_imm(jit->ip, stack_correction);
}

static void emit_get_arg_int(struct jit * jit, struct jit_inp_arg * arg, int dest_reg, int associated)
{
	int read_from_stack = 0;
	int stack_pos;

	if (!arg->passed_by_reg) {
		read_from_stack = 1;
		stack_pos = arg->location.stack_pos;
	}

	if (arg->passed_by_reg && !associated) {
		// the register is not associated and the value has to be read from the memory
		read_from_stack = 1;
		stack_pos = arg->spill_pos;
	}

	if (read_from_stack) arm32_ld_fp_imm(jit->ip, dest_reg, stack_pos);
	else arm32_mov_reg_reg(jit->ip, dest_reg, arg->location.reg);
}
/*
static void emit_get_arg_float(struct jit * jit, struct jit_inp_arg * arg, int dest_reg, int associated)
{
	if (associated) {
		sparc_st_imm(jit->ip, arg->location.reg, sparc_fp, -8);
		sparc_ldf_imm(jit->ip, sparc_fp, -8, sparc_f30);
		sparc_fstod(jit->ip, sparc_f30, dest_reg);
	} else {
		sparc_ldf_imm(jit->ip, sparc_fp, arg->location.stack_pos, sparc_f30);
		sparc_fstod(jit->ip, sparc_f30, dest_reg);
	}
}

static void emit_get_arg_double(struct jit * jit, jit_op * op, struct jit_inp_arg * arg, int dest_reg, int associated)
{
	int arg_id = op->r_arg[1];

	// moves the first part of the register
	if (associated) {
		sparc_st_imm(jit->ip, arg->location.reg, sparc_fp, -8);
		sparc_ldf_imm(jit->ip, sparc_fp, -8, dest_reg);
	} else {
		sparc_ldf_imm(jit->ip, sparc_fp, arg->location.stack_pos, dest_reg);
	}

	// moves the second part
	int reg_id = jit_mkreg_ex(arg->type == JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, arg_id);
	associated = (rmap_get(op->regmap, reg_id) == NULL);

	struct jit_inp_arg arg2;
	init_arg(&arg2, arg->phys_reg + 1);

	if (associated && arg2.passed_by_reg) {
		sparc_st_imm(jit->ip, arg2.location.reg, sparc_fp, -4);
		sparc_ldf_imm(jit->ip, sparc_fp, -4, dest_reg + 1);
	} else {
		sparc_ldf_imm(jit->ip, sparc_fp, arg2.spill_pos, dest_reg + 1);
	}
}
*/
static void emit_get_arg(struct jit * jit, jit_op * op)
{
	int dreg = op->r_arg[0];
	int arg_id = op->r_arg[1];

	struct jit_inp_arg * arg = &(jit_current_func_info(jit)->args[arg_id]);
	int reg_id = jit_mkreg(arg->type == JIT_FLOAT_NUM ? JIT_RTYPE_FLOAT : JIT_RTYPE_INT, JIT_RTYPE_ARG, arg_id);

	int associated = (rmap_get(op->regmap, reg_id) != NULL);
	
	if (arg->type != JIT_FLOAT_NUM) emit_get_arg_int(jit, arg, dreg, associated);
/*
	FIXME: FP args
	else {
		if (arg->size == sizeof(float)) emit_get_arg_float(jit, arg, dreg, associated);
		if (arg->size == sizeof(double)) emit_get_arg_double(jit, op, arg, dreg, associated);

	}
*/
}
void jit_patch_external_calls(struct jit * jit)
{
/*
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		if ((op->code == (JIT_CALL | IMM)) && (!jit_is_label(jit, (void *)op->arg[0])))
			sparc_patch(jit->buf + (long)op->patch_addr, (long)op->r_arg[0]);
	}
*/
}

void jit_patch_local_addrs(struct jit *jit)
{
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
	
		if ((GET_OP(op) == JIT_REF_CODE) || (GET_OP(op) == JIT_REF_DATA)) {
			unsigned char *buf = jit->buf + (long) op->patch_addr;
			jit_value addr = jit_is_label(jit, (void *)op->arg[1]) ? ((jit_label *)op->arg[1])->pos : op->arg[1];
			arm32_mov_reg_imm32x(buf, op->r_arg[0], jit->buf + addr);
		}

		if ((GET_OP(op) == JIT_DATA_REF_CODE) || (GET_OP(op) == JIT_DATA_REF_DATA)) {
			unsigned char *buf = jit->buf + (long) op->patch_addr;
			jit_value addr = jit_is_label(jit, (void *)op->arg[0]) ? ((jit_label *)op->arg[0])->pos : op->arg[0];
			*((jit_value *)buf) = (jit_value) (jit->buf + addr);
		}
	}
}

void emit_mul(struct jit * jit, jit_op * op)
{
	long a1 = op->r_arg[0];
	long a2 = op->r_arg[1];
	long a3 = op->r_arg[2];
	if (IS_IMM(op)) {
		switch (a3) {
			case 0: arm32_mov_reg_imm32(jit->ip, a1, 0); return;
			case 1: if (a1 != a2) arm32_mov_reg_reg(jit->ip, a1, a2); return; 
			case 2:  arm32_rsh_imm(jit->ip, a1, a2, 1); return;
			case 4:  arm32_rsh_imm(jit->ip, a1, a2, 2); return;
			case 8:  arm32_rsh_imm(jit->ip, a1, a2, 3); return;
			case 16: arm32_rsh_imm(jit->ip, a1, a2, 4); return;
			case 32: arm32_rsh_imm(jit->ip, a1, a2, 5); return;
			default: abort(); // other values on permitted
		}
	} else {
		arm32_mul(jit->ip, a1, a2, a3);
	}
}

static void emit_trace_op(struct jit *jit, jit_op *op)
{
        arm32_pushall(jit->ip);

        int trace = 0;
        jit_opcode prev_code = GET_OP(op->prev);
        jit_opcode next_code = GET_OP(op->next);
        if ((prev_code == JIT_PROLOG) || (prev_code == JIT_LABEL) || (prev_code == JIT_PATCH)) trace |= TRACE_PREV;
        if ((next_code != JIT_PROLOG) && (next_code != JIT_LABEL) && (next_code != JIT_PATCH)) trace |= TRACE_NEXT;

        arm32_mov_reg_imm32(jit->ip, ARMREG_R0, jit);
        arm32_mov_reg_imm32(jit->ip, ARMREG_R1, op);
        arm32_mov_reg_imm32(jit->ip, ARMREG_R2, op->r_arg[0]);
        arm32_mov_reg_imm32(jit->ip, ARMREG_R3, trace);
        arm32_mov_reg_imm32(jit->ip, ARMREG_R4, JIT_PROC_VALUE(jit_trace_callback));
        arm32_blx_reg(jit->ip, ARMREG_R4);

        arm32_popall(jit->ip);
}
/*
// common function for floor & ceil ops
static void emit_sparc_round(struct jit * jit, long a1, long a2)
{
	static double zero_point_5 = 0.5;
	sparc_set32(jit->ip, 0, sparc_g1);

	// tests the sign of the value
	sparc_fabss(jit->ip, a2, sparc_f30);
	sparc_fmovs(jit->ip, a2 + 1, sparc_f31);
	sparc_fcmpd(jit->ip, a2, sparc_f30);

	sparc_set32(jit->ip, (long)&zero_point_5, sparc_g1);
	sparc_lddf(jit->ip, sparc_g1, sparc_g0, sparc_f30);

	// if a2 is greater than 0 -> skip negation
	unsigned char * br1 = jit->ip;
	sparc_fbranch(jit->ip, FALSE, sparc_fbge, 0); 
	sparc_nop(jit->ip);
	sparc_fnegs(jit->ip, sparc_f30, sparc_f30);

	sparc_patch(br1, jit->ip);

	// conversion 
	sparc_faddd(jit->ip, a2, sparc_f30, sparc_f30);
	sparc_fdtoi(jit->ip, sparc_f30, sparc_f30);
	sparc_stdf_imm(jit->ip, sparc_f30, sparc_fp, -8);
	sparc_ld_imm(jit->ip, sparc_fp, -8, a1);
}

// common function for floor & ceil ops
static void emit_sparc_floor(struct jit * jit, long a1, long a2, int floor)
{
	sparc_set32(jit->ip, 0, sparc_g1);

	// tests the sign of the value
	sparc_fabss(jit->ip, a2, sparc_f30);
	sparc_fmovs(jit->ip, a2 + 1, sparc_f31);
	sparc_fcmpd(jit->ip, a2, sparc_f30);

	unsigned char * br1 = jit->ip;
	sparc_fbranch(jit->ip, FALSE, (floor ? sparc_fbe : sparc_fbne), 0);
	sparc_nop(jit->ip);

	// branch1: conversion with correction
	sparc_fdtoi(jit->ip, a2, sparc_f30);
	sparc_fitod(jit->ip, sparc_f30, sparc_f30);

	sparc_fcmpd(jit->ip, a2, sparc_f30);

	unsigned char * br2 = jit->ip;
	sparc_fbranch(jit->ip, FALSE, sparc_fbe, 0); // skip correction if not desired
	sparc_nop(jit->ip);

	sparc_set32(jit->ip, (floor ? -1 : 1), sparc_g1);

	sparc_patch(br1, jit->ip);
	sparc_patch(br2, jit->ip);

	// branch2: direct conversion 
	sparc_fdtoi(jit->ip, a2, sparc_f30);
	sparc_stdf_imm(jit->ip, sparc_f30, sparc_fp, -8);
	sparc_ld_imm(jit->ip, sparc_fp, -8, a1);

	// adds correction
	sparc_add(jit->ip, FALSE, a1, sparc_g1, a1);
}

*/
struct transfer_info {
	int sourcereg;
	int destreg;
	int scrapreg;
	int scrap_in_use;
	int counterreg;
	int counter_in_use;
	int block_size;
	unsigned char *loop_addr;
};

static void emit_transfer_init(struct jit * jit, jit_op * op, jit_value destreg, jit_value srcreg, jit_value cnt, int block_size)
{
	struct transfer_info *tinf = JIT_MALLOC(sizeof(struct transfer_info));	
	tinf->sourcereg = srcreg;
	tinf->destreg = destreg;
	tinf->block_size = block_size;


	// XXX: duplicitni kod s common86-specific
	// FIXME: pokud je jako scrapreg nebo destreg vybran callee-saved register
	// je to potreba osetrit v prologu
	jit_hw_reg *scrap = jit_get_unused_reg_with_index(jit->reg_al, op, 0, 0);
	if (scrap) tinf->scrapreg = scrap->id;
	else {
		for (int i = 0; i < jit->reg_al->gp_reg_cnt; i++) {
			jit_hw_reg *r = &jit->reg_al->gp_regs[i];
			if ((r->id != srcreg) && (r->id != destreg) && (!IS_IMM(op) && (r->id != cnt))) {
				tinf->scrapreg = r->id;
				break;
			}
		}
	}
	tinf->scrap_in_use = jit_reg_in_use(op, tinf->scrapreg, 0);

	if (IS_IMM(op)) {
		jit_hw_reg * counter = jit_get_unused_reg_with_index(jit->reg_al, op, 0, 1);
		if (counter) tinf->counterreg = counter->id;
		else {
			for (int i = 0; i < jit->reg_al->gp_reg_cnt; i++) {
				jit_hw_reg *r = &jit->reg_al->gp_regs[i];
				if ((r->id != srcreg) && (r->id != destreg) && (r->id != tinf->scrapreg)) {
					tinf->counterreg = r->id;
					break;
				}
			}
		}
		tinf->counter_in_use = jit_reg_in_use(op, tinf->counterreg, 0);
	} else {
		if (jit_set_get(op->live_out, op->arg[2])) {
			jit_hw_reg *counter = jit_get_unused_reg_with_index(jit->reg_al, op, 0, 1);
			tinf->counterreg = (counter ? counter->id : cnt);
			tinf->counter_in_use = jit_reg_in_use(op, tinf->counterreg, 0);
		} else {
			tinf->counterreg = cnt;
			tinf->counter_in_use = 0;
		}
	}
	/// konci podpobny kod

	if (tinf->counter_in_use) arm32_push_reg(jit->ip, tinf->counterreg);
	if (tinf->scrap_in_use) arm32_push_reg(jit->ip, tinf->scrapreg);

	if (IS_IMM(op)) arm32_mov_reg_imm32(jit->ip, tinf->counterreg, cnt * block_size);
	else if ((tinf->counterreg != cnt) || block_size > 1) {
		switch (block_size) {
			case 1: arm32_mov_reg_reg(jit->ip, tinf->counterreg, cnt); break;
			case 2: arm32_lsh_imm(jit->ip, tinf->counterreg, tinf->counterreg, 1); break; 
			case 4: arm32_lsh_imm(jit->ip, tinf->counterreg, tinf->counterreg, 2); break;
			default: abort();
		}
	}

	tinf->loop_addr = jit->ip;
	op->addendum = tinf;

	arm32_alucc_reg_imm(jit->ip, ARMOP_SUB, 1, tinf->counterreg, tinf->counterreg, block_size);
	switch (block_size) {
		case 1: arm32_ldsb_reg(jit->ip, tinf->scrapreg, srcreg, tinf->counterreg); break;
		case 2: arm32_ldsh_reg(jit->ip, tinf->scrapreg, srcreg, tinf->counterreg); break;
		case 4: arm32_ld_reg(jit->ip, tinf->scrapreg, srcreg, tinf->counterreg); break;
		default: abort();
	}
}

static void emit_transfer_loop(struct jit *jit, jit_op *op)
{
	struct transfer_info *tinf = (struct transfer_info *)op->addendum;
	jit_value loop = (jit_value) tinf->loop_addr;

	switch (tinf->block_size) {
		case 1: arm32_stb_reg(jit->ip, tinf->scrapreg, tinf->destreg, tinf->counterreg); break;
		case 2: arm32_sth_reg(jit->ip, tinf->scrapreg, tinf->destreg, tinf->counterreg); break;
		case 4: arm32_st_reg(jit->ip, tinf->scrapreg, tinf->destreg, tinf->counterreg); break;
		default: abort();
	}

	arm32_branch(jit->ip, ARMCOND_NE, (loop - (jit_value) jit->ip) / 4);
	if (tinf->scrap_in_use) arm32_pop_reg(jit->ip, tinf->scrapreg);
	if (tinf->counter_in_use) arm32_pop_reg(jit->ip, tinf->counterreg);
}

static void emit_transfer_op(struct jit *jit, jit_op *op, int alu_op)
{
	jit_op *init_op = op->prev;
	while (GET_OP(init_op) != JIT_TRANSFER)
		init_op = init_op->prev;

	struct transfer_info *tinf = (struct transfer_info *)init_op->addendum;
	int valreg = ARMREG_R12;

	if (op->arg[1] == R_OUT) {
		switch (tinf->block_size) {
			case 1: arm32_ldsb_reg(jit->ip, ARMREG_R12, tinf->destreg, tinf->counterreg); break;
			case 2: arm32_ldsh_reg(jit->ip, ARMREG_R12, tinf->destreg, tinf->counterreg); break;
			case 4: arm32_ld_reg(jit->ip, ARMREG_R12, tinf->destreg, tinf->counterreg); break;
			default: abort();
		}
	} else if (op->r_arg[1] != -1) {
		if (op->r_arg[1] == tinf->counterreg) {
			arm32_ld_imm(jit->ip, ARMREG_R12, ARMREG_SP, (tinf->scrap_in_use ? 4 : 0));
		} else if (op->r_arg[1] == tinf->scrapreg) {
			arm32_ld_imm(jit->ip, ARMREG_R12, ARMREG_SP, 0);
		} else valreg = op->r_arg[1];
	} else {
		arm32_ld_fp_imm(jit->ip, ARMREG_R12, GET_REG_POS(jit, op->arg[1]));
	}

	arm32_alu_reg_reg(jit->ip, alu_op, tinf->scrapreg, tinf->scrapreg, valreg);

	if (op->arg[0]) emit_transfer_loop(jit, (jit_op *)op->arg[0]);
}

static void emit_memcpy(struct jit *jit, jit_op *op, jit_value a1, jit_value a2, jit_value a3)
{
	emit_transfer_init(jit, op, a1, a2, a3, 1);
	emit_transfer_loop(jit, op);
}

static void emit_memset(struct jit *jit, jit_op *op, jit_value tgt_reg, jit_value count_reg, jit_value val, int block_size)
{
	int counter_reg = ARMREG_R12;
	int value_reg = val;
	int value_reg_in_use = 0;

	if (IS_IMM(op)) {
		value_reg = ARMREG_R9;
		value_reg_in_use = jit_reg_in_use(op, value_reg, 0);
		if (value_reg_in_use) arm32_push_reg(jit->ip, value_reg);
		arm32_mov_reg_imm32(jit->ip, value_reg, val);
	}
	switch (block_size) {
		case 1: arm32_mov_reg_reg(jit->ip, counter_reg, count_reg); break;
		case 2: arm32_lsh_imm(jit->ip, counter_reg, count_reg, 1); break; 
		case 4: arm32_lsh_imm(jit->ip, counter_reg, count_reg, 2); break;
		default: abort();
	}

	arm32_alucc_reg_imm(jit->ip, ARMOP_SUB, 1, counter_reg, counter_reg, block_size);
	switch (block_size) {
		case 1: arm32_cond_stb_reg(jit->ip, ARMCOND_CS, value_reg, tgt_reg, counter_reg); break;
		case 2: arm32_cond_sth_reg(jit->ip, ARMCOND_CS, value_reg, tgt_reg, counter_reg); break; // FIXME
		case 4: arm32_cond_st_reg(jit->ip, ARMCOND_CS, value_reg, tgt_reg, counter_reg); break;
		default: abort();
	}
	arm32_branch(jit->ip, ARMCOND_CS, -2);

	if (value_reg_in_use) arm32_pop_reg(jit->ip, value_reg);
}

static inline void emit_ureg(struct jit *jit, long vreg, long hreg_id)
{
	if (JIT_REG_SPEC(vreg) == JIT_RTYPE_ARG) {
		if (JIT_REG_TYPE(vreg) == JIT_RTYPE_INT) {
			arm32_st_fp_imm(jit->ip, hreg_id, GET_REG_POS(jit, vreg));
		} else {
			int arg_id = JIT_REG_ID(vreg);
			struct jit_inp_arg *a = &(jit_current_func_info(jit)->args[arg_id]);
			if (a->passed_by_reg) arm32_st_fp_imm(jit->ip, hreg_id, a->spill_pos);
		} 
	}
	if (JIT_REG_SPEC(vreg)== JIT_RTYPE_REG) {

		if (JIT_REG_TYPE(vreg)!= JIT_RTYPE_FLOAT)
			arm32_st_fp_imm(jit->ip, hreg_id, GET_REG_POS(jit, vreg));
		else arm32_st_fp_imm(jit->ip, hreg_id, GET_REG_POS(jit, vreg));
	}
}

#define emit_alu_op(cc, arm_op) \
	do {\
		if (IS_IMM(op)) { arm32_alucc_reg_imm(jit->ip, arm_op, cc, a1, a2, a3); }\
		else { arm32_alucc_reg_reg(jit->ip, arm_op, cc, a1, a2, a3); } \
	} while (0)

int frame_size(struct jit *jit, struct jit_func_info *info) {
	int stack_mem = 0;
	stack_mem += SHADOW_REG_SPACE;
	stack_mem += info->gp_reg_count * REG_SIZE;
	stack_mem += info->allocai_mem;
//	stack_mem += info->fp_reg_count * sizeof(double);
//	stack_mem += jit->reg_al->gp_arg_reg_cnt * REG_SIZE; 
//	stack_mem += info->float_arg_cnt * sizeof(double);
	return stack_mem;
}


void jit_gen_op(struct jit * jit, struct jit_op * op)
{
	long a1 = op->r_arg[0];
	long a2 = op->r_arg[1];
	long a3 = op->r_arg[2];
	int found = 1;
	switch (GET_OP(op)) {
		case JIT_ADD: emit_alu_op(0, ARMOP_ADD); break;
		case JIT_ADDC: emit_alu_op(1, ARMOP_ADD); break;
		// FIXME: should set carry flag
		case JIT_ADDX: emit_alu_op(0, ARMOP_ADC); break;
		case JIT_SUB: emit_alu_op(0, ARMOP_SUB); break;
		case JIT_SUBC:
			emit_alu_op(1, ARMOP_SUB);
			if (IS_IMM(op)) abort();
			// ARM has different semantics of the carry flag wrt. to borrowing
			// to make this consistent across multiple platforms, we need to set
			// the carry flag separately
			arm32_cmp_reg_reg(jit->ip, a3, a2);
			break;
		// FIXME: should set carry flag
		case JIT_SUBX: 
			emit_alu_op(0, ARMOP_SUB); 
			arm32_encode_dataop(jit->ip, ARMCOND_CS, 1, ARMOP_SUB, 0, a1, a1, 1);
			break;
		case JIT_RSB: emit_alu_op(0, ARMOP_RSB); break;
		case JIT_NEG: arm32_alu_reg_imm(jit->ip, ARMOP_RSB, a1, a2, 0); break;
		case JIT_NOT: arm32_alu_reg_reg(jit->ip, ARMOP_MVN,  a1, 0, a2); break;

		case JIT_OR:  emit_alu_op(0, ARMOP_ORR); break;
		case JIT_AND: emit_alu_op(0, ARMOP_AND); break;
		case JIT_XOR: emit_alu_op(0, ARMOP_EOR); break;
		case JIT_LSH: if (IS_IMM(op)) arm32_lsh_imm(jit->ip, a1, a2, a3);
			else arm32_lsh_reg(jit->ip, a1, a2, a3);
			break;
		case JIT_RSH:
			if (IS_SIGNED(op)) {
				if (IS_IMM(op)) arm32_rsa_imm(jit->ip, a1, a2, a3);
				else arm32_rsa_reg(jit->ip, a1, a2, a3);
			} else {
				if (IS_IMM(op)) arm32_rsh_imm(jit->ip, a1, a2, a3);
				else arm32_rsh_reg(jit->ip, a1, a2, a3);
			}
			break;

		case JIT_MUL:  emit_mul(jit, op); break;

		case JIT_HMUL: 
			if (IS_IMM(op)) abort();
			arm32_hmul(jit->ip, a1, a2, a3);
			break;
		case JIT_DIV: 
			if (IS_SIGNED(op)) {
				if (IS_IMM(op)) {
					switch (a3) {
						case 2:  arm32_rsa_imm(jit->ip, a1, a2, 1); goto op_complete;
						case 4:  arm32_rsa_imm(jit->ip, a1, a2, 2); goto op_complete;
						case 8:  arm32_rsa_imm(jit->ip, a1, a2, 3); goto op_complete;
						case 16: arm32_rsa_imm(jit->ip, a1, a2, 4); goto op_complete;
						case 32: arm32_rsa_imm(jit->ip, a1, a2, 5); goto op_complete;
					}
				} 
				if (IS_IMM(op)) abort();
				else arm32_sdiv(jit->ip, a1, a2, a3);
			} else { // UNSIGNED
				if (IS_IMM(op)) {
					switch (a3) {
						case 2:  arm32_rsh_imm(jit->ip, a1, a2, 1); goto op_complete;
						case 4:  arm32_rsh_imm(jit->ip, a1, a2, 2); goto op_complete;
						case 8:  arm32_rsh_imm(jit->ip, a1, a2, 3); goto op_complete;
						case 16: arm32_rsh_imm(jit->ip, a1, a2, 4); goto op_complete;
						case 32: arm32_rsh_imm(jit->ip, a1, a2, 5); goto op_complete;
					}
				} 

				if (IS_IMM(op)) abort();
				else arm32_udiv(jit->ip, a1, a2, a3);
			}
			break;
		case JIT_MOD: 
			if (IS_IMM(op)) {
				switch (a3) {
					case 2:  arm32_and_reg_imm(jit->ip, a1, a2, 0x01); goto op_complete;
					case 4:  arm32_and_reg_imm(jit->ip, a1, a2, 0x03); goto op_complete;
					case 8:  arm32_and_reg_imm(jit->ip, a1, a2, 0x07); goto op_complete;
					case 16: arm32_and_reg_imm(jit->ip, a1, a2, 0x0f); goto op_complete;
					case 32: arm32_and_reg_imm(jit->ip, a1, a2, 0x1f); goto op_complete;
				}
			}

			if (IS_IMM(op)) abort();

			arm32_sdiv(jit->ip, ARMREG_R12, a2, a3);
			arm32_mul(jit->ip, ARMREG_R12, a3, ARMREG_R12);
			arm32_sub_reg_reg(jit->ip, a1, a2, ARMREG_R12);

			break;

		case JIT_LT: emit_cond_op(jit, op, IS_SIGNED(op) ? ARMCOND_LT : ARMCOND_CC, IS_IMM(op)); break;
		case JIT_LE: emit_cond_op(jit, op, IS_SIGNED(op) ? ARMCOND_LE : ARMCOND_LS, IS_IMM(op)); break;
		case JIT_GT: emit_cond_op(jit, op, IS_SIGNED(op) ? ARMCOND_GT : ARMCOND_HI, IS_IMM(op)); break;
		case JIT_GE: emit_cond_op(jit, op, IS_SIGNED(op) ? ARMCOND_GE : ARMCOND_CS, IS_IMM(op)); break;
		case JIT_EQ: emit_cond_op(jit, op, ARMCOND_EQ, IS_IMM(op)); break;
		case JIT_NE: emit_cond_op(jit, op, ARMCOND_NE, IS_IMM(op)); break;

		case JIT_BLT: emit_branch_op(jit, op, IS_SIGNED(op) ? ARMCOND_LT : ARMCOND_CC, IS_IMM(op)); break;
		case JIT_BGT: emit_branch_op(jit, op, IS_SIGNED(op) ? ARMCOND_GT : ARMCOND_HI, IS_IMM(op)); break;
		case JIT_BLE: emit_branch_op(jit, op, IS_SIGNED(op) ? ARMCOND_LE : ARMCOND_LS, IS_IMM(op)); break;
		case JIT_BGE: emit_branch_op(jit, op, IS_SIGNED(op) ? ARMCOND_GE : ARMCOND_CS, IS_IMM(op)); break;
		case JIT_BEQ: emit_branch_op(jit, op, ARMCOND_EQ, IS_IMM(op)); break;
		case JIT_BNE: emit_branch_op(jit, op, ARMCOND_NE, IS_IMM(op)); break;
		case JIT_BMS: emit_branch_mask_op(jit, op, ARMCOND_NE, IS_IMM(op)); break;
		case JIT_BMC: emit_branch_mask_op(jit, op, ARMCOND_EQ, IS_IMM(op)); break;

		case JIT_BOADD: emit_op_and_overflow_branch(jit, op, JIT_ADD, IS_IMM(op), 0); break;
		case JIT_BOSUB: emit_op_and_overflow_branch(jit, op, JIT_SUB, IS_IMM(op), 0); break;
		case JIT_BNOADD: emit_op_and_overflow_branch(jit, op, JIT_ADD, IS_IMM(op), 1); break;
		case JIT_BNOSUB: emit_op_and_overflow_branch(jit, op, JIT_SUB, IS_IMM(op), 1); break;

		case JIT_FBLT: emit_fpbranch_op(jit, op, ARMCOND_LT, a2, a3); break;
		case JIT_FBGT: emit_fpbranch_op(jit, op, ARMCOND_GT, a2, a3); break;
		case JIT_FBLE: emit_fpbranch_op(jit, op, ARMCOND_LE, a2, a3); break;
		case JIT_FBGE: emit_fpbranch_op(jit, op, ARMCOND_GE, a2, a3); break;
		case JIT_FBEQ: emit_fpbranch_op(jit, op, ARMCOND_EQ, a2, a3); break;
		case JIT_FBNE: emit_fpbranch_op(jit, op, ARMCOND_NE, a2, a3); break;

		case JIT_CALL: emit_funcall(jit, op, IS_IMM(op)); break;

		case JIT_PATCH: do {
				struct jit_op *target = (struct jit_op *) a1;
				switch (GET_OP(target)) {
					case JIT_REF_CODE:
					case JIT_REF_DATA:
						target->arg[1] = JIT_BUFFER_OFFSET(jit);
						break;
					case JIT_DATA_REF_CODE:
					case JIT_DATA_REF_DATA:
						target->arg[0] = JIT_BUFFER_OFFSET(jit);
						break;
					default: {
						long pa = ((struct jit_op *)a1)->patch_addr;
						arm32_patch(jit->buf + pa, jit->ip);
					}
				}
			} while (0);
			break;

		case JIT_JMP:
			op->patch_addr = JIT_BUFFER_OFFSET(jit);
			if (op->code & REG) arm32_bx(jit->ip, ARMCOND_AL, a1);
			else arm32_branch(jit->ip, ARMCOND_AL, JIT_GET_ADDR(jit, op->r_arg[0]));
			break;

		case JIT_RET:
			do {
				struct jit_func_info *info = jit_current_func_info(jit);
				arm32_add_sp_imm(jit->ip, frame_size(jit, info));
				if (!IS_IMM(op) && (a1 != ARMREG_R0)) arm32_mov_reg_reg(jit->ip, ARMREG_R0, a1);
				if (IS_IMM(op)) arm32_mov_reg_imm32(jit->ip, ARMREG_R0, a1);
				arm32_popall_but_r0123(jit->ip);
				arm32_bx(jit->ip, ARMCOND_AL, ARMREG_LR);
			} while (0);
			break;
		case JIT_PUTARG: funcall_put_arg(jit, op); break;
/*
		case JIT_FPUTARG: funcall_fput_arg(jit, op); break;
*/
		case JIT_GETARG: emit_get_arg(jit, op); break;

		case JIT_MSG:
				arm32_pushall(jit->ip);
				if (!IS_IMM(op)) arm32_mov_reg_reg(jit->ip, ARMREG_R1, op->r_arg[1]);
				arm32_mov_reg_imm32(jit->ip, ARMREG_R0, op->r_arg[0]);
				arm32_mov_reg_imm32(jit->ip, ARMREG_R2, JIT_PROC_VALUE(printf));
				arm32_blx_reg(jit->ip, ARMREG_R2);
				arm32_popall(jit->ip);
				break;
		case JIT_TRACE: emit_trace_op(jit, op); break;

		case JIT_ALLOCA: break;
		case JIT_CODE_ALIGN: { 
				int count = op->arg[0]; 
				assert(!(count % 4));
				while ((unsigned long)jit->ip % count) {
					if ((unsigned long)jit->ip % 4) {
						*jit->ip = 0;
						jit->ip++;
					}
				        else arm32_nop(jit->ip);
				}	
			}
			break;


		case JIT_REF_CODE:
		case JIT_REF_DATA:
			op->patch_addr = JIT_BUFFER_OFFSET(jit);
			arm32_mov_reg_imm32(jit->ip, a1, 0xdeadbeef);
			break;
		case JIT_MEMCPY: emit_memcpy(jit, op, a1, a2, a3); break;
		case JIT_MEMSET: emit_memset(jit, op, a1, a2, a3, op->arg_size); break;
		case JIT_TRANSFER: emit_transfer_init(jit, op, a1, a2, a3, op->arg_size); break;
		case JIT_TRANSFER_CPY: emit_transfer_loop(jit, (jit_op *)a1); break;
		case JIT_TRANSFER_XOR: emit_transfer_op(jit, op, ARMOP_EOR); break;
		case JIT_TRANSFER_AND: emit_transfer_op(jit, op, ARMOP_AND); break;
		case JIT_TRANSFER_OR:  emit_transfer_op(jit, op, ARMOP_ORR); break;
		case JIT_TRANSFER_ADD: emit_transfer_op(jit, op, ARMOP_ADD); break;
		case JIT_TRANSFER_SUB: emit_transfer_op(jit, op, ARMOP_SUB); break;


		 // platform independent opcodes handled in the jitlib-core.c
		case JIT_DATA_BYTE: break;
		case JIT_FULL_SPILL: break;

		default: found = 0;
	}
op_complete:
	if (found) return;

	switch (op->code) {
		case (JIT_MOV | REG): if (a1 != a2) arm32_mov_reg_reg(jit->ip, a1, a2); break;
		case (JIT_MOV | IMM): arm32_mov_reg_imm32(jit->ip, a1, a2); break;
		case JIT_PREPARE: funcall_prepare(jit, op, a1 + a2); break;
		case JIT_PROLOG:
			do {
				jit->current_func = op;
				struct jit_func_info * info = jit_current_func_info(jit);
				op->patch_addr = JIT_BUFFER_OFFSET(jit);
				arm32_pushall_but_r0123(jit->ip);
				arm32_mov_reg_reg(jit->ip, ARMREG_FP, ARMREG_SP);
				arm32_sub_sp_imm(jit->ip, frame_size(jit, info));
			} while (0);
			break;
		case JIT_RETVAL: break; // reg. allocator takes care of the proper register assignment
		case JIT_DECL_ARG: break;

		case JIT_LABEL: ((jit_label *)a1)->pos = JIT_BUFFER_OFFSET(jit); break; 

		case (JIT_LD | REG | SIGNED): 
				switch (op->arg_size) {
					case 1: arm32_ldsb_imm(jit->ip, a1, a2, 0); break;
					case 2: arm32_ldsh_imm(jit->ip, a1, a2, 0); break;
					case 4: arm32_ld_imm(jit->ip, a1, a2, 0); break;
					default: abort();
				} break;

		case (JIT_LD | REG | UNSIGNED): 
				switch (op->arg_size) {
					case 1: arm32_ldub_imm(jit->ip, a1, a2, 0); break;
					case 2: arm32_lduh_imm(jit->ip, a1, a2, 0); break;
					case 4: arm32_ld_imm(jit->ip, a1, a2, 0); break;
					default: abort();
				} break;
		case (JIT_LD | IMM | SIGNED): abort(); break; 

		case (JIT_LD | IMM | UNSIGNED): abort(); break; 


		case (JIT_LDX | IMM | SIGNED): 
				switch (op->arg_size) {
					case 1: arm32_ldsb_imm(jit->ip, a1, a2, a3); break;
					case 2: arm32_ldsh_imm(jit->ip, a1, a2, a3); break;
					case 4: arm32_ld_imm(jit->ip, a1, a2, a3); break;
					default: abort();
				} break;

		case (JIT_LDX | IMM | UNSIGNED): 
				switch (op->arg_size) {
					case 1: arm32_ldub_imm(jit->ip, a1, a2, a3); break;
					case 2: arm32_lduh_imm(jit->ip, a1, a2, a3); break;
					case 4: arm32_ld_imm(jit->ip, a1, a2, a3); break;
					default: abort();
				} break;

		case (JIT_LDX | REG | SIGNED): 
				switch (op->arg_size) {
					case 1: arm32_ldsb_reg(jit->ip, a1, a2, a3); break;
					case 2: arm32_ldsh_reg(jit->ip, a1, a2, a3); break;
					case 4: arm32_ld_reg(jit->ip, a1, a2, a3); break;
					default: abort();
				} break;

		case (JIT_LDX | REG | UNSIGNED): 
				switch (op->arg_size) {
					case 1: arm32_ldub_reg(jit->ip, a1, a2, a3); break;
					case 2: arm32_lduh_reg(jit->ip, a1, a2, a3); break;
					case 4: arm32_ld_reg(jit->ip, a1, a2, a3); break;
					default: abort();
				} break;
		case (JIT_ST | REG):
				switch (op->arg_size) {
					case 1: arm32_stb_imm(jit->ip, a2, a1, 0); break;
					case 2: arm32_sth_imm(jit->ip, a2, a1, 0); break;
					case 4: arm32_st_imm(jit->ip, a2, a1, 0); break;
					default: abort();
				} break;

		case (JIT_ST | IMM): abort(); break;

		case (JIT_STX | REG):
			switch (op->arg_size) {
				case 1: arm32_stb_reg(jit->ip, a3, a2, a1); break;
				case 2: arm32_sth_reg(jit->ip, a3, a2, a1); break;
				case 4: arm32_st_reg(jit->ip, a3, a2, a1); break;
				default: abort();
			} break;

		case (JIT_STX | IMM):
			switch (op->arg_size) {
				case 1: arm32_stb_imm(jit->ip, a3, a2, a1); break;
				case 2: arm32_sth_imm(jit->ip, a3, a2, a1); break;
				case 4: arm32_st_imm(jit->ip, a3, a2, a1); break;
				default: abort();
			} break;

		//
		// Floating-point operations;
		//
		case (JIT_FMOV | REG):
			arm32_vmov_vreg_vreg_double(jit->ip, a1, a2);
			break;

		case (JIT_FMOV | IMM):
			arm32_mov_reg_imm32(jit->ip, ARMREG_R12, (long)&op->flt_imm);
			arm32_vldr_size(jit->ip, a1, ARMREG_R12, 0, sizeof(double));
			break;

		case (JIT_FADD | REG): arm32_vadd_double(jit->ip, a1, a2, a3); break;
		case (JIT_FSUB | REG): arm32_vsub_double(jit->ip, a1, a2, a3); break;
		case (JIT_FRSB | REG): arm32_vsub_double(jit->ip, a1, a3, a2); break;
		case (JIT_FMUL | REG): arm32_vmul_double(jit->ip, a1, a2, a3); break;
		case (JIT_FDIV | REG): arm32_vdiv_double(jit->ip, a1, a2, a3); break;
		case (JIT_FNEG | REG): arm32_vneg_double(jit->ip, a1, a2); break;
/*
		case (JIT_TRUNC | REG): 
			sparc_fdtoi(jit->ip, a2, sparc_f30);
			sparc_stdf_imm(jit->ip, sparc_f30, sparc_fp, -8);
			sparc_ld_imm(jit->ip, sparc_fp, -8, a1);
			break;
		case (JIT_EXT | REG):
			sparc_st_imm(jit->ip, a2, sparc_fp, -8); 
			sparc_ldf_imm(jit->ip, sparc_fp, -8, sparc_f30);
			sparc_fitod(jit->ip, sparc_f30, a1);
			break;

		case (JIT_FLOOR | REG): emit_sparc_floor(jit, a1, a2, 1); break;
		case (JIT_CEIL | REG): emit_sparc_floor(jit, a1, a2, 0); break;
		case (JIT_ROUND | REG): emit_sparc_round(jit, a1, a2); break;
*/
		case (JIT_FRET | REG):
			do {
				struct jit_func_info *info = jit_current_func_info(jit);
				arm32_add_sp_imm(jit->ip, frame_size(jit, info));
				if (op->arg_size == sizeof(float)) {
//					arm32_vmov_vreg_reg_float(jit->ip, ARMREG_R0, a1);
					arm32_vcvt_dtos(jit->ip, ARMREG_D0, a1);
				} else {
					arm32_vmov_vreg_vreg_double(jit->ip, ARMREG_D0, a1);
			//		arm32_vmov_vreg_reg_double(jit->ip, ARMREG_R0, ARMREG_R1, a1);
				}
				arm32_popall_but_r0123(jit->ip);
				arm32_bx(jit->ip, ARMCOND_AL, ARMREG_LR);
			} while (0);
			break;
/*
		case (JIT_FRETVAL):
			// reg. allocator takes care of proper assignment of the register
			if (op->arg_size == sizeof(float)) sparc_fstod(jit->ip, sparc_f0, sparc_f0);
			break;
*/
		case (JIT_FLD | REG):
			arm32_vldr_size(jit->ip, a1, a2, 0, op->arg_size);
			break;

		case (JIT_FLD | IMM):
			arm32_mov_reg_imm32(jit->ip, ARMREG_R12, a2);
			arm32_vldr_size(jit->ip, a1, ARMREG_R12, 0, op->arg_size);
			break;

		case (JIT_FLDX | REG):
			arm32_alu_reg_reg(jit->ip, ARMOP_ADD, ARMREG_R12, a2, a3);
			arm32_vldr_size(jit->ip, a1, ARMREG_R12, 0, op->arg_size);
			break;

		case (JIT_FLDX | IMM):
			if ((a3 >= -255 * 4) && (a3 <= 255 * 4)) arm32_vldr_size(jit->ip, a1, a2, a3, op->arg_size);
			else {
				arm32_mov_reg_imm32(jit->ip, ARMREG_R12, a3);
				arm32_alu_reg_reg(jit->ip, ARMOP_ADD, ARMREG_R12, ARMREG_R12, a2);
				arm32_vldr_size(jit->ip, a1, ARMREG_R12, 0, op->arg_size);
			}
			break;

		case (JIT_FST | REG):
			if (op->arg_size == sizeof(double)) arm32_vstr_size(jit->ip, a2, a1, 0, op->arg_size);
			else {
				int in_use = jit_reg_in_use(op, a2, 1);
				if (in_use) arm32_vpush(jit->ip, a2);
				arm32_vcvt_dtos(jit->ip, a2, a2);
				arm32_vstr_size(jit->ip, a2, a1, 0, op->arg_size);
				if (in_use) arm32_vpop(jit->ip, a2);
			}
			break;

		case (JIT_FST | IMM):
			arm32_mov_reg_imm32(jit->ip, ARMREG_R12, a2);
			if (op->arg_size == sizeof(double)) arm32_vstr_size(jit->ip, a1, ARMREG_R12, 0, op->arg_size);
			else {
				int in_use = jit_reg_in_use(op, a2, 1);
				if (in_use) arm32_vpush(jit->ip, a2);
				arm32_vcvt_dtos(jit->ip, a2, a2);
				arm32_vstr_size(jit->ip, a1, ARMREG_R12, 0, op->arg_size);
				if (in_use) arm32_vpop(jit->ip, a2);
			}
			break;

		case (JIT_FSTX | REG):
			arm32_alu_reg_reg(jit->ip, ARMOP_ADD, ARMREG_R12, a1, a2);
			if (op->arg_size == sizeof(double)) arm32_vstr_size(jit->ip, a3, ARMREG_R12, 0, op->arg_size);
			else {
				int in_use = jit_reg_in_use(op, a3, 1);
				if (in_use) arm32_vpush(jit->ip, a2);
				arm32_vcvt_dtos(jit->ip, a3, a3);
				arm32_vstr_size(jit->ip, a3, ARMREG_R12, 0, op->arg_size);
				if (in_use) arm32_vpop(jit->ip, a3);
			}

			break;

		case (JIT_FSTX | IMM):
			if (op->arg_size == sizeof(double)) {
				if ((a1 >= -255 * 4) && (a1 <= 255 * 4)) arm32_vstr_size(jit->ip, a3, a2, a1, op->arg_size);
				else {
					arm32_mov_reg_imm32(jit->ip, ARMREG_R12, a1);
					arm32_alu_reg_reg(jit->ip, ARMOP_ADD, ARMREG_R12, ARMREG_R12, a2);
					arm32_vstr_size(jit->ip, a3, ARMREG_R12, 0, op->arg_size);
				}
			} else {
				int in_use = jit_reg_in_use(op, a3, 1);
				if (in_use) arm32_vpush(jit->ip, a2);
				arm32_vcvt_dtos(jit->ip, a3, a3);
				if ((a1 >= -255 * 4) && (a1 <= 255 * 4)) arm32_vstr_size(jit->ip, a3, a2, a1, op->arg_size);
				else {
					arm32_mov_reg_imm32(jit->ip, ARMREG_R12, a1);
					arm32_alu_reg_reg(jit->ip, ARMOP_ADD, ARMREG_R12, ARMREG_R12, a2);
					arm32_vstr_size(jit->ip, a3, ARMREG_R12, 0, op->arg_size);
				}

				if (in_use) arm32_vpop(jit->ip, a3);
			}
			break;

		case (JIT_UREG): emit_ureg(jit, a1, a2); break;
		case (JIT_SYNCREG): emit_ureg(jit, a1, a2); break;
		case (JIT_LREG): 
			if (JIT_REG_SPEC(a2) == JIT_RTYPE_ARG) assert(0);
			if (JIT_REG_TYPE(a2) == JIT_RTYPE_INT)
				arm32_ld_fp_imm(jit->ip, a1, GET_REG_POS(jit, a2));
			else arm32_ld_fp_imm(jit->ip, a1, GET_REG_POS(jit, a2));
			break;
		case JIT_RENAMEREG: arm32_mov_reg_reg(jit->ip, a1, a2); break;

		case JIT_CODESTART: break;
		case JIT_NOP: break;
		default: printf("arm32: unknown operation (opcode: 0x%x)\n", GET_OP(op) >> 3);
	}
}

struct jit_reg_allocator * jit_reg_allocator_create()
{
	struct jit_reg_allocator * a = JIT_MALLOC(sizeof(struct jit_reg_allocator));
	a->gp_reg_cnt = 11;
#ifdef JIT_REGISTER_TEST
	a->gp_reg_cnt = 4;
#endif 
	a->gp_regs = JIT_MALLOC(sizeof(jit_hw_reg) * (a->gp_reg_cnt));

	a->gp_regs[0] = (jit_hw_reg) { ARMREG_R0, "R0", 0, 0, 0 };
	a->gp_regs[1] = (jit_hw_reg) { ARMREG_R1, "R1", 0, 0, 1 };
	a->gp_regs[2] = (jit_hw_reg) { ARMREG_R2, "R2", 0, 0, 2 };
	a->gp_regs[3] = (jit_hw_reg) { ARMREG_R3, "R3", 0, 0, 3 };
#ifndef JIT_REGISTER_TEST
	a->gp_regs[4] = (jit_hw_reg) { ARMREG_R4, "R4", 1, 0, 4 };
	a->gp_regs[5] = (jit_hw_reg) { ARMREG_R5, "R5", 1, 0, 5 };
	a->gp_regs[6] = (jit_hw_reg) { ARMREG_R6, "R6", 1, 0, 6 };
	a->gp_regs[7] = (jit_hw_reg) { ARMREG_R7, "R7", 1, 0, 7 };
	a->gp_regs[8] = (jit_hw_reg) { ARMREG_R8, "R8", 1, 0, 8 };
	a->gp_regs[9] = (jit_hw_reg) { ARMREG_R9, "R9", 1, 0, 9 };
	a->gp_regs[10] = (jit_hw_reg) { ARMREG_R10, "R10", 1, 0, 10 };
#endif


	a->fp_reg_cnt = 4;

	a->fp_regs = JIT_MALLOC(sizeof(jit_hw_reg) * a->fp_reg_cnt);


	a->fp_regs[0] = (jit_hw_reg) { ARMREG_D0, "D0", 0, 1, 1 };
	a->fp_regs[1] = (jit_hw_reg) { ARMREG_D1, "D1", 0, 1, 2 };
	a->fp_regs[2] = (jit_hw_reg) { ARMREG_D2, "D2", 0, 1, 3 };
	a->fp_regs[3] = (jit_hw_reg) { ARMREG_D3, "D3", 0, 1, 4 };
	// FIXME: more registers


	a->fp_reg = ARMREG_SP;
	a->ret_reg = &(a->gp_regs[0]);
	a->fpret_reg = NULL;
//	a->fpret_reg = &(a->fp_regs[0]);


	a->gp_arg_reg_cnt = 4;
	a->gp_arg_regs = JIT_MALLOC(sizeof(jit_hw_reg *) * 4);
	for (int i = 0; i < 4; i++)
		a->gp_arg_regs[i] = &(a->gp_regs[i]);

	a->fp_arg_reg_cnt = 0;
	a->fp_arg_regs = NULL;
	return a;
}
