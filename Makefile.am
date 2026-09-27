bin_PROGRAMS = farrar

farrar_SOURCES = \
    main.cpp \
    src/CLIParameters.cpp \
    src/Farrar.cpp \
    src/execution.cpp

farrar_CPPFLAGS = \
    -I$(srcdir)/include

farrar_CXXFLAGS = \
    -std=c++17 \
    -Wall \
    -O3

if USE_AVX
farrar_CXXFLAGS += \
    -mavx2
endif

if USE_RVV
farrar_CXXFLAGS += \
    -march=rv64gcv \
    -mabi=lp64d
endif

distclean-local:
	rm -rf autom4te.cache
	rm -f aclocal.m4
	rm -f configure
	rm -f compile
	rm -f depcomp
	rm -f install-sh
	rm -f missing
	rm -f config.guess
	rm -f config.sub
	rm -f Makefile.in
	rm -f src/Makefile.in
	rm -f config.h.in~
	rm -f configure~
	rm -rf src/.debs