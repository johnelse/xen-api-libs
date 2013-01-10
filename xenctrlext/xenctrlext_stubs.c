/*
 * Copyright (C) Citrix Systems Inc.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License as published
 * by the Free Software Foundation; version 2.1 only. with the special
 * exception on linking described in file LICENSE.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License for more details.
 */

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <xenctrl.h>

#include <sys/mman.h>

#include <caml/mlvalues.h>
#include <caml/memory.h>
#include <caml/alloc.h>
#include <caml/custom.h>
#include <caml/fail.h>
#include <caml/signals.h>
#include <caml/callback.h>
#include <caml/bigarray.h>

#define _H(__h) ((xc_interface *)(__h))
#define _D(__d) ((uint32_t)Int_val(__d))

/* From xenctrl_stubs */
#define ERROR_STRLEN 1024
static void failwith_xc(xc_interface *xch)
{
        static char error_str[ERROR_STRLEN];
        if (xch) {
                const xc_error *error = xc_get_last_error(xch);
                if (error->code == XC_ERROR_NONE)
                        snprintf(error_str, ERROR_STRLEN, "%d: %s", errno, strerror(errno));
                else
                        snprintf(error_str, ERROR_STRLEN, "%d: %s: %s",
                                 error->code,
                                 xc_error_code_to_desc(error->code),
                                 error->message);
        } else {
                snprintf(error_str, ERROR_STRLEN, "Unable to open XC interface");
        }
        caml_raise_with_string(*caml_named_value("xc.error"), error_str);
}

CAMLprim value stub_xenctrlext_get_runstate_info(value xch, value domid)
{
	CAMLparam2(xch, domid);
#if defined(XENCTRL_HAS_GET_RUNSTATE_INFO)
	CAMLlocal1(result);
	xc_runstate_info_t info;
	int retval;

	retval = xc_get_runstate_info(_H(xch), _D(domid), &info);
	if (retval < 0)
		failwith_xc(_H(xch));

	/* Store
	   0 : state (int32)
	   1 : missed_changes (int32)
	   2 : state_entry_time (int64)
	   3-8 : times (int64s)
	*/
	result = caml_alloc_tuple(9);
	Store_field(result, 0, caml_copy_int32(info.state));
	Store_field(result, 1, caml_copy_int32(info.missed_changes));
	Store_field(result, 2, caml_copy_int64(info.state_entry_time));
	Store_field(result, 3, caml_copy_int64(info.time[0]));
	Store_field(result, 4, caml_copy_int64(info.time[1]));
	Store_field(result, 5, caml_copy_int64(info.time[2]));
	Store_field(result, 6, caml_copy_int64(info.time[3]));
	Store_field(result, 7, caml_copy_int64(info.time[4]));
	Store_field(result, 8, caml_copy_int64(info.time[5]));

	CAMLreturn(result);
#else
	caml_failwith("XENCTRL_HAS_GET_RUNSTATE_INFO not defined");
#endif
}

CAMLprim value stub_xenctrlext_get_boot_cpufeatures(value xch)
{
	CAMLparam1(xch);
#if defined(XENCTRL_HAS_GET_CPUFEATURES)
	CAMLlocal1(v);
	uint32_t a, b, c, d, e, f, g, h;
	int ret;

	ret = xc_get_boot_cpufeatures(_H(xch), &a, &b, &c, &d, &e, &f, &g, &h);
	if (ret < 0)
	  failwith_xc(_H(xch));
 
	v = caml_alloc_tuple(8);
	Store_field(v, 0, caml_copy_int32(a));
	Store_field(v, 1, caml_copy_int32(b));
	Store_field(v, 2, caml_copy_int32(c));
	Store_field(v, 3, caml_copy_int32(d));
	Store_field(v, 4, caml_copy_int32(e));
	Store_field(v, 5, caml_copy_int32(f));
	Store_field(v, 6, caml_copy_int32(g));
	Store_field(v, 7, caml_copy_int32(h));

	CAMLreturn(v);
#else
	caml_failwith("XENCTRL_HAS_GET_CPUFEATURES not defined");
#endif
}

static int xcext_domain_send_s3resume(xc_interface *xch, unsigned int domid)
{
	return xc_set_hvm_param(xch, domid, HVM_PARAM_ACPI_S_STATE, 0);
}

static int xcext_domain_set_timer_mode(xc_interface *xch, unsigned int domid, int mode)
{
	return xc_set_hvm_param(xch, domid,
                            HVM_PARAM_TIMER_MODE, (unsigned long) mode);
}

static int xcext_domain_set_hpet(xc_interface *xch, unsigned int domid, int hpet)
{
	return xc_set_hvm_param(xch, domid, HVM_PARAM_HPET_ENABLED, (unsigned long) hpet);
}
static int xcext_domain_set_vpt_align(xc_interface *xch, unsigned int domid, int vpt_align)
{
	return xc_set_hvm_param(xch, domid, HVM_PARAM_HPET_ENABLED, (unsigned long) vpt_align);
}

CAMLprim value stub_xenctrlext_domain_get_acpi_s_state(value xch, value domid)
{
	CAMLparam2(xch, domid);
	unsigned long v;
	int ret;

	ret = xc_get_hvm_param(_H(xch), _D(domid), HVM_PARAM_ACPI_S_STATE, &v);
	if (ret != 0)
		failwith_xc(_H(xch));

	CAMLreturn(Val_int(v));
}

CAMLprim value stub_xenctrlext_domain_send_s3resume(value xch, value domid)
{
	CAMLparam2(xch, domid);
	xcext_domain_send_s3resume(_H(xch), _D(domid));
	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_domain_set_timer_mode(value xch, value id, value mode)
{
	CAMLparam3(xch, id, mode);
	int ret;

	ret = xcext_domain_set_timer_mode(_H(xch), _D(id), Int_val(mode));
	if (ret < 0)
		failwith_xc(_H(xch));
	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_domain_set_hpet(value xch, value id, value mode)
{
	CAMLparam3(xch, id, mode);
	int ret;

	ret = xcext_domain_set_hpet(_H(xch), _D(id), Int_val(mode));
	if (ret < 0)
		failwith_xc(_H(xch));
	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_domain_set_vpt_align(value xch, value id, value mode)
{
	CAMLparam3(xch, id, mode);
	int ret;

	ret = xcext_domain_set_vpt_align(_H(xch), _D(id), Int_val(mode));
	if (ret < 0)
		failwith_xc(_H(xch));
	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_domain_trigger_power(value xch, value domid)
{
	CAMLparam2(xch, domid);
#if defined(XENCTRL_HAS_TRIGGER_POWER)
	xc_domain_trigger_power(_H(xch), _D(domid));
#endif
	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_domain_trigger_sleep(value xch, value domid)
{
	CAMLparam2(xch, domid);
#if defined(XENCTRL_HAS_TRIGGER_SLEEP)
	xc_domain_trigger_sleep(_H(xch), _D(domid));
#endif
	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_domain_suppress_spurious_page_faults(value xch,
                                                           value domid)
{
	CAMLparam2(xch, domid);

	int retval = xc_domain_suppress_spurious_page_faults(_H(xch), _D(domid));
	if (retval)
		failwith_xc(_H(xch));
	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_get_max_nr_cpus(value xch)
{
	CAMLparam1(xch);
	xc_physinfo_t c_physinfo;
	int r;

	caml_enter_blocking_section();
	r = xc_physinfo(_H(xch), &c_physinfo);
	caml_leave_blocking_section();

	if (r)
		failwith_xc(_H(xch));

	CAMLreturn(Val_int(c_physinfo.max_cpu_id + 1));
}

CAMLprim value stub_xenctrlext_domain_set_target(value xch,
					 value domid,
					 value target)
{
	CAMLparam3(xch, domid, target);

	int retval = xc_domain_set_target(_H(xch), _D(domid), _D(target));
	if (retval)
		failwith_xc(_H(xch));
	CAMLreturn(Val_unit);
}

/* Common grant table code. */
#define XC_GNTTAB_BIGARRAY (CAML_BA_UINT8 | CAML_BA_C_LAYOUT | CAML_BA_EXTERNAL)

/* xc_gntshr_* bindings */
#define _GS(__gs) ((xc_gntshr *)(__gs))

CAMLprim value stub_xenctrlext_gntshr_open(void)
{
	CAMLparam0();
	xc_gntshr *xgh;

	xgh = xc_gntshr_open(NULL, 0);
	if (NULL == xgh)
		failwith_xc(NULL);
	CAMLreturn((value)xgh);
}

CAMLprim value stub_xenctrlext_gntshr_close(value xgh)
{
	CAMLparam1(xgh);

	xc_gntshr_close(_GS(xgh));

	CAMLreturn(Val_unit);
}

CAMLprim value stub_xenctrlext_gntshr_share_pages(value xgh, value domid, value count, value writeable) {
	CAMLparam4(xgh, domid, count, writeable);
	CAMLlocal4(result, ml_refs, ml_refs_cons, ml_map);
	void *map;
	uint32_t *refs;
	uint32_t c_domid;
	int c_count;
	int i;

	c_count = Int_val(count);
	c_domid = Int32_val(domid);
	result = caml_alloc(2, 0);
	refs = (uint32_t *) malloc(c_count * sizeof(uint32_t));

	map = xc_gntshr_share_pages(_GS(xgh), c_domid, c_count, refs, Bool_val(writeable));

	if(NULL == map) {
		free(refs);
		failwith_xc(_GS(xgh));
	}

	// Construct the list of grant references.
	ml_refs = Val_emptylist;
	for(i = c_count - 1; i >= 0; i--) {
		ml_refs_cons = caml_alloc(2, 0);

		Store_field(ml_refs_cons, 0, caml_copy_int32(refs[i]));
		Store_field(ml_refs_cons, 1, ml_refs);

		ml_refs = ml_refs_cons;
	}

	ml_map = caml_ba_alloc_dims(XC_GNTTAB_BIGARRAY, 1,
		map, c_count << XC_PAGE_SHIFT);

	Store_field(result, 0, ml_refs);
	Store_field(result, 1, ml_map);

	free(refs);
	CAMLreturn(result);
}

CAMLprim value stub_xenctrlext_gntshr_munmap(value xgh, value share) {
	CAMLparam2(xgh, share);
	CAMLlocal1(ml_map);

	ml_map = Field(share, 1);

	int size = Caml_ba_array_val(ml_map)->dim[0];
	int pages = size >> XC_PAGE_SHIFT;
	int result = xc_gntshr_munmap(_GS(xgh), Caml_ba_data_val(ml_map), pages);
	if(result != 0)
		failwith_xc(_GS(xgh));

	CAMLreturn(Val_unit);
}

/* xc_gnttab_* bindings */
#define _GT(__g) ((xc_gnttab *)(__g))
CAMLprim value stub_xc_gnttab_open(void)
{
	CAMLparam0();
	xc_gnttab *xgh;
	xgh = xc_gnttab_open(NULL, 0);
	if (xgh == NULL)
		caml_failwith("Failed to open interface");
	CAMLreturn((value)xgh);
}

CAMLprim value stub_xc_gnttab_close(value xgh)
{
	CAMLparam1(xgh);

	xc_gnttab_close(_GT(xgh));

	CAMLreturn(Val_unit);
}

CAMLprim value stub_xc_gnttab_get_perm(value perm)
{
	CAMLparam1(perm);
	int result;
	switch (Int_val(perm)){
	case 0:
		result = PROT_READ;
		break;
	case 1:
		result = PROT_WRITE;
		break;
	default:
		result = PROT_NONE;
		break;
	}
	CAMLreturn(Val_int(result));
}

#define XC_GNTTAB_BIGARRAY (CAML_BA_UINT8 | CAML_BA_C_LAYOUT | CAML_BA_EXTERNAL)

CAMLprim value stub_xc_gnttab_map_grant_ref(
	value xgh,
	value domid,
	value reference,
	value perms
	)
{
	CAMLparam4(xgh, domid, reference, perms);
	CAMLlocal1(contents);
	uint32_t c_domid, c_reference;
	int c_perm;

	c_domid = Int32_val(domid);
	c_reference = Int32_val(reference);
	c_perm = Int_val(perms);

	void *map = xc_gnttab_map_grant_ref(_GT(xgh),
		c_domid, c_reference, c_perm);

	if(map==NULL) {
		caml_failwith("Failed to map grant ref");
	}

	contents = caml_ba_alloc_dims(XC_GNTTAB_BIGARRAY, 1,
		map, 1 << XC_PAGE_SHIFT);
	CAMLreturn(contents);
}

CAMLprim value stub_xc_gnttab_map_grant_refs(
	value xgh,
	value array,
	value perms)
{
	CAMLparam3(xgh, array, perms);
	CAMLlocal3(domid, reference, contents);
	int c_perms = Int_val(perms);
	int count = Wosize_val(array) / 2;
	uint32_t domids[count];
	uint32_t refs[count];
	int i;

	for (i = 0; i < count; i++){
		domids[i] = Int32_val(Field(array, i * 2 + 0));
		refs[i] = Int32_val(Field(array, i * 2 + 1));
	}
	void *map = xc_gnttab_map_grant_refs(
		_GT(xgh),
		count,
		domids,
		refs,
		c_perms
	);

	if(map==NULL) {
		caml_failwith("Failed to map grant ref");
	}

	contents = caml_ba_alloc_dims(XC_GNTTAB_BIGARRAY, 1,
		map, count << XC_PAGE_SHIFT);
	CAMLreturn(contents);
}

CAMLprim value stub_xc_gnttab_unmap(value xgh, value array) 
{
	CAMLparam2(xgh, array);

	int size = Caml_ba_array_val(array)->dim[0];
	int pages = size >> XC_PAGE_SHIFT;
	int result = xc_gnttab_munmap(_GT(xgh), Caml_ba_data_val(array), pages);
	if(result!=0) {
		caml_failwith("Failed to unmap grant");
	}

	CAMLreturn(Val_unit);
}

/* 
* Local variables: 
* indent-tabs-mode: t
*/
