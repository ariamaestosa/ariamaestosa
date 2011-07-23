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

USE_WX_CONFIG="/Developer/svn/wxWidgets/cocoa_build/wx-config "
OUTPUT="$HOME/Desktop/aria-build/"
ADDITIONAL_BUILD_FLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.4 -Wfatal-errors"
ADDITIONAL_LINK_FLAGS="-isysroot /Developer/SDKs/MacOSX10.5.sdk -mmacosx-version-min=10.4"
export CC="gcc-4.0"
export CXX="g++-4.0"

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
#cd libjdkmidi
#./configure
#sed 's/CXXFLAGS=/CXXFLAGS=-isysroot \/Developer\/SDKs\/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch ppc -arch i386/' < GNUMakefile > GNUMakefile1
#sed 's/LDFLAGS=/LDFLAGS=-isysroot \/Developer\/SDKs\/MacOSX10.5.sdk -mmacosx-version-min=10.5 -arch ppc -arch i386/' < GNUMakefile1 > GNUMakefile2
#sed 's/CXX=g++/CXX=g++-4.0/' < GNUMakefile2 > GNUMakefile3
#sed 's/CC=gcc/CC=gcc-4.0/' < GNUMakefile3 > GNUMakefile4
#rm GNUMakefile3
#rm GNUMakefile2
#rm GNUMakefile1
#rm GNUMakefile
#mv GNUMakefile4 GNUMakefile
#make -f GNUMakefile clean
#make -f GNUMakefile all
#cd ..

python scons/scons.py config=release WXCONFIG=$USE_WX_CONFIG CXXFLAGS="$ADDITIONAL_BUILD_FLAGS" LDFLAGS="$ADDITIONAL_LINK_FLAGS" compiler_arch=32bit -j 2
python scons/scons.py install

mkdir -p "$OUTPUT/AriaMaestosa-$VERSION/"
cp -R "$OUTPUT/build/Aria Maestosa.app" "$OUTPUT/AriaMaestosa-$VERSION"
cp "./license.txt" "$OUTPUT/AriaMaestosa-$VERSION/license.txt"

# find "$OUTPUT/AriaMaestosa-$VERSION/" -name ".svn" -exec rm -rf '{}' \;

#zip -9 -r "$OUTPUT/AriaMaestosa-$VERSION" foo
#rm -rf "$OUTPUT/AriaMaestosa-$VERSION"

#rm -rf "$OUTPUT/AriaSrc-$VERSION"