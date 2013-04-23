let _ =
  
  (* create a zmq context *)
  let ctx = Zmq.init () in

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

  (* close sockets and term *)
  Zmq.close push;
  Zmq.close pull;

  Zmq.term ctx
