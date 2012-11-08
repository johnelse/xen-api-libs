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

module Queueopext = struct
  open Xenstore
  include Queueop
  let set_target domid target con =
    let data = data_concat
      [ Printf.sprintf "%u" domid;
        Printf.sprintf "%u" target; ] in
    Xenbus.Xb.queue con (Xenbus.Xb.Packet.create 0 0 Xenbus.Xb.Op.Set_target data)
end

module Xsrawext = struct
  open Xenstore
  include Xsraw
  (* xs.ml has "type con = Xsraw.con" *)
  let unsafe_con (con: Xs.con) : Xsraw.con = Obj.magic con
  let set_target domid target con =
    ack (sync (Queueopext.set_target domid target) (unsafe_con con))
end

module Gntcommon = struct
	type contents = (char, Bigarray.int8_unsigned_elt, Bigarray.c_layout) Bigarray.Array1.t
	type mapping = contents
	let contents x = x
end

module Gntshr = struct
	type handle

	type share = {
		references: int32 list;
		mapping: Gntcommon.mapping;
	}

	external interface_open : unit -> handle = "stub_xenctrlext_gntshr_open"
	external interface_close : handle -> unit = "stub_xenctrlext_gntshr_close"

	external share_pages : handle -> int32 -> int -> bool -> share =
		"stub_xenctrlext_gntshr_share_pages"
	external munmap : handle -> share -> unit =
		"stub_xenctrlext_gntshr_munmap"
end

module Gnttab = struct
	type handle

	external interface_open: unit -> handle = "stub_xc_gnttab_open"

	external interface_close: handle -> unit = "stub_xc_gnttab_close"

	type grant = {
		domid: int32;
		reference: int32;
	}

	external map_exn: handle -> int32 -> int32 -> int -> Gntcommon.mapping =
		"stub_xc_gnttab_map_grant_ref"
	external mapv_exn: handle -> int32 array -> int -> Gntcommon.mapping =
		"stub_xc_gnttab_map_grant_refs"
	external unmap_exn: handle -> Gntcommon.mapping -> unit =
		"stub_xc_gnttab_unmap"

	(* Look up the values of PROT_{READ,WRITE} from the C headers. *)
	type perm = PROT_READ | PROT_WRITE
	external get_perm: perm -> int =
			"stub_xc_gnttab_get_perm"
	let _PROT_READ = get_perm PROT_READ
	let _PROT_WRITE = get_perm PROT_WRITE

	type permission = RO | RW

	let int_of_permission = function
	| RO -> _PROT_READ
	| RW -> _PROT_READ lor _PROT_WRITE

	let map h g p =
		try
			Some (map_exn h g.domid g.reference (int_of_permission p))
		with _ ->
			None

	let mapv h gs p =
		try
			let count = List.length gs in
			let grant_array = Array.create (count * 2) 0l in
			let (_: int) = List.fold_left (fun i g ->
				grant_array.(i * 2 + 0) <- g.domid;
				grant_array.(i * 2 + 1) <- g.reference;
				i + 1
			) 0 gs in
			Some (mapv_exn h grant_array (int_of_permission p))
		with _ ->
			None
end

