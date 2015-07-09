MINGW_PREFIX = $(shell which gcc | awk -F/bin/ '{ printf $$1 }')
ARCHITECTURE = $(shell gcc -dumpmachine | awk -F- '{ printf $$1 }')
ifeq ($(ARCHITECTURE), x86_64)
	LIBGCC = libgcc_s_seh-1.dll
else
	LIBGCC = libgcc_s_dw2-1.dll
endif

all:
	make -C noteshider
	make -C togglehidden
	make -C webshutdown

	make -C sendtray
	cp $(MINGW_PREFIX)/bin/$(LIBGCC) ../build
	cp $(MINGW_PREFIX)/bin/libwinpthread-1.dll ../build

ifeq ($(ARCHITECTURE), i686)
ifdef ECLIPSE_HOME
	make -C screenwrite
else
	@echo warning: skipping screenwrite, no Eclipse home specified
endif
else
	@echo warning: skipping screenwrite on $(ARCHITECTURE)
endif
