#! /bin/sh

. ../../project.sh || exit 1

cd tmp && make package "$@"
