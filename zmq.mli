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

     val init: ?io_threads:int -> ?max_sockets:int -> unit -> context
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

val subscribe:  socket -> string -> unit
val unsubscribe:  socket -> string -> unit

type 'a socket_option
val get_socket_option: 'a socket_option -> socket -> 'a
val set_socket_option: 'a socket_option -> socket -> 'a -> unit

val sndhwm: nativeint socket_option
val rcvhwm: nativeint socket_option
val identity: string socket_option
val rate: nativeint socket_option
val recovery_ivl: nativeint socket_option
val sndbuf: nativeint socket_option
val rcvbuf: nativeint socket_option
val linger: nativeint socket_option
val reconnect_ivl: nativeint socket_option
val reconnect_ivl_max: nativeint socket_option
val backlog: nativeint socket_option
val maxmsgsize: int64 socket_option
val multicast_hops: nativeint socket_option
val rcvtimeo: nativeint socket_option
val sndtimeo: nativeint socket_option
val ipv4only: bool socket_option
val tcp_keepalive: int socket_option
val tcp_keepalive_idle: int socket_option
val tcp_keepalive_cnt: int socket_option
val tcp_keepalive_intvl: int socket_option
val tcp_accept_filter: string socket_option

type error =
    EINVAL
  | EFAULT
  | EMTHREAD
  | ETERM
  | ENODEV
  | EADDRNOTAVAIL
  | EADDRINUSE
  | ENOCOMPATPROTO
  | EPROTONOSUPPORT
  | EAGAIN
  | ENOTSUP
  | EFSM
  | ENOMEM
  | EINTR
  | EUNKNOWN

exception Error of error * string
