# automatically makes OS X releases from scons project
# not really portable i'm afraid. adapt variables as needed
# tries to preserve compatibility with older OS X versions
# does a universal build of jdkmidi; Aria will be built Universal if wxWidgets is.
#
# invoke like this :
#    % export VERSION="1.2b3"
#    % ./make_release.sh

if [ -z $VERSION ]; then
    echo "ERROR, please specify a VERSION env variable!!"
    exit 1
fi

echo "making packgage for version $VERSION"

USE_WX_CONFIG="/Developer/svn/wxWidgets/cocoa_build2/wx-config "
OUTPUT="$HOME/Desktop/aria-build/"
ADDITIONAL_BUILD_FLAGS="-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -mmacosx-version-min=10.9 -Wfatal-errors"
ADDITIONAL_LINK_FLAGS="-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -mmacosx-version-min=10.9"

#--------- copy repository -----
rm -rf $OUTPUT
mkdir -p $OUTPUT/build/
svn export . $OUTPUT/build/. --force
#cp -R . $OUTPUT/build
cd $OUTPUT/build
#find "." -name ".svn" -exec rm -rf '{}' \;

# ----- make source archive -----
mkdir -p "$OUTPUT/AriaSrc-$VERSION/"
cp -R "." "$OUTPUT/AriaSrc-$VERSION"
cp "./Resources/Documentation/building.html" "$OUTPUT/AriaSrc-$VERSION/"
cp "./Resources/Documentation/linux.png" "$OUTPUT/AriaSrc-$VERSION/"
cp "./Resources/Documentation/osx.png" "$OUTPUT/AriaSrc-$VERSION/"
cp "./Resources/Documentation/windows.png" "$OUTPUT/AriaSrc-$VERSION/"
cp "./Resources/Documentation/section_code.js" "$OUTPUT/AriaSrc-$VERSION/"
cp "./Resources/Documentation/style.css" "$OUTPUT/AriaSrc-$VERSION/"
rm "$OUTPUT/AriaSrc-$VERSION/install.txt"
cd $OUTPUT/
tar cj --exclude '.svn' --exclude '.DS_Store' --exclude '.sconsign' -f "./AriaSrc-$VERSION.tar.bz2" "./AriaSrc-$VERSION"
cd $OUTPUT/build

# ------ build mac binaries -----
echo "Building..."
export CXX="/usr/bin/llvm-g++"
export CC="/usr/bin/llvm-gcc"
export LD="/usr/bin/llvm-g++"
python scons/scons.py config=release WXCONFIG=$USE_WX_CONFIG CC="/usr/bin/llvm-gcc" CXX="/usr/bin/llvm-g++" LD="/uasr/bin/llvm-g++" CXXFLAGS="$ADDITIONAL_BUILD_FLAGS" LDFLAGS="$ADDITIONAL_LINK_FLAGS" compiler_arch=32bit -j 2
python scons/scons.py install

echo "Copying..."
mkdir -p "$OUTPUT/AriaMaestosa-$VERSION/"
cp -R "$OUTPUT/build/Aria Maestosa.app" "$OUTPUT/AriaMaestosa-$VERSION"
cp "./license.txt" "$OUTPUT/AriaMaestosa-$VERSION/license.txt"

# find "$OUTPUT/AriaMaestaosa-$VERSION/" -name ".svn" -exec rm -rf '{}' \;

#zip -9 -r "$OUTPUT/AriaMaestosa-$VERSION" foo
#rm -rf "$OUTPUT/AriaMaestosa-$VERSION"

#rm -rf "$OUTPUT/AriaSrc-$VERSION"