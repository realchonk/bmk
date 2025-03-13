#!/bin/sh
set -e
AUTOCONF_VERSION=2.52
export AUTOCONF_VERSION

autoconf
autoheader
sed	-e 's/defined (\{0,1\}\([a-zA-Z0-9_$$]\{1,\}\))\{0,1\}/defined(\1)/'	\
	-e 's/^[[:space:]]*#/#/'							\
	configure > configure.new
mv configure.new configure
chmod +x configure
rm -rf autom4te.cache

