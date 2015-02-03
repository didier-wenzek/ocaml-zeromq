TARGETS = ozmq.cma ozmq.cmxa ozmq.cmxs dllozmqw.so zmq.cmi zmq.cma zmq.cmx
LIB = $(addprefix _build/, $(TARGETS))

all:
	ocamlbuild $(TARGETS)

install: all
	ocamlfind install ozmq META $(LIB)

uninstall:
	ocamlfind remove ozmq

tests.native:
	ocamlbuild tests.native

tests: tests.native
	./tests.native

clean:
	ocamlbuild -clean

.PHONY: all clean tests install
