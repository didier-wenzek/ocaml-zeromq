let _ =
  
  (* create a zmq context *)
  let ctx = Zmq.ctx_new () in

  (* create sockets *)
  let push = Zmq.socket ctx Zmq.PUSH in
  let pull = Zmq.socket ctx Zmq.PULL in

  (* bind and connect sockets *)
  Zmq.bind push "inproc://foo";
  Zmq.connect pull "inproc://foo";

  (* send and receive messages *)
  Zmq.send push "Hello world!";
  assert (Zmq.receive pull = "Hello world!");

  (* send and receive multi-parts messages *)
  Zmq.send_multiparts push ["Hello"; "world"; "!"];
  assert (Zmq.receive_multiparts pull = ["Hello"; "world"; "!"]);

  (* send and receive messages unless this would block  *)
  assert (Zmq.send_nowait push "Hello world!");
  assert (Zmq.receive_nowait pull = Some "Hello world!");
  assert (Zmq.receive_nowait pull = None);
  assert (Zmq.send_multiparts_nowait push ["Hello"; "world"; "!"]);
  assert (Zmq.receive_multiparts_nowait pull = ["Hello"; "world"; "!"]);
  assert (Zmq.receive_multiparts_nowait pull = []);

  (* close sockets and term *)
  Zmq.close push;
  Zmq.close pull;

  Zmq.ctx_destroy ctx
