/*
 * MyJIT 
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


/**
 * computes number of 1's in the given binary number
 * this was taken from the Hacker's Delight book by Henry S. Warren
 */
static inline int _bit_pop(unsigned int x) {
        x = (x & 0x55555555) + ((x >> 1) & 0x55555555);
        x = (x & 0x33333333) + ((x >> 2) & 0x33333333);
        x = (x & 0x0F0F0F0F) + ((x >> 4) & 0x0F0F0F0F);
        x = (x & 0x00FF00FF) + ((x >> 8) & 0x00FF00FF);
        x = (x & 0x0000FFFF) + ((x >>16) & 0x0000FFFF);
        return x;
}

static int uses_hw_reg(struct jit_op * op, jit_value reg, int fp)
{
	if ((GET_OP(op) == JIT_RENAMEREG) && (op->r_arg[0] == reg)) return 1; // not a regular operation
	for (int i = 0; i < 3; i++)
		if ((ARG_TYPE(op, i + 1) == REG) || (ARG_TYPE(op, i + 1) == TREG)) {
			if (fp && (JIT_REG_TYPE(op->arg[i]) == JIT_RTYPE_INT)) continue;
			if (!fp && (JIT_REG_TYPE(op->arg[i]) == JIT_RTYPE_FLOAT)) continue;
			if (op->r_arg[i] == reg) return 1;
		}
	return 0;
}
