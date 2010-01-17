/*-
 * Copyright (c) 2010 Kai Wang
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 */

#include <assert.h>
#include "_libdwarf.h"

static struct _Dwarf_P_Expr_Entry *
_dwarf_add_expr(Dwarf_P_Expr expr, Dwarf_Small opcode, Dwarf_Unsigned val1,
    Dwarf_Unsigned val2, Dwarf_Error *error)
{
	struct _Dwarf_P_Expr_Entry *ee;
	int len;

	if (_dwarf_loc_expr_add_atom(expr->pe_dbg, NULL, NULL, opcode, val1,
	    val2, &len, error) != DWARF_E_NONE)
		return (NULL);
	assert(len > 0);

	if ((ee = calloc(1, sizeof(*ee))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (NULL);
	}

	STAILQ_INSERT_TAIL(&expr->pe_eelist, ee, ee_next);

	ee->ee_loc.lr_atom = opcode;
	ee->ee_loc.lr_number = val1;
	ee->ee_loc.lr_number2 = val2;
	ee->ee_loc.lr_offset = expr->pe_length;
	expr->pe_length += len;
	expr->pe_invalid = 1;

	return (ee);
}

static int
_dwarf_expr_into_block(Dwarf_P_Expr expr, Dwarf_Error *error)
{
	struct _Dwarf_P_Expr_Entry *ee;
	int len, pos, ret;

	if (expr->pe_block != NULL) {
		free(expr->pe_block);
		expr->pe_block = NULL;
	}

	if (expr->pe_length <= 0) {
		DWARF_SET_ERROR(error, DWARF_E_INVALID_EXPR);
		return (DWARF_E_INVALID_EXPR);
	}


	if ((expr->pe_block = calloc((size_t) expr->pe_length, 1)) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	pos = 0;
	STAILQ_FOREACH(ee, &expr->pe_eelist, ee_next) {
		assert((Dwarf_Unsigned) pos < expr->pe_length);
		ret = _dwarf_loc_expr_add_atom(expr->pe_dbg,
		    &expr->pe_block[pos], &expr->pe_block[expr->pe_length],
		    ee->ee_loc.lr_atom, ee->ee_loc.lr_number,
		    ee->ee_loc.lr_number2, &len, error);
		assert(ret == DWARF_E_NONE);
		assert(len > 0);
		pos += len;
	}

	expr->pe_invalid = 0;

	return (DWARF_E_NONE);
}

Dwarf_P_Expr
dwarf_new_expr(Dwarf_P_Debug dbg, Dwarf_Error *error)
{
	Dwarf_P_Expr pe;

	if (dbg == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_BADADDR);
	}

	if ((pe = calloc(1, sizeof(struct _Dwarf_P_Expr))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DW_DLV_BADADDR);
	}
	STAILQ_INIT(&pe->pe_eelist);

	STAILQ_INSERT_TAIL(&dbg->dbgp_pelist, pe, pe_next);
	pe->pe_dbg = dbg;

	return (pe);
}

Dwarf_Unsigned
dwarf_add_expr_gen(Dwarf_P_Expr expr, Dwarf_Small opcode, Dwarf_Unsigned val1,
    Dwarf_Unsigned val2, Dwarf_Error *error)
{

	if (expr == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_NOCOUNT);
	}

	if (_dwarf_add_expr(expr, opcode, val1, val2, error) == NULL)
		return (DW_DLV_NOCOUNT);

	return (expr->pe_length);
}

Dwarf_Unsigned
dwarf_add_expr_addr(Dwarf_P_Expr expr, Dwarf_Unsigned address,
    Dwarf_Signed sym_index, Dwarf_Error *error)
{

	return (dwarf_add_expr_addr_b(expr, address, sym_index, error));
}

Dwarf_Unsigned
dwarf_add_expr_addr_b(Dwarf_P_Expr expr, Dwarf_Unsigned address,
    Dwarf_Unsigned sym_index, Dwarf_Error *error)
{
	struct _Dwarf_P_Expr_Entry *ee;

	if (expr == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_NOCOUNT);
	}

	if ((ee = _dwarf_add_expr(expr, DW_OP_addr, address, 0, error)) == NULL)
		return (DW_DLV_NOCOUNT);

	ee->ee_sym = sym_index;

	return (expr->pe_length);
}

Dwarf_Unsigned
dwarf_expr_current_offset(Dwarf_P_Expr expr, Dwarf_Error *error)
{

	if (expr == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return (DW_DLV_NOCOUNT);
	}

	return (expr->pe_length);
}

Dwarf_Addr
dwarf_expr_into_block(Dwarf_P_Expr expr, Dwarf_Unsigned *length,
    Dwarf_Error *error)
{

	if (expr == NULL || length == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_ARGUMENT);
		return ((Dwarf_Addr) DW_DLV_BADADDR);
	}

	if (expr->pe_block == NULL || expr->pe_invalid)
		if (_dwarf_expr_into_block(expr, error) != DWARF_E_NONE)
			return ((Dwarf_Addr) DW_DLV_BADADDR);

	*length = expr->pe_length;

	return ((Dwarf_Addr) expr->pe_block);
}