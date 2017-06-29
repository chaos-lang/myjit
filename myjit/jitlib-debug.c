/*
 * MyJIT 
 * Copyright (C) 2010, 2015, 2017, 2018 Petr Krajca, <petr.krajca@upol.cz>
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
#include <stdlib.h>
#include <stdarg.h>
#include <assert.h>
#include <stdarg.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define ABS(x)	((x) < 0 ? - (x) : x)

#include "llrb.c"

#define OUTPUT_BUF_SIZE         (8192)
//#define print_padding(buf, size) while (strlen((buf)) < (size)) { strcat((buf), " "); }

static jit_hw_reg * rmap_is_associated(jit_rmap * rmap, int reg_id, int fp, jit_value * virt_reg);

/*
static inline int bufprint(char *buf, const char *format, ...) {
	va_list ap;
	va_start(ap, format);
	int result = vsprintf(buf + strlen(buf), format, ap);
	va_end(ap);
	return result;
}
*/
struct output_buf
{
	char *buf;
	size_t size;
	size_t capacity;
};

static struct output_buf *ob_new()
{
	struct output_buf *ob = malloc(sizeof(struct output_buf));
	ob->buf = malloc(OUTPUT_BUF_SIZE);
	ob->capacity = OUTPUT_BUF_SIZE;
	ob->size = 0;
	ob->buf[0] = '\0';
	return ob;
}

static void ob_free(struct output_buf *ob)
{
	free(ob->buf);
	free(ob);
}

static void ob_assert_space(struct output_buf *ob, int len)
{
	if (ob->size + len + 2 > ob->capacity) {
		ob->capacity += len + OUTPUT_BUF_SIZE;
		ob->buf = realloc(ob->buf, ob->capacity);
	}
}

static void ob_printf(struct output_buf *ob, const char *fmt, ...)
{
	char str[OUTPUT_BUF_SIZE];

	va_list ap;
	va_start(ap, fmt);
	int len = vsnprintf(str, OUTPUT_BUF_SIZE, fmt, ap);
	va_end(ap);

	len = strlen(str);
	ob_assert_space(ob, len);
	strcat(ob->buf, str);
	ob->size += len;
}

static void ob_append(struct output_buf *ob, char *str)
{
	int len = strlen(str);
	ob_assert_space(ob, len);
	strcat(ob->buf, str);
	ob->size += len;
}

static void ob_pad(struct output_buf *ob, int size)
{
	while (strlen(ob->buf) < (size))
		ob_append(ob, " ");
}



static void compiler_based_debugger(struct jit * jit)
{
	char obj_file_name[] = "myjitXXXXXX";
	int obj_file_fd = mkstemp(obj_file_name);	


#ifndef __APPLE_CC__
	char * cmd1_fmt = "gcc -x assembler -c -o %s -";
	#ifdef JIT_ARCH_ARM32
	char * cmd2_fmt = "objdump -d %s";
	#else
	char * cmd2_fmt = "objdump -d -M intel %s";
	#endif
#else
	char * cmd1_fmt = "cc -x assembler -c -o %s -";
	char * cmd2_fmt = "otool -tvVj %s";
#endif

	char cmd1[strlen(cmd1_fmt) + strlen(obj_file_name) + 1];
	char cmd2[strlen(cmd2_fmt) + strlen(obj_file_name) + 1];

	sprintf(cmd1, cmd1_fmt, obj_file_name);

	FILE * f = popen(cmd1, "w");

	int size = jit->ip - jit->buf;
#ifndef __APPLE_CC__
	#ifndef JIT_ARCH_ARM32
	fprintf (f, ".text\n.align 4\n.globl main\n.type main,@function\nmain:\n");
	#else
	fprintf (f, ".text\n.globl main\n.type main,STT_FUNC\nmain:\n");
	#endif
	for (int i = 0; i < size; i++)
		fprintf(f, ".byte %d\n", (unsigned int) jit->buf[i]);
#else
// clang & OS X
	fprintf (f, ".text\n.align 4\n.globl main\n\nmain:\n");
	for (int i = 0; i < size; i++)
		fprintf(f, ".byte 0x%x\n", (unsigned int) jit->buf[i]);
#endif
	pclose(f);
	

	sprintf(cmd2, cmd2_fmt, obj_file_name);
	system(cmd2);

	close(obj_file_fd);
	unlink(obj_file_name);
}

typedef struct jit_disasm {
	char *indent_template;
	char *reg_template;
	char *freg_template;
	char *arg_template;
	char *farg_template;
	char *reg_fp_template;
	char *reg_out_template;
	char *reg_imm_template;
	char *reg_fimm_template;
	char *reg_unknown_template;
	char *label_template;
	char *label_forward_template;
	char *generic_addr_template;
	char *generic_value_template;
} jit_disasm;

struct jit_disasm jit_disasm_general = {
	.indent_template = "    ",   
	.reg_template = "r%i",
	.freg_template = "fr%i",
	.arg_template = "arg%i",
	.farg_template = "farg%i",
	.reg_out_template = "out",
	.reg_fp_template = "fp",
	.reg_imm_template = "imm",
	.reg_fimm_template = "fimm",
	.reg_unknown_template = "(unknown reg.)",
	.label_template = "L%i",
	.label_forward_template = "L%i",
	.generic_addr_template = "<addr: 0x%lx>",
	.generic_value_template = "0x%lx",
};

struct jit_disasm jit_disasm_compilable = {
	.indent_template = "    ",   
	.reg_template = "R(%i)",
	.freg_template = "FR(%i)",
	.arg_template = "arg(%i)",
	.farg_template = "farg(%i)",
	.reg_fp_template = "R_FP",
	.reg_out_template = "R_OUT",
	.reg_imm_template = "R_IMM",
	.reg_fimm_template = "FR_IMM",
	.reg_unknown_template = "(unknown reg.)",
	.label_template = "label_%03i",
	.label_forward_template = "/* label_%03i */ JIT_FORWARD",
	.generic_addr_template = "<addr: 0x%lx>",
	.generic_value_template = "%li",
};



char * jit_get_op_name(struct jit_op * op)
{
	switch (GET_OP(op)) {
		case JIT_MOV:	return "mov";
		case JIT_LD:	return "ld";
		case JIT_LDX:	return "ldx";
		case JIT_ST:	return "st";
		case JIT_STX:	return "stx";
		case JIT_MEMCPY:return "memcpy";
		case JIT_MEMSET:return "memset";

		case JIT_JMP:		return "jmp";
		case JIT_PATCH:		return ".patch";
		case JIT_PREPARE:	return "prepare";
		case JIT_PUTARG:	return "putarg";
		case JIT_CALL:		return "call";
		case JIT_RET:		return "ret";
		case JIT_PROLOG:	return "prolog";
		case JIT_GETARG:	return "getarg";
		case JIT_RETVAL:	return "retval";
		case JIT_ALLOCA:	return "alloca";
		case JIT_DECL_ARG:	return "declare_arg";

		case JIT_ADD:	return "add";
		case JIT_ADDC:	return "addc";
		case JIT_ADDX:	return "addx";
		case JIT_SUB:	return "sub";
		case JIT_SUBC:	return "subc";
		case JIT_SUBX:	return "subx";
		case JIT_RSB:	return "rsb";
		case JIT_NEG:	return "neg";
		case JIT_MUL:	return "mul";
		case JIT_HMUL:	return "hmul";
		case JIT_DIV:	return "div";
		case JIT_MOD:	return "mod";

		case JIT_OR:	return "or";
		case JIT_XOR:	return "xor";
		case JIT_AND:	return "and";
		case JIT_LSH:	return "lsh";
		case JIT_RSH:	return "rsh";
		case JIT_NOT:	return "not";

		case JIT_LT:	return "lt";
		case JIT_LE:	return "le";
		case JIT_GT:	return "gt";
		case JIT_GE:	return "ge";
		case JIT_EQ:	return "eq";
		case JIT_NE:	return "ne";

		case JIT_BLT:	return "blt";
		case JIT_BLE:	return "ble";
		case JIT_BGT:	return "bgt";
		case JIT_BGE:	return "bge";
		case JIT_BEQ:	return "beq";
		case JIT_BNE:	return "bne";
		case JIT_BMS:	return "bms";
		case JIT_BMC:	return "bmc";
		case JIT_BOADD:	return "boadd";
		case JIT_BOSUB:	return "bosub";
		case JIT_BNOADD:return "bnoadd";
		case JIT_BNOSUB:return "bnosub";

		case JIT_UREG:		return ".ureg";
		case JIT_LREG:		return ".lreg";
		case JIT_CODESTART:	return ".code";
		case JIT_LABEL:		return ".label";
		case JIT_SYNCREG:	return ".syncreg";
		case JIT_RENAMEREG:	return ".renamereg";
		case JIT_MSG:		return "msg";
		case JIT_COMMENT:	return ".comment";
		case JIT_NOP:		return "nop";
		case JIT_CODE_ALIGN:	return ".align";
		case JIT_DATA_BYTE:	return ".byte";
		case JIT_DATA_BYTES:	return ".bytes";
		case JIT_DATA_REF_CODE:	return ".ref_code";
		case JIT_DATA_REF_DATA:	return ".ref_data";
		case JIT_REF_CODE:	return "ref_code";
		case JIT_REF_DATA:	return "ref_data";
		case JIT_FULL_SPILL:	return ".full_spill";
		case JIT_TRACE:		return ".trace";
		case JIT_FORCE_SPILL:	return "force_spill";
		case JIT_FORCE_ASSOC:	return "force_assoc";
		case JIT_MARK:	return "mark";
		case JIT_TOUCH:	return "touch";

		case JIT_TRANSFER: return "transfer";
		case JIT_TRANSFER_CPY: return "transfer_cpy";
		case JIT_TRANSFER_AND: return "transfer_and";
		case JIT_TRANSFER_OR:  return "transfer_or";
		case JIT_TRANSFER_XOR: return "transfer_xor";
		case JIT_TRANSFER_ADD: return "transfer_add";
		case JIT_TRANSFER_SUB: return "transfer_sub";

		case JIT_FMOV:	return "fmov";
		case JIT_FADD: 	return "fadd";
		case JIT_FSUB: 	return "fsub";
		case JIT_FRSB: 	return "frsb";
		case JIT_FMUL: 	return "fmul";
		case JIT_FDIV: 	return "fdiv";
		case JIT_FNEG: 	return "fneg";
		case JIT_FRETVAL: return "fretval";
		case JIT_FPUTARG: return "fputarg";

		case JIT_EXT: 	return "ext";
		case JIT_ROUND: return "round";
		case JIT_TRUNC: return "trunc";
		case JIT_FLOOR: return "floor";
		case JIT_CEIL: 	return "ceil";

		case JIT_FBLT: return "fblt";
		case JIT_FBLE: return "fble";
		case JIT_FBGT: return "fbgt";
		case JIT_FBGE: return "fbge";
		case JIT_FBEQ: return "fbeq";
		case JIT_FBNE: return "fbne";

		case JIT_FLD: 	return "fld";
		case JIT_FLDX:  return "fldx";
		case JIT_FST:	return "fst";
		case JIT_FSTX:	return "fstx";

		case JIT_FRET: return "fret";
		default: return "(unknown)";
	}
}

void jit_get_reg_name(struct jit_disasm *disasm, char *r, int reg)
{
	if (reg == R_FP) strcpy(r, disasm->reg_fp_template);
	else if (reg == R_OUT) strcpy(r, disasm->reg_out_template);
	else if (reg == R_IMM) strcpy(r, disasm->reg_imm_template);
	else if (reg == FR_IMM) strcpy(r, disasm->reg_fimm_template);
	else {
		if (JIT_REG_SPEC(reg) == JIT_RTYPE_REG) {
			if (JIT_REG_TYPE(reg) == JIT_RTYPE_INT) sprintf(r, disasm->reg_template, JIT_REG_ID(reg));
			else sprintf(r, disasm->freg_template, JIT_REG_ID(reg));
		}
		else if (JIT_REG_SPEC(reg) == JIT_RTYPE_ARG) {
			if (JIT_REG_TYPE(reg) == JIT_RTYPE_INT) sprintf(r, disasm->arg_template, JIT_REG_ID(reg));
			else sprintf(r, disasm->farg_template, JIT_REG_ID(reg));
		} else sprintf(r, "%s", disasm->reg_unknown_template);
	} 
}

static void print_rmap_callback(jit_tree_key key, jit_tree_value value, void *disasm)
{
	char buf[256];
	jit_get_reg_name((struct jit_disasm *)disasm, buf, key);
	printf("%s=%s ", buf, ((jit_hw_reg *)value)->name);
}

static void print_reg_liveness_callback(jit_tree_key key, jit_tree_value value, void *disasm)
{
	char buf[256];
	jit_get_reg_name(disasm, buf, key);
	printf("%s ", buf);
}

#define print_rmap(disasm, n) jit_tree_walk(n, print_rmap_callback, disasm)
#define print_reg_liveness(disasm, n) jit_tree_walk(n, print_reg_liveness_callback, disasm)


static inline int jit_op_is_cflow(jit_op * op)
{
	if (((GET_OP(op) == JIT_CALL) || (GET_OP(op) == JIT_JMP)) && (IS_IMM(op))) return 1;
	if (is_cond_branch_op(op)) return 1;
 
	return 0;
}

static jit_tree * prepare_labels(struct jit * jit)
{
	long x = 1;
	jit_tree * n = NULL;
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		if (GET_OP(op) == JIT_PATCH) {
			n = jit_tree_insert(n, (long) op, (void *)x, NULL);
			n = jit_tree_insert(n, op->arg[0], (void *) -x, NULL);
			x++;
		}
		if (GET_OP(op) == JIT_LABEL) {
			n = jit_tree_insert(n, op->arg[0], (void *)x, NULL);
			x++;
		}
	}
	return n;
}

static inline void print_addr(struct jit_disasm *disasm, struct output_buf *buf, jit_tree *labels, jit_op *op, int arg_pos)
{
	void *arg = (void *)op->arg[arg_pos];

	jit_tree *label_item = jit_tree_search(labels, (long) op);
	if (label_item) ob_printf(buf, disasm->label_forward_template, - (long) label_item->value);
	else {
		label_item = jit_tree_search(labels, (long) arg);
		if (label_item) ob_printf(buf, disasm->label_template, (long) label_item->value);
		else ob_printf(buf, disasm->generic_addr_template, arg);
	}
}

static inline void print_arg(struct jit_disasm *disasm, struct output_buf *buf, struct jit_op *op, int arg)
{
	long a = op->arg[arg - 1];
	if (ARG_TYPE(op, arg) == IMM) ob_printf(buf, disasm->generic_value_template, a);
	if ((ARG_TYPE(op, arg) == REG) || (ARG_TYPE(op, arg) == TREG)) {
		char value[256];
		jit_get_reg_name(disasm, value, a);
		ob_append(buf, value);
	}
}

static inline void print_str(struct output_buf *buf, char *str)
{
	ob_append(buf, " \"");
	for (int i = 0; i < strlen(str); i++) {
		if (str[i] >= 32) {
		//	int s = strlen(buf);
			ob_printf(buf, "%c", str[i]);
		//	buf[s++] = str[i];
		//	buf[s] = '\0';
		} else  {
			char xbuf[16];
			switch (str[i]) {
				case 9: strcpy(xbuf, "\\t"); break;
				case 10: strcpy(xbuf, "\\n"); break;
				case 13: strcpy(xbuf, "\\r"); break;
				default: sprintf(xbuf, "\\x%02x", str[i]);
			}
			ob_append(buf, xbuf);
		}
	}
	ob_append(buf, "\"");
}

static void print_args(struct jit_disasm *disasm, struct output_buf *linebuf, jit_op *op, jit_tree *labels) 
{
	for (int i = 1; i <= 3; i++) {
		if (ARG_TYPE(op, i) == NO) continue;
		ob_append(linebuf, i == 1 ? " " : ", ");
		if ((i == 1) && jit_op_is_cflow(op)) print_addr(disasm, linebuf, labels, op, 0); 
		else print_arg(disasm, linebuf, op, i);
	}
}

void print_full_op_name(struct output_buf *linebuf, jit_op *op)
{
	char *op_name = jit_get_op_name(op);
	ob_append(linebuf, op_name);
	if ((GET_OP(op) == JIT_CALL) && (GET_OP_SUFFIX(op) & IMM)) return;
	if (GET_OP_SUFFIX(op) & IMM) ob_append(linebuf, "i");
	if (GET_OP_SUFFIX(op) & REG) ob_append(linebuf, "r");
	if (GET_OP_SUFFIX(op) & UNSIGNED) ob_append(linebuf, "_u");
}

static int print_load_op(struct jit_disasm *disasm, struct output_buf *linebuf, jit_op *op)
{
	char rbuf[256];
	switch (GET_OP(op)) {
		case JIT_LREG:
			ob_append(linebuf, disasm->indent_template);
			ob_append(linebuf, jit_get_op_name(op));
			ob_pad(linebuf, 13);
			jit_get_reg_name(disasm, rbuf, op->arg[1]);
			ob_append(linebuf, rbuf);
			return 1;
		case JIT_UREG:
		case JIT_SYNCREG:
			ob_append(linebuf, disasm->indent_template);
			ob_append(linebuf, jit_get_op_name(op));
			ob_pad(linebuf, 13);
			jit_get_reg_name(disasm, rbuf, op->arg[0]);
			ob_append(linebuf, rbuf);
			return 1;
		case JIT_RENAMEREG: {
				jit_value reg;
				rmap_is_associated(op->prev->regmap, op->arg[1], 0, &reg);
				ob_append(linebuf, disasm->indent_template);
				ob_append(linebuf, jit_get_op_name(op));
				ob_append(linebuf, " ");
				ob_pad(linebuf, 13);
				jit_get_reg_name(disasm, rbuf, reg);
				ob_append(linebuf, rbuf);
				return 1;
			}
		case JIT_FULL_SPILL:
			ob_append(linebuf, disasm->indent_template);
			ob_append(linebuf, jit_get_op_name(op));
			return 1;
		default:
			return 0;

	}
}

void print_comment(struct output_buf *linebuf, jit_op *op)
{
	char *str = (char *)op->arg[0];
	ob_append(linebuf, "// ");	
	for (int i = 0; i < strlen(str); i++) {
		if ((str[i] == '\r') || (str[i] == '\n')) ob_append(linebuf, "\n// ");
		else ob_printf(linebuf, "%c", str[i]);
	}
}


int print_op(FILE *f, struct jit_disasm * disasm, struct jit_op *op, jit_tree *labels, int verbosity)
{
	//char linebuf[OUTPUT_BUF_SIZE];
	//linebuf[0] = '\0';
	struct output_buf *linebuf = ob_new();

	if ((GET_OP(op) == JIT_LABEL) || (GET_OP(op) == JIT_PATCH)) {
		jit_tree * lab = jit_tree_search(labels, (long)op->arg[0]);
		if (lab) {
			ob_printf(linebuf, disasm->label_template, ABS((long)lab->value));
			ob_printf(linebuf, ":");
		}
		goto print;
	}

	if (GET_OP(op) == JIT_COMMENT) {
		print_comment(linebuf, op);
		goto print;
	}

	if (GET_OP(op) == JIT_TRACE) {
		ob_append(linebuf, disasm->indent_template);
		ob_append(linebuf, ".trace");
		goto print;
	}

	
	char * op_name = jit_get_op_name(op);
	if ((op_name[0] == '.') && (verbosity & JIT_DEBUG_LOADS)) {
		if (print_load_op(disasm, linebuf, op)) goto print;
	}

	ob_append(linebuf, disasm->indent_template);
	if (op_name[0] == '.') {
		switch (GET_OP(op)) {
			case JIT_DATA_BYTE:
			case JIT_CODE_ALIGN:
				ob_printf(linebuf, "%s ", op_name);
				ob_pad(linebuf, 13);
				ob_printf(linebuf, disasm->generic_value_template, op->arg[0]);
				goto print;
			case JIT_DATA_BYTES:
				ob_printf(linebuf, "%s ", op_name);
				ob_pad(linebuf, 13);
				for (int i = 0; i < op->arg[0]; i++) {
					ob_printf(linebuf, disasm->generic_value_template, ((unsigned char *)op->addendum)[i]);
					ob_printf(linebuf, " ");
				}
				goto print;

			case JIT_DATA_REF_CODE:
			case JIT_DATA_REF_DATA:
				ob_printf(linebuf, "%s ", op_name);
				ob_pad(linebuf, 13);
				print_addr(disasm, linebuf, labels, op, 0); 
				goto print;
			default: 
				goto print;
				
		}
	}
	print_full_op_name(linebuf, op);

	ob_pad(linebuf, 12);

	if (op->arg_size == 1) ob_append(linebuf, " (byte)");
	if (op->arg_size == 2) ob_append(linebuf, " (word)");
	if (op->arg_size == 4) ob_append(linebuf, " (dword)");
	if (op->arg_size == 8) ob_append(linebuf, " (qword)");

	switch (GET_OP(op)) {
		case JIT_PREPARE: break;
		case JIT_MSG:
			print_str(linebuf, (char *)op->arg[0]);
			if (!IS_IMM(op)) {
				ob_append(linebuf, ", ");
				print_arg(disasm, linebuf, op, 2);
			}
			break;
		case JIT_REF_CODE:
		case JIT_REF_DATA:
			ob_append(linebuf, " ");
			print_arg(disasm, linebuf, op, 1);
			ob_append(linebuf, ", ");
			print_addr(disasm, linebuf, labels, op, 1); 
			break;
		case JIT_DECL_ARG:
			switch (op->arg[0]) {
				case JIT_SIGNED_NUM:   ob_append(linebuf, " integer"); break;
				case JIT_UNSIGNED_NUM: ob_append(linebuf, " uns. integer"); break;
				case JIT_FLOAT_NUM:    ob_append(linebuf, " float"); break;
				case JIT_PTR:          ob_append(linebuf, " ptr"); break;
				default: assert(0);
			};
			ob_append(linebuf, ", ");
			print_arg(disasm, linebuf, op, 2);
			break;
		default:
			print_args(disasm, linebuf, op, labels);
	}
print:
	fprintf(f, "%s", linebuf->buf);
	int len = strlen(linebuf->buf);
	ob_free(linebuf); 
	return len;
}


#define OP_ID(op) (((unsigned long) (op)) >> 4)

int print_op_compilable(struct jit_disasm *disasm, struct jit_op * op, jit_tree * labels)
{
	struct output_buf *linebuf = ob_new();

	jit_tree * lab = jit_tree_search(labels, (long)op);
	if (lab && ((long) lab->value > 0)) {
		ob_printf(linebuf, "// ");
		ob_printf(linebuf, disasm->label_template, (long)lab->value);
		ob_printf(linebuf, ":\n");
	}

	if (GET_OP(op) == JIT_COMMENT) {
		print_comment(linebuf, op);
		goto direct_print;
	}


	ob_append(linebuf, disasm->indent_template);

	if ((jit_op_is_cflow(op) && ((void *)op->arg[0] == JIT_FORWARD))
	|| (GET_OP(op) == JIT_REF_CODE) || (GET_OP(op) == JIT_REF_DATA)
	|| (GET_OP(op) == JIT_DATA_REF_CODE) || (GET_OP(op) == JIT_DATA_REF_DATA))
		ob_printf(linebuf, "jit_op * op_%li = ", OP_ID(op));

	switch (GET_OP(op)) {
		case JIT_LABEL: {
			ob_printf(linebuf, "jit_label * ");

			jit_tree * lab = jit_tree_search(labels, (long)op->arg[0]);
			if (lab) ob_printf(linebuf, disasm->label_template, (long) lab->value);
			ob_printf(linebuf, " = jit_get_label(p");
			goto print;
		}
		case JIT_PATCH:
			ob_printf(linebuf, "jit_patch  (p, op_%li", OP_ID(op->arg[0]));
			goto print;

		case JIT_DATA_BYTE:
			ob_printf(linebuf, "jit_data_byte(p, ");
			ob_printf(linebuf, disasm->generic_value_template, op->arg[0]);
			goto print;
		case JIT_DATA_BYTES:
			for (int i = 0; i < op->arg[0]; i++) {
				ob_printf(linebuf, "jit_data_byte(p, ");
				ob_printf(linebuf, disasm->generic_value_template, ((unsigned char *)op->addendum) [i]);
				if (i < op->arg[0] - 1) ob_printf(linebuf, ");\n");
			}	
			goto print;
		case JIT_REF_CODE:
		case JIT_REF_DATA:
			ob_printf(linebuf, "jit_%s(p, ", jit_get_op_name(op));
			print_arg(disasm, linebuf, op, 1);
			ob_append(linebuf, ", ");
			print_addr(disasm, linebuf, labels, op, 1); 
			goto print;
		case JIT_DATA_REF_CODE:
		case JIT_DATA_REF_DATA:
			ob_printf(linebuf, "jit_data_%s(p, ", jit_get_op_name(op) + 1); // +1 skips leading '.'
			print_addr(disasm, linebuf, labels, op, 0); 
			goto print;
		case JIT_CODE_ALIGN:
			ob_printf(linebuf, "jit_code_align  (p, ");
			ob_printf(linebuf, disasm->generic_value_template, op->arg[0]);
			goto print;
		case JIT_PREPARE:
			ob_printf(linebuf, "jit_prepare(p");
			goto print;
		default:
			break;
	}

	if (jit_get_op_name(op)[0] == '.') goto direct_print;

	ob_append(linebuf, "jit_");
	print_full_op_name(linebuf, op);

	ob_pad(linebuf, 15);

	ob_append(linebuf, "(p,");

	switch (GET_OP(op)) {
		case JIT_MSG:
			print_str(linebuf, (char *)op->arg[0]);
			if (!IS_IMM(op)) {
				ob_append(linebuf, ", ");
				print_arg(disasm, linebuf, op, 2);
			}
			break;
		case JIT_DECL_ARG:
			switch (op->arg[0]) {
				case JIT_SIGNED_NUM: ob_append(linebuf, "JIT_SIGNED_NUM"); break;
				case JIT_UNSIGNED_NUM: ob_append(linebuf, "JIT_UNSIGNED_NUM"); break;
				case JIT_FLOAT_NUM: ob_append(linebuf, "JIT_FLOAT_NUM"); break;
				case JIT_PTR: ob_append(linebuf, "JIT_PTR"); break;
				default: assert(0);
			};
			ob_append(linebuf, ", ");
			print_arg(disasm, linebuf, op, 2);
			break;
		default:
			print_args(disasm, linebuf, op, labels);
	}
	
	if (op->arg_size) ob_printf(linebuf, ", %i", op->arg_size);
	
print:
	ob_append(linebuf, ");");
direct_print:
	printf("%s", linebuf->buf);
	int len = linebuf->size;
	ob_free(linebuf);
	return len;
}

static void jit_dump_ops_compilable(struct jit *jit, jit_tree *labels)
{
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		print_op_compilable(&jit_disasm_compilable, op, labels);
		printf("\n");
	}
}

static void jit_dump_ops_general(struct jit *jit, jit_tree *labels, int verbosity)
{
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		int size = print_op(stdout, &jit_disasm_general, op, labels, verbosity);

		if (size == 0) return;

		for (; size < 35; size++)
			printf(" ");

		if ((verbosity & JIT_DEBUG_LIVENESS) && (op->live_in) && (op->live_out)) {
			printf("In: ");
			print_reg_liveness(&jit_disasm_general, op->live_in->root);
			printf("\tOut: ");
			print_reg_liveness(&jit_disasm_general, op->live_out->root);
		}

		if ((verbosity & JIT_DEBUG_ASSOC) && (op->regmap)) {
			printf("\tAssoc: ");
			print_rmap(&jit_disasm_general, op->regmap->map);
		}

		printf("\n");
	}
}

static char *platform_id()
{
#ifdef JIT_ARCH_AMD64
	return "amd64";
#elif defined(JIT_ARCH_I386)
	return "i386";
#elif defined(JIT_ARCH_ARM32)
	return "arm32";
#else
	return "sparc";
#endif
}

static inline void print_op_bytes(FILE *f, struct jit *jit, jit_op *op) {
	for (int i = 0; i < op->code_length; i++)
		fprintf(f, " %02x", jit->buf[op->code_offset + i]);
	fprintf(f, "\n.nl\n");
}

static FILE *open_disasm()
{
	int fds[2];
	pipe(fds);

	pid_t child = fork();
	if (child == 0) {
		close(fds[1]); // write end
		dup2(fds[0], STDIN_FILENO);

		char *path;
		
		path = "./myjit-disasm";
		execlp(path, path, NULL);

		path = "myjit-disasm";
		execlp(path, path, NULL);

		path = getenv("MYJIT_DISASM");
		if (path) execlp(path, path, NULL);

		printf("myjit-disasm not found\n\n");
		printf("In order to list myjit operations along with the machine code, the MyJIT disassembler has to be present in the current directory or its path has to be specified in the MYJIT_DISASM environment variable.\nThe disassembler's source code can be found in the \"disasm/\" directory.\n\n");
		exit(1);
	}

	// parent
	close(fds[0]); // read end
	FILE * f = fdopen(fds[1], "w");
	return f;
}

static jit_op *print_combined_op(FILE *f, struct jit *jit, struct jit_op *op, jit_tree *labels)
{
	jit_opcode opcode = GET_OP(op);
	if ((opcode == JIT_DATA_BYTE) || (opcode == JIT_DATA_BYTES)) {
		fprintf(f, ".text\n%s.byte\n", jit_disasm_general.indent_template);
		fprintf(f, ".data\n");
		while (op && ((GET_OP(op) == JIT_DATA_BYTE) || (GET_OP(op) == JIT_DATA_BYTES))) {
			if (GET_OP(op) == JIT_DATA_BYTE) fprintf(f, "%02x ", (unsigned char) op->arg[0]);
			if (GET_OP(op) == JIT_DATA_BYTES) {
				for (int i = 0; i < op->arg[0]; i++)
					fprintf(f, "%02x ", ((unsigned char *) op->addendum)[i]);
			}
			
			op = op->next;
		}
		fprintf(f, "\n");

		if (!op) return NULL;
		op = op->prev;
		return op;	
	}

	if (opcode == JIT_COMMENT) {
		fprintf(f, ".comment\n");
		print_op(f, &jit_disasm_general, op, labels, JIT_DEBUG_LOADS);
		fprintf(f, "\n");
		return op;
	}

	fprintf(f, ".text\n");
	print_op(f, &jit_disasm_general, op, labels, JIT_DEBUG_LOADS);
	fprintf(f, "\n");

	switch (opcode) {
		case JIT_CODE_ALIGN:
			if (op->next) {
				fprintf(f, "\n.nl\n");
				fprintf(f, ".addr=%lx\n", (unsigned long) (jit->buf + op->next->code_offset));
			}
			break;
		case JIT_DATA_REF_CODE:
		case JIT_DATA_REF_DATA:
			fprintf(f, ".data\n");
			print_op_bytes(f, jit, op);
			break;
		default:
			if (!op->code_length) break;
			fprintf(f, ".%s\n", platform_id());
			print_op_bytes(f, jit, op);
	}
	return op;
}


static void jit_dump_ops_combined(struct jit *jit, jit_tree *labels)
{
	FILE *f = open_disasm();

	fprintf(f, ".addr=%lx\n", (unsigned long)jit->buf);
	for (jit_op * op = jit_op_first(jit->ops); op != NULL; op = op->next) {
		op = print_combined_op(f, jit, op, labels);
		if (!op) break;
	}

	fclose(f);
	wait(NULL);
}

void jit_dump_ops(struct jit * jit, int verbosity)
{
	if (!(verbosity & (JIT_DEBUG_CODE | JIT_DEBUG_COMPILABLE | JIT_DEBUG_COMBINED)))
		verbosity |= JIT_DEBUG_OPS;

	jit_tree * labels = prepare_labels(jit);
	if (verbosity & JIT_DEBUG_OPS) jit_dump_ops_general(jit, labels, verbosity);
	if (verbosity & JIT_DEBUG_CODE) compiler_based_debugger(jit);
	if (verbosity & JIT_DEBUG_COMPILABLE) jit_dump_ops_compilable(jit, labels);
	if (verbosity & JIT_DEBUG_COMBINED) jit_dump_ops_combined(jit, labels);
	jit_tree_free(labels);
}

void jit_trace_op(struct jit *jit, jit_op *op, int verbosity)
{
	jit_tree * labels = prepare_labels(jit);
	if (verbosity & JIT_DEBUG_OPS) {
		print_op(stdout, &jit_disasm_general, op, labels, verbosity);
		printf("\n");
	}
	if (verbosity & JIT_DEBUG_COMBINED) {
		FILE *f = open_disasm();
		fprintf(f, "..addr=%lx\n", (unsigned long) op->code_offset);
		print_combined_op(f, jit, op, labels);
		fclose(f);
		wait(NULL);
	}
	
	jit_tree_free(labels);	
}

#define TRACE_PREV      (1)
#define TRACE_NEXT      (2)

void jit_trace_callback(struct jit *jit, jit_op *op, int verbosity, int trace) 
{
	if (trace & TRACE_PREV) jit_trace_op(jit, op->prev, verbosity);
	if (trace & TRACE_NEXT) jit_trace_op(jit, op->next, verbosity);
}
