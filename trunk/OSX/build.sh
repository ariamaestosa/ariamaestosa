# /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk
#aapython scons/scons.py WXCONFIG=/Developer/svn/wxWidgets/cocoa_build2/wx-config compiler_arch="32bit" CC="/usr/bin/llvm-gcc" CXX="/usr/bin/llvm-g++" CXXFLAGS="-std=gnu++11 -stdlib=libc++ -mmacosx-version-min=10.9" -j2

USE_WX_CONFIG="/Developer/svn/wxWidgets/cocoa_build2/wx-config "
OUTPUT="$HOME/Desktop/aria-build/"
ADDITIONAL_BUILD_FLAGS="-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -mmacosx-version-min=10.9 -Wfatal-errors"
ADDITIONAL_LINK_FLAGS="-isysroot /Applications/Xcode.app/Contents/Developer/Platforms/MacOSX.platform/Developer/SDKs/MacOSX10.9.sdk -mmacosx-version-min=10.9"

#export CXX="/usr/bin/llvm-g++"
#export CC="/usr/bin/llvm-gcc"
#export LD="/usr/bin/llvm-g++"
#python scons/scons.py config=release WXCONFIG=$USE_WX_CONFIG CC="/usr/bin/llvm-gcc" CXX="/usr/bin/llvm-g++" LD="/uasr/bin/llvm-g++" CXXFLAGS="$ADDITIONAL_BUILD_FLAGS" LDFLAGS="$ADDITIONAL_LINK_FLAGS" compiler_arch="32bit" -j 2
python scons/scons.py config=release WXCONFIG=$USE_WX_CONFIG compiler_arch="32bit" CXXFLAGS="$ADDITIONAL_BUILD_FLAGS" LDFLAGS="$ADDITIONAL_LINK_FLAGS" -j 2
python scons/scons.py install

