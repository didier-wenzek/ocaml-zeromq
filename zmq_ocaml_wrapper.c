#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/alloc.h>

#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

/* Must be synchronized with Zmq.socket_type. */
static int const SOCKET_TYPES[] = {
  ZMQ_REQ,
  ZMQ_REP,
  ZMQ_DEALER,
  ZMQ_ROUTER,
  ZMQ_PUB,
  ZMQ_SUB,
  ZMQ_XPUB,
  ZMQ_XSUB,
  ZMQ_PUSH,
  ZMQ_PULL,
  ZMQ_PAIR
};

/* Must be synchronized with Zmq.sock_opt. */
static int const SOCKET_OPTS[] = {
  ZMQ_TYPE,
  ZMQ_RCVMORE,
  ZMQ_SNDHWM,
  ZMQ_RCVHWM,
  ZMQ_AFFINITY,
  ZMQ_IDENTITY,
  ZMQ_SUBSCRIBE,
  ZMQ_UNSUBSCRIBE,
  ZMQ_RATE,
  ZMQ_RECOVERY_IVL,
  ZMQ_SNDBUF,
  ZMQ_RCVBUF,
  ZMQ_LINGER,
  ZMQ_RECONNECT_IVL,
  ZMQ_RECONNECT_IVL_MAX,
  ZMQ_BACKLOG,
  ZMQ_MAXMSGSIZE,
  ZMQ_MULTICAST_HOPS,
  ZMQ_RCVTIMEO,
  ZMQ_SNDTIMEO,
  ZMQ_IPV4ONLY,
  ZMQ_FD,
  ZMQ_EVENTS,
  ZMQ_LAST_ENDPOINT,
  ZMQ_TCP_KEEPALIVE,
  ZMQ_TCP_KEEPALIVE_IDLE,
  ZMQ_TCP_KEEPALIVE_CNT,
  ZMQ_TCP_KEEPALIVE_INTVL,
  ZMQ_TCP_ACCEPT_FILTER
};

/* Must be synchronized with Zmq.Error. */
static int const ERROR_CODES[] = {
    EINVAL,
    EFAULT,
    EMTHREAD,
    ETERM,
    ENODEV,
    EADDRNOTAVAIL,
    EADDRINUSE,
    ENOCOMPATPROTO,
    EPROTONOSUPPORT,
    EAGAIN,
    ENOTSUP,
    EFSM,
    ENOMEM,
    EINTR
};

/* This must be the last value of Zmq.Error. */
static int const EUNKNOWN = (sizeof ERROR_CODES) / (sizeof ERROR_CODES[0]);

static
void RAISE(const char *error, ...)
{
  CAMLlocalN(error_parameters, 2);
  static value *exception_handler = NULL;
  static char msg[100];
  va_list ap;
  va_start(ap, error);
  vsnprintf(msg, sizeof(msg), error, ap);
  va_end(ap);

  if (exception_handler == NULL) {
    exception_handler = caml_named_value("zmq.error");
    if (exception_handler == NULL) {
      caml_failwith(msg);
    }
  }

  int err = zmq_errno();
  int caml_errno = EUNKNOWN;
  int i;
  for (i = 0; i < EUNKNOWN; i++) {
    if (err == ERROR_CODES[i]) {
      caml_errno = i;
      break;
    }
  }

  error_parameters[0] = Val_int(caml_errno);
  error_parameters[1] = caml_copy_string(msg);
  caml_raise_with_args(*exception_handler, 2, error_parameters);
}

#define handler_val(v) *((void **) &Field(v, 0))

inline static value alloc_caml_handler(void* hdl)
{
  value caml_handler = alloc_small(1, Abstract_tag);
  handler_val(caml_handler) = hdl;
  return caml_handler;
}    

inline static void free_caml_handler(value caml_handler)
{
  handler_val(caml_handler) = NULL;
}    

inline static void* get_handler(value caml_handler)
{
  void* hdl = handler_val(caml_handler);
  if (! hdl) {
    RAISE("Handler has been released");
  }

  return hdl;
}

extern CAMLprim
value caml_zmq_init(value caml_io_threads, value caml_max_sockets)
{
  CAMLparam2(caml_io_threads, caml_max_sockets);
 
  int io_threads = Int_val(caml_io_threads);
  int max_sockets = Int_val(caml_max_sockets);

  void* zmq_context = zmq_init(io_threads);
  if (! zmq_context) {
    RAISE("init failed");
  }
  zmq_ctx_set(zmq_context, ZMQ_MAX_SOCKETS, max_sockets);

  value caml_context = alloc_caml_handler(zmq_context);
  CAMLreturn(caml_context);
}    

extern CAMLprim
value caml_zmq_term(value caml_context)
{
  CAMLparam1(caml_context);

  void* zmq_context = get_handler(caml_context);
  if (zmq_context) {
    if (-1 == zmq_ctx_destroy(zmq_context)) {
       RAISE("term failed (%s)", zmq_strerror(errno));
    }
    free_caml_handler(caml_context);
  }

  CAMLreturn(Val_unit);
}    

extern CAMLprim
value socket_new(value caml_context, value caml_socket_type)
{
  CAMLparam2(caml_context, caml_socket_type);

  void* zmq_context = get_handler(caml_context);
  int socket_type = SOCKET_TYPES[Int_val(caml_socket_type)];

  void* socket = zmq_socket(zmq_context, socket_type);
  if (! socket) {
    RAISE("socket failed (%s)", zmq_strerror(errno));
  }

  value caml_socket = alloc_caml_handler(socket);
  CAMLreturn(caml_socket);
}

extern CAMLprim
value socket_close(value caml_socket)
{
  CAMLparam1(caml_socket);
  
  void* socket = handler_val(caml_socket);
  if (socket) {
    if (-1 == zmq_close(socket)) {
       RAISE("socket failed (%s)", zmq_strerror(errno));
    }
    free_caml_handler(caml_socket);
  }

  CAMLreturn(Val_unit);
}    

extern CAMLprim
value socket_bind(value caml_socket, value caml_endpoint)
{
  CAMLparam2(caml_socket, caml_endpoint);
  
  void* socket = get_handler(caml_socket);
  const char * endpoint = String_val(caml_endpoint);
  if (-1 == zmq_bind(socket, endpoint)) {
     RAISE("bind failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}    

extern CAMLprim
value socket_connect(value caml_socket, value caml_endpoint)
{
  CAMLparam2(caml_socket, caml_endpoint);
  
  void* socket = get_handler(caml_socket);
  const char* endpoint = String_val(caml_endpoint);
  if (-1 == zmq_connect(socket, endpoint)) {
     RAISE("connect failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}    

extern CAMLprim
value socket_unbind(value caml_socket, value caml_endpoint)
{
  CAMLparam2(caml_socket, caml_endpoint);
  
  void* socket = get_handler(caml_socket);
  const char * endpoint = String_val(caml_endpoint);
  if (-1 == zmq_unbind(socket, endpoint)) {
     RAISE("unbind failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}    

extern CAMLprim
value socket_disconnect(value caml_socket, value caml_endpoint)
{
  CAMLparam2(caml_socket, caml_endpoint);
  
  void* socket = get_handler(caml_socket);
  const char* endpoint = String_val(caml_endpoint);
  if (-1 == zmq_disconnect(socket, endpoint)) {
     RAISE("disconnect failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}    

extern CAMLprim
value socket_send(value caml_socket, value caml_msg)
{
  CAMLparam2(caml_socket, caml_msg);
  
  void* socket = get_handler(caml_socket);
  void* buf = String_val(caml_msg);
  size_t len = caml_string_length(caml_msg);
  if (-1 == zmq_send(socket, buf, len, 0)) {
     RAISE("send failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}

extern CAMLprim
value socket_receive(value caml_socket) 
{
  CAMLparam1(caml_socket);
  CAMLlocal1(caml_msg);

  void* socket = get_handler(caml_socket);
  zmq_msg_t msg;
  zmq_msg_init (&msg);
  if (-1 == zmq_recvmsg (socket, &msg, 0)) {
    zmq_msg_close (&msg);
    RAISE("receive failed (%s)", zmq_strerror(errno));
  }

  size_t len = zmq_msg_size(&msg);
  caml_msg = caml_alloc_string(len);
  memcpy(String_val(caml_msg), zmq_msg_data(&msg), len);

  zmq_msg_close (&msg);
  CAMLreturn(caml_msg);
}

extern CAMLprim
value socket_send_multiparts(value caml_socket, value caml_parts)
{
  CAMLparam2(caml_socket, caml_parts);
  CAMLlocal1(caml_msg);
  
  void* socket = get_handler(caml_socket);
  while (caml_parts != Val_emptylist) {
    caml_msg = Field(caml_parts, 0);
    caml_parts = Field(caml_parts, 1);

    void* buf = String_val(caml_msg);
    size_t len = caml_string_length(caml_msg);
    int last = caml_parts == Val_emptylist;

    if (-1 == zmq_send(socket, buf, len, last?0:ZMQ_SNDMORE)) {
      RAISE("send failed (%s)", zmq_strerror(errno));
    }
  }

  CAMLreturn(Val_unit);
}

extern CAMLprim
value socket_receive_multiparts(value caml_socket) 
{
  CAMLparam1(caml_socket);
  CAMLlocal3(caml_parts,caml_lastpart,caml_msg);

  void* socket = get_handler(caml_socket);

  // We expect at least one part.
  caml_parts = caml_alloc(2,0); // (cons)
  caml_lastpart = caml_parts;

  int expect_more = 1;
  size_t more_size = sizeof(expect_more);
  while(expect_more) {
    zmq_msg_t msg;
    zmq_msg_init (&msg);
    if (-1 == zmq_recvmsg (socket, &msg, 0)) {
      zmq_msg_close (&msg);
      RAISE("receive failed (%s)", zmq_strerror(errno));
    }

    size_t len = zmq_msg_size(&msg);
    caml_msg = caml_alloc_string(len);
    memcpy(String_val(caml_msg), zmq_msg_data(&msg), len);

    Store_field(caml_lastpart, 0, caml_msg);
    zmq_msg_close (&msg);

    zmq_getsockopt (socket, ZMQ_RCVMORE, &expect_more, &more_size);
    if (expect_more) {
      Store_field(caml_lastpart, 1, caml_alloc(2,0)); // new cons
      caml_lastpart = Field(caml_lastpart, 1);
    }
  }
  Store_field(caml_lastpart, 1, Val_emptylist);

  CAMLreturn(caml_parts);
}

extern CAMLprim
value get_int_option(value caml_opt, value caml_socket)
{
  CAMLparam2(caml_opt, caml_socket);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];

  int res;
  size_t size = sizeof(res);
  if (-1 == zmq_getsockopt(socket, opt, &res, &size)) {
    RAISE("socket get option failed (%s)", zmq_strerror(errno));
  }

  value result = caml_copy_nativeint(res);
  CAMLreturn(result);
}

extern CAMLprim
value set_int_option(value caml_opt, value caml_socket, value caml_val)
{
  CAMLparam3(caml_opt, caml_socket, caml_val);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];
  int val = Nativeint_val(caml_val);

  size_t size = sizeof(val);
  if (-1 == zmq_setsockopt(socket, opt, &val, size)) {
    RAISE("socket set option failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}

extern CAMLprim
value get_string_option(value caml_size, value caml_opt, value caml_socket)
{
  CAMLparam3(caml_size, caml_opt, caml_socket);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];
  size_t size = Int_val(caml_size);

  char res[size+1];
  if (-1 == zmq_getsockopt(socket, opt, res, &size)) {
    RAISE("socket get option failed (%s)", zmq_strerror(errno));
  }

  res[size] = 0;
  value result = caml_copy_string(res);
  CAMLreturn(result);
}

extern CAMLprim
value set_string_option(value caml_opt, value caml_socket, value caml_val)
{
  CAMLparam3(caml_opt, caml_socket, caml_val);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];
  char* val = String_val(caml_val);

  size_t size = strlen(val);
  if (-1 == zmq_setsockopt(socket, opt, val, size)) {
    RAISE("socket set option failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}

extern CAMLprim
value get_int64_option(value caml_opt, value caml_socket)
{
  CAMLparam2(caml_opt, caml_socket);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];

  int64 res;
  size_t size = sizeof(res);
  if (-1 == zmq_getsockopt(socket, opt, &res, &size)) {
    RAISE("socket get option failed (%s)", zmq_strerror(errno));
  }

  value result = caml_copy_int64(res);
  CAMLreturn(result);
}

extern CAMLprim
value set_int64_option(value caml_opt, value caml_socket, value caml_val)
{
  CAMLparam3(caml_opt, caml_socket, caml_val);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];
  int64 val = Int64_val(caml_val);

  size_t size = sizeof(val);
  if (-1 == zmq_setsockopt(socket, opt, &val, size)) {
    RAISE("socket set option failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}

extern CAMLprim
value get_bool_option(value caml_opt, value caml_socket)
{
  CAMLparam2(caml_opt, caml_socket);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];

  int res;
  size_t size = sizeof(res);
  if (-1 == zmq_getsockopt(socket, opt, &res, &size)) {
    RAISE("socket get option failed (%s)", zmq_strerror(errno));
  }

  value result = res ? Val_true : Val_false;
  CAMLreturn(result);
}

extern CAMLprim
value set_bool_option(value caml_opt, value caml_socket, value caml_val)
{
  CAMLparam3(caml_opt, caml_socket, caml_val);
  
  void* socket = get_handler(caml_socket);
  int opt = SOCKET_OPTS[Int_val(caml_opt)];
  int val = Bool_val(caml_val);

  size_t size = sizeof(val);
  if (-1 == zmq_setsockopt(socket, opt, &val, size)) {
    RAISE("socket set option failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}

