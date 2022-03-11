# PREFIX is environment variable, but if it is not set, then set default value
ifeq ($(PREFIX),)
    PREFIX := /usr/local
endif

install:
	cd buddy/src; make all
	cd bsat/src; make clean; make all
	cd xtree/src; make clean; make all
	install -d $(DESTDIR)$(PREFIX)/lib/
	install -m 644 buddy/lib/libtbuddy.so $(DESTDIR)$(PREFIX)/lib/
	install -m 644 buddy/lib/tbuddy.a $(DESTDIR)$(PREFIX)/lib/
	install -m 644 buddy/lib/buddy.a $(DESTDIR)$(PREFIX)/lib/
	install -d $(DESTDIR)$(PREFIX)/include/
	install -m 644 buddy/include/bdd.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 buddy/include/ilist.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 buddy/include/prover.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 buddy/include/pseudoboolean.h $(DESTDIR)$(PREFIX)/include/
	install -m 644 buddy/include/tbdd.h $(DESTDIR)$(PREFIX)/include/
