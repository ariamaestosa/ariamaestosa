echo "python scons/scons.py install"
python scons/scons.py install

echo "dylibbundler"
dylibbundler -od -b -x "./AriaMaestosa.app/Contents/MacOS/AriaMaestosa" -d "./AriaMaestosa.app/Contents/libs/" -p @executable_path/../libs/; fi

echo "patching libraries..."
#Work around the likely bug in dylibbundler leading in copying two exemplars of the same dylib with different names instead of creating symlinks or changing the dependency name in the depending binaries
pushd ./AriaMaestosa.app/Contents/libs/
wx_version="$(wx-config --version|cut -c1-3)"
for lib in $(ls libwx_*.dylib); do
  lib_basename="${lib%%\-*}"
  if [[ "${lib}" != "${lib_basename}-${wx_version}.dylib" ]] && [ -f "${lib_basename}-${wx_version}.dylib" ]; then
    rm -f "${lib_basename}-${wx_version}.dylib"
    ln -sf "${lib}" "${lib_basename}-${wx_version}.dylib"
  fi
done
popd > /dev/null

echo "Done packaging"