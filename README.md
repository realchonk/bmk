# Dependencies
Generating the configure script requires a few dependencies:
- autoconf (>= 2.64, for extra portability version 2.64 should be used, as later versions make more assumptions about the host)
- automake (any somewhat recent version should work, as automake itself isn't being used directly)

# Bootstrap
```sh
# This is needed, beacuse not every make under the sun supports optional include statements.
touch config.mk

# Generate the configure script.
make configure

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
- amd64-openbsd
- amd64-freebsd
- amd64-netbsd
- amd64-alpine-linux
- arm64-macos

## Other Platforms, which have been tested at some point
- powerpc64-freebsd
- hppa-netbsd
- hppa-openbsd
- Minix-vmd (i486) (configure broken)
- XENIX 2.3.4
