#!/bin/sh
ROOT=`pwd`
cd transifex/translations/ariamaestosa.english-pot-file/
for i in *; do echo ${i%%.*}/aria_maestosa.po && cp $i $ROOT/${i%%.*}/aria_maestosa.po && msgfmt -o $ROOT/${i%%.*}/aria_maestosa.mo $ROOT/${i%%.*}/aria_maestosa.po && cp $ROOT/${i%%.*}/aria_maestosa.mo $ROOT/../OSX/mac-i18n/${i%%.*}.lproj/aria_maestosa.mo; done