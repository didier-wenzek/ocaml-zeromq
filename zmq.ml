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
let _ = 
  Callback.register_exception "zmq.error" (Error(EUNKNOWN,"msg string"));

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

external context: int -> int -> context = "caml_zmq_init"
external term: context -> unit = "caml_zmq_term"
let init ?(io_threads = 1) ?(max_sockets = 1024) () = context io_threads max_sockets

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

type sock_opt =
  | ZMQ_TYPE
  | ZMQ_RCVMORE
  | ZMQ_SNDHWM
  | ZMQ_RCVHWM
  | ZMQ_AFFINITY
  | ZMQ_IDENTITY
  | ZMQ_SUBSCRIBE
  | ZMQ_UNSUBSCRIBE
  | ZMQ_RATE
  | ZMQ_RECOVERY_IVL
  | ZMQ_SNDBUF
  | ZMQ_RCVBUF
  | ZMQ_LINGER
  | ZMQ_RECONNECT_IVL
  | ZMQ_RECONNECT_IVL_MAX
  | ZMQ_BACKLOG
  | ZMQ_MAXMSGSIZE
  | ZMQ_MULTICAST_HOPS
  | ZMQ_RCVTIMEO
  | ZMQ_SNDTIMEO
  | ZMQ_IPV4ONLY
  | ZMQ_FD
  | ZMQ_EVENTS
  | ZMQ_LAST_ENDPOINT
  | ZMQ_TCP_KEEPALIVE
  | ZMQ_TCP_KEEPALIVE_IDLE
  | ZMQ_TCP_KEEPALIVE_CNT
  | ZMQ_TCP_KEEPALIVE_INTVL
  | ZMQ_TCP_ACCEPT_FILTER

type 'a socket_option = sock_opt * (sock_opt -> socket -> 'a) * (sock_opt -> socket -> 'a -> unit)

let get_socket_option (opt, get, _) socket = get opt socket 
let set_socket_option (opt, _, set) socket value = set opt socket value

let write_only_option: sock_opt -> socket -> 'a = fun opt socket -> raise (Error (EINVAL,"write only option"))
let read_only_option: sock_opt -> socket -> 'a -> unit = fun opt socket value -> raise (Error (EINVAL,"read only option"))

external get_int_option: sock_opt -> socket -> nativeint = "get_int_option"
external get_int64_option: sock_opt -> socket -> int64 = "get_int64_option"
external get_string_option: int -> sock_opt -> socket -> string = "get_string_option"
external get_bool_option: sock_opt -> socket -> bool = "get_bool_option"

external set_int_option: sock_opt -> socket -> nativeint -> unit = "set_int_option"
external set_int64_option: sock_opt -> socket -> int64 -> unit = "set_int64_option"
external set_string_option: sock_opt -> socket -> string -> unit = "set_string_option"
external set_bool_option: sock_opt -> socket -> bool -> unit = "set_bool_option"

let get_caml_int_option opt socket = Nativeint.to_int (get_int_option opt socket)
let set_caml_int_option opt socket value = set_int_option opt socket (Nativeint.of_int value)

let subscribe = set_string_option ZMQ_SUBSCRIBE
let unsubscribe = set_string_option ZMQ_UNSUBSCRIBE

let sndhwm = (ZMQ_SNDHWM, get_int_option, set_int_option)
let rcvhwm = (ZMQ_RCVHWM, get_int_option, set_int_option)
let identity = (ZMQ_IDENTITY, get_string_option 255, set_string_option)
let rate = (ZMQ_RATE, get_int_option, set_int_option)
let recovery_ivl = (ZMQ_RECOVERY_IVL, get_int_option, set_int_option)
let sndbuf = (ZMQ_SNDBUF, get_int_option, set_int_option)
let rcvbuf = (ZMQ_RCVBUF, get_int_option, set_int_option)
let linger = (ZMQ_LINGER, get_int_option, set_int_option)
let reconnect_ivl = (ZMQ_RECONNECT_IVL, get_int_option, set_int_option)
let reconnect_ivl_max = (ZMQ_RECONNECT_IVL_MAX, get_int_option, set_int_option)
let backlog = (ZMQ_BACKLOG, get_int_option, set_int_option)
let maxmsgsize = (ZMQ_MAXMSGSIZE, get_int64_option, set_int64_option)
let multicast_hops = (ZMQ_MULTICAST_HOPS, get_int_option, set_int_option)
let rcvtimeo = (ZMQ_RCVTIMEO, get_int_option, set_int_option)
let sndtimeo = (ZMQ_SNDTIMEO, get_int_option, set_int_option)
let ipv4only = (ZMQ_IPV4ONLY, get_bool_option, set_bool_option)
let tcp_keepalive = (ZMQ_TCP_KEEPALIVE, get_caml_int_option, set_caml_int_option)
let tcp_keepalive_idle = (ZMQ_TCP_KEEPALIVE_IDLE, get_caml_int_option, set_caml_int_option)
let tcp_keepalive_cnt = (ZMQ_TCP_KEEPALIVE_CNT, get_caml_int_option, set_caml_int_option)
let tcp_keepalive_intvl = (ZMQ_TCP_KEEPALIVE_INTVL, get_caml_int_option, set_caml_int_option)
let tcp_accept_filter = (ZMQ_TCP_ACCEPT_FILTER, get_string_option 255, set_string_option)

