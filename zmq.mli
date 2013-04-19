type context
type socket
type socket_type =
  | REQ
  | REP
  | DEALER
  | ROUTER
  | PUB
  | SUB
  | XPUB
  | XSUB
  | PUSH
  | PULL
  | PAIR

external init: int -> context = "caml_zmq_init"
external term: context -> unit = "caml_zmq_term"

external socket: context -> socket_type -> socket = "socket_new"
external close: socket -> unit = "socket_close"
external bind: socket -> string -> unit = "socket_bind"
external connect: socket -> string -> unit = "socket_connect"
external unbind: socket -> string -> unit = "socket_unbind"
external disconnect: socket -> string -> unit = "socket_disconnect"

external send: socket -> string -> unit = "socket_send"
external receive: socket -> string = "socket_receive"
external send_multiparts: socket -> string list -> unit = "socket_send_multiparts"
external receive_multiparts: socket -> string list = "socket_receive_multiparts"

exception Error of string
