all: zmq.cmxa zmq.cma

DESTDIR=`ocamlc -where`
install: zmq.cmxa zmq.cma
	cp zmq.mli zmq.cmi zmq.cmxa zmq.cma libzmq_ocaml.a dllzmq_ocaml.so $(DESTDIR)

.SUFFIXES: .c .cpp .o .ml .mli .cmo .cmx .cmi

.c.o:
	gcc -O2 -fpic -Wall -c $<

.cpp.o:
	g++ -O2 -fpic -c $<

.mli.cmi: 
	ocamlc -c $<

.ml.cmo: 
	ocamlc -c $<

.ml.cmx: 
	ocamlopt -c $<

.ml.o: 
	ocamlopt -c $<

libzmq_ocaml.a: zmq_ocaml_wrapper.o
	rm -f $@
	ar rc $@ zmq_ocaml_wrapper.o

zmq.cmxa: zmq.cmi zmq.cmx libzmq_ocaml.a
	ocamlopt -a -o zmq.cmxa zmq.cmx -cclib -lzmq_ocaml -cclib -lzmq

zmq.cma: zmq.cmi zmq.cmo zmq_ocaml_wrapper.o
	ocamlmklib -o zmq_ocaml zmq.cmo zmq_ocaml_wrapper.o -lzmq
	mv zmq_ocaml.cma zmq.cma

test: zmq.cmxa tests.cmx
	ocamlopt -I . -o tests.exe zmq.cmxa tests.cmx
	./tests.exe

clean:
	rm -f *.o *.cmo *.cmx *.cmi *.so *.a *.cma *.cmxa tests.exe
