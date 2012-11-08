(*
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
 *)

open Xenctrl

external get_boot_cpufeatures: handle ->  (int32 * int32 * int32 * int32 * int32 * int32 * int32 * int32) = "stub_xenctrlext_get_boot_cpufeatures" 

external domain_set_timer_mode: handle -> domid -> int -> unit = "stub_xenctrlext_domain_set_timer_mode"
external domain_set_hpet: handle -> domid -> int -> unit = "stub_xenctrlext_domain_set_hpet"
external domain_set_vpt_align: handle -> domid -> int -> unit = "stub_xenctrlext_domain_set_vpt_align"

external domain_send_s3resume: handle -> domid -> unit = "stub_xenctrlext_domain_send_s3resume"
external domain_get_acpi_s_state: handle -> domid -> int = "stub_xenctrlext_domain_get_acpi_s_state"

external domain_trigger_power: handle -> domid -> unit = "stub_xenctrlext_domain_trigger_power"
external domain_trigger_sleep: handle -> domid -> unit = "stub_xenctrlext_domain_trigger_sleep"

external domain_suppress_spurious_page_faults: handle -> domid -> unit = "stub_xenctrlext_domain_suppress_spurious_page_faults"

type runstateinfo = {
  state : int32;
  missed_changes: int32;
  state_entry_time : int64;
  time0 : int64;
  time1 : int64;
  time2 : int64;
  time3 : int64;
  time4 : int64;
  time5 : int64;
}

external domain_get_runstate_info : handle -> int -> runstateinfo = "stub_xenctrlext_get_runstate_info"

external get_max_nr_cpus: handle -> int = "stub_xenctrlext_get_max_nr_cpus"

external domain_set_target: handle -> domid -> domid -> unit = "stub_xenctrlext_domain_set_target"

module Xsrawext : sig
  val set_target : int -> int -> Xenstore.Xs.con -> unit
end

module Gntcommon : sig
	type mapping
	(** A memory region associated with one or more mapped grant reference *)

	type contents = (char, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t
	(** Raw mapped memory *)

	val contents: mapping -> contents
	(** Expose the contents of a mapped memory region as a bigarray *)
end

module Gntshr : sig
	type share = {
		references: int32 list;
		(** Grant table references of the shared pages. *)
		mapping: Gntcommon.mapping;
		(** Mapping to the shared memory. *)
	}

	type handle

	external interface_open : unit -> handle = "stub_xenctrlext_gntshr_open"
	external interface_close : handle -> unit = "stub_xenctrlext_gntshr_close"

	external share_pages : handle -> int32 -> int -> bool -> share = "stub_xenctrlext_gntshr_share_pages"
	external munmap : handle -> share -> unit = "stub_xenctrlext_gntshr_munmap"
end

module Gnttab : sig
	type handle
	(** A connection to the grant device, needed for mapping/unmapping *)

	val interface_open: unit -> handle
	(** Open a connection to the grant device. This must be done before any
			calls to map or unmap. *)

	val interface_close: handle -> unit
	(** Close a connection to the grant device. Any future calls to map or
			unmap will fail. *)

	type grant = {
			domid: int32;     (** foreign domain who is exporting memory *)
			reference: int32; (** id which identifies the specific export in the foreign domain *)
	}
	(** A foreign domain must explicitly "grant" us memory and send us the
			"reference". The pair of (foreign domain id, reference) uniquely
			identifies the block of memory. This pair ("grant") is transmitted
			to us out-of-band, usually either via xenstore during device setup or
			via a shared memory ring structure. *)

	type permission =
	| RO  (** contents may only be read *)
	| RW  (** contents may be read and written *)
	(** Permissions associated with each mapping. *)

	val map: handle -> grant -> permission -> Gntcommon.mapping option
	(** Create a single mapping from a grant using a given list of permissions.
			On error this function returns None. Diagnostic details will be logged. *) 

	val mapv: handle -> grant list -> permission -> Gntcommon.mapping option
	(** Create a single contiguous mapping from a list of grants using a common
			list of permissions. Note the grant list can involve grants from multiple
			domains. On error this function returns None. Diagnostic details will
			be logged. *)

	val unmap_exn: handle -> Gntcommon.mapping -> unit
	(** Unmap a single mapping (which may involve multiple grants) *)
end

