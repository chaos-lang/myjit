/*
 * MyJIT 
 * Copyright (C) 2015 Petr Krajca, <petr.krajca@upol.cz>
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

#define _BSD_SOURCE
#define _DARWIN_C_SOURCE


#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <unistd.h>

#include "cpu-detect.h"
#include "jitlib.h"
#include "jitlib-core.h"
#include "set.h"


#ifdef JIT_ARCH_COMMON86
#include "common86-specific.h"
#endif

#ifdef JIT_ARCH_SPARC
#include "sparc-specific.h"
#endif


#ifdef JIT_ARCH_ARM32
#include "arm32-specific.h"
#endif

#include "jitlib-debug.c"
#include "code-check.c"
#include "flow-analysis.h"
#include "rmap.h"
#include "reg-allocator.h"



#define BUF_SIZE		(4096)
#define MINIMAL_BUF_SPACE	(1024)

struct jit_op * jit_add_op(struct jit * jit, unsigned short code, unsigned char spec, long arg1, long arg2, long arg3, unsigned char arg_size, struct jit_debug_info *debug_info)
{
	struct jit_op * r = jit_op_new(code, spec, arg1, arg2, arg3, arg_size);
	r->debug_info = debug_info;
	jit_op_append(jit->last_op, r);
	jit->last_op = r;

	return r;
}

struct jit_op * jit_add_fop(struct jit * jit, unsigned short code, unsigned char spec, long arg1, long arg2, long arg3, double flt_imm, unsigned char arg_size, struct jit_debug_info *debug_info)
{
	struct jit_op * r = jit_add_op(jit, code, spec, arg1, arg2, arg3, arg_size, debug_info);
	r->fp = 1;
	r->flt_imm = flt_imm;

	return r;
}

struct jit_debug_info *jit_debug_info_new(const char *filename, const char *function, int lineno)
{
	struct jit_debug_info *r = JIT_MALLOC(sizeof(struct jit_debug_info));
	r->filename = filename;
	r->function = function;
	r->lineno = lineno;
	r->warnings = 0;
	return r;
}

struct jit * jit_init()
{
	struct jit * r = JIT_MALLOC(sizeof(struct jit));

	r->ops = jit_op_new(JIT_CODESTART, SPEC(NO, NO, NO), 0, 0, 0, 0);
	r->last_op = r->ops;
	r->optimizations = 0;

	r->buf = NULL;
	r->mmaped_buf = 0;
	r->labels = NULL;
	r->reg_al = jit_reg_allocator_create();
	jit_enable_optimization(r, JIT_OPT_JOIN_ADDMUL | JIT_OPT_OMIT_FRAME_PTR | JIT_OPT_DEAD_CODE);

	return r;
}

jit_op *jit_add_prolog(struct jit * jit, void * func, struct jit_debug_info *debug_info)
{
        jit_op * op = jit_add_op(jit, JIT_PROLOG , SPEC(IMM, NO, NO), (long)func, 0, 0, 0, NULL);
        struct jit_func_info * info = JIT_MALLOC(sizeof(struct jit_func_info));
        op->arg[1] = (long)info;
	op->debug_info = debug_info;

        jit->current_func = op;

	info->first_op = op;
        info->allocai_mem = 0;
        info->general_arg_cnt = 0;
        info->float_arg_cnt = 0;
	return op;
}

jit_label * jit_get_label(struct jit * jit)
{
        jit_label * r = JIT_MALLOC(sizeof(jit_label));
        jit_add_op(jit, JIT_LABEL, SPEC(IMM, NO, NO), (long)r, 0, 0, 0, NULL);
        r->next = jit->labels;
        jit->labels = r;
        return r;
}


#if JIT_IMM_BITS > 0
/**
 * returns 1 if the immediate value has to be transformed into register
 */
static int jit_imm_overflow(struct jit *jit, jit_op *op, long value)
{
#ifndef JIT_ARCH_ARM32
	unsigned long mask = ~((1UL << JIT_IMM_BITS) - 1);
	unsigned long high_bits = value & mask;

	if (IS_SIGNED(op)) {
		if ((high_bits != 0) && (high_bits != mask)) return 1;
	} else {
		if (high_bits != 0) return 1;
	}
	return 0;
#else
	long abs_value = (value < 0 ? - value : value);
	if (GET_OP(op) == JIT_HMUL) return 1;
	if (GET_OP(op) == JIT_SUBC) return 1;
	if (op->code == (JIT_LD | IMM | SIGNED)) return 1;
	if (op->code == (JIT_LD | IMM | UNSIGNED)) return 1;

	if (op->code == (JIT_LSH | IMM | SIGNED)) return (value < 0) || (value > 31);
	if (op->code == (JIT_RSH | IMM | SIGNED)) return (value < 0) || (value > 31);
	if (op->code == (JIT_RSH | IMM | UNSIGNED)) return (value < 0) || (value > 31);

	if ((op->code == (JIT_LDX | IMM | SIGNED)) && (op->arg_size == 1)) return (value < -255) || (value > 255);
	if ((op->code == (JIT_LDX | IMM | SIGNED)) && (op->arg_size == 2)) return (value < -255) || (value > 255);;
	if ((op->code == (JIT_LDX | IMM | UNSIGNED)) && (op->arg_size == 2)) return (value < -255) || (value > 255);;
	if ((op->code == (JIT_STX | IMM)) && (op->arg_size == 2)) return (value < -255) || (value > 255);;


	if ((op->code == (JIT_LDX | IMM | UNSIGNED)) && (op->arg_size == 1) && (arm32_imm_rotate(abs_value) >= 0)) return 0;
	if ((op->code == (JIT_LDX | IMM | UNSIGNED)) && (op->arg_size == 4) && (arm32_imm_rotate(abs_value) >= 0)) return 0;
	if ((op->code == (JIT_LDX | IMM | SIGNED)) && (op->arg_size == 4) && (arm32_imm_rotate(abs_value) >= 0)) return 0;
	if ((op->code == (JIT_STX | IMM)) && (op->arg_size == 4) && (arm32_imm_rotate(abs_value) >= 0)) return 0;

	if (GET_OP(op) == JIT_FLD) return 0;
	if (GET_OP(op) == JIT_FLDX) return 0;
	if (GET_OP(op) == JIT_FST) return 0;
	if (GET_OP(op) == JIT_FSTX) return 0;

	if ((GET_OP(op) == JIT_MUL) && (value == 0)) return 0;
	if (GET_OP(op) == JIT_MOD) {
		if ((op->code == (JIT_MOD | IMM | SIGNED)) && is_pow2(value)) return 0;
		return 1;
	}
	if ((GET_OP(op) == JIT_DIV) || (GET_OP(op) == JIT_MUL)) {
		if (IS_IMM(op)) return !is_pow2(value);
		return 0;
	}
	return arm32_imm_rotate(value) == -1;
#endif
}

/**
 * converts long immediates to MOV operation
 */
static void jit_correct_long_imms(struct jit * jit)
{
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		if (!IS_IMM(op)) continue;
		if (GET_OP(op) == JIT_JMP) continue;
		if (GET_OP(op) == JIT_CALL) continue;
		if (GET_OP(op) == JIT_PATCH) continue;
		if (GET_OP(op) == JIT_MOV) continue;
		if (GET_OP(op) == JIT_PUTARG) continue;
		if (GET_OP(op) == JIT_MSG) continue;
		if (GET_OP(op) == JIT_COMMENT) continue;
		if (GET_OP(op) == JIT_PROLOG) continue;
		if (GET_OP(op) == JIT_DATA_REF_CODE) continue;
		if (GET_OP(op) == JIT_DATA_REF_DATA) continue;
		if (GET_OP(op) == JIT_REF_DATA) continue;
		if (GET_OP(op) == JIT_REF_CODE) continue;
		if (GET_OP(op) == JIT_FORCE_ASSOC) continue;
		if (GET_OP(op) == JIT_TRACE) continue;
		int imm_arg;
		for (int i = 1; i < 4; i++)
			if (ARG_TYPE(op, i) == IMM) imm_arg = i - 1;
		long value = op->arg[imm_arg];

		if (jit_imm_overflow(jit, op, value)) {
			jit_op * newop = jit_op_new(JIT_MOV | IMM, SPEC(TREG, IMM, NO), R_IMM, value, 0, REG_SIZE);
			jit_op_prepend(op, newop);

			op->code &= ~(0x3);
			op->code |= REG;

			op->spec &= ~(0x3 << (2 * imm_arg));
			op->spec |=  (REG << (2 * imm_arg));
			op->arg[imm_arg] = R_IMM;
		}
	}
}
#endif

static inline void jit_correct_float_imms(struct jit * jit)
{
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		if (!IS_IMM(op)) continue;
		if (!op->fp) continue;
		if (GET_OP(op) == JIT_FMOV) continue;
		if (GET_OP(op) == JIT_FPUTARG) continue;
		if (GET_OP(op) == JIT_FLD) continue;
		if (GET_OP(op) == JIT_FLDX) continue;
		if (GET_OP(op) == JIT_FST) continue;
		if (GET_OP(op) == JIT_FSTX) continue;
#if defined(JIT_ARCH_ARM32)
		if (is_cond_branch_op(op) && IS_IMM(op) && (op->flt_imm == 0.0)) continue;
#endif
// FIXME: TODO		if (GET_OP(op) == JIT_FMSG) continue;
		int imm_arg;
		for (int i = 1; i < 4; i++)
			if (ARG_TYPE(op, i) == IMM) imm_arg = i - 1;

		jit_op * newop = jit_op_new(JIT_FMOV | IMM, SPEC(TREG, IMM, NO), (jit_value) FR_IMM, 0, 0, 0);
		newop->fp = 1;
		newop->flt_imm = op->flt_imm;
		jit_op_prepend(op, newop);

		op->code &= ~(0x3);
		op->code |= REG;

		op->spec &= ~(0x3 << (2 * imm_arg));
		op->spec |=  (REG << (2 * imm_arg));
		op->arg[imm_arg] = FR_IMM;
	}
}

static inline void jit_expand_patches_and_labels(struct jit * jit)
{
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		if (GET_OP(op) == JIT_PATCH) {
			((jit_op *)(op->arg[0]))->jmp_addr = op;
		}
		if (GET_OP(op) == JIT_LABEL) {
			((jit_label *)(op->arg[0]))->op = op;
		}

		if ((GET_OP(op) != JIT_LABEL) && (jit_is_label(jit, (void *)op->arg[0]))) {
			op->jmp_addr = ((jit_label *)(op->arg[0]))->op;
		}

		if ((GET_OP(op) != JIT_LABEL) && (jit_is_label(jit, (void *)op->arg[1]))) {
			op->jmp_addr = ((jit_label *)(op->arg[1]))->op;
		}
	}
}

static inline void jit_prepare_reg_counts(struct jit * jit)
{
	int declared_args = 0;
	int last_gp = -1;
	int last_fp = -1;
	int gp_args = 0;
	int fp_args = 0;
	struct jit_func_info * info = NULL;
	jit_op * op = jit_op_first(jit->ops);

	while (1) {
		if (!op || (GET_OP(op) == JIT_PROLOG)) {
			if (info) {
				info->gp_reg_count = last_gp + 1;
				info->fp_reg_count = last_fp + 1;
				info->general_arg_cnt = gp_args;
				info->float_arg_cnt = fp_args;

#if defined(JIT_ARCH_AMD64)
				// stack has to be aligned to 16 bytes
				while ((info->gp_reg_count + info->fp_reg_count) % 2) info->gp_reg_count ++; 
#endif
				info->args = JIT_MALLOC(sizeof(struct jit_inp_arg) * declared_args);
			}
			if (op) {
				declared_args = 0;
				last_gp = -1;
				last_fp = -1;
				gp_args = 0;
				fp_args = 0;
				info = (struct jit_func_info *)op->arg[1];
			}
			if (!op) break;
		}
	
		for (int i = 0; i < 3; i++)
			if ((ARG_TYPE(op, i + 1) == TREG) || (ARG_TYPE(op, i + 1) == REG)) {
				jit_reg r = (jit_reg) op->arg[i];
				if ((JIT_REG_TYPE(r) == JIT_RTYPE_INT) && (JIT_REG_ID(r) > last_gp)) last_gp = JIT_REG_ID(r);
				if ((JIT_REG_TYPE(r) == JIT_RTYPE_FLOAT) && (JIT_REG_ID(r) > last_fp)) last_fp = JIT_REG_ID(r);
			}
				
		if (GET_OP(op) == JIT_DECL_ARG) {
			declared_args++;
			if (op->arg[0] == JIT_FLOAT_NUM) fp_args++;
			else gp_args++;
		}

		if (GET_OP(op) == JIT_PREPARE) {
			jit_op * xop = op;
			while (1) {
				if (GET_OP(op->next) == JIT_PUTARG) xop->arg[0]++;
				else if (GET_OP(op->next) == JIT_FPUTARG) xop->arg[1]++;
				else {
					jit_opcode next_code = GET_OP(op->next);
					if (next_code == JIT_CALL) break;
					if ((next_code != JIT_TRACE) && (next_code != JIT_CODE_ALIGN)
					&& (next_code != JIT_UREG) && (next_code != JIT_LREG)
					&& (next_code != JIT_RENAMEREG) && (next_code != JIT_SYNCREG))
					{
						printf("Garbage in the prepare-call block. Opcode: %x\n", next_code >> 3);
						abort();
					}
				}
				op = op->next;
			}
		}
		op = op->next;
	}

}

static inline void jit_prepare_arguments(struct jit * jit)
{
	jit_op * op = jit_op_first(jit->ops);
	struct jit_func_info * info = NULL;
	int gp_arg_pos = 0;
	int fp_arg_pos = 0;
	int argpos = 0;
	int phys_reg = 0;

	while (op) {
		if (GET_OP(op) == JIT_PROLOG) {
			info = (struct jit_func_info *)op->arg[1];
			info->has_prolog = 1;
			gp_arg_pos = 0;
			fp_arg_pos = 0;
			argpos = 0;
			phys_reg = 0;
		}
		if (GET_OP(op) == JIT_DECL_ARG) {
			info->args[argpos].type = op->arg[0];
			info->args[argpos].size = op->arg[1];
			if (op->arg[0] == JIT_FLOAT_NUM) {
				info->args[argpos].gp_pos = gp_arg_pos;
				info->args[argpos].fp_pos = fp_arg_pos++;
			} else {
				info->args[argpos].gp_pos = gp_arg_pos++;
				info->args[argpos].fp_pos = fp_arg_pos;
			}
			jit_init_arg_params(jit, info, argpos, &phys_reg);
			argpos++;
		}
		op = op->next;
	}

}

static inline void jit_prepare_spills_on_jmpr_targets(struct jit *jit)
{
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next)
		if ((GET_OP(op) == JIT_REF_CODE) || (GET_OP(op) == JIT_DATA_REF_CODE))  {
			jit_op * newop = jit_op_new(JIT_FULL_SPILL | IMM, SPEC(NO, NO, NO), 0, 0, 0, 0);
			jit_op_prepend(op->jmp_addr, newop);
		}
}

static inline void jit_buf_expand(struct jit * jit)
{
	long pos = jit->ip - jit->buf;
	jit->buf_capacity *= 2;
	jit->buf = JIT_REALLOC(jit->buf, jit->buf_capacity);
	jit->ip = jit->buf + pos;
}

void jit_generate_code(struct jit * jit)
{
	jit_expand_patches_and_labels(jit);
#if JIT_IMM_BITS > 0
	jit_correct_long_imms(jit);
#endif
	jit_correct_float_imms(jit);

	jit_prepare_reg_counts(jit);
	jit_prepare_arguments(jit);
	jit_prepare_spills_on_jmpr_targets(jit);

	if (jit->optimizations & JIT_OPT_DEAD_CODE) {
		jit_dead_code_analysis(jit, 1);
	}
	jit_flw_analysis(jit);


	if (jit->optimizations & JIT_OPT_OMIT_UNUSED_ASSIGNEMENTS) jit_optimize_unused_assignments(jit);


#if defined(JIT_ARCH_I386) || defined(JIT_ARCH_AMD64)
	int change = 0;
	jit_optimize_st_ops(jit);
	if (jit->optimizations & JIT_OPT_JOIN_ADDMUL) {
		change |= jit_optimize_join_addmul(jit);
		change |= jit_optimize_join_addimm(jit);
	}
	// oops, we have changed the code structure, we have to do the analysis again
	if (change) jit_flw_analysis(jit);
#endif
	jit_collect_statistics(jit);
	jit_assign_regs(jit);

#ifdef JIT_ARCH_COMMON86
	if (jit->optimizations & JIT_OPT_OMIT_FRAME_PTR) jit_optimize_frame_ptr(jit);
#endif

	jit->buf_capacity = BUF_SIZE;
	jit->buf = JIT_MALLOC(jit->buf_capacity);
	jit->ip = jit->buf;

	for (struct jit_op * op = jit->ops; op != NULL; op = op->next) {
		if (jit->buf_capacity - (jit->ip - jit->buf) < MINIMAL_BUF_SPACE) jit_buf_expand(jit);
		// platform unspecific opcodes
		unsigned long offset_1 = (jit->ip - jit->buf);
		switch (GET_OP(op)) {
			case JIT_DATA_BYTE: *(jit->ip)++ = (unsigned char) op->arg[0]; break;
			case JIT_DATA_BYTES: 
				while (jit->buf_capacity - (jit->ip - jit->buf) < op->arg[0])
					jit_buf_expand(jit);
				
				for (int i = 0; i < op->arg[0]; i++)
					*(jit->ip)++ = *(((unsigned char *) op->addendum) + i);
				break;
			case JIT_DATA_REF_CODE: 
			case JIT_DATA_REF_DATA: 
				op->patch_addr = JIT_BUFFER_OFFSET(jit);
				for (int i = 0; i < sizeof(void *); i++) {
					*jit->ip = 0;
					jit->ip++;
				}
				break; 
			case JIT_FORCE_SPILL:
			case JIT_FORCE_ASSOC:
			case JIT_COMMENT:
			case JIT_MARK:
			case JIT_TOUCH:
				break;
			// platform specific opcodes
			default: jit_gen_op(jit, op);
		}
		unsigned long offset_2 = (jit->ip - jit->buf);
		op->code_offset = offset_1;
		op->code_length = offset_2 - offset_1;
	}

	/* moves the code to its final destination */
	int code_size = jit->ip - jit->buf;  
	//void * mem;
	//posix_memalign(&mem, sysconf(_SC_PAGE_SIZE), code_size);
	//mprotect(mem, code_size, PROT_READ | PROT_EXEC | PROT_WRITE);
	void *mem = mmap(NULL, jit->buf_capacity, PROT_READ | PROT_EXEC | PROT_WRITE, MAP_ANON | MAP_PRIVATE, -1, 0);
	if (mem == MAP_FAILED) perror("mmap");
	memcpy(mem, jit->buf, code_size);
	JIT_FREE(jit->buf);

	// FIXME: duplicitni vypocet?
	long pos = jit->ip - jit->buf;
	jit->buf = mem;
	jit->ip = jit->buf + pos;
	jit->mmaped_buf = 1;

	jit_patch_external_calls(jit);
	jit_patch_local_addrs(jit);

	/* assigns functions */
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		if (GET_OP(op) == JIT_PROLOG)
			*(void **)(op->arg[0]) = jit->buf + (long)op->patch_addr;
	}
}

void jit_trace(struct jit *jit, int verbosity)
{
#if defined(JIT_ARCH_COMMON86) || defined(JIT_ARCH_ARM32)
	for (jit_op *op = jit_op_first(jit->ops)->next; op != NULL; op = op->next) {
		if (GET_OP(op) == JIT_PROLOG) continue;
		if (GET_OP(op) == JIT_DATA_BYTE) continue;
		if (GET_OP(op) == JIT_DATA_REF_CODE) continue;
		if (GET_OP(op) == JIT_DATA_REF_DATA) continue;
		jit_op * o = jit_op_new(JIT_TRACE, SPEC(IMM, NO, NO), verbosity, 0, 0, 0);
		o->r_arg[0] = o->arg[0];
		jit_op_prepend(op, o);
	}
#else
	printf("jit_trace is not supported on this architecture\n");
#endif
}

static void free_ops(struct jit_op * op)
{
	if (op == NULL) return;
	free_ops(op->next);
	if (op->addendum) JIT_FREE(op->addendum);
	jit_free_op(op);

}

static void free_labels(jit_label * lab)
{
	if (lab == NULL) return;
	free_labels(lab->next);
	JIT_FREE(lab);
}

static int is_cond_branch_op(jit_op *op)
{
	jit_opcode code = GET_OP(op);
	return (code == JIT_BLT) || (code == JIT_BLE) || (code == JIT_BGT)
	|| (code == JIT_BGE) || (code == JIT_BEQ) ||  (code == JIT_BNE)
	|| (code == JIT_FBLT) || (code == JIT_FBLE) || (code == JIT_FBGT)
	|| (code == JIT_FBGE) || (code == JIT_FBEQ) ||  (code == JIT_FBNE)
	|| (code == JIT_BOADD) || (code == JIT_BOSUB) || (code == JIT_BNOADD)
	|| (code == JIT_BNOSUB);
}


void jit_enable_optimization(struct jit * jit, int opt)
{
	jit->optimizations |= opt;
}

void jit_disable_optimization(struct jit * jit, int opt)
{
	jit->optimizations &= ~opt;
}

void jit_free(struct jit * jit)
{
	jit_reg_allocator_free(jit->reg_al);
	free_ops(jit_op_first(jit->ops));
	free_labels(jit->labels);
	if (jit->buf) {
		if (jit->mmaped_buf) munmap(jit->buf, jit->buf_capacity);
		else JIT_FREE(jit->buf);
	}
	JIT_FREE(jit);
}

int jit_regs_active_count(jit_op *op)
{
	return jit_set_size(op->live_out);
}

void jit_regs_active(jit_op *op, jit_value *dest)
{
	jit_set_to_array(op->live_out, dest);
}
