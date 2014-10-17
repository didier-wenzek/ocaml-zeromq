TARGETS = ozmq.cma ozmq.cmxa ozmq.cmxs
LIB = $(addprefix _build/, $(TARGETS))
INSTALL = $(LIB)

all:
	ocamlbuild $(TARGETS)

DESTDIR=`ocamlc -where`
install: all
	ocamlfind install ozmq META $(INSTALL)

tests.native:
	ocamlbuild tests.native

test: tests.native
	./tests.native

clean:
	ocamlbuild -clean

.PHONY: all clean tests
