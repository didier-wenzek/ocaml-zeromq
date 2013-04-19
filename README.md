OCaml bindings for ZeroMQ
====================================
Merge of :
* [pdhborges/ocaml-zmq](https://github.com/pdhborges/ocaml-zmq)
* [pdhborges/ocaml-zmq3](https://github.com/pdhborges/ocaml-zmq3)
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
    let ctx = Zmq.init 1;;

    (* create sockets *)
    let push = Zmq.socket ctx Zmq.PUSH;;
    let pull = Zmq.socket ctx Zmq.PULL;;

    (* bind and connect sockets *)
    Zmq.bind push "inproc://foo";;
    Zmq.connect pull "inproc://foo";;

    (* send and receive messages *)
    Zmq.send push "Hello world!";;
    Zmq.receive pull;;
