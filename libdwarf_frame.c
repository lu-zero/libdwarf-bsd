/*-
 * Copyright (c) 2009 Kai Wang
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

#include "_libdwarf.h"
#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

static int
_dwarf_frame_find_cie(Dwarf_FrameSec fs, Dwarf_Unsigned offset,
    Dwarf_Cie *ret_cie)
{
	Dwarf_Cie cie;

	STAILQ_FOREACH(cie, &fs->fs_cielist, cie_next) {
		if (cie->cie_offset == offset)
			break;
	}

	if (cie == NULL)
		return (DWARF_E_NO_ENTRY);

	if (ret_cie != NULL)
		*ret_cie = cie;

	return (DWARF_E_NONE);
}

static int
_dwarf_frame_add_cie(Dwarf_Debug dbg, Dwarf_FrameSec fs, Dwarf_Section *ds,
    Dwarf_Unsigned *off, Dwarf_Cie *ret_cie, Dwarf_Error *error)
{
	Dwarf_Cie cie;
	uint64_t length;
	int dwarf_size;
	char *p;

	/* Check if we already added this CIE. */
	if (_dwarf_frame_find_cie(fs, *off, NULL) != DWARF_E_NO_ENTRY)
		return (DWARF_E_NONE);

	if ((cie = calloc(1, sizeof(struct _Dwarf_Cie))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	STAILQ_INSERT_TAIL(&fs->fs_cielist, cie, cie_next);

	cie->cie_index = fs->fs_cielen;
	cie->cie_offset = *off;

	length = dbg->read(ds->ds_data, off, 4);
	if (length == 0xffffffff) {
		dwarf_size = 8;
		length = dbg->read(ds->ds_data, off, 8);
	} else
		dwarf_size = 4;

	if (length > ds->ds_size - *off) {
		DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
		return (DWARF_E_INVALID_FRAME);
	}

	(void) dbg->read(ds->ds_data, off, dwarf_size); /* Skip CIE id. */
	cie->cie_length = length;

	cie->cie_version = dbg->read(ds->ds_data, off, 1);
	if (cie->cie_version != 1 && cie->cie_version != 3) {
		DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
		return (DWARF_E_INVALID_FRAME);
	}

	cie->cie_augment = ds->ds_data + *off;
	p = (char *) ds->ds_data;
	while (p[(*off)++] != '\0')
		;

	/* We only recognize normal .dwarf_frame and GNU .eh_frame sections. */
	if (*cie->cie_augment != 0 && *cie->cie_augment != 'z') {
		*off = cie->cie_offset + ((dwarf_size == 4) ? 4 : 12) +
		    cie->cie_length;
		return (DWARF_E_NONE);
	}

	/* Optional EH Data field for .eh_frame section. */
	if (strstr((char *)cie->cie_augment, "eh") != NULL)
		cie->cie_ehdata = dbg->read(ds->ds_data, off,
		    dbg->dbg_pointer_size);

	cie->cie_caf = _dwarf_read_uleb128(ds->ds_data, off);
	cie->cie_daf = _dwarf_read_sleb128(ds->ds_data, off);

	/* Return address register. */
	if (cie->cie_version == 1)
		cie->cie_ra = dbg->read(ds->ds_data, off, 1);
	else
		cie->cie_ra = _dwarf_read_uleb128(ds->ds_data, off);

	/* Optional augmentation data for .eh_frame section. */
	if (*cie->cie_augment == 'z') {
		cie->cie_auglen = _dwarf_read_uleb128(ds->ds_data, off);
		cie->cie_augdata = ds->ds_data + *off;
		*off += cie->cie_auglen;
	}

	/* CIE Initial instructions. */
	cie->cie_initinst = ds->ds_data + *off;
	if (dwarf_size == 4)
		cie->cie_instlen = cie->cie_offset + 4 + length - *off;
	else
		cie->cie_instlen = cie->cie_offset + 12 + length - *off;

	*off += cie->cie_instlen;

#ifdef FRAME_DEBUG
	printf("cie:\n");
	printf("\tcie_version=%u cie_offset=%ju cie_length=%ju cie_augment=%s"
	    " cie_instlen=%ju cie->cie_caf=%ju cie->cie_daf=%jd off=%ju\n",
	    cie->cie_version, cie->cie_offset, cie->cie_length,
	    (char *)cie->cie_augment, cie->cie_instlen, cie->cie_caf,
	    cie->cie_daf, *off);
#endif

	if (ret_cie != NULL)
		*ret_cie = cie;

	fs->fs_cielen++;

	return (DWARF_E_NONE);
}

static int
_dwarf_frame_add_fde(Dwarf_Debug dbg, Dwarf_FrameSec fs, Dwarf_Section *ds,
    Dwarf_Unsigned *off, int eh_frame, Dwarf_Error *error)
{
	Dwarf_Cie cie;
	Dwarf_Fde fde;
	uint64_t length, delta;
	int dwarf_size, ret;

	if ((fde = calloc(1, sizeof(struct _Dwarf_Fde))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	STAILQ_INSERT_TAIL(&fs->fs_fdelist, fde, fde_next);

	fde->fde_dbg = dbg;
	fde->fde_fs = fs;
	fde->fde_addr = ds->ds_data + *off;
	fde->fde_offset = *off;

	length = dbg->read(ds->ds_data, off, 4);
	if (length == 0xffffffff) {
		dwarf_size = 8;
		length = dbg->read(ds->ds_data, off, 8);
	} else
		dwarf_size = 4;

	if (length > ds->ds_size - *off) {
		DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
		return (DWARF_E_INVALID_FRAME);
	}

	fde->fde_length = length;

	if (eh_frame) {
		delta = dbg->read(ds->ds_data, off, 4);
		fde->fde_cieoff = *off - (4 + delta);
		/* This delta should never be 0. */
		if (fde->fde_cieoff == fde->fde_offset) {
			DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
			return (DWARF_E_INVALID_FRAME);
		}
	} else
		fde->fde_cieoff = dbg->read(ds->ds_data, off, dwarf_size);

	if (_dwarf_frame_find_cie(fs, fde->fde_cieoff, &cie) ==
	    DWARF_E_NO_ENTRY) {
		ret = _dwarf_frame_add_cie(dbg, fs, ds, &fde->fde_cieoff, &cie,
		    error);
		if (ret != DWARF_E_NONE)
			return (ret);
	}
	fde->fde_cieoff = cie->cie_offset;
	fde->fde_cie = cie;
	fde->fde_initloc = dbg->read(ds->ds_data, off, dbg->dbg_pointer_size);
	fde->fde_adrange = dbg->read(ds->ds_data, off, dbg->dbg_pointer_size);

	/* Optional augmentation data for .eh_frame section. */
	if (eh_frame && *cie->cie_augment == 'z') {
		fde->fde_auglen = _dwarf_read_uleb128(ds->ds_data, off);
		fde->fde_augdata = ds->ds_data + *off;
		*off += fde->fde_auglen;
	}

	fde->fde_inst = ds->ds_data + *off;
	if (dwarf_size == 4)
		fde->fde_instlen = fde->fde_offset + 4 + length - *off;
	else
		fde->fde_instlen = fde->fde_offset + 12 + length - *off;

	*off += fde->fde_instlen;

#ifdef FRAME_DEBUG
	printf("fde:");
	if (eh_frame)
		printf("(eh_frame)");
	putchar('\n');
	printf("\tfde_offset=%ju fde_length=%ju fde_cieoff=%ju"
	    " fde_instlen=%ju off=%ju\n", fde->fde_offset, fde->fde_length,
	    fde->fde_cieoff, fde->fde_instlen, *off);
#endif

	fs->fs_fdelen++;

	return (DWARF_E_NONE);
}

static void
_dwarf_frame_section_cleanup(Dwarf_FrameSec fs)
{
	Dwarf_Cie cie, tcie;
	Dwarf_Fde fde, tfde;

	STAILQ_FOREACH_SAFE(cie, &fs->fs_cielist, cie_next, tcie) {
		STAILQ_REMOVE(&fs->fs_cielist, cie, _Dwarf_Cie, cie_next);
		free(cie);
	}

	STAILQ_FOREACH_SAFE(fde, &fs->fs_fdelist, fde_next, tfde) {
		STAILQ_REMOVE(&fs->fs_fdelist, fde, _Dwarf_Fde, fde_next);
		free(fde);
	}

	if (fs->fs_ciearray != NULL)
		free(fs->fs_ciearray);
	if (fs->fs_fdearray != NULL)
		free(fs->fs_fdearray);

	free(fs);
}

static int
_dwarf_frame_section_init(Dwarf_Debug dbg, Dwarf_FrameSec *frame_sec,
    Dwarf_Section *ds, int eh_frame, Dwarf_Error *error)
{
	Dwarf_FrameSec fs;
	Dwarf_Cie cie;
	Dwarf_Fde fde;
	uint64_t length, offset, cie_id, entry_off;
	int dwarf_size, i, ret;

	assert(frame_sec != NULL);
	assert(*frame_sec == NULL);

	if ((fs = calloc(1, sizeof(struct _Dwarf_FrameSec))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}
	STAILQ_INIT(&fs->fs_cielist);
	STAILQ_INIT(&fs->fs_fdelist);

	offset = 0;
	while (offset < ds->ds_size) {
		entry_off = offset;
		length = dbg->read(ds->ds_data, &offset, 4);
		if (length == 0xffffffff) {
			dwarf_size = 8;
			length = dbg->read(ds->ds_data, &offset, 8);
		} else
			dwarf_size = 4;

		if (length > ds->ds_size - offset ||
		    (length == 0 && !eh_frame)) {
			DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
			return (DWARF_E_INVALID_FRAME);
		}

		/* Check terminator for .eh_frame */
		if (eh_frame && length == 0)
			break;

		cie_id = dbg->read(ds->ds_data, &offset, dwarf_size);

		if (eh_frame) {
			/* GNU .eh_frame use CIE id 0. */
			if (cie_id == 0)
				ret = _dwarf_frame_add_cie(dbg, fs, ds,
				    &entry_off, NULL, error);
			else
				ret = _dwarf_frame_add_fde(dbg, fs, ds,
				    &entry_off, 1, error);
		} else {
			/* .dwarf_frame use CIE id ~0 */
			if ((dwarf_size == 4 && cie_id == ~0U) ||
			    (dwarf_size == 8 && cie_id == ~0ULL))
				ret = _dwarf_frame_add_cie(dbg, fs, ds,
				    &entry_off, NULL, error);
			else
				ret = _dwarf_frame_add_fde(dbg, fs, ds,
				    &entry_off, 0, error);
		}

		if (ret != DWARF_E_NONE)
			goto fail_cleanup;

		offset = entry_off;
	}

	/* Create CIE array. */
	if (fs->fs_cielen > 0) {
		if ((fs->fs_ciearray = malloc(sizeof(Dwarf_Cie) *
		    fs->fs_cielen)) == NULL) {
			ret = DWARF_E_MEMORY;
			DWARF_SET_ERROR(error, ret);
			goto fail_cleanup;
		}
		i = 0;
		STAILQ_FOREACH(cie, &fs->fs_cielist, cie_next) {
			fs->fs_ciearray[i++] = cie;
		}
		assert((Dwarf_Unsigned)i == fs->fs_cielen);
	}

	/* Create FDE array. */
	if (fs->fs_fdelen > 0) {
		if ((fs->fs_fdearray = malloc(sizeof(Dwarf_Fde) *
		    fs->fs_fdelen)) == NULL) {
			ret = DWARF_E_MEMORY;
			DWARF_SET_ERROR(error, ret);
			goto fail_cleanup;
		}
		i = 0;
		STAILQ_FOREACH(fde, &fs->fs_fdelist, fde_next) {
			fs->fs_fdearray[i++] = fde;
		}
		assert((Dwarf_Unsigned)i == fs->fs_fdelen);
	}

	*frame_sec = fs;

	return (DWARF_E_NONE);

fail_cleanup:

	_dwarf_frame_section_cleanup(fs);

	return (ret);
}

static int
_dwarf_frame_run_inst(Dwarf_Debug dbg, Dwarf_Regtable3 *rt, uint8_t *insts,
    Dwarf_Unsigned len, Dwarf_Unsigned caf, Dwarf_Signed daf, Dwarf_Addr pc,
    Dwarf_Addr pc_req, Dwarf_Addr *row_pc, Dwarf_Error *error)
{
	Dwarf_Regtable3 *init_rt, *saved_rt;
	uint8_t *p, *pe;
	uint8_t high2, low6;
	uint64_t reg, reg2, uoff, soff;
	int ret;

#define	CFA	rt->rt3_cfa_rule
#define	INITCFA	init_rt->rt3_cfa_rule
#define	RL	rt->rt3_rules
#define	INITRL	init_rt->rt3_rules

#define CHECK_TABLE_SIZE(x)						\
	do {								\
		if ((x) >= rt->rt3_reg_table_size) {			\
			DWARF_SET_ERROR(error, DWARF_E_REGTABLE_SPACE);	\
			ret = DWARF_E_REGTABLE_SPACE;			\
			goto program_done;				\
		}							\
	} while(0)

#ifdef FRAME_DEBUG
	printf("frame_run_inst: (caf=%ju, daf=%jd)\n", caf, daf);
#endif

	ret = DWARF_E_NONE;
	init_rt = saved_rt = NULL;
	*row_pc = pc;

	/* Save a copy of the table as initial state. */
	_dwarf_frame_regtable_copy(dbg, &init_rt, rt, error);

	p = insts;
	pe = p + len;

	while (p < pe) {

#ifdef FRAME_DEBUG
		printf("p=%p pe=%p pc=%#jx pc_req=%#jx\n", p, pe, pc, pc_req);
#endif

		if (*p == DW_CFA_nop) {
#ifdef FRAME_DEBUG
			printf("DW_CFA_nop\n");
#endif
			p++;
			continue;
		}

		high2 = *p & 0xc0;
		low6 = *p & 0x3f;
		p++;

		if (high2 > 0) {
			switch (high2) {
			case DW_CFA_advance_loc:
				pc += low6 * caf;
#ifdef FRAME_DEBUG
				printf("DW_CFA_advance_loc(%#jx(%u))\n", pc,
				    low6);
#endif
				if (pc_req < pc)
					goto program_done;
				break;
			case DW_CFA_offset:
				*row_pc = pc;
				CHECK_TABLE_SIZE(low6);
				RL[low6].dw_offset_relevant = 1;
				RL[low6].dw_value_type = DW_EXPR_OFFSET;
				RL[low6].dw_regnum = dbg->dbg_frame_cfa_value;
				RL[low6].dw_offset_or_block_len =
				    _dwarf_decode_uleb128(&p) * daf;
#ifdef FRAME_DEBUG
				printf("DW_CFA_offset(%jd)\n",
				    RL[low6].dw_offset_or_block_len);
#endif
				break;
			case DW_CFA_restore:
				*row_pc = pc;
				CHECK_TABLE_SIZE(low6);
				memcpy(&RL[low6], &INITRL[low6],
				    sizeof(Dwarf_Regtable_Entry3));
#ifdef FRAME_DEBUG
				printf("DW_CFA_restore(%u)\n", low6);
#endif
				break;
			default:
				DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
				ret = DWARF_E_INVALID_FRAME;
				goto program_done;
			}

			continue;
		}

		switch (low6) {
		case DW_CFA_set_loc:
			pc = dbg->decode(&p, dbg->dbg_pointer_size);
#ifdef FRAME_DEBUG
			printf("DW_CFA_set_loc(pc=%#jx)\n", pc);
#endif
			if (pc_req < pc)
				goto program_done;
			break;
		case DW_CFA_advance_loc1:
			pc += dbg->decode(&p, 1) * caf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_set_loc1(pc=%#jx)\n", pc);
#endif
			if (pc_req < pc)
				goto program_done;
			break;
		case DW_CFA_advance_loc2:
			pc += dbg->decode(&p, 2) * caf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_set_loc2(pc=%#jx)\n", pc);
#endif
			if (pc_req < pc)
				goto program_done;
			break;
		case DW_CFA_advance_loc4:
			pc += dbg->decode(&p, 4) * caf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_set_loc4(pc=%#jx)\n", pc);
#endif
			if (pc_req < pc)
				goto program_done;
			break;
		case DW_CFA_offset_extended:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			uoff = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 1;
			RL[reg].dw_value_type = DW_EXPR_OFFSET;
			RL[reg].dw_regnum = dbg->dbg_frame_cfa_value;
			RL[reg].dw_offset_or_block_len = uoff * daf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_offset_extended(reg=%ju,uoff=%ju)\n",
			    reg, uoff);
#endif
			break;
		case DW_CFA_restore_extended:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			memcpy(&RL[reg], &INITRL[reg],
			    sizeof(Dwarf_Regtable_Entry3));
#ifdef FRAME_DEBUG
			printf("DW_CFA_restore_extended(%ju)\n", reg);
#endif
			break;
		case DW_CFA_undefined:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 0;
			RL[reg].dw_regnum = dbg->dbg_frame_undefined_value;
#ifdef FRAME_DEBUG
			printf("DW_CFA_undefined(%ju)\n", reg);
#endif
			break;
		case DW_CFA_same_value:
			reg = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 0;
			RL[reg].dw_regnum = dbg->dbg_frame_same_value;
#ifdef FRAME_DEBUG
			printf("DW_CFA_same_value(%ju)\n", reg);
#endif
			break;
		case DW_CFA_register:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			reg2 = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 0;
			RL[reg].dw_regnum = reg2;
#ifdef FRAME_DEBUG
			printf("DW_CFA_register(reg=%ju,reg2=%ju)\n", reg,
			    reg2);
#endif
			break;
		case DW_CFA_remember_state:
			_dwarf_frame_regtable_copy(dbg, &saved_rt, rt, error);
#ifdef FRAME_DEBUG
			printf("DW_CFA_remember_state\n");
#endif
			break;
		case DW_CFA_restore_state:
			*row_pc = pc;
			_dwarf_frame_regtable_copy(dbg, &rt, saved_rt, error);
#ifdef FRAME_DEBUG
			printf("DW_CFA_restore_state\n");
#endif
			break;
		case DW_CFA_def_cfa:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			uoff = _dwarf_decode_uleb128(&p);
			CFA.dw_offset_relevant = 1;
			CFA.dw_value_type = DW_EXPR_OFFSET;
			CFA.dw_regnum = reg;
			CFA.dw_offset_or_block_len = uoff;
#ifdef FRAME_DEBUG
			printf("DW_CFA_def_cfa(reg=%ju,uoff=%ju)\n", reg, uoff);
#endif
			break;
		case DW_CFA_def_cfa_register:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			CFA.dw_regnum = reg;
#ifdef FRAME_DEBUG
			printf("DW_CFA_def_cfa_register(%ju)\n", reg);
#endif
			break;
		case DW_CFA_def_cfa_offset:
			*row_pc = pc;
			uoff = _dwarf_decode_uleb128(&p);
			CFA.dw_offset_relevant = 1;
			CFA.dw_value_type = DW_EXPR_OFFSET;
			CFA.dw_offset_or_block_len = uoff;
#ifdef FRAME_DEBUG
			printf("DW_CFA_def_cfa_offset(%ju)\n", uoff);
#endif
			break;
		case DW_CFA_def_cfa_expression:
			*row_pc = pc;
			CFA.dw_offset_relevant = 0;
			CFA.dw_value_type = DW_EXPR_EXPRESSION;
			CFA.dw_offset_or_block_len = _dwarf_decode_uleb128(&p);
			CFA.dw_block_ptr = p;
			p += CFA.dw_offset_or_block_len;
#ifdef FRAME_DEBUG
			printf("DW_CFA_def_cfa_expression\n");
#endif
			break;
		case DW_CFA_expression:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 0;
			RL[reg].dw_value_type = DW_EXPR_EXPRESSION;
			RL[reg].dw_offset_or_block_len =
			    _dwarf_decode_uleb128(&p);
			RL[reg].dw_block_ptr = p;
			p += RL[reg].dw_offset_or_block_len;
#ifdef FRAME_DEBUG
			printf("DW_CFA_expression\n");
#endif
			break;
		case DW_CFA_offset_extended_sf:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			soff = _dwarf_decode_sleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 1;
			RL[reg].dw_value_type = DW_EXPR_OFFSET;
			RL[reg].dw_regnum = dbg->dbg_frame_cfa_value;
			RL[reg].dw_offset_or_block_len = soff * daf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_offset_extended_sf(reg=%ju,soff=%jd)\n",
			    reg, soff);
#endif
			break;
		case DW_CFA_def_cfa_sf:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			soff = _dwarf_decode_sleb128(&p);
			CFA.dw_offset_relevant = 1;
			CFA.dw_value_type = DW_EXPR_OFFSET;
			CFA.dw_regnum = reg;
			CFA.dw_offset_or_block_len = soff * daf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_def_cfa_sf(reg=%ju,soff=%jd)\n", reg,
			    soff);
#endif
			break;
		case DW_CFA_def_cfa_offset_sf:
			*row_pc = pc;
			soff = _dwarf_decode_sleb128(&p);
			CFA.dw_offset_relevant = 1;
			CFA.dw_value_type = DW_EXPR_OFFSET;
			CFA.dw_offset_or_block_len = soff * daf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_def_cfa_offset_sf(soff=%jd)\n", soff);
#endif
			break;
		case DW_CFA_val_offset:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			uoff = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 1;
			RL[reg].dw_value_type = DW_EXPR_VAL_OFFSET;
			RL[reg].dw_regnum = dbg->dbg_frame_cfa_value;
			RL[reg].dw_offset_or_block_len = uoff * daf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_val_offset(reg=%ju,uoff=%ju)\n", reg,
			    uoff);
#endif
			break;
		case DW_CFA_val_offset_sf:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			soff = _dwarf_decode_sleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 1;
			RL[reg].dw_value_type = DW_EXPR_VAL_OFFSET;
			RL[reg].dw_regnum = dbg->dbg_frame_cfa_value;
			RL[reg].dw_offset_or_block_len = soff * daf;
#ifdef FRAME_DEBUG
			printf("DW_CFA_val_offset_sf(reg=%ju,soff=%jd)\n", reg,
			    soff);
#endif
			break;
		case DW_CFA_val_expression:
			*row_pc = pc;
			reg = _dwarf_decode_uleb128(&p);
			CHECK_TABLE_SIZE(reg);
			RL[reg].dw_offset_relevant = 0;
			RL[reg].dw_value_type = DW_EXPR_VAL_EXPRESSION;
			RL[reg].dw_offset_or_block_len =
			    _dwarf_decode_uleb128(&p);
			RL[reg].dw_block_ptr = p;
			p += RL[reg].dw_offset_or_block_len;
#ifdef FRAME_DEBUG
			printf("DW_CFA_val_expression\n");
#endif
			break;
		default:
			DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
			ret = DWARF_E_INVALID_FRAME;
			goto program_done;
		}
	}

program_done:

	free(init_rt->rt3_rules);
	free(init_rt);
	if (saved_rt) {
		free(saved_rt->rt3_rules);
		free(saved_rt);
	}

	return (ret);

#undef	CFA
#undef	INITCFA
#undef	RL
#undef	INITRL
#undef	CHECK_TABLE_SIZE
}

static int
_dwarf_frame_convert_inst(Dwarf_Debug dbg, uint8_t *insts, Dwarf_Unsigned len,
    Dwarf_Unsigned *count, Dwarf_Frame_Op *fop, Dwarf_Frame_Op3 *fop3,
    Dwarf_Error *error)
{
	uint8_t *p, *pe;
	uint8_t high2, low6;
	uint64_t reg, reg2, uoff, soff, blen;
	int ret;

#define	SET_BASE_OP(x)						\
	do {							\
		if (fop != NULL)				\
			fop[*count].fp_base_op = (x);		\
		if (fop3 != NULL)				\
			fop3[*count].fp_base_op = (x);		\
	} while(0)

#define	SET_EXTENDED_OP(x)					\
	do {							\
		if (fop != NULL)				\
			fop[*count].fp_extended_op = (x);	\
		if (fop3 != NULL)				\
			fop3[*count].fp_extended_op = (x);	\
	} while(0)

#define	SET_REGISTER(x)						\
	do {							\
		if (fop != NULL)				\
			fop[*count].fp_register = (x);		\
		if (fop3 != NULL)				\
			fop3[*count].fp_register = (x);		\
	} while(0)

#define	SET_OFFSET(x)						\
	do {							\
		if (fop != NULL)				\
			fop[*count].fp_offset = (x);		\
		if (fop3 != NULL)				\
			fop3[*count].fp_offset_or_block_len =	\
			    (x);				\
	} while(0)

#define	SET_INSTR_OFFSET(x)					\
	do {							\
		if (fop != NULL)				\
			fop[*count].fp_instr_offset = (x);	\
		if (fop3 != NULL)				\
			fop3[*count].fp_instr_offset = (x);	\
	} while(0)

#define	SET_BLOCK_LEN(x)					\
	do {							\
		if (fop3 != NULL)				\
			fop3[*count].fp_offset_or_block_len =	\
			    (x);				\
	} while(0)

#define	SET_EXPR_BLOCK(addr, len)					\
	do {								\
		if (fop3 != NULL) {					\
			fop3[*count].fp_expr_block = malloc((len));	\
			if (fop3[*count].fp_expr_block == NULL)	{	\
				DWARF_SET_ERROR(error, DWARF_E_MEMORY);	\
				return (DWARF_E_MEMORY);		\
			}						\
			memcpy(&fop3[*count].fp_expr_block,		\
			    (addr), (len));				\
		}							\
	} while(0)

	ret = DWARF_E_NONE;
	*count = 0;

	p = insts;
	pe = p + len;

	while (p < pe) {

		SET_INSTR_OFFSET(p - insts);

		if (*p == DW_CFA_nop) {
			p++;
			(*count)++;
			continue;
		}

		high2 = *p & 0xc0;
		low6 = *p & 0x3f;
		p++;

		if (high2 > 0) {
			switch (high2) {
			case DW_CFA_advance_loc:
				SET_BASE_OP(high2);
				SET_OFFSET(low6);
				break;
			case DW_CFA_offset:
				SET_BASE_OP(high2);
				SET_REGISTER(low6);
				uoff = _dwarf_decode_uleb128(&p);
				SET_OFFSET(uoff);
				break;
			case DW_CFA_restore:
				SET_BASE_OP(high2);
				SET_REGISTER(low6);
				break;
			default:
				DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
				return (DWARF_E_INVALID_FRAME);
			}

			(*count)++;
			continue;
		}

		SET_EXTENDED_OP(low6);

		switch (low6) {
		case DW_CFA_set_loc:
			uoff = dbg->decode(&p, dbg->dbg_pointer_size);
			SET_OFFSET(uoff);
			break;
		case DW_CFA_advance_loc1:
			uoff = dbg->decode(&p, 1);
			SET_OFFSET(uoff);
			break;
		case DW_CFA_advance_loc2:
			uoff = dbg->decode(&p, 2);
			SET_OFFSET(uoff);
			break;
		case DW_CFA_advance_loc4:
			uoff = dbg->decode(&p, 4);
			SET_OFFSET(uoff);
			break;
		case DW_CFA_offset_extended:
		case DW_CFA_def_cfa:
		case DW_CFA_val_offset:
			reg = _dwarf_decode_uleb128(&p);
			uoff = _dwarf_decode_uleb128(&p);
			SET_REGISTER(reg);
			SET_OFFSET(uoff);
			break;
		case DW_CFA_restore_extended:
		case DW_CFA_undefined:
		case DW_CFA_same_value:
		case DW_CFA_def_cfa_register:
			reg = _dwarf_decode_uleb128(&p);
			SET_REGISTER(reg);
			break;
		case DW_CFA_register:
			reg = _dwarf_decode_uleb128(&p);
			reg2 = _dwarf_decode_uleb128(&p);
			SET_REGISTER(reg);
			SET_OFFSET(reg2);
			break;
		case DW_CFA_remember_state:
		case DW_CFA_restore_state:
			break;
		case DW_CFA_def_cfa_offset:
			uoff = _dwarf_decode_uleb128(&p);
			SET_OFFSET(uoff);
			break;
		case DW_CFA_def_cfa_expression:
			blen = _dwarf_decode_uleb128(&p);
			SET_BLOCK_LEN(blen);
			SET_EXPR_BLOCK(p, blen);
			p += blen;
			break;
		case DW_CFA_expression:
		case DW_CFA_val_expression:
			reg = _dwarf_decode_uleb128(&p);
			blen = _dwarf_decode_uleb128(&p);
			SET_REGISTER(reg);
			SET_BLOCK_LEN(blen);
			SET_EXPR_BLOCK(p, blen);
			p += blen;
			break;
		case DW_CFA_offset_extended_sf:
		case DW_CFA_def_cfa_sf:
		case DW_CFA_val_offset_sf:
			reg = _dwarf_decode_uleb128(&p);
			soff = _dwarf_decode_sleb128(&p);
			SET_REGISTER(reg);
			SET_OFFSET(soff);
			break;
		case DW_CFA_def_cfa_offset_sf:
			soff = _dwarf_decode_sleb128(&p);
			SET_OFFSET(soff);
			break;
		default:
			DWARF_SET_ERROR(error, DWARF_E_INVALID_FRAME);
			return (DWARF_E_INVALID_FRAME);
		}

		(*count)++;
	}

	return (DWARF_E_NONE);
}

int
_dwarf_frame_get_fop(Dwarf_Debug dbg, uint8_t *insts, Dwarf_Unsigned len,
    Dwarf_Frame_Op **ret_oplist, Dwarf_Signed *ret_opcnt, Dwarf_Error *error)
{
	Dwarf_Frame_Op *oplist;
	Dwarf_Unsigned count;
	int ret;

	ret = _dwarf_frame_convert_inst(dbg, insts, len, &count, NULL, NULL,
	    error);
	if (ret != DWARF_E_NONE)
		return (ret);

	if ((oplist = calloc(count, sizeof(Dwarf_Frame_Op))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	ret = _dwarf_frame_convert_inst(dbg, insts, len, &count, oplist, NULL,
	    error);
	if (ret != DWARF_E_NONE) {
		_dwarf_frame_free_fop(oplist, count);
		return (ret);
	}

	*ret_oplist = oplist;
	*ret_opcnt = count;

	return (DWARF_E_NONE);
}

void
_dwarf_frame_free_fop(Dwarf_Frame_Op *oplist, Dwarf_Unsigned count)
{

	(void) count;
	free(oplist);
}

int
_dwarf_frame_regtable_copy(Dwarf_Debug dbg, Dwarf_Regtable3 **dest,
    Dwarf_Regtable3 *src, Dwarf_Error *error)
{
	int i;

	assert(dest != NULL);
	assert(src != NULL);

	if (*dest == NULL) {
		if ((*dest = malloc(sizeof(Dwarf_Regtable3))) == NULL) {
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			return (DWARF_E_MEMORY);
		}
		(*dest)->rt3_reg_table_size = src->rt3_reg_table_size;
		(*dest)->rt3_rules = malloc(src->rt3_reg_table_size *
		    sizeof(Dwarf_Regtable_Entry3));
		if ((*dest)->rt3_rules == NULL) {
			free(*dest);
			DWARF_SET_ERROR(error, DWARF_E_MEMORY);
			return (DWARF_E_MEMORY);
		}
	}

	memcpy(&(*dest)->rt3_cfa_rule, &src->rt3_cfa_rule,
	    sizeof(Dwarf_Regtable_Entry3));

	for (i = 0; i < (*dest)->rt3_reg_table_size &&
	     i < src->rt3_reg_table_size; i++)
		memcpy(&(*dest)->rt3_rules[i], &src->rt3_rules[i],
		    sizeof(Dwarf_Regtable_Entry3));

	for (; i < (*dest)->rt3_reg_table_size; i++)
		(*dest)->rt3_rules[i].dw_regnum =
		    dbg->dbg_frame_undefined_value;

	return (DWARF_E_NONE);
}

int
_dwarf_frame_get_internal_table(Dwarf_Fde fde, Dwarf_Addr pc_req,
    Dwarf_Regtable3 **ret_rt, Dwarf_Addr *ret_row_pc, Dwarf_Error *error)
{
	Dwarf_Debug dbg;
	Dwarf_Cie cie;
	Dwarf_Regtable3 *rt;
	Dwarf_Addr row_pc;
	int i, ret;

	assert(ret_rt != NULL);

	dbg = fde->fde_dbg;
	assert(dbg != NULL);

	rt = dbg->dbg_internal_reg_table;

	/* Set rules to initial values. */
	for (i = 0; i < rt->rt3_reg_table_size; i++)
		rt->rt3_rules[i].dw_regnum = dbg->dbg_frame_rule_initial_value;

	/* Run initial instructions in CIE. */
	cie = fde->fde_cie;
	assert(cie != NULL);
	ret = _dwarf_frame_run_inst(dbg, rt, cie->cie_initinst,
	    cie->cie_instlen, cie->cie_caf, cie->cie_daf, 0, ~0ULL, &row_pc,
	    error);
	if (ret != DWARF_E_NONE)
		return (ret);

	/* Run instructions in FDE. */
	ret = _dwarf_frame_run_inst(dbg, rt, fde->fde_inst, fde->fde_instlen,
	    cie->cie_caf, cie->cie_daf, fde->fde_initloc, pc_req, &row_pc,
	    error);
	if (ret != DWARF_E_NONE)
		return (ret);

	*ret_rt = rt;
	*ret_row_pc = row_pc;

	return (DWARF_E_NONE);
}

void
_dwarf_frame_cleanup(Dwarf_Debug dbg)
{
	Dwarf_Regtable3 *rt;

	if (dbg->dbg_internal_reg_table) {
		rt = dbg->dbg_internal_reg_table;
		free(rt->rt3_rules);
		free(rt);
	}

	if (dbg->dbg_frame)
		_dwarf_frame_section_cleanup(dbg->dbg_frame);

	if (dbg->dbg_eh_frame)
		_dwarf_frame_section_cleanup(dbg->dbg_eh_frame);
}

int
_dwarf_frame_init(Dwarf_Debug dbg, Dwarf_Error *error)
{
	Dwarf_Regtable3 *rt;
	Dwarf_Section *ds;
	int ret;

	/* Initialise call frame related parameters. */
	dbg->dbg_frame_rule_table_size = DW_FRAME_LAST_REG_NUM;
	dbg->dbg_frame_rule_initial_value = DW_FRAME_REG_INITIAL_VALUE;
	dbg->dbg_frame_cfa_value = DW_FRAME_CFA_COL3;
	dbg->dbg_frame_same_value = DW_FRAME_SAME_VAL;
	dbg->dbg_frame_undefined_value = DW_FRAME_UNDEFINED_VAL;

	/* Initialise internal register table. */
	if ((rt = calloc(1, sizeof(Dwarf_Regtable3))) == NULL) {
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	rt->rt3_reg_table_size = dbg->dbg_frame_rule_table_size;
	if ((rt->rt3_rules = calloc(rt->rt3_reg_table_size,
	    sizeof(Dwarf_Regtable_Entry3))) == NULL) {
		free(rt);
		DWARF_SET_ERROR(error, DWARF_E_MEMORY);
		return (DWARF_E_MEMORY);
	}

	/*
	 * Initialise call frame sections.
	 */

	if ((ds = _dwarf_find_section(dbg, ".debug_frame")) != NULL) {
		ret = _dwarf_frame_section_init(dbg, &dbg->dbg_frame, ds, 0,
		    error);
		if (ret != DWARF_E_NONE) {
			free(rt);
			return (ret);
		}
	}
	if ((ds = _dwarf_find_section(dbg, ".eh_frame")) != NULL) {
		ret = _dwarf_frame_section_init(dbg, &dbg->dbg_eh_frame, ds, 1,
		    error);
		if (ret != DWARF_E_NONE) {
			free(rt);
			return (ret);
		}
	}

	dbg->dbg_internal_reg_table = rt;

	return (DWARF_E_NONE);
}
