# Dependencies
Generating the configure script requires a few dependencies:
- autoconf (>= 2.52, for extra portability version 2.52 should be used, as later versions make more assumptions about the host)

# Bootstrap
```sh
# Generate the configure script.
./autogen.sh

# Configure bmk and generate a config.mk file.
./configure [--prefix=/usr/local]

# Build bmk.
make

# Install bmk.
make install
```

# Building without Autoconf
bmk can be built without autoconf, but this requires writing your own config.h and config.mk.
This might be necessary on platforms that are too broken to run a configure script, such as Minix-vmd.

# Regularly Tested Platforms (CI)
- [amd64-openbsd](https://builds.sr.ht/~realchonk/bmk/commits/main/openbsd)
- [amd64-freebsd](https://builds.sr.ht/~realchonk/bmk/commits/main/freebsd)
- [amd64-netbsd](https://builds.sr.ht/~realchonk/bmk/commits/main/netbsd)
- [amd64-alpine-linux](https://builds.sr.ht/~realchonk/bmk/commits/main/alpine)
- [arm64-macos](https://cirrus-ci.com/github/realchonk/bmk/main)

## Other Platforms, which have been tested at some point
- [pdp11-bsd211](https://github.com/realchonk/bmk/pull/3)
- powerpc64-freebsd
- hppa-netbsd
- hppa-openbsd
- Minix-vmd (i486) (configure broken)
- XENIX 2.3.4

## TODO
- create CI for 2.11BSD
