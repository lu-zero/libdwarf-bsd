/*-
 * Copyright (c) 2009, 2010 Kai Wang
 * All rights reserved.
 * Copyright (c) 2007 John Birrell (jb@freebsd.org)
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
 *
 * $FreeBSD$
 */

#ifndef	_LIBDWARF_H_
#define	_LIBDWARF_H_

#include <libelf.h>

typedef int		Dwarf_Bool;
typedef off_t		Dwarf_Off;
typedef uint64_t	Dwarf_Unsigned;
typedef uint16_t	Dwarf_Half;
typedef uint8_t		Dwarf_Small;
typedef int64_t		Dwarf_Signed;
typedef uint64_t	Dwarf_Addr;
typedef void		*Dwarf_Ptr;

typedef struct _Dwarf_Abbrev	*Dwarf_Abbrev;
typedef struct _Dwarf_Arange	*Dwarf_Arange;
typedef struct _Dwarf_ArangeSet	*Dwarf_ArangeSet;
typedef struct _Dwarf_Attribute	*Dwarf_Attribute;
typedef struct _Dwarf_Attribute *Dwarf_P_Attribute;
typedef struct _Dwarf_AttrDef	*Dwarf_AttrDef;
typedef struct _Dwarf_CU	*Dwarf_CU;
typedef struct _Dwarf_Cie	*Dwarf_Cie;
typedef struct _Dwarf_Cie	*Dwarf_P_Cie;
typedef struct _Dwarf_Debug	*Dwarf_Debug;
typedef struct _Dwarf_Debug	*Dwarf_P_Debug;
typedef struct _Dwarf_Die	*Dwarf_Die;
typedef struct _Dwarf_Die	*Dwarf_P_Die;
typedef struct _Dwarf_Fde	*Dwarf_Fde;
typedef struct _Dwarf_Fde	*Dwarf_P_Fde;
typedef struct _Dwarf_FrameSec	*Dwarf_FrameSec;
typedef struct _Dwarf_Line	*Dwarf_Line;
typedef struct _Dwarf_LineFile	*Dwarf_LineFile;
typedef struct _Dwarf_LineInfo	*Dwarf_LineInfo;
typedef struct _Dwarf_Loclist	*Dwarf_Loclist;
typedef struct _Dwarf_MacroSet	*Dwarf_MacroSet;
typedef struct _Dwarf_NamePair	*Dwarf_NamePair;
typedef struct _Dwarf_NamePair	*Dwarf_Func;
typedef struct _Dwarf_NamePair	*Dwarf_Global;
typedef struct _Dwarf_NamePair	*Dwarf_Type;
typedef struct _Dwarf_NamePair	*Dwarf_Var;
typedef struct _Dwarf_NamePair	*Dwarf_Weak;
typedef struct _Dwarf_NameTbl	*Dwarf_NameTbl;
typedef struct _Dwarf_NameSec	*Dwarf_NameSec;
typedef struct _Dwarf_P_Expr	*Dwarf_P_Expr;
typedef struct _Dwarf_Rangelist	*Dwarf_Rangelist;

typedef enum {
	DW_OBJECT_MSB,
	DW_OBJECT_LSB
} Dwarf_Endianness;

typedef struct {
	Dwarf_Addr addr;
	Dwarf_Unsigned size;
	const char *name;
} Dwarf_Obj_Access_Section;

typedef struct {
	int (*get_section_info)(void *_obj, Dwarf_Half _index,
	    Dwarf_Obj_Access_Section *_ret_section, int *_error);
	Dwarf_Endianness (*get_byte_order)(void *_obj);
	Dwarf_Small (*get_length_size)(void *_obj);
	Dwarf_Small (*get_pointer_size)(void *_obj);
	Dwarf_Unsigned (*get_section_count)(void *_obj);
	int (*load_section)(void *_obj, Dwarf_Half _index,
	    Dwarf_Small **_ret_data, int *_error);
} Dwarf_Obj_Access_Methods;

typedef struct {
	void *object;
	const Dwarf_Obj_Access_Methods *methods;
} Dwarf_Obj_Access_Interface;

typedef int (*Dwarf_Callback_Func)(char *_name, int _size,
    Dwarf_Unsigned _type, Dwarf_Unsigned _flags, Dwarf_Unsigned _link,
    Dwarf_Unsigned _info, int *_index, int *_error);

typedef int (*Dwarf_Callback_Func_b)(char *_name, int _size,
    Dwarf_Unsigned _type, Dwarf_Unsigned _flags, Dwarf_Unsigned _link,
    Dwarf_Unsigned _info, Dwarf_Unsigned *_index, int *_error);

typedef Dwarf_Unsigned Dwarf_Tag;

typedef struct {
        Dwarf_Small	lr_atom;
        Dwarf_Unsigned	lr_number;
	Dwarf_Unsigned	lr_number2;
	Dwarf_Unsigned	lr_offset;
} Dwarf_Loc;

typedef struct {
	Dwarf_Addr      ld_lopc;
	Dwarf_Addr      ld_hipc;
	Dwarf_Half      ld_cents;
	Dwarf_Loc	*ld_s;
} Dwarf_Locdesc;

typedef struct {
	Dwarf_Unsigned	bl_len;
	Dwarf_Ptr	bl_data;
} Dwarf_Block;

typedef struct {
	Dwarf_Unsigned	rg_start;	/* Beginning of address range. */
	Dwarf_Unsigned	rg_end;		/* End of address range. */
} Dwarf_Ranges;

#ifndef	DW_FRAME_HIGHEST_NORMAL_REGISTER
#define	DW_FRAME_HIGHEST_NORMAL_REGISTER 63
#endif

#define	DW_FRAME_RA_COL		(DW_FRAME_HIGHEST_NORMAL_REGISTER + 1)
#define	DW_FRAME_STATIC_LINK	(DW_FRAME_HIGHEST_NORMAL_REGISTER + 2)

#ifndef	DW_FRAME_LAST_REG_NUM
#define DW_FRAME_LAST_REG_NUM	(DW_FRAME_HIGHEST_NORMAL_REGISTER + 3)
#endif

#ifndef	DW_FRAME_REG_INITIAL_VALUE
#define	DW_FRAME_REG_INITIAL_VALUE DW_FRAME_SAME_VAL
#endif

#define	DW_FRAME_UNDEFINED_VAL		1034
#define DW_FRAME_SAME_VAL		1035
#define DW_FRAME_CFA_COL3		1436

#define	DW_EXPR_OFFSET 0
#define	DW_EXPR_VAL_OFFSET 1
#define	DW_EXPR_EXPRESSION 2
#define	DW_EXPR_VAL_EXPRESSION 3

/*
 * Frame operation only for DWARF 2.
 */

#define DW_FRAME_CFA_COL 0

typedef struct {
	Dwarf_Small	fp_base_op;
	Dwarf_Small	fp_extended_op;
	Dwarf_Half	fp_register;
	Dwarf_Signed	fp_offset;
	Dwarf_Off	fp_instr_offset;
} Dwarf_Frame_Op;

#ifndef	DW_REG_TABLE_SIZE
#define	DW_REG_TABLE_SIZE	66
#endif

typedef struct {
	struct {
		Dwarf_Small	dw_offset_relevant;
		Dwarf_Half	dw_regnum;
		Dwarf_Addr	dw_offset;
	} rules[DW_REG_TABLE_SIZE];
} Dwarf_Regtable;

/*
 * Frame operation for DWARF 3 and DWARF 2.
 */

typedef struct {
	Dwarf_Small	fp_base_op;
	Dwarf_Small	fp_extended_op;
	Dwarf_Half	fp_register;
	Dwarf_Unsigned	fp_offset_or_block_len;
	Dwarf_Small	*fp_expr_block;
	Dwarf_Off	fp_instr_offset;
} Dwarf_Frame_Op3;

typedef struct {
	Dwarf_Small	dw_offset_relevant;
	Dwarf_Small	dw_value_type;
	Dwarf_Half	dw_regnum;
	Dwarf_Unsigned	dw_offset_or_block_len;
	Dwarf_Ptr	dw_block_ptr;
} Dwarf_Regtable_Entry3;

typedef struct {
	Dwarf_Regtable_Entry3	rt3_cfa_rule;
	Dwarf_Half		rt3_reg_table_size;
	Dwarf_Regtable_Entry3	*rt3_rules;
} Dwarf_Regtable3;

typedef struct {
	Dwarf_Off	dmd_offset;
	Dwarf_Small	dmd_type;
	Dwarf_Signed	dmd_lineno;
	Dwarf_Signed	dmd_fileindex;
	char		*dmd_macro;
} Dwarf_Macro_Details;

/*
 * Relocation Type.
 */
enum Dwarf_Rel_Type {
	dwarf_drt_none = 0,
	dwarf_drt_data_reloc,
	dwarf_drt_segment_rel,
	dwarf_drt_first_of_length_pair,
	dwarf_drt_second_of_length_pair
};

/*
 * Relocation Entry.
 */
typedef struct Dwarf_Relocation_Data_s {
	unsigned char drd_type;
	unsigned char drd_length;
	Dwarf_Unsigned drd_offset;
	Dwarf_Unsigned drd_symbol_index;
} *Dwarf_Relocation_Data;

/*
 * Error numbers which are specific to this implementation.
 */
enum {
	DWARF_E_NONE,			/* No error. */
	DWARF_E_ERROR,			/* An error! */
	DWARF_E_NO_ENTRY,		/* No entry. */
	DWARF_E_ARGUMENT,		/* Invalid argument. */
	DWARF_E_DEBUG_INFO,		/* Debug info NULL. */
	DWARF_E_MEMORY,			/* Insufficient memory. */
	DWARF_E_ELF,			/* ELF error. */
	DWARF_E_INVALID_CU,		/* Invalid compilation unit data. */
	DWARF_E_CU_VERSION,		/* Wrong CU version. */
	DWARF_E_MISSING_ABBREV,		/* Abbrev not found. */
	DWARF_E_NOT_IMPLEMENTED,	/* Not implemented. */
	DWARF_E_CU_CURRENT,		/* No current compilation unit. */
	DWARF_E_BAD_FORM,		/* Wrong form type for attrib value. */
	DWARF_E_INVALID_EXPR,		/* Invalid DWARF expression. */
	DWARF_E_INVALID_LOCLIST,	/* Invalid loclist data. */
	DWARF_E_INVALID_ATTR,		/* Invalid attribute. */
	DWARF_E_INVALID_LINE,		/* Invalid line info data. */
	DWARF_E_INVALID_FRAME,		/* Invalid call frame data. */
	DWARF_E_REGTABLE_SPACE,		/* Insufficient regtable space. */
	DWARF_E_INVALID_ARANGE,		/* Invalid address range data. */
	DWARF_E_INVALID_MACINFO,	/* Invalid macinfo data. */
	DWARF_E_SEQUENCE,		/* API called in wrong sequence. */
	DWARF_E_NO_ROOT_DIE,		/* Root DIE is not specified. */
	DWARF_E_DIE_NULL_ATTR,		/* DIE doesn't have any attr. */
	DWARF_E_USER_CALLBACK,		/* Application callback failed. */
	DWARF_E_NUM			/* Max error number. */
};

typedef struct _Dwarf_Error {
	int		err_error;	/* DWARF error. */
	int		elf_error;	/* ELF error. */
	const char	*err_func;	/* Function name where error occurred. */
	int		err_line;	/* Line number where error occurred. */
	char		err_msg[1024];	/* Formatted error message. */
} Dwarf_Error;

/*
 * Dwarf error handler.
 */
typedef void (*Dwarf_Handler)(Dwarf_Error, Dwarf_Ptr);

#define	dwarf_errno(error)	error.err_error
#define	dwarf_errmsg(error)	_dwarf_errmsg(&error)

/*
 * Return values which have to be compatible with other
 * implementations of libdwarf.
 */
#define DW_DLV_NO_ENTRY		-1
#define DW_DLV_OK		0
#define	DW_DLV_ERROR		1
#define DW_DLV_BADADDR		NULL
#define DW_DLV_NOCOUNT		((Dwarf_Signed) -1)
#define DW_DLE_DEBUG_INFO_NULL	DWARF_E_DEBUG_INFO

/*
 * Access modes.
 */
#define DW_DLC_READ        	0
#define DW_DLC_WRITE		1
#define	DW_DLC_RDWR		2

/*
 * Flags used by libdwarf producer.
 */
#define DW_DLC_SIZE_64			0x40000000
#define DW_DLC_SIZE_32			0x20000000
#define DW_DLC_OFFSET_SIZE_64		0x10000000
#define DW_DLC_ISA_MIPS			0x00000000
#define DW_DLC_ISA_IA64			0x01000000
#define DW_DLC_STREAM_RELOCATIONS	0x02000000
#define DW_DLC_SYMBOLIC_RELOCATIONS	0x04000000
#define DW_DLC_TARGET_BIGENDIAN		0x08000000
#define DW_DLC_TARGET_LITTLEENDIAN	0x00100000

/*
 * ISA supported by elftoolchain libdwarf.
 */
enum {
	DW_DLC_ISA_X86 = 1,
	DW_DLC_ISA_X86_64,
	DW_DLC_ISA_SPARC,
	DW_DLC_ISA_PPC,
	DW_DLC_ISA_ARM,
	DW_DLC_ISA_MAX
};

/* Function prototype definitions. */
__BEGIN_DECLS
const char	*_dwarf_errmsg(Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_name(Dwarf_P_Die, char *, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_comp_dir(Dwarf_P_Die, char *, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_producer(Dwarf_P_Die, char *, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_const_value_signedint(Dwarf_P_Die, Dwarf_Signed,
		    Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_const_value_unsignedint(Dwarf_P_Die,
		    Dwarf_Unsigned, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_const_value_string(Dwarf_P_Die, char *,
		    Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_targ_address(Dwarf_P_Debug, Dwarf_P_Die,
		    Dwarf_Half, Dwarf_Unsigned, Dwarf_Signed, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_targ_address_b(Dwarf_P_Debug, Dwarf_P_Die,
		    Dwarf_Half, Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_dataref(Dwarf_P_Debug, Dwarf_P_Die, Dwarf_Half,
		    Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_ref_address(Dwarf_P_Debug, Dwarf_P_Die,
		    Dwarf_Half, Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_unsigned_const(Dwarf_P_Debug, Dwarf_P_Die,
		    Dwarf_Half, Dwarf_Unsigned, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_signed_const(Dwarf_P_Debug, Dwarf_P_Die,
		    Dwarf_Half, Dwarf_Signed, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_reference(Dwarf_P_Debug, Dwarf_P_Die, Dwarf_Half,
		    Dwarf_P_Die, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_flag(Dwarf_P_Debug, Dwarf_P_Die, Dwarf_Half,
		    Dwarf_Small, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_string(Dwarf_P_Debug, Dwarf_P_Die, Dwarf_Half,
		    char *, Dwarf_Error *);
Dwarf_P_Attribute dwarf_add_AT_location_expr(Dwarf_P_Debug, Dwarf_P_Die,
		    Dwarf_Half, Dwarf_P_Expr, Dwarf_Error *error);
Dwarf_Unsigned	dwarf_add_arange(Dwarf_P_Debug, Dwarf_Addr, Dwarf_Unsigned,
		    Dwarf_Signed, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_arange_b(Dwarf_P_Debug, Dwarf_Addr, Dwarf_Unsigned,
		    Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Addr, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_die_to_debug(Dwarf_P_Debug, Dwarf_P_Die,
		    Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_directory_decl(Dwarf_P_Debug, char *, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_expr_addr(Dwarf_P_Expr, Dwarf_Unsigned,
		    Dwarf_Signed, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_expr_addr_b(Dwarf_P_Expr, Dwarf_Unsigned,
		    Dwarf_Unsigned, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_expr_gen(Dwarf_P_Expr, Dwarf_Small, Dwarf_Unsigned,
		    Dwarf_Unsigned, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_file_decl(Dwarf_P_Debug, char *, Dwarf_Unsigned,
		    Dwarf_Unsigned, Dwarf_Unsigned, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_frame_cie(Dwarf_P_Debug, char *, Dwarf_Small,
		    Dwarf_Small, Dwarf_Small, Dwarf_Ptr, Dwarf_Unsigned,
		    Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_frame_fde(Dwarf_P_Debug, Dwarf_P_Fde, Dwarf_P_Die,
		    Dwarf_Unsigned, Dwarf_Addr, Dwarf_Unsigned, Dwarf_Unsigned,
		    Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_frame_fde_b(Dwarf_P_Debug, Dwarf_P_Fde, Dwarf_P_Die,
		    Dwarf_Unsigned, Dwarf_Addr, Dwarf_Unsigned, Dwarf_Unsigned,
		    Dwarf_Unsigned, Dwarf_Addr, Dwarf_Error *);
Dwarf_Unsigned	dwarf_add_line_entry(Dwarf_P_Debug, Dwarf_Unsigned,
		    Dwarf_Addr, Dwarf_Unsigned, Dwarf_Signed, Dwarf_Bool,
		    Dwarf_Bool, Dwarf_Error *);
int		dwarf_arrayorder(Dwarf_Die, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_attr(Dwarf_Die, Dwarf_Half, Dwarf_Attribute *,
		    Dwarf_Error *);
int		dwarf_attrlist(Dwarf_Die, Dwarf_Attribute **,
		    Dwarf_Signed *, Dwarf_Error *);
int		dwarf_attrval_flag(Dwarf_Die, uint64_t, Dwarf_Bool *,
		    Dwarf_Error *);
int		dwarf_attrval_signed(Dwarf_Die, uint64_t, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_attrval_string(Dwarf_Die, uint64_t, const char **,
		    Dwarf_Error *);
int		dwarf_attrval_unsigned(Dwarf_Die, uint64_t, Dwarf_Unsigned *,
		    Dwarf_Error *);
int		dwarf_bitoffset(Dwarf_Die, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_bitsize(Dwarf_Die, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_bytesize(Dwarf_Die, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_child(Dwarf_Die, Dwarf_Die *, Dwarf_Error *);
int		dwarf_def_macro(Dwarf_P_Debug, Dwarf_Unsigned, char *, char *,
		    Dwarf_Error *);
int		dwarf_diename(Dwarf_Die, char **, Dwarf_Error *);
int		dwarf_dieoffset(Dwarf_Die, Dwarf_Off *, Dwarf_Error *);
int		dwarf_die_abbrev_code(Dwarf_Die);
int		dwarf_die_CU_offset(Dwarf_Die, Dwarf_Off *, Dwarf_Error *);
int		dwarf_die_CU_offset_range(Dwarf_Die, Dwarf_Off *, Dwarf_Off *,
		    Dwarf_Error *);
Dwarf_P_Die	dwarf_die_link(Dwarf_P_Die, Dwarf_P_Die, Dwarf_P_Die,
		    Dwarf_P_Die, Dwarf_P_Die, Dwarf_Error *);
int		dwarf_elf_init(Elf *, int, Dwarf_Handler, Dwarf_Ptr,
		    Dwarf_Debug *, Dwarf_Error *);
int		dwarf_end_macro_file(Dwarf_P_Debug, Dwarf_Error *);
int		dwarf_expand_frame_instructions(Dwarf_Debug, Dwarf_Ptr,
		    Dwarf_Unsigned, Dwarf_Frame_Op **, Dwarf_Signed *,
		    Dwarf_Error *);
Dwarf_Unsigned	dwarf_expr_current_offset(Dwarf_P_Expr, Dwarf_Error *);
Dwarf_Addr	dwarf_expr_into_block(Dwarf_P_Expr, Dwarf_Unsigned *,
		    Dwarf_Error *);
char		*dwarf_find_macro_value_start(char *);
int		dwarf_finish(Dwarf_Debug, Dwarf_Error *);
int		dwarf_formref(Dwarf_Attribute, Dwarf_Off *, Dwarf_Error *);
int		dwarf_frame_instructions_dealloc(Dwarf_Frame_Op *, Dwarf_Signed,
		    Dwarf_Error *);
int		dwarf_func_cu_offset(Dwarf_Func, Dwarf_Off *, Dwarf_Error *);
int		dwarf_func_die_offset(Dwarf_Func, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_func_name_offsets(Dwarf_Func, char **,
		    Dwarf_Off *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_funcname(Dwarf_Func, char **, Dwarf_Error *);
int		dwarf_global_formref(Dwarf_Attribute, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_formaddr(Dwarf_Attribute, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_formblock(Dwarf_Attribute, Dwarf_Block *, Dwarf_Error *);
int		dwarf_formflag(Dwarf_Attribute, Dwarf_Bool *, Dwarf_Error *);
int		dwarf_formstring(Dwarf_Attribute, char **, Dwarf_Error *);
int		dwarf_formsdata(Dwarf_Attribute, Dwarf_Signed *, Dwarf_Error *);
int		dwarf_formudata(Dwarf_Attribute, Dwarf_Unsigned *,
		    Dwarf_Error *);
int		dwarf_get_ACCESS_name(unsigned, const char **);
int		dwarf_get_AT_name(unsigned, const char **);
int		dwarf_get_ATE_name(unsigned, const char **);
int		dwarf_get_CC_name(unsigned, const char **);
int		dwarf_get_CFA_name(unsigned, const char **);
int		dwarf_get_CHILDREN_name(unsigned, const char **);
int		dwarf_get_DSC_name(unsigned, const char **);
int		dwarf_get_FORM_name(unsigned, const char **);
int		dwarf_get_ID_name(unsigned, const char **);
int		dwarf_get_INL_name(unsigned, const char **);
int		dwarf_get_LANG_name(unsigned, const char **);
int		dwarf_get_LNE_name(unsigned, const char **);
int		dwarf_get_LNS_name(unsigned, const char **);
int		dwarf_get_MACINFO_name(unsigned, const char **);
int		dwarf_get_OP_name(unsigned, const char **);
int		dwarf_get_ORD_name(unsigned, const char **);
int		dwarf_get_TAG_name(unsigned, const char **);
int		dwarf_get_VIRTUALITY_name(unsigned, const char **);
int		dwarf_get_VIS_name(unsigned, const char **);
int		dwarf_get_abbrev(Dwarf_Debug, Dwarf_Unsigned, Dwarf_Abbrev *,
		    Dwarf_Unsigned *, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_get_abbrev_children_flag(Dwarf_Abbrev, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_abbrev_code(Dwarf_Abbrev, Dwarf_Unsigned *,
		    Dwarf_Error *);
int		dwarf_get_abbrev_entry(Dwarf_Abbrev, Dwarf_Signed, Dwarf_Half *,
		    Dwarf_Signed *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_get_abbrev_tag(Dwarf_Abbrev, Dwarf_Half *, Dwarf_Error *);
int		dwarf_get_address_size(Dwarf_Debug, Dwarf_Half *,
		    Dwarf_Error *);
int		dwarf_get_arange(Dwarf_Arange *, Dwarf_Unsigned, Dwarf_Addr,
		    Dwarf_Arange *, Dwarf_Error *);
int		dwarf_get_aranges(Dwarf_Debug, Dwarf_Arange **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_arange_cu_header_offset(Dwarf_Arange, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_get_arange_info(Dwarf_Arange, Dwarf_Addr *,
		    Dwarf_Unsigned *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_get_cie_info(Dwarf_Cie, Dwarf_Unsigned *, Dwarf_Small *,
		    char **, Dwarf_Unsigned *, Dwarf_Unsigned *, Dwarf_Half *,
		    Dwarf_Ptr *, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_get_cie_of_fde(Dwarf_Fde, Dwarf_Cie *, Dwarf_Error *);
int		dwarf_get_cu_die_offset(Dwarf_Arange, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_get_cu_die_offset_given_cu_header_offset(Dwarf_Debug,
		    Dwarf_Off, Dwarf_Off *, Dwarf_Error *);
int		dwarf_get_elf(Dwarf_Debug, Elf **, Dwarf_Error *);
int		dwarf_get_fde_at_pc(Dwarf_Fde *, Dwarf_Addr, Dwarf_Fde *,
		    Dwarf_Addr *, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_get_fde_info_for_all_regs(Dwarf_Fde, Dwarf_Addr,
		    Dwarf_Regtable *, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_get_fde_info_for_all_regs3(Dwarf_Fde, Dwarf_Addr,
		    Dwarf_Regtable3 *, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_get_fde_info_for_cfa_reg3(Dwarf_Fde, Dwarf_Addr,
		    Dwarf_Small *, Dwarf_Signed *, Dwarf_Signed *, Dwarf_Signed *,
		    Dwarf_Ptr *, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_get_fde_info_for_reg(Dwarf_Fde, Dwarf_Half, Dwarf_Addr,
		    Dwarf_Signed *, Dwarf_Signed *, Dwarf_Signed *,
		    Dwarf_Addr *, Dwarf_Error *);
int		dwarf_get_fde_info_for_reg3(Dwarf_Fde, Dwarf_Half, Dwarf_Addr,
		    Dwarf_Small *, Dwarf_Signed *, Dwarf_Signed *,
		    Dwarf_Signed *, Dwarf_Ptr *, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_get_fde_instr_bytes(Dwarf_Fde, Dwarf_Ptr *,
		    Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_get_fde_list(Dwarf_Debug, Dwarf_Cie **, Dwarf_Signed *,
		    Dwarf_Fde **, Dwarf_Signed *, Dwarf_Error *);
int		dwarf_get_fde_list_eh(Dwarf_Debug, Dwarf_Cie **, Dwarf_Signed *,
		    Dwarf_Fde **, Dwarf_Signed *, Dwarf_Error *);
int		dwarf_get_fde_n(Dwarf_Fde *, Dwarf_Unsigned, Dwarf_Fde *,
		    Dwarf_Error *);
int		dwarf_get_fde_range(Dwarf_Fde, Dwarf_Addr *, Dwarf_Unsigned *,
		    Dwarf_Ptr *, Dwarf_Unsigned *, Dwarf_Off *, Dwarf_Signed *,
		    Dwarf_Off *, Dwarf_Error *);
int		dwarf_get_funcs(Dwarf_Debug, Dwarf_Func **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_globals(Dwarf_Debug, Dwarf_Global **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_loclist_entry(Dwarf_Debug, Dwarf_Unsigned,
		    Dwarf_Addr *, Dwarf_Addr *, Dwarf_Ptr *, Dwarf_Unsigned *,
		    Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_get_macro_details(Dwarf_Debug, Dwarf_Off, Dwarf_Unsigned,
		    Dwarf_Signed *, Dwarf_Macro_Details **, Dwarf_Error *);
int		dwarf_get_pubtypes(Dwarf_Debug, Dwarf_Type **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_ranges(Dwarf_Debug, Dwarf_Off, Dwarf_Ranges **,
		    Dwarf_Signed *, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_get_ranges_a(Dwarf_Debug, Dwarf_Off, Dwarf_Die,
		    Dwarf_Ranges **, Dwarf_Signed *, Dwarf_Unsigned *,
		    Dwarf_Error *);
Dwarf_Ptr	dwarf_get_section_bytes(Dwarf_P_Debug, Dwarf_Signed,
		    Dwarf_Signed *, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_get_str(Dwarf_Debug, Dwarf_Off, char **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_types(Dwarf_Debug, Dwarf_Type **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_vars(Dwarf_Debug, Dwarf_Var **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_get_weaks(Dwarf_Debug, Dwarf_Weak **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_global_cu_offset(Dwarf_Global, Dwarf_Off *, Dwarf_Error *);
int		dwarf_global_die_offset(Dwarf_Global, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_global_name_offsets(Dwarf_Global, char **,
		    Dwarf_Off *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_globname(Dwarf_Global, char **, Dwarf_Error *);
int		dwarf_hasattr(Dwarf_Die, Dwarf_Half, Dwarf_Bool *,
		    Dwarf_Error *);
int		dwarf_hasform(Dwarf_Attribute, Dwarf_Half, Dwarf_Bool *,
		    Dwarf_Error *);
int		dwarf_highpc(Dwarf_Die, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_line_srcfileno(Dwarf_Line, Dwarf_Unsigned *,
		    Dwarf_Error *);
int		dwarf_lineaddr(Dwarf_Line, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_linebeginstatement(Dwarf_Line, Dwarf_Bool *,
		    Dwarf_Error *);
int		dwarf_lineblock(Dwarf_Line, Dwarf_Bool *, Dwarf_Error *);
int		dwarf_lineendsequence(Dwarf_Line, Dwarf_Bool *, Dwarf_Error *);
int		dwarf_lineno(Dwarf_Line, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_lineoff(Dwarf_Line, Dwarf_Signed *, Dwarf_Error *);
int		dwarf_linesrc(Dwarf_Line, char **, Dwarf_Error *);
Dwarf_Unsigned	dwarf_lne_end_sequence(Dwarf_P_Debug, Dwarf_Addr, Dwarf_Error *);
Dwarf_Unsigned	dwarf_lne_set_address(Dwarf_P_Debug, Dwarf_Addr, Dwarf_Unsigned,
		    Dwarf_Error *);
int		dwarf_locdesc(Dwarf_Die, uint64_t, Dwarf_Locdesc **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_locdesc_free(Dwarf_Locdesc *, Dwarf_Error *);
int		dwarf_loclist(Dwarf_Attribute, Dwarf_Locdesc **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_loclist_n(Dwarf_Attribute, Dwarf_Locdesc ***,
		    Dwarf_Signed *, Dwarf_Error *);
int		dwarf_loclist_from_expr(Dwarf_Debug, Dwarf_Ptr, Dwarf_Unsigned,
		    Dwarf_Locdesc **, Dwarf_Signed *, Dwarf_Error *);
int		dwarf_loclist_from_expr_a(Dwarf_Debug, Dwarf_Ptr,
		    Dwarf_Unsigned, Dwarf_Half, Dwarf_Locdesc **,
		    Dwarf_Signed *, Dwarf_Error *);
int		dwarf_loclist_from_expr_dealloc(Dwarf_Locdesc *, Dwarf_Error *);
int		dwarf_lowpc(Dwarf_Die, Dwarf_Addr *, Dwarf_Error *);
int		dwarf_init(int, int, Dwarf_Handler, Dwarf_Ptr, Dwarf_Debug *,
		    Dwarf_Error *);
Dwarf_P_Die	dwarf_new_die(Dwarf_P_Debug, Dwarf_Tag, Dwarf_P_Die,
		    Dwarf_P_Die, Dwarf_P_Die, Dwarf_P_Die, Dwarf_Error *);
Dwarf_P_Expr	dwarf_new_expr(Dwarf_P_Debug, Dwarf_Error *);
Dwarf_P_Fde	dwarf_new_fde(Dwarf_P_Debug, Dwarf_Error *);
int		dwarf_next_cu_header(Dwarf_Debug, Dwarf_Unsigned *,
		    Dwarf_Half *, Dwarf_Unsigned *, Dwarf_Half *,
		    Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_object_finish(Dwarf_Debug, Dwarf_Error *);
int		dwarf_object_init(Dwarf_Obj_Access_Interface *, Dwarf_Handler,
		    Dwarf_Ptr, Dwarf_Debug *, Dwarf_Error *);
int		dwarf_offdie(Dwarf_Debug, Dwarf_Off, Dwarf_Die *,
		    Dwarf_Error *);
Dwarf_P_Debug	dwarf_producer_init(Dwarf_Unsigned, Dwarf_Callback_Func,
		    Dwarf_Handler, Dwarf_Ptr, Dwarf_Error *);
Dwarf_P_Debug	dwarf_producer_init_b(Dwarf_Unsigned, Dwarf_Callback_Func_b,
		    Dwarf_Handler, Dwarf_Ptr, Dwarf_Error *);
int		dwarf_producer_set_isa(Dwarf_P_Debug, Dwarf_Unsigned,
		    Dwarf_Error *);
int		dwarf_pubtype_cu_offset(Dwarf_Type, Dwarf_Off *, Dwarf_Error *);
int		dwarf_pubtype_die_offset(Dwarf_Type, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_pubtype_name_offsets(Dwarf_Type, char **,
		    Dwarf_Off *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_pubtypename(Dwarf_Type, char **, Dwarf_Error *);
int		dwarf_ranges_dealloc(Dwarf_Debug, Dwarf_Ranges *, Dwarf_Signed);
Dwarf_Half	dwarf_set_frame_cfa_value(Dwarf_Debug, Dwarf_Half);
Dwarf_Half	dwarf_set_frame_rule_initial_value(Dwarf_Debug, Dwarf_Half);
Dwarf_Half	dwarf_set_frame_rule_table_size(Dwarf_Debug, Dwarf_Half);
Dwarf_Half	dwarf_set_frame_same_value(Dwarf_Debug, Dwarf_Half);
Dwarf_Half	dwarf_set_frame_undefined_value(Dwarf_Debug, Dwarf_Half);
int		dwarf_type_cu_offset(Dwarf_Type, Dwarf_Off *, Dwarf_Error *);
int		dwarf_type_die_offset(Dwarf_Type, Dwarf_Off *, Dwarf_Error *);
int		dwarf_type_name_offsets(Dwarf_Type, char **,
		    Dwarf_Off *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_typename(Dwarf_Type, char **, Dwarf_Error *);
int		dwarf_var_cu_offset(Dwarf_Var, Dwarf_Off *, Dwarf_Error *);
int		dwarf_var_die_offset(Dwarf_Var, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_var_name_offsets(Dwarf_Var, char **,
		    Dwarf_Off *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_varname(Dwarf_Var, char **, Dwarf_Error *);
int		dwarf_weak_cu_offset(Dwarf_Weak, Dwarf_Off *, Dwarf_Error *);
int		dwarf_weak_die_offset(Dwarf_Weak, Dwarf_Off *,
		    Dwarf_Error *);
int		dwarf_weak_name_offsets(Dwarf_Weak, char **,
		    Dwarf_Off *, Dwarf_Off *, Dwarf_Error *);
int		dwarf_weakname(Dwarf_Weak, char **, Dwarf_Error *);
int		dwarf_siblingof(Dwarf_Debug, Dwarf_Die, Dwarf_Die *, Dwarf_Error *);
int		dwarf_srcfiles(Dwarf_Die, char ***, Dwarf_Signed *, Dwarf_Error *);
int		dwarf_srclang(Dwarf_Die, Dwarf_Unsigned *, Dwarf_Error *);
int		dwarf_srclines(Dwarf_Die, Dwarf_Line **, Dwarf_Signed *,
		    Dwarf_Error *);
int		dwarf_start_macro_file(Dwarf_P_Debug, Dwarf_Unsigned,
		    Dwarf_Unsigned, Dwarf_Error *);
int		dwarf_tag(Dwarf_Die, Dwarf_Half *, Dwarf_Error *);
Dwarf_Signed	dwarf_transform_to_disk_form(Dwarf_P_Debug, Dwarf_Error *);
int		dwarf_undef_macro(Dwarf_P_Debug, Dwarf_Unsigned, char *,
		    Dwarf_Error *);
int		dwarf_vendor_ext(Dwarf_P_Debug, Dwarf_Unsigned, char *,
		    Dwarf_Error *);
int		dwarf_whatattr(Dwarf_Attribute, Dwarf_Half *, Dwarf_Error *);
int		dwarf_whatform(Dwarf_Attribute, Dwarf_Half *, Dwarf_Error *);
int		dwarf_whatform_direct(Dwarf_Attribute, Dwarf_Half *,
		    Dwarf_Error *);
void		dwarf_dealloc(Dwarf_Debug, Dwarf_Ptr, Dwarf_Unsigned);
__END_DECLS

#endif /* !_LIBDWARF_H_ */
