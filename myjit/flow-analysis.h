/*
 * MyJIT 
 * Copyright (C) 2010, 2015 Petr Krajca, <petr.krajca@upol.cz>
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

#include "set.h"
static inline void jit_flw_initialize(struct jit * jit)
{
	struct jit_func_info *func_info;
	jit_op * op = jit_op_first(jit->ops);
	while (op) {
		op->live_in = jit_set_new();
		op->live_out = jit_set_new();

		for (int i = 0; i < 3; i++)
			if (ARG_TYPE(op, i + 1) == REG)
				jit_set_add(op->live_in, op->arg[i]);

		if (GET_OP(op) == JIT_PROLOG) {
			func_info = (struct jit_func_info *)op->arg[1];
		}

#if defined(JIT_ARCH_AMD64) || defined(JIT_ARCH_SPARC)
		if (GET_OP(op) == JIT_GETARG) {
			int arg_id = op->arg[1];
			if (func_info->args[arg_id].type != JIT_FLOAT_NUM) {
				jit_set_add(op->live_in, jit_mkreg(JIT_RTYPE_INT, JIT_RTYPE_ARG, arg_id)); 
			} else {
				jit_set_add(op->live_in, jit_mkreg(JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, arg_id));
				if (func_info->args[arg_id].overflow)
					jit_set_add(op->live_in, jit_mkreg_ex(JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, arg_id)); 
			}
		}
#endif

		op = op->next;
	}
}

static inline void flw_analyze_prolog(struct jit * jit, jit_op * op, struct jit_func_info * func_info)
{
#if defined(JIT_ARCH_AMD64) 
	for (int i = 0; i < func_info->general_arg_cnt + func_info->float_arg_cnt; i++) {
		if (func_info->args[i].type == JIT_FLOAT_NUM) {
			jit_set_remove(op->live_in, jit_mkreg(JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, i)); 
		} else {
			jit_set_remove(op->live_in, jit_mkreg(JIT_RTYPE_INT, JIT_RTYPE_ARG, i)); 
		}
	}
#endif


#if defined(JIT_ARCH_SPARC)
	int assoc_gp = 0;
	int argcount = func_info->general_arg_cnt + func_info->float_arg_cnt;
	for (int j = 0; j < argcount; j++) {
		if (assoc_gp >= jit->reg_al->gp_arg_reg_cnt) break;
		if (func_info->args[j].type != JIT_FLOAT_NUM) {
			jit_set_remove(op->live_in, jit_mkreg(JIT_RTYPE_INT, JIT_RTYPE_ARG, j));	
			assoc_gp++;
		} else {
			jit_set_remove(op->live_in, jit_mkreg(JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, j));	
			assoc_gp++;
			if (func_info->args[j].size == sizeof(double)) {
				jit_set_remove(op->live_in, jit_mkreg_ex(JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, j));	
				assoc_gp++;
			}
		}
	}
#endif
}

struct code_refs_cache {
	int size;
	jit_op **ops;
};

static inline void initialize_code_refs(struct code_refs_cache *code_refs, struct jit_func_info *func_info)
{
	jit_op *op = func_info->first_op->next;
	code_refs->size = 0;
	while (op && (GET_OP(op) != JIT_PROLOG)) {
		if ((GET_OP(op) == JIT_REF_CODE) || (GET_OP(op) == JIT_DATA_REF_CODE))
			code_refs->size++;
		op = op->next;
	}
	code_refs->ops = JIT_MALLOC(sizeof(jit_op *) * code_refs->size);

	op = func_info->first_op->next;
	int i = 0;
	while (op && (GET_OP(op) != JIT_PROLOG)) {
		if ((GET_OP(op) == JIT_REF_CODE) || (GET_OP(op) == JIT_DATA_REF_CODE))
			code_refs->ops[i++] = op;	
		op = op->next;
	}
}

/*
static inline int old_flw_analyze_op(struct jit * jit, jit_op * op, struct jit_func_info * func_info, int changed, struct code_refs_cache *code_refs)
{
	int result;
	jit_set *out1 = op->live_out;
	jit_set *in1 = op->live_in;

	op->live_in = jit_set_clone(op->live_out);

	for (int i = 0; i < 3; i++)
		if (ARG_TYPE(op, i + 1) == TREG) jit_set_remove(op->live_in, op->arg[i]);

	if (GET_OP(op) == JIT_PROLOG) flw_analyze_prolog(jit, op, func_info);

	for (int i = 0; i < 3; i++)
		if (ARG_TYPE(op, i + 1) == REG)
			jit_set_add(op->live_in, op->arg[i]);

#if defined(JIT_ARCH_AMD64) || defined(JIT_ARCH_SPARC)
	if (GET_OP(op) == JIT_GETARG) {
		int arg_id = op->arg[1];
		if (func_info->args[arg_id].type != JIT_FLOAT_NUM) {
			jit_set_add(op->live_in, jit_mkreg(JIT_RTYPE_INT, JIT_RTYPE_ARG, arg_id)); 
		} else {
			jit_set_add(op->live_in, jit_mkreg(JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, arg_id));
			if (func_info->args[arg_id].overflow)
				jit_set_add(op->live_in, jit_mkreg_ex(JIT_RTYPE_FLOAT, JIT_RTYPE_ARG, arg_id)); 
		}
	}
#endif


	if ((GET_OP(op) == JIT_RET) || (GET_OP(op) == JIT_FRET)) {
		op->live_out = jit_set_new(); 
		goto skip;
	}

	if ((op->code == (JIT_JMP | IMM)) && op->jmp_addr) {
		op->live_out = jit_set_clone(op->jmp_addr->live_in);
		goto skip;
	}

	if (op->code == (JIT_JMP | REG)) {
		op->live_out = jit_set_new();

		if (code_refs->size < 0) initialize_code_refs(code_refs, func_info);
		for (int i = 0; i < code_refs->size; i++) { 
			jit_set_addall(op->live_out, code_refs->ops[i]->jmp_addr->live_in);
		}

		goto skip;
	}

	if (op->next) op->live_out = jit_set_clone(op->next->live_in);
	else op->live_out = jit_set_new();

	if (op->jmp_addr && (GET_OP(op) != JIT_REF_CODE) && (GET_OP(op) != JIT_DATA_REF_CODE))
		jit_set_addall(op->live_out, op->jmp_addr->live_in);
skip:

	// if something has changed, we have to do the analysis again anyway
	if (changed) result = changed;
	else result = !(jit_set_equal(in1, op->live_in) && jit_set_equal(out1, op->live_out));
	jit_set_free(in1);
	jit_set_free(out1);
	return result;
}
*/

static inline int flw_analyze_op(struct jit * jit, jit_op * op, struct jit_func_info * func_info, int changed, struct code_refs_cache *code_refs)
{
	int live_out_size = jit_set_size(op->live_out);
	int live_in_size = jit_set_size(op->live_in);
	
	if (op->jmp_addr && (GET_OP(op) != JIT_REF_CODE) && (GET_OP(op) != JIT_DATA_REF_CODE))
		jit_set_addall(op->live_out, op->jmp_addr->live_in);

	if (op->code == (JIT_JMP | REG)) {

		if (code_refs->size < 0) initialize_code_refs(code_refs, func_info);
		for (int i = 0; i < code_refs->size; i++) { 
			jit_set_addall(op->live_out, code_refs->ops[i]->jmp_addr->live_in);
		}

		goto skip;
	}
	
	if (op->code == (JIT_JMP | IMM)) goto skip;

	if (op->next) jit_set_addall(op->live_out, op->next->live_in);


skip:
	jit_set_addall(op->live_in, op->live_out);

	for (int i = 0; i < 3; i++)
		if (ARG_TYPE(op, i + 1) == TREG) jit_set_remove(op->live_in, op->arg[i]);

	for (int i = 0; i < 3; i++)
		if (ARG_TYPE(op, i + 1) == REG) jit_set_add(op->live_in, op->arg[i]);

	if (GET_OP(op) == JIT_PROLOG) flw_analyze_prolog(jit, op, func_info);

	if (changed) return changed;
	if (live_out_size != jit_set_size(op->live_out)) return 1;
	if (live_in_size != jit_set_size(op->live_in)) return 1;
	 
	return 0;
}

static inline void analyze_function(struct jit *jit, jit_op *first_op, jit_op *last_op)
{
	int changed;
	struct code_refs_cache code_refs = { -1, NULL };

	struct jit_func_info * func_info = (struct jit_func_info *)first_op->arg[1];

	do {
		changed = 0;
		jit_op * op = last_op;
		while (1) {
			changed |= flw_analyze_op(jit, op, func_info, changed, &code_refs);
			if (op == first_op) break;
			op = op->prev;
		}
	} while (changed);
	if (code_refs.ops) JIT_FREE(code_refs.ops);

}

static inline void jit_flw_analysis(struct jit * jit)
{
	jit_flw_initialize(jit);
	jit_op *op = jit_op_first(jit->ops);

	while (op) {
		if (GET_OP(op) == JIT_PROLOG) {
			jit_op *first = op;
			while (1) {
				if ((op->next == NULL) || (GET_OP(op->next) == JIT_PROLOG)) {
					jit_op *second = op;
					analyze_function(jit, first, second);
					break;
				}
				op = op->next;
			}
		}
		op = op->next;
	}
}

static inline void mark_livecode(jit_op *op)
{
	while (op) {
		if (op->in_use) return;
		op->in_use = 1;
		if (op->jmp_addr) mark_livecode(op->jmp_addr);
		if (GET_OP(op) == JIT_RET) return;
		if (GET_OP(op) == JIT_FRET) return;
		if (GET_OP(op) == JIT_JMP) return;
		op = op->next;
	}	
}


static void jit_dead_code_analysis(struct jit *jit, int remove_dead_code) 
{
	for (jit_op *op = jit_op_first(jit->ops); op; op = op->next)
		op->in_use = 0;

	// marks ordinary operations
	for (jit_op *op = jit_op_first(jit->ops); op; op = op->next) {
		if (GET_OP(op) == JIT_PROLOG) mark_livecode(op);
		if (GET_OP(op) == JIT_DATA_REF_CODE) mark_livecode(op->jmp_addr);
	}


	// marks directives
	for (jit_op *op = jit_op_first(jit->ops); op; op = op->next) {
		if (GET_OP(op) == JIT_CODESTART) op->in_use = 1; 
		if (GET_OP(op) == JIT_DATA_BYTE) op->in_use = 1;
		if (GET_OP(op) == JIT_DATA_REF_CODE) op->in_use = 1; 
		if (GET_OP(op) == JIT_DATA_REF_DATA) op->in_use = 1; 
		if (GET_OP(op) == JIT_CODE_ALIGN) op->in_use = 1; 
		if (GET_OP(op) == JIT_LABEL) op->in_use = 1; 
		if (GET_OP(op) == JIT_PATCH) op->in_use = 1; 
		if (GET_OP(op) == JIT_COMMENT) op->in_use = 1; 
		if (GET_OP(op) == JIT_MARK) op->in_use = 1; 
	}

	if (!remove_dead_code) return;

	jit_op *op = jit_op_first(jit->ops);

	while (op) {
		if (!op->in_use) {
			if (GET_OP(op) == JIT_FULL_SPILL) goto skip; /* only marks whether code is accessible or not */
			jit_op *next = op->next;
			jit_op_delete(op);
			op = next;
			continue;
		} 
skip:
		op = op->next;
	}

}
