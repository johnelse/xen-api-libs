(*
 * Copyright (C) 2010-2011 Citrix Systems Inc.
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

type 'a t = {
	data: 'a option ref;
	m: Mutex.t;
	c: Condition.t;
}

val create_empty : unit -> 'a t

val create : 'a -> 'a t

val take : 'a t -> 'a

val try_take : 'a t -> 'a option

val put : 'a t -> 'a -> unit

val try_put : 'a t -> 'a -> bool

val is_empty : 'a t -> bool

val swap : 'a t -> 'a -> 'a

val modify : 'a t -> ('a -> 'a) -> unit
