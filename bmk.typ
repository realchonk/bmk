#import "@preview/slydst:0.1.4": *
#import "@preview/treet:0.1.1": *
#import "@preview/cades:0.3.0": qr-code

#show: slides.with(
    title: "bmk",
    subtitle: "A redesigned make(1) with a few nice features",
    authors: "Benjamin Stürz <benni@stuerz.xyz>",
)

#show raw: set block(fill: silver.lighten(65%), width: 100%, inset: 1em)

= Motivation

== Why yet another make?
- GNU Make
- multiple variants of BSD Make
- Microsoft Make (yes really)

== Example C Toolchain Project
#tree-list[
- yacc (vendored)
- cc (C compiler)
    - cc (compiler driver)
    - cpp (C preprocessor)
    - cc1 (C -> IR)
        - depends on yacc
    - irc (IR -> ASM)
        - depends on yacc
- libelf
- as (ASM -> OBJ)
    - depends on libelf
- ld (OBJ -> BIN)
    - depends on libelf
]

== Approach 1: Recursive Make
```make
# Top-level Makefile
yacc:
    ${MAKE} -C yacc

cc: yacc
    ${MAKE} -C cc

libelf:
    ${MAKE} -C libelf

as: libelf
    ${MAKE} -C as

ld: libelf
    ${MAKE} -C ld
```

== Approach 2: One Big Makefile
```make
YACCOBJ = yacc/y1.o yacc/y2.o yacc/y3.o yacc/y4.o
CCOBJ = cc/cc/cc.o
IRCOBJ = cc/irc/main.o cc/irc/gen.o cc/irc/parse.o

yacc/yacc: ${YACCOBJ}
    ${CC} -o $@ ${YACCOBJ}

cc/cc/cc: ${CCOBJ}
    ${CC} -o $@ ${CCOBJ}

cc/irc/irc: yacc/yacc ${IRCOBJ}
    ${CC} -o $@ ${IRCOBJ}

```

== Approach 2.5: One Big Makefile, but using `include`
```make
# /yacc/Makefile
YACCOBJ = yacc/y1.o yacc/y2.o yacc/y3.o yacc/y4.o
yacc/yacc: ${YACCOBJ}
    ${CC} -o $@ ${YACCOBJ}
```

```make
# /cc/irc/Makefile
IRCOBJ = cc/irc/main.o cc/irc/gen.o cc/irc/parse.o
cc/irc/irc: yacc/yacc ${IRCOBJ}
    ${CC} -o $@ ${IRCOBJ}

```

```make
# /Makefile
include yacc/Makefile
include cc/Makefile
...
```

== Conclusion (Drawbacks)
=== Recursive Make:
    - broken dependency handling
    - needs special handling for parallel builds (`-jN`)

=== One Big Makefile:
    - every rule needs to be a full path
    - must always use the "top-level" Makefile

=== Both:
    - too much complexity
    - nobody wants to maintain a build system

== Other tools: Automake, CMake, Meson
=== Benefits:
- They just generate these ugly "Makefiles" for you
- Built-in support for nice features, like build directories

=== Drawbacks:
- Need to learn yet another build system language
- Generate huge, complicated, and difficult to debug Makefiles
- Additional build step (e.g. `./configure`)
- May not be available for your platform

== What if there was a make, that could do this?

```make
# /yacc/Makefile
YACCOBJ = y1.o y2.o y3.o y4.o
yacc: ${YACCOBJ}
    ${CC} -o $@ ${YACCOBJ}
```

```make
# /cc/irc/Makefile
IRCOBJ = main.o gen.o parse.o
irc: ../../yacc ${IRCOBJ}
    ${CC} -o $@ ${IRCOBJ}
```
```make
# /Makefile
.SUBDIRS: yacc cc libelf as ld

all: yacc cc libelf as ld
clean: yacc/clean cc/clean libelf/clean as/clean ld/clean
```

= This is what `bmk` allows you to do

= Other nice to have Features

== Documentation Comments
```make
# /cc/irc/Mkfile
NAME = irc

## C Compiler
CC = cc

## C Compiler Flags
CFLAGS = -O2 -Wall -Wextra

## Build ${NAME}
all: ${NAME}

## Remove build artifacts for ${NAME}
clean:
    rm -f ${NAME}
```

#pagebreak()
```sh
/cc/irc $ mk -h
...
MACROS:
CC              - C Compiler
CFLAGS          - C Compiler Flags

TARGETS:
all             - Build irc
clean           - Remove build artifacts for irc
```

#pagebreak()
```sh
/ $ mk -hv
MACROS:
CC              - Path to the C Compiler
CFLAGS          - Default C Compiler Flags

TARGETS:
all             - Build: yacc, cc, libelf, as, ld
clean           - Clean: yacc, cc, libelf, as, ld
cc/all          - Build: cc, cpp, cc1, irc
cc/clean        - Clean: cc, cpp, cc1, irc
yacc/all        - Build yacc
yacc/clean      - Remove build artifacts for yacc
cc/cc1/all      - Build cc1
cc/cc1/clean    - Remove build artifacts for cc1
cc/irc/all      - Build irc
cc/irc/clean    - Remove build artifacts for irc
...
```

== Templates
```make
# /Mkfile
.template dir
## Build: ${.SUBDIRS:J, }
all: ${.SUBDIRS}

## Clean: ${.SUBDIRS:J, }
clean: ${.SUBDIRS:=/clean}
.endt
```

```make
# /cc/Mkfile
.SUBDIRS: cc cc1 cpp irc

.expand dir
```

== Build directories
```sh
/ $ mk -o obj cc/irc
...
/ $ find obj
obj/
obj/yacc/
obj/yacc/yacc*
obj/cc/
obj/cc/irc/
obj/cc/irc/irc*
```

Note: This feature requires minor changes in the Mkfiles,
like using `${.OBJDIR}`, or the `:F` modifier.

== Transparent integration with other build systems
```make
# /Mkfile
.FOREIGN: libelf

.libelf-configure:
    (cd libelf && ./configure --enable-static --disable-shared)
    touch $@

libelf?:
    test -e libelf/Makefile
    gmake -q -C libelf $<

libelf!: .libelf-configure
    gmake -C libelf $<

example-elf: example.c libelf/libelf.a
    ${CC} -o $@ example.c -Llibelf -lelf
```

== Features
- Variables/Macros (`=`, `+=`, `!=`, ...)
  - Macro Modifiers (`:J`, `:U`, ...)
- Rules
 - Inference Rules (`.c.o:`)
- Subdirectories (`.SUBDIRS: yacc cc ...`)
  - "foreign" subdirectories (`.FOREIGN: libelf`)
  - exporting variables into subdirectories (`.EXPORTS: CC CFLAGS ...`)
- Documentation System (`## DOC Comment`)
- Build directories
  - `mk -o objdir`

= Where can I get this?

== Getting bmk(1)
#pad(rest: 24pt)[
    #align(center)[
        #qr-code("https://github.com/realchonk/bmk", width: 4.5cm)
        #text(size: 18pt)[https://github.com/realchonk/bmk]
    ]
]

== Installation
```sh
# Generate a configure script, if not already available:
# This requires at least autoconf version 2.52.
$ ./autogen.sh

$ ./configure [--prefix=/usr/local]
$ make
$ make install
```

= Portability

== Usual platforms
`bmk` will "just" work on your usual Unix-like plaforms:

- Linux
- OpenBSD
- FreeBSD
- NetBSD
- MacOS

Note: all of them have CI

== Unusual plaforms
`bmk` even works on some unconventional platforms:

- Haiku (libbsd required for some reason)
- 2.11BSD (PDP-11)
- 4.3BSD (VAX)
- Xenix
- Minix-vmd

Yes, it was #emph("very") painful. \
For your own mental sanity, \
please don't look at the code.

== Missing/Untested platforms
- Solaris
- Plan 9
- AIX
- HP-UX
- Windows (just use WSL)

= Thanks for listening

/* vim: set ts=4 sw=4 et: */
