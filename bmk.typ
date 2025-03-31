#import "@preview/slydst:0.1.4": *

#show: slides.with(
    title: "bmk: TODO",
    subtitle: "A make(1) that solves subdirectories or something I dunno",
    authors: "Benjamin Stürz <benni@stuerz.xyz>",
)

#show raw: set block(fill: silver.lighten(65%), width: 100%, inset: 1em)

== Outline

#outline()

= Motivation

== Traditional make(1): no concept of subdirectories
TODO

== Other tools
TODO

Automake, CMake and Meson just generate "Makefiles" that look horrendous and are undebuggable.

= Features
== Features
- Variables/Macros (`=`, `+=`, `!=`, ...)
  - Macro Modifiers
- Rules
 - Inference Rules (`.c.o:`)
- Documentation System
- Subdirectories
  - "foreign" subdirectories
  - exporting variables into subdirectories
- Build directories
  - `mk -o objdir`


= Usage

== Variables/Macros
```make
## Name of the program
NAME = calculator

## C compiler
CC ?= cc

## C compiler flags
CFLAGS ?= -ansi -O2 -Wall -Wextra

# Source files
SRC != echo *.[ch]

# C source files
CSRC = ${SRC:M*.c}

# Object files
OBJS = ${CSRC:.c=.o}
```

== Rules
```make
## Build ${NAME}
all: ${NAME}

## Clean build artifacts
clean:
    rm -f ${NAME:F}

.c.o:
    ${CC} -c -o $@ $< ${CFLAGS}
```

== Subdirectories
```make
# export variables CC and CFLAGS into subdirectories
.EXPORT: CC CFLAGS

.SUBDIRS: foo   # foo is just a normal subdirectory

calc: ${OBJS} foo/libfoo.a bar/libbar.a
    ${CC} -o $@ ${OBJS:F} -Lfoo -L${.OBJDIR}/bar -lfoo -lbar
```

== Foreign subdirectories
```make
.FOREIGN: bar   # bar is a "foreign" different

bar.tgz:
    ftp -o $@ https://example.com/libbar-1.2.3.tar.gz

extract-bar: bar.tgz
    tar -xzf ${.OBJDIR}/bar.tgz -C ${.OBJDIR}
    (cd ${.OBJDIR} && mv libbar-1.2.3 bar)

# Check if the target $< in bar/ is fresh
bar?:
    test -d ${.OBJDIR}/bar && gmake -q -C ${.OBJDIR}/bar $<

# Build target $< in bar/
bar!: extract-bar
    gmake -C ${.OBJDIR}/bar $<
```

= Portability

== Usual platforms
bmk will "just" work on your usual Unix-like plaforms:

- Linux
- OpenBSD
- FreeBSD
- NetBSD
- MacOS

Note: all of them have CI

== Unusual plaforms
It even works on some unusual platforms:

- Haiku (libbsd required for some reason)
- 2.11BSD (PDP-11)
- 4.3BSD (VAX)
- Xenix
- Minix-vmd

== Missing/Untested
- Windows
- Solaris
- Plan 9
- AIX
- HP-UX

= Setup

== Getting bmk(1)
https://github.com/realchonk/bmk

== Installation

```sh
# Generate a configure script, if not already available:
# This requires at least autoconf version 2.52.
$ ./autogen.sh

$ ./configure [--prefix=/usr/local]
$ make
$ make install
```

= Summary
== Summary
TODO

/* vim: set ts=4 sw=4 et: */
