import sys
import os

Help("""
      Usage:
        % scons
            does a release build, auto-detects your system
            
        Flags you can pass when calling 'scons' :
            platform=[macosx/linux/unix/windows]
                specify platform, if not auto-detected
            config=[debug/release]
                specify build type
            WXCONFIG=/path/to/wx-config
                build using a specified wx-config
            CXXFLAGS="custom build flags"
            LDFLAGS="custom link flags"
                
        % scons install
            Installs Aria, auto-detects system (run as root if necessary)
            
        Flags you can pass when calling 'scons install' :
            platform=[macosx/linux/unix]
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

        if 'libjdkmidi' in self.directory:
            # ignore libjdkmidi stuff
            self.pop_dir()
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
        elif os.uname()[0] == 'Linux':
            which_os = "linux"
        elif os.uname()[0] == 'Darwin':
            which_os = "macosx"
        else:
            print "!! Unknown operating system '" + os.uname()[0] + "', defaulting to Linux-like Unix"
            which_os = "linux"

    if which_os != 'linux' and which_os != 'macosx' and which_os != 'unix' and which_os != 'windows':
        print "!! Unknown operating system '" + which_os + "', please specify 'platform=[linux/macosx/unix/windows]'"
        sys.exit(0) 
    
    print">> Operating system : " + which_os 
        
    if which_os == "windows":
        print "!! Warning :  Windows is unsupported at this point" 
        
    # check build style
    build_type = ARGUMENTS.get('config', 'release')
    if build_type != 'release' and build_type != 'debug':
        print "!! Unknown build config " + build_type
        sys.exit(0) 
        
    print ">> Build type : " + build_type

    # check what to do
    if 'uninstall' in COMMAND_LINE_TARGETS:
        # uninstall
        if which_os == "linux":
            uninstall_Aria_linux()
        else:
            print "!! Unknown operation or system (uninstall is not valid on your system)"
            sys.exit(0)
    elif 'install' in COMMAND_LINE_TARGETS:
        # install
        if which_os == "linux":
            compile_Aria(build_type, which_os)
        elif which_os == "macosx":
            install_Aria_mac()
        else:
            print "!! Unknown operation or system (install is not valid on your system)"
            sys.exit(0)     
    else:
        # compile
        compile_Aria(build_type, which_os)

# ---------------------------- Install Mac OS X -----------------------------

def install_Aria_mac():
    sys_command("mkdir -p ./AriaMaestosa.app/Contents/MacOS")
    sys_command("cp ./Aria ./AriaMaestosa.app/Contents/MacOS/Aria\ Maestosa")
    sys_command("cp ./release.plist ./AriaMaestosa.app/Contents/info.plist")
    sys_command("cp -r ./Resources ./AriaMaestosa.app/Contents/")
    sys_command("cp -r ./mac-i18n/. ./AriaMaestosa.app/Contents/Resources/.")
    
    print "*** Cleaning up..."
    os.system("cd ./AriaMaestosa.app && find . -name \".svn\" -exec rm -rf '{}' \;")
    
    print "*** Done"
    sys.exit(0)

# ---------------------------- Uninstall Linux -----------------------------

def uninstall_Aria_linux():
    
    # check if user defined his own prefix, else use defaults
    prefix = ARGUMENTS.get('prefix', 0)
    
    if prefix == 0:
        print ">> No prefix specified, defaulting to /usr/local/"
        prefix = '/usr/local/'
    else:
         print ">> Prefix: " + prefix
    
    if prefix[-1] != "/":
        prefix += "/"
        
    resource_path = prefix + "share/Aria/"
    app_path = prefix + "bin/Aria"
    locale_path = prefix + "share/locale/"
    
    os.system("rm -r " + resource_path)
    os.system("rm " + app_path)
    os.system("rm " + locale_path + "*/LC_MESSAGES/aria_maestosa.mo")

    print "\n*** Uninstall done"
    sys.exit(0)

# -- small helper func
# executes a command on the system shell and prints it to stdout
def sys_command(command):
    print command
    return_status = os.system(command)
    if return_status != 0:
        print "An error occured"
        sys.exit(0)
        
# ---------------------------- Compile -----------------------------
def compile_Aria(build_type, which_os):

    env = Environment()

    # add wxWidgets flags
    # check if user defined his own WXCONFIG, else use defaults
    WXCONFIG = ARGUMENTS.get('WXCONFIG', 'wx-config')
    print ">> wx-config : " + WXCONFIG
        
    env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,base,gl'])

    # check build type and init build flags
    if build_type == "debug":
        env.Append(CCFLAGS=['-g','-D_MORE_DEBUG_CHECKS','-D_CHECK_FOR_LEAKS','-Wfatal-errors'])
        
    elif build_type == "release":
        env.Append(CCFLAGS=['-O3'])
    
    else:
        print 'Unknown build type, cannot continue'
        sys.exit(0)
        
    # init common library and header search paths
    env.Append(CPPPATH = ['wxAdditions','.','./libjdkmidi/include'])
    env.Append(LIBPATH = ['.','./libjdkmidi/tmp/build/lib','./libjdkmidi/tmp-target/build/lib'])
    env.Append(LIBS = ['libjdkmidi'])

    print " "

    # add common sources
    print "*** Adding source files"
    
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

    # OS X (QTKit, CoreAudio, audiotoolbox)
    if which_os == "macosx":

        print "*** Adding mac source files and libraries"
        env.Append(CCFLAGS=['-D_MAC_QUICKTIME_COREAUDIO'])
        sources = sources + ['Midi/Players/Mac/QTKitPlayer.mm']
        env.Append(CPPPATH=['Midi/Players/Mac'])
    
        env.Append(LINKFLAGS = ['-framework','OpenGL','-framework','GLUT','-framework','AGL',
        '-framework','QTKit','-framework', 'Quicktime','-framework','CoreAudio',
        '-framework','AudioToolbox','-framework','AudioUnit','-framework','AppKit',
        '-framework','Carbon','-framework','Cocoa','-framework','IOKit','-framework','System'])
        
    # linux (Alsa/tiMidity)
    elif which_os == "linux":
    
        print "*** Adding Alsa libraries and defines"
        
        env.Append(CCFLAGS=['-DwxUSE_GLCANVAS=1','-D_ALSA'])
        
        env.Append(CPPPATH = ['/usr/include'])
        env.Append(LINKFLAGS = ['-Wl,--rpath,/usr/local/lib/'])
        env.Append(LIBPATH = ['usr/local/lib/','usr/lib/', '/opt/gnome/lib'])
        
        env.Append(LIBS = ['GL', 'GLU', 'glut'])
        env.Append(LIBS = ['asound'])
        env.Append(LIBS = ['dl','m'])
        env.ParseConfig( 'pkg-config --cflags glib-2.0' )

    # Windows (currently unsupported)
    elif which_os == "windows":
        pass
    else:
    
        print "\n\n/!\\ Platform must be either mac or linux "
        sys.exit(0) 
    
    
    # *********************************************************************************************
    # **************************************** COMPILE ********************************************
    # *********************************************************************************************
    
    print " "
    print "====================="
    print "     Setup done "
    print "====================="
    print " "

    # compile to .o
    object_list = env.Object(source = sources)
    
    # link program
    executable = env.Program( target = 'Aria', source = object_list )

    # install target
    if 'install' in COMMAND_LINE_TARGETS:

        # check if user defined his own prefix, else use defaults
        prefix = ARGUMENTS.get('prefix', 0)
    
        if prefix == 0:
            print ">> No prefix specified, defaulting to /usr/local/"
            prefix = '/usr/local/'
        else:
            print ">> Prefix : " + prefix

        # set umask so created directories have the correct permissions
        try:
            umask = os.umask(022)
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
        Chmod("$TARGET", 0775),
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
            Chmod("$TARGET", 0664),
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
