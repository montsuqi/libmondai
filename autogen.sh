#!/bin/sh

echo "Running aclocal..."
aclocal

echo "Running autoheader..."
autoheader

echo "Running automake..."
automake --copy --add-missing

echo "Running autoconf..."
autoconf

echo 'Done.  Run "configure --enable-maintainer-mode" now'
