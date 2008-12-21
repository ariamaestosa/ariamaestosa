# automatically makes OS X releases from scons project
# not really portable i'm afraid. adapt variables as needed
# tries to preserve compatibility with older OS X versions
# does a universal build of jdkmidi; Aria will be built Universal if wxWidgets is.
#
# invoke like this :
#    % export VERSION="1.2b3"
#    % ./make_release.sh

echo "making packgage for version $VERSION"

USE_WX_CONFIG="/Developer/libs/wxMac-2.8.9/universal-build/wx-config"
OUTPUT="$HOME/Desktop/aria-build/"
ADDITIONAL_BUILD_FLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.3.9 -Wfatal-errors"
ADDITIONAL_LINK_FLAGS="-isysroot /Developer/SDKs/MacOSX10.4u.sdk -mmacosx-version-min=10.3.9"

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
cd $OUTPUT/
tar cj --exclude '.svn' --exclude '.DS_Store' --exclude '.sconsign' -f "./AriaSrc-$VERSION.tar.bz2" "./AriaSrc-$VERSION"
cd $OUTPUT/build

# ------ build mac binaries -----
cd libjdkmidi
./configure
sed 's/CXXFLAGS=/CXXFLAGS=-isysroot \/Developer\/SDKs\/MacOSX10.4u.sdk -mmacosx-version-min=10.3.9 -arch ppc -arch i386/' < GNUMakefile > GNUMakefile1
sed 's/LDFLAGS=/LDFLAGS=-isysroot \/Developer\/SDKs\/MacOSX10.4u.sdk -mmacosx-version-min=10.3.9 -arch ppc -arch i386/' < GNUMakefile1 > GNUMakefile2
rm GNUMakefile1
rm GNUMakefile
mv GNUMakefile2 GNUMakefile
make -f GNUMakefile clean
make -f GNUMakefile all
cd ..

python scons/scons.py config=release WXCONFIG=$USE_WX_CONFIG CXXFLAGS="$ADDITIONAL_BUILD_FLAGS" LDFLAGS="$ADDITIONAL_LINK_FLAGS" -j 2
python scons/scons.py install

mkdir -p "$OUTPUT/AriaMaestosa-$VERSION/"
cp -R "$OUTPUT/build/AriaMaestosa.app" "$OUTPUT/AriaMaestosa-$VERSION"
cp "./license.txt" "$OUTPUT/AriaMaestosa-$VERSION/license.txt"

# find "$OUTPUT/AriaMaestosa-$VERSION/" -name ".svn" -exec rm -rf '{}' \;

#zip -9 -r "$OUTPUT/AriaMaestosa-$VERSION" foo
#rm -rf "$OUTPUT/AriaMaestosa-$VERSION"

#rm -rf "$OUTPUT/AriaSrc-$VERSION"