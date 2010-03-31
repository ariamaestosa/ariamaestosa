# generates xcconfig files from a wx-config given as parameter

if [ -z $1 ]; then
    echo "ERROR, please specify the path to wx-config!!"
    exit 1
fi

WHICH=`which $1`
WX_CFLAGS=`$1 --cxxflags`
WX_LDFLAGS=`$1 --libs core,base,gl`

echo "== Using wx-config '$WHICH' =="

echo ""
echo "Compile Flags :"
echo $WX_CFLAGS

echo ""
echo "Link Flags :"
echo $WX_LDFLAGS


sed "s#\$(WXCONFIG_BUILDFLAGS)#$WX_CFLAGS#" <  DebugConfig.xcconfig.in  > DebugConfig.xcconfig.tmp
sed "s#\$(WXCONFIG_LINKFLAGS)#$WX_LDFLAGS#" <  DebugConfig.xcconfig.tmp > DebugConfig.xcconfig
rm DebugConfig.xcconfig.tmp

sed "s#\$(WXCONFIG_BUILDFLAGS)#$WX_CFLAGS#" <  ReleaseConfig.xcconfig.in  > ReleaseConfig.xcconfig.tmp
sed "s#\$(WXCONFIG_LINKFLAGS)#$WX_LDFLAGS#" <  ReleaseConfig.xcconfig.tmp > ReleaseConfig.xcconfig
rm ReleaseConfig.xcconfig.tmp

sed "s#\$(WXCONFIG_BUILDFLAGS)#$WX_CFLAGS#" <  DebugConfigNoGL.xcconfig.in  > DebugConfigNoGL.xcconfig.tmp
sed "s#\$(WXCONFIG_LINKFLAGS)#$WX_LDFLAGS#" <  DebugConfigNoGL.xcconfig.tmp > DebugConfigNoGL.xcconfig
rm DebugConfigNoGL.xcconfig.tmp

sed "s#\$(WXCONFIG_BUILDFLAGS)#$WX_CFLAGS#" <  ReleaseConfigNoGL.xcconfig.in  > ReleaseConfigNoGL.xcconfig.tmp
sed "s#\$(WXCONFIG_LINKFLAGS)#$WX_LDFLAGS#" <  ReleaseConfigNoGL.xcconfig.tmp > ReleaseConfigNoGL.xcconfig
rm ReleaseConfigNoGL.xcconfig.tmp

echo ""
echo "Files Written"
