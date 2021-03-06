import sys
import os
import platform
import subprocess

Help("""
      Usage:
        % scons
            does a release build, auto-detects your system
            
        Flags you can pass when calling 'scons' :
            platform=[macosx/linux/unix/windows/netbsd]
                specify platform, if not auto-detected
            config=[debug/release]
                specify build type
            jack=[0/1]
                whether to enable the Jack MIDI driver (on by default for non-Linux unices, disabled everywhere else by default)
            WXCONFIG=/path/to/wx-config
                build using a specified wx-config (unix)
            compiler_arch=[32bit/64bit]
                specify whether the compiler will build as 32 bits or 64 bits
                (does _not_ add flags to cross-compile, only selects the right lib dirs)
                * currently only has an effect on Linux.
            renderer=[opengl/wxwidgets]
                choose whether to use the OpenGL renderer or the software (wxWidgets-based) renderer
            CXXFLAGS="custom build flags"
                To add other flags to pass when compiling
            LDFLAGS="custom link flags"
                To add other flags to pass when linking
            WX_HOME="C:\wxWidgets-2.8.10"
                for windows only, define the wx home directory
             
        Furthermore, the CXX environment variable is read if it exists, allowing
        you to choose which g++ executable you wish to use.
        The PATH environment variable is also considered.
                
        % scons install
            Installs Aria, auto-detects system (run as root if necessary)
            
        Flags you can pass when calling 'scons install' :
            platform=[macosx/linux/unix/netbsd]
                specify platform, if not auto-detected
            prefix=[/opt/local or something else]
                install to a different prefix than default /usr/local
            
        % scons uninstall
            Uninstalls Aria, takes same flags as 'scons install'.
            If you specified a custom install prefix, you need to specify it again.
            * Not available on mac OS X, just drag the generated app to the trash.
            
        Use scons -H for help about scons itself, and its terminal flags.
      """)

# recursive Glob
import fnmatch
class RecursiveGlob:

    def __init__(self, directory, pattern="*"):
        self.dir_stack = [directory]
        self.pattern = pattern
        self.files = []
        self.index = 0

    def pop_dir(self):
        # pop next directory from stack
        self.directory = self.dir_stack.pop()
        self.files = os.listdir(self.directory)
        self.index = 0

        #if 'libjdkmidi' in self.directory:
        #    # ignore libjdkmidi stuff
        #    self.pop_dir()
        if 'scons' in self.directory:
            # ignore scons stuff
            self.pop_dir()
            
    def __getitem__(self, index):
        while 1:
            try:
                file = self.files[self.index]
                self.index = self.index + 1
            except IndexError:
                # pop next directory from stack
                self.pop_dir()
            else:
                # got a filename
                fullname = os.path.join(self.directory, file)
                if os.path.isdir(fullname) and not os.path.islink(fullname):
                    self.dir_stack.append(fullname)
                if fnmatch.fnmatch(file, self.pattern):
                    return fullname

# ------------------------------- find system, build type ----------------------
def main_Aria_func():
    
    # find operating system
    which_os = ARGUMENTS.get('platform', 0)
    if which_os == 0:
        #auto-detect
        if os.name == 'nt':
            which_os = "windows"
        elif os.uname()[0] == 'NetBSD' or os.uname()[0] == 'FreeBSD':
            which_os = "netbsd"
        elif os.uname()[0] == 'Linux':
            which_os = "linux"
        elif os.uname()[0] == 'Darwin':
            which_os = "macosx"
        else:
            print("!! Unknown operating system '" + os.uname()[0] + "', defaulting to Unix")
            which_os = "unix"

    if not which_os in ["netbsd", "linux", "macosx", "unix", "windows"]:
        print("!! Unknown operating system '" + which_os + "', please specify 'platform=[linux/macosx/unix/windows/netbsd]'")
        sys.exit(0) 
    
    print(">> Operating system : " + which_os)

    # check what to do
    if 'uninstall' in COMMAND_LINE_TARGETS:
        # uninstall
        if which_os in ["linux", "unix", "netbsd"]:
            uninstall_Aria_unix()
        else:
            print("!! Unknown operation or system (uninstall is not valid on your system)")
            sys.exit(0)
    elif 'install' in COMMAND_LINE_TARGETS:
        # install
        if which_os in ["linux", "unix", "netbsd"]:
            compile_Aria(which_os)
        elif which_os == "macosx":
            install_Aria_mac()
        else:
            print("!! Unknown operation or system (install is not valid on your system)")
            sys.exit(0)     
    else:
        # compile
        compile_Aria(which_os)

# ---------------------------- Install Mac OS X -----------------------------

def install_Aria_mac():
    sys_command("mkdir -p ./Aria\ Maestosa.app/Contents/MacOS")
    sys_command("cp ./Aria ./Aria\ Maestosa.app/Contents/MacOS/Aria\ Maestosa")
    sys_command("cp ./OSX/release.plist ./Aria\ Maestosa.app/Contents/Info.plist")
    sys_command("cp -r ./Resources ./Aria\ Maestosa.app/Contents/")
    sys_command("cp -r ./OSX/mac-i18n/. ./Aria\ Maestosa.app/Contents/Resources/.")
    sys_command("cp -r ./OSX/*.icns ./Aria\ Maestosa.app/Contents/Resources/.")
    sys_command("touch ./Aria\ Maestosa.app")

    print("*** Cleaning up...")
    os.system("cd ./Aria\ Maestosa.app && find . -name \".svn\" -exec rm -rf '{}' \;")
    
    print("*** Done")
    sys.exit(0)

# ---------------------------- Uninstall Linux -----------------------------

def uninstall_Aria_unix():
    
    # check if user defined his own prefix, else use defaults
    prefix = ARGUMENTS.get('prefix', 0)
    
    if prefix == 0:
        print(">> No prefix specified, defaulting to /usr/local/")
        prefix = '/usr/local/'
    else:
         print(">> Prefix: " + prefix)
    
    if prefix[-1] != "/":
        prefix += "/"
        
    resource_path = prefix + "share/Aria/"
    app_path = prefix + "bin/Aria"
    locale_path = prefix + "share/locale/"
    
    os.system("rm -r " + resource_path)
    os.system("rm " + app_path)
    os.system("rm " + locale_path + "*/LC_MESSAGES/aria_maestosa.mo")

    print("\n*** Uninstall done")
    sys.exit(0)

# -- small helper func
# executes a command on the system shell and prints it to stdout
def sys_command(command):
    print(command)
    return_status = os.system(command)
    if return_status != 0:
        print("An error occured")
        sys.exit(0)
        
# ---------------------------- Compile -----------------------------
def compile_Aria(which_os):

    if which_os == "windows":
        # on Windows ask for MinGW, VC++ can't handle Aria
        env = Environment(tools = ['mingw'])
    else:
        env = Environment()
    
    env.Decider('MD5-timestamp')
    
    #env.Append(PATH = os.environ['PATH'])
    env['ENV']['PATH'] = os.environ.get('PATH')
    
    if 'CXX' in os.environ:
        print(">> Using compiler " + os.environ['CXX'])
        env.Replace(CXX = os.environ['CXX'])

    # check build style
    build_type = ARGUMENTS.get('config', 'release')
    if build_type != 'release' and build_type != 'debug':
        print("!! Unknown build config " + build_type)
        sys.exit(0) 
        
    print(">> Build type : " + build_type)
    
    # check renderer
    if which_os == "macosx":
        renderer = ARGUMENTS.get('renderer', 'opengl')
    else:
        renderer = ARGUMENTS.get('renderer', 'wxwidgets')
    if renderer != 'opengl' and renderer != 'wxwidgets':
        print("!! Unknown renderer " + renderer)
        sys.exit(0)

    print(">> Renderer : " + renderer)
    if renderer == 'opengl':
        env.Append(CCFLAGS=['-DRENDERER_OPENGL'])
    elif renderer == 'wxwidgets':
        env.Append(CCFLAGS=['-DRENDERER_WXWIDGETS'])

    # Check architecture
    compiler_arch = ARGUMENTS.get('compiler_arch', platform.architecture(env['CXX']))[0]
    if compiler_arch != '32bit' and compiler_arch != '64bit':
        print('Invalid architecture : ', compiler_arch, '; assuming 32bit')
        compiler_arch = '32bit'
        
    print(">> Architecture : " + compiler_arch)
    
    # add wxWidgets flags
    # check if user defined his own WXCONFIG, else use defaults
    WXCONFIG = ARGUMENTS.get('WXCONFIG', 'wx-config')
    if which_os != 'windows':
        print(">> wx-config : " + WXCONFIG)
        
    if which_os == 'windows':
    
        wxHomePath = ARGUMENTS.get('WX_HOME', None)
        if wxHomePath is None:
            sys.stderr.write("Please pass WX_HOME for Windows builds")
            sys.exit(1)
           
        winCppFlags = ['-mthreads', '-DHAVE_W32API_H', '-D__WXMSW__', '-D_UNICODE', '-I' + wxHomePath + '\lib\gcc_dll\mswu',
                       '-I' + wxHomePath + '\include', '-DWXUSINGDLL', '-Wno-ctor-dtor-privacy', '-std=gnu++11'] 

        winLdFlags = ['-mthreads', '-L' + wxHomePath + '\lib\gcc_dll', '-lwxbase312u_gcc_custom', '-lwxmsw312u_core_gcc_custom',
                      '-lwxmsw312u_adv_gcc_custom', '-lwxbase312u_net_gcc_custom', '-lwxtiff', '-lwxjpeg', '-lwxpng',
                      '-lwxzlib', '-lwxregexu', '-lwxexpat', '-lkernel32', '-luser32', '-lgdi32', '-lcomdlg32', '-lwxregexu', '-lwinspool',
                      '-lwinmm', '-lshell32', '-lcomctl32', '-lole32', '-loleaut32', '-luuid', '-lrpcrt4', '-ladvapi32', '-lwsock32']
        
        if renderer == "opengl":
            winLdFlags = winLdFlags + ['-lopengl32','-lwxmsw312u_gl_gcc_custom','-lglu32']
        
        print("Build flags :", winCppFlags)
        print("Link flags :", winLdFlags)
        
        try:
            command = ["windres", "--include-dir="+wxHomePath+"\include", "--input", "win32\Aria.rc", "--output", "msvcr.o"]
            print(command)
            out = subprocess.Popen(command, stdout = subprocess.PIPE, stderr = subprocess.PIPE).communicate()
        except:
            sys.stderr.write("could not execute 'windres', is mingw installed?\n")
        
        env.Append(CCFLAGS=winCppFlags)
        
        #env.Append(LINKFLAGS=['-mwindows'] + winLdFlags.split())
        # Ugly hack : wx flags need to appear at the end of the command, but scons doesn't support that, so I need to hack their link command
        env['LINKCOM']     = '$LINK -o $TARGET $LINKFLAGS $SOURCES $_LIBDIRFLAGS $_LIBFLAGS ' + (' -mwindows ' if build_type == 'release' else '') + ' '.join(winLdFlags)
    else:
        wxversion = subprocess.check_output([WXCONFIG,"--version"]).decode().strip()
        print(">> wxWidgets version : " + wxversion)
        is_wx_3 = (wxversion[0] == '3' or (wxversion[0] == '2' and wxversion[2] == '9'))
        if is_wx_3:
            if renderer == "opengl":
                if which_os == "macosx":
                    env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,base,adv,gl,net,webview'])
                else:
                    env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,base,adv,gl,net'])
            else:
                env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,base,adv,net'])
                # env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,base,adv,net,webview'])
        else:
            if renderer == "opengl":
                env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,base,adv,net,gl'])
            else:
                env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,net,adv,base'])
            
    # check build type and init build flags
    if build_type == "debug":
        env.Append(CCFLAGS=['-g','-Wall','-Wextra','-Wno-unused-parameter','-D_MORE_DEBUG_CHECKS','-D_CHECK_FOR_LEAKS','-Wfatal-errors','-DDEBUG=1'])
        
    elif build_type == "release":
        if which_os == "macosx":
            # Workaround optimisation bug in clang
            env.Append(CCFLAGS=['-O1','-DNDEBUG=1'])
        else:
            env.Append(CCFLAGS=['-O2','-DNDEBUG=1'])
    
    else:
        print('Unknown build type, cannot continue')
        sys.exit(0)
        
    # init common header search paths
    env.Append(CPPPATH = ['./Src','.','./libjdkmidi/include','./rtmidi'])

    print(" ")

    # add common sources
    print("*** Adding source files")
    
    sources = []
    for file in RecursiveGlob(".", "*.cpp"):
        sources = sources + [file]

    # add additional flags if any
    user_flags = ARGUMENTS.get('CXXFLAGS', 0)
    if user_flags != 0:
        env.Append(CCFLAGS=Split(user_flags))
    user_flags = ARGUMENTS.get('LDFLAGS', 0)
    if user_flags != 0:
        env.Append(LINKFLAGS=Split(user_flags))


    # **********************************************************************************************
    # ********************************* PLATFORM SPECIFIC ******************************************
    # ********************************************************************************************** 

    use_jack = ARGUMENTS.get('jack', False)

    # OS X (QTKit, CoreAudio, audiotoolbox)
    if which_os == "macosx":

        print("*** Adding mac source files and libraries")
        env.Append(CCFLAGS=['-D_MAC_QUICKTIME_COREAUDIO'])
        sources = sources + ['Src/Midi/Players/Mac/QuickTimeExport.mm','Src/GUI/Machelper.mm']
        env.Append(CPPPATH=['Src/Midi/Players/Mac'])
    
        env.Append(LINKFLAGS = ['-framework','QTKit','-framework', 'Quicktime','-framework','CoreAudio',
        '-framework','AudioToolbox','-framework','AudioUnit','-framework','AppKit',
        '-framework','Carbon','-framework','Cocoa','-framework','IOKit','-framework','System',
        '-framework','CoreMIDI'])
        
        if compiler_arch == '32bit':
            env.Append(CCFLAGS=['-arch','i386'])
            env.Append(LINKFLAGS = ['-arch','i386'])
        elif compiler_arch == '64bit':
            env.Append(CCFLAGS=['-arch','x86_64'])
            env.Append(LINKFLAGS = ['-arch','x86_64'])
            
            
        if renderer == 'opengl':
            env.Append(LINKFLAGS = ['-framework','OpenGL','-framework','AGL'])
        
    # NetBSD, FreeBSD (Alsa/tiMidity)
    elif which_os == "netbsd":
    
        print("*** Adding Alsa libraries and defines")
        
        if renderer == 'opengl':
            env.Append(CCFLAGS=['-DwxUSE_GLCANVAS=1'])
        
        env.Append(CCFLAGS=['-D_ALSA'])
        
        env.Append(CPPPATH = ['/usr/include'])
        
	# env.Append(LINKFLAGS = ['-Wl,--rpath,/usr/local/lib/'])
	# env.Append(LIBPATH = ['usr/local/lib/','usr/lib/', '/opt/gnome/lib'])
        if compiler_arch == '64bit':
            env.Append(CCFLAGS=['-D__X86_64__'])
            
        if renderer == 'opengl':
            env.Append(LIBS = ['GL', 'GLU'])
            
        env.Append(LIBS = ['asound'])
        env.ParseConfig( 'pkg-config --cflags glib-2.0' )
        env.ParseConfig( 'pkg-config --libs glib-2.0' )
        
    # linux (Alsa/tiMidity)
    elif which_os == "linux":
    
        print("*** Adding Alsa libraries and defines")
        
        if renderer == 'opengl':
            env.Append(CCFLAGS=['-DwxUSE_GLCANVAS=1'])
        
        env.Append(CCFLAGS=['-D_ALSA'])
        
        env.Append(CPPPATH = ['/usr/include'])
        
        if compiler_arch == '32bit':
            env.Append(LINKFLAGS = ['-Wl,--rpath,/usr/local/lib/'])
            env.Append(LIBPATH = ['usr/local/lib/','usr/lib/', '/opt/gnome/lib'])
        elif compiler_arch == '64bit':
            env.Append(LINKFLAGS = ['-Wl,--rpath,/usr/local/lib64/'])
            env.Append(LIBPATH = ['usr/local/lib64/','usr/lib64/'])
            env.Append(CCFLAGS=['-D__X86_64__'])
            
        if renderer == 'opengl':
            env.Append(LIBS = ['GL', 'GLU'])
            
        env.Append(LIBS = ['asound'])
        env.Append(LIBS = ['dl','m'])
        env.ParseConfig( 'pkg-config --cflags glib-2.0' )
        env.ParseConfig( 'pkg-config --libs glib-2.0' )
        
    elif which_os == "unix":
        print("*** Adding libraries and defines for Unix")
        
        if renderer == 'opengl':
            env.Append(CCFLAGS=['-DwxUSE_GLCANVAS=1'])
            env.Append(LIBS = ['GL', 'GLU'])

        # default sound driver for Unix, if not explicitely set
        if ARGUMENTS.get('jack', '!') == '!':
            use_jack = True

        env.Append(CPPPATH = ['/usr/local/include'])
        env.Append(LIBPATH = ['/usr/local/lib'])
        env.ParseConfig('pkg-config --cflags glib-2.0')
        env.ParseConfig('pkg-config --libs glib-2.0')
        
    # Windows
    elif which_os == "windows":
        pass

    else:
    
        print("\n\n/!\\ Platform ", which_os, " is unknown")
        sys.exit(0)


    if use_jack:
        env.Append(CCFLAGS=['-DUSE_JACK'])
        env.Append(LIBS = ['jack'])
    
    
    # *********************************************************************************************
    # **************************************** COMPILE ********************************************
    # *********************************************************************************************
    
    print(" ")
    print("=====================")
    print("     Setup done ")
    print("=====================")
    print(" ")

    # compile to .o
    object_list = env.Object(source = sources)
    
    if which_os == "windows":
        object_list = object_list + ["msvcr.o"]
    
    # link program
    executable = env.Program( target = 'Aria', source = object_list)

    # install target
    if 'install' in COMMAND_LINE_TARGETS:

        # check if user defined his own prefix, else use defaults
        prefix = ARGUMENTS.get('prefix', 0)
    
        if prefix == 0:
            print(">> No prefix specified, defaulting to /usr/local/")
            prefix = '/usr/local/'
        else:
            print(">> Prefix : " + prefix)

        # set umask so created directories have the correct permissions
        try:
            umask = os.umask(0o022)
        except OSError:     # ignore on systems that don't support umask
            pass
    
        bin_dir = os.path.join(prefix, "bin")
        data_dir = os.path.join(prefix, "share/Aria")
        locale_dir = os.path.join(prefix, "share/locale")

        if not os.path.exists(prefix):
            Execute(Mkdir(prefix))

        # install executable
        executable_target = bin_dir + "/Aria"
        env.Alias("install", executable_target)
        env.Command( executable_target, executable,
        [
        Copy("$TARGET","$SOURCE"),
        Chmod("$TARGET", 0o0775),
        ])        


        # install data files
        data_files = []
        for file in RecursiveGlob("./Resources", "*"):
            if ".svn" in file or ".icns" in file or "*" in file:
                continue
            index = file.find("Resources/") + len("Resources/")
            filename_relative = file[index:]
            source = os.path.join("./Resources", filename_relative)
            target = os.path.join(data_dir, filename_relative)
            
            env.Alias("install", target)
            env.Command( target, source,
            [
            Copy("$TARGET","$SOURCE"),
            Chmod("$TARGET", 0o0664),
            ])

        # install .mo files
        mo_files = Glob("./international/*/aria_maestosa.mo",strings=True)
        for mo in mo_files:
            index_lo = mo.find("international/") + len("international/")
            index_hi = mo.find("/aria_maestosa.mo")
            lang_name = mo[index_lo:index_hi]
            install_location = locale_dir + "/" + lang_name + "/LC_MESSAGES/aria_maestosa.mo"
            env.Alias("install", env.InstallAs( install_location, mo ) )

main_Aria_func()
