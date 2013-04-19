let _ =
  
  (* create a zmq context *)
  let ctx = Zmq.init 1 in

  (* create sockets *)
  let push = Zmq.socket ctx Zmq.PUSH in
  let pull = Zmq.socket ctx Zmq.PULL in

  (* bind and connect sockets *)
  Zmq.bind push "inproc://foo";
  Zmq.connect pull "inproc://foo";

  (* send and receive messages *)
  Zmq.send push "Hello world!";
  assert (Zmq.receive pull = "Hello world!");

