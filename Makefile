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

mandir ?= ${prefix}/share/man

## Build mk
all: mk

## Generate a configure script and Makefile
configure: configure.ac
	AUTOCONF_VERSION=2.69 AUTOMAKE_VERSION=1.16 autoreconf -i
	rm -rf autom4te.cache
	sed 's/\([ !]\)defined \([a-zA-Z0-9_$$]*\)/\1defined(\2)/g' configure > configure.new
	mv configure.new configure
	chmod +x configure
	rm -f Makefile
	sed 's/^-include config.mk/include config.mk/; s/^[A-Z]* ?=.*$$/# &/' Mkfile > Makefile

## Remove build artifacts
clean:
	rm -f mk compile_flags.txt

## Remove even more stuff
distclean: clean
	rm -rf autom4te.cache
	rm -f configure config.status config.log config.h.in config.h config.mk aclocal.m4 *~

## Install mk into ${PREFIX}
install: mk mk.1
	mkdir -p ${DESTDIR}${bindir} ${DESTDIR}${mandir}/man1
	cp -f mk ${DESTDIR}${bindir}/
	cp -f mk.1 ${DESTDIR}${mandir}/man1/

mk: mk.c compats.c mk.h
	${CC} -o mk.tmp mk.c compats.c ${CFLAGS} ${CPPFLAGS} ${LDFLAGS}
	mv mk.tmp mk

## Generate compile_flags.txt, for use with clangd
compile_flags.txt:
	printf '%s\n' "${CFLAGS}" | tr ' ' '\n' > $@
