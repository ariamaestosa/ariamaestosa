#! /bin/sh

. ../../project.sh || exit 1

if [ "x$COMPILER_PREFIX" = "x" ]; then
    echo "you must set COMPILER_PREFIX"
    exit 1
fi

rm -r -f tmp 2>/dev/null && mkdir tmp 

cd tmp && ../../../configure "--cross-compiling=1" "--compiler-prefix=$COMPILER_PREFIX" "--target-platform-linux-ppc=1" "$@"


