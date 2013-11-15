OCaml bindings for ZeroMQ
====================================
Merge of :
* [pdhborges/ocaml-zmq](https://github.com/pdhborges/ocaml-zmq)
* [bashi-bazouk/Caravan](https://github.com/bashi-bazouk/Caravan)

Pre-requisites
--------------
* [OCaml](http://caml.inria.fr/)
* [ZeroMQ](http://www.zeromq.org/)

License
-------
GNU General Public License.

Install
-------
    $ make
    $ make test
    $ make install

Usage
-----

    #load "zmq.cma";;

    (* create a zmq context *)
    let ctx = Zmq.ctx_new ();;

    (* create sockets *)
    let push = Zmq.socket ctx Zmq.PUSH;;
    let pull = Zmq.socket ctx Zmq.PULL;;

    (* bind and connect sockets *)
    Zmq.bind push "inproc://foo";;
    Zmq.connect pull "inproc://foo";;

    (* send and receive messages *)
    Zmq.send push "Hello world!";;
    Zmq.receive pull;;

    (* send and receive multi-parts messages *)
    Zmq.send_multiparts push ["Hello"; "world"; "!"];;
    Zmq.receive_multiparts pull;;

    (* send and receive messages unless this would block  *)
    assert (Zmq.send_nowait push "Hello world!");;
    assert (Zmq.receive_nowait pull = Some "Hello world!");;
    assert (Zmq.receive_nowait pull = None);;
    assert (Zmq.send_multiparts_nowait push ["Hello"; "world"; "!"]);
    assert (Zmq.receive_multiparts_nowait pull = ["Hello"; "world"; "!"]);
    assert (Zmq.receive_multiparts_nowait pull = []);

    (* close sockets and term *)
    Zmq.close push;;
    Zmq.close pull;;

    Zmq.ctx_destroy ctx;;

