#include <caml/memory.h>
#include <caml/mlvalues.h>
#include <caml/callback.h>
#include <caml/fail.h>
#include <caml/alloc.h>

#include <zmq.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>

static
int const SOCKET_TYPES[] = {
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

int decode_flags(value options, int const codes[])
{
  CAMLlocal1(head);
  int flags = 0;
  while (options!=Val_emptylist) {
    head = Field(options, 0);
    flags = flags | (codes[Int_val(head)]);
    options = Field(options, 1);
  }
  return flags;
}

static
void RAISE(const char *error, ...)
{
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
  caml_raise_with_string(*exception_handler, msg);
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
value caml_zmq_init(value io_threads)
{
  CAMLparam1(io_threads);

  void* zmq_context = zmq_ctx_new();
  if (! zmq_context) {
    RAISE("init failed");
  }

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

  CAMLreturn((value) socket);
}

extern CAMLprim
value socket_close(value caml_socket)
{
  CAMLparam1(caml_socket);
  
  void* socket = (void*) caml_socket;
  if (-1 == zmq_close(socket)) {
     RAISE("socket failed (%s)", zmq_strerror(errno));
  }

  CAMLreturn(Val_unit);
}    

extern CAMLprim
value socket_bind(value caml_socket, value caml_endpoint)
{
  CAMLparam2(caml_socket, caml_endpoint);
  
  void* socket = (void*) caml_socket;
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
  
  void* socket = (void*) caml_socket;
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
  
  void* socket = (void*) caml_socket;
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
  
  void* socket = (void*) caml_socket;
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
  
  void* socket = (void*) caml_socket;
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

  void* socket = (void*) caml_socket;
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
  
  void* socket = (void*) caml_socket;
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

  void* socket = (void*) caml_socket;

  // We expect at least one part.
  caml_parts = caml_alloc(2,0); // (cons)
  caml_lastpart = caml_parts;

  int64_t expect_more = 1;
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
