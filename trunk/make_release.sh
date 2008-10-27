# automatically makes OS X releases from the XCode project
# not really portable, i'm afraid. adapt as needed

echo "making packgage for version $VERSION"


XCODE_TARGET="StableUniversal"
XCODE_BUILD="./build"
SRC_HOME="./Src"
OUTPUT="$HOME/Desktop"

# ------ mac binaries -----
cd ..
xcodebuild -configuration $XCODE_TARGET

mkdir -p "$OUTPUT/AriaMaestosa-$VERSION/"
cp -R "$XCODE_BUILD/$XCODE_TARGET/" "$OUTPUT/AriaMaestosa-$VERSION"
cp "$SRC_HOME/GPL-license.txt" "$OUTPUT/AriaMaestosa-$VERSION/GPL-license.txt"

find "$OUTPUT/AriaMaestosa-$VERSION/" -name ".svn" -exec rm -rf '{}' \;

#zip -9 -r "$OUTPUT/AriaMaestosa-$VERSION" foo
#rm -rf "$OUTPUT/AriaMaestosa-$VERSION"

# ----- source archive -----
mkdir -p "$OUTPUT/AriaSrc-$VERSION/"
cp -R "$SRC_HOME" "$OUTPUT/AriaSrc-$VERSION"
cp "$SRC_HOME/Resources/Documentation/building.html" "$OUTPUT/AriaSrc-$VERSION/"
cd $OUTPUT
tar cj --exclude '.svn' --exclude '.DS_Store' --exclude '.sconsign' -f "./AriaSrc-$VERSION.tar.bz2" "./AriaSrc-$VERSION"

#rm -rf "$OUTPUT/AriaSrc-$VERSION"