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

#ifndef JITLIB_DEBUG_H
#define JITLIB_DEBUG_H

#include "jitlib.h"
#include "llrb.h"
#include "jitlib-core.h"

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

int print_op(FILE *, struct jit_disasm *, struct jit_op *, jit_tree *, int);

#endif /* JITLIB_DEBUG_H */
