# If your make is too ancient to support this syntax,
# either create config.mk and remove the -,
# or remove this line;
include config.mk

## C Compiler
# CC ?= cc

## C Compiler Flags
# CFLAGS ?= -g -O0 -ansi -Wall -Wextra -Wno-deprecated-non-prototype -Wno-implicit-int -Wno-return-type

## C Preprocessor Flags
# CPPFLAGS ?= -D_GNU_SOURCE -D_BSD_SOURCE -D_XOPEN_SOURCE=700

## Linker Flags
# LDFLAGS ?=

## Installation directory
prefix ?= /usr/local

bindir ?= ${prefix}/bin

## Build mk
all: mk

## Generate a configure script and Makefile
configure: configure.ac
	AUTOCONF_VERSION=2.69 autoreconf -i
	cp /usr/local/share/automake-$${AUTOMAKE_VERSION}/install-sh build-aux/
	rm -rf autom4te.cache
	sed -i 's/\([ !]\)defined \([a-zA-Z0-9_$$]*\)/\1defined(\2)/g' configure
	rm -f Makefile
	sed 's/^-include config.mk/include config.mk/; s/^[A-Z]* ?=.*$$/# &/' Mkfile > Makefile

## Remove build artifacts
clean:
	rm -f mk compile_flags.txt

## Remove even more stuff
distclean: clean
	rm -rf build-aux autom4te.cache
	rm -f configure config.status config.log config.h.in config.h config.mk aclocal.m4 *~

## Install mk into ${PREFIX}
install: mk
	mkdir -p ${DESTDIR}${bindir}
	cp -f mk ${DESTDIR}${bindir}/

mk: mk.c compats.c mk.h
	${CC} -o mk.tmp mk.c compats.c ${CFLAGS} ${CPPFLAGS} ${LDFLAGS}
	mv mk.tmp mk

## Generate compile_flags.txt, for use with clangd
compile_flags.txt:
	printf '%s\n' "${CFLAGS}" | tr ' ' '\n' > $@
