# Bootstrap
```sh
# Configure bmk and generate a config.mk file.
./configure [--prefix=/usr/local]

# Build bmk.
make

# Install bmk.
make install
```

# Regularly Tested Platforms (CI)
- [amd64-openbsd](https://builds.sr.ht/~realchonk/bmk/commits/main/openbsd)
- [amd64-freebsd](https://builds.sr.ht/~realchonk/bmk/commits/main/freebsd)
- [amd64-netbsd](https://builds.sr.ht/~realchonk/bmk/commits/main/netbsd)
- [amd64-alpine-linux](https://builds.sr.ht/~realchonk/bmk/commits/main/alpine)
- [arm64-macos](https://cirrus-ci.com/github/realchonk/bmk/main)

## Other Platforms, which have been tested at some point
- [pdp11-bsd211](https://github.com/realchonk/bmk/pull/3)
- [vax11-bsd43](https://github.com/realchonk/bmk/pull/5)
- powerpc64-freebsd
- hppa-netbsd
- hppa-openbsd
- Minix-vmd (i486) (configure broken)
- XENIX 2.3.4

## Projects using bmk(1)
- https://got.stuerz.xyz/?action=summary&path=286bsd.git
- https://got.stuerz.xyz/?action=summary&path=desktop.git

## TODO
- automate CI for 2.11BSD and 4.3BSD
- modernize code base
  - use const where possible
  - allow declaring variables at the start of blocks,
    not just the start of a function
