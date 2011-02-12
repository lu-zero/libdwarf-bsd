# $Id$

TOP=	${.CURDIR}/..

LIB=	dwarf

SRCS=	\
	dwarf_abbrev.c		\
	dwarf_arange.c		\
	dwarf_attr.c		\
	dwarf_attrval.c		\
	dwarf_cu.c		\
	dwarf_dealloc.c		\
	dwarf_die.c		\
	dwarf_dump.c		\
	dwarf_errmsg.c		\
	dwarf_finish.c		\
	dwarf_form.c		\
	dwarf_frame.c		\
	dwarf_funcs.c		\
	dwarf_init.c		\
	dwarf_lineno.c		\
	dwarf_loclist.c		\
	dwarf_macinfo.c		\
	dwarf_pro_arange.c	\
	dwarf_pro_attr.c	\
	dwarf_pro_die.c		\
	dwarf_pro_expr.c	\
	dwarf_pro_finish.c	\
	dwarf_pro_frame.c	\
	dwarf_pro_funcs.c	\
	dwarf_pro_init.c	\
	dwarf_pro_lineno.c	\
	dwarf_pro_macinfo.c	\
	dwarf_pro_pubnames.c	\
	dwarf_pro_reloc.c	\
	dwarf_pro_sections.c	\
	dwarf_pro_types.c	\
	dwarf_pro_vars.c	\
	dwarf_pro_weaks.c	\
	dwarf_pubnames.c	\
	dwarf_pubtypes.c	\
	dwarf_ranges.c		\
	dwarf_reloc.c		\
	dwarf_seterror.c	\
	dwarf_str.c		\
	dwarf_types.c		\
	dwarf_vars.c		\
	dwarf_weaks.c		\
	libdwarf.c		\
	libdwarf_abbrev.c	\
	libdwarf_arange.c	\
	libdwarf_attr.c		\
	libdwarf_die.c		\
	libdwarf_error.c	\
	libdwarf_elf_access.c	\
	libdwarf_elf_init.c	\
	libdwarf_frame.c	\
	libdwarf_info.c		\
	libdwarf_init.c		\
	libdwarf_lineno.c	\
	libdwarf_loc.c		\
	libdwarf_loclist.c	\
	libdwarf_macinfo.c	\
	libdwarf_nametbl.c	\
	libdwarf_ranges.c	\
	libdwarf_reloc.c	\
	libdwarf_rw.c		\
	libdwarf_sections.c	\
	libdwarf_str.c

INCS=	dwarf.h libdwarf.h

GENSRCS=	dwarf_pubnames.c dwarf_pubtypes.c dwarf_weaks.c \
		dwarf_funcs.c dwarf_vars.c dwarf_types.c	\
		dwarf_pro_pubnames.c dwarf_pro_weaks.c		\
		dwarf_pro_funcs.c dwarf_pro_types.c		\
		dwarf_pro_vars.c
CLEANFILES=	${GENSRCS}

SHLIB_MAJOR=	3

WARNS?=	6

LDADD+=		-lelf

MAN=	dwarf_attr.3					\
	dwarf_attrlist.3				\
	dwarf_attrval_signed.3				\
	dwarf_child.3					\
	dwarf_dealloc.3					\
	dwarf_die_abbrev_code.3				\
	dwarf_diename.3					\
	dwarf_dieoffset.3				\
	dwarf_errmsg.3					\
	dwarf_errno.3					\
	dwarf_finish.3					\
	dwarf_formaddr.3				\
	dwarf_formblock.3				\
	dwarf_formexprloc.3				\
	dwarf_formflag.3				\
	dwarf_formref.3					\
	dwarf_formsig8.3				\
	dwarf_formstring.3				\
	dwarf_formudata.3				\
	dwarf_get_address_size.3			\
	dwarf_get_elf.3					\
	dwarf_hasattr.3					\
	dwarf_hasform.3					\
	dwarf_highpc.3					\
	dwarf_init.3					\
	dwarf_lineno.3					\
	dwarf_next_cu_header.3				\
	dwarf_seterrarg.3				\
	dwarf_srcfiles.3				\
	dwarf_srclines.3				\
	dwarf_tag.3					\
	dwarf_whatattr.3

MLINKS+= \
	dwarf_attrval_signed.3	dwarf_attrval_flag.3	\
	dwarf_attrval_signed.3	dwarf_attrval_string.3	\
	dwarf_attrval_signed.3	dwarf_attrval_unsigned.3 \
	dwarf_child.3	dwarf_offdie.3			\
	dwarf_child.3	dwarf_siblingof.3		\
	dwarf_dealloc.3	dwarf_ranges_dealloc.3		\
	dwarf_dealloc.3	dwarf_srclines_dealloc.3	\
	dwarf_init.3	dwarf_elf_init.3		\
	dwarf_dieoffset.3	dwarf_die_CU_offset.3	\
	dwarf_dieoffset.3	dwarf_die_CU_offset_range.3 \
	dwarf_dieoffset.3	dwarf_get_cu_die_offset_given_cu_header_offset.3 \
	dwarf_formref.3	dwarf_global_formref.3		\
	dwarf_formudata.3	dwarf_formsdata.3	\
	dwarf_hasform.3	dwarf_whatform.3		\
	dwarf_hasform.3	dwarf_whatform_direct.3		\
	dwarf_highpc.3	dwarf_arrayorder.3		\
	dwarf_highpc.3	dwarf_bitoffset.3		\
	dwarf_highpc.3	dwarf_bitsize.3			\
	dwarf_highpc.3	dwarf_bytesize.3		\
	dwarf_highpc.3	dwarf_lowpc.3			\
	dwarf_highpc.3	dwarf_srclang.3			\
	dwarf_lineno.3	dwarf_lineaddr.3		\
	dwarf_lineno.3	dwarf_linebeginstatement.3	\
	dwarf_lineno.3	dwarf_lineblock.3		\
	dwarf_lineno.3	dwarf_lineendsequence.3		\
	dwarf_lineno.3	dwarf_lineoff.3			\
	dwarf_lineno.3	dwarf_linesrc.3			\
	dwarf_lineno.3	dwarf_line_srcfileno.3		\
	dwarf_seterrarg.3	dwarf_seterrhand.3

dwarf_pubnames.c:	dwarf_nametbl.m4 dwarf_pubnames.m4
dwarf_pubtypes.c:	dwarf_nametbl.m4 dwarf_pubtypes.m4
dwarf_weaks.c:		dwarf_nametbl.m4 dwarf_weaks.m4
dwarf_funcs.c:		dwarf_nametbl.m4 dwarf_funcs.m4
dwarf_vars.c:		dwarf_nametbl.m4 dwarf_vars.m4
dwarf_types.c:		dwarf_nametbl.m4 dwarf_types.m4
dwarf_pro_pubnames.c:	dwarf_pro_nametbl.m4 dwarf_pro_pubnames.m4
dwarf_pro_weaks.c:	dwarf_pro_nametbl.m4 dwarf_pro_weaks.m4
dwarf_pro_funcs.c:	dwarf_pro_nametbl.m4 dwarf_pro_funcs.m4
dwarf_pro_types.c:	dwarf_pro_nametbl.m4 dwarf_pro_types.m4
dwarf_pro_vars.c:	dwarf_pro_nametbl.m4 dwarf_pro_vars.m4

.include "${TOP}/mk/elftoolchain.lib.mk"
