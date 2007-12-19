message = "\n\nOPTIONS:\n\nplatform=[mac,linux] (required)\nplayer=[macframeworks, pmidialsa](facultative if your platform has a default player)\nrelease=[debug, beta, final] (if unspecified, defaults to final.)\n\n"

import sys
import os

def main_Aria_func():
    
    install = ARGUMENTS.get('install', 0)
    uninstall = ARGUMENTS.get('uninstall', 0)
    
    if install and 'linux' in install:
        install_Aria_linux()
    elif install and 'mac' in install:
        install_Aria_mac()
    if uninstall and 'linux' in uninstall:
        uninstall_Aria_linux()
    else:
        compile_Aria()


def uninstall_Aria_linux():
    
    # check if user defined his own prefix, else use defaults
    prefix = ARGUMENTS.get('prefix', 0)
    
    if prefix == 0:
        print "*** No prefix specified, defaulting to /usr/local/"
        prefix = '/usr/local/'
    else:
         print "*** Installing to prefix " + prefix
    
    if prefix[-1] != "/":
        prefix += "/"
        
    resource_path = prefix + "share/Aria/"
    app_path = prefix + "bin/Aria"
    locale_path = prefix + "share/locale/"
    
    os.system("sudo rm -r " + resource_path)
    os.system("sudo rm " + app_path)
    os.system("sudo rm " + locale_path + "fr/LC_MESSAGES/aria_maestosa.mo")
   
def sys_command(command):
    print command
    return_status = os.system(command)
    if return_status != 0:
        print "An error occured"
        sys.exit(0)
        
def install_Aria_mac():
    sys_command("mkdir -p ./AriaMaestosa.app/Contents/MacOS")
    sys_command("cp ./Aria ./AriaMaestosa.app/Contents/MacOS/Aria\ Maestosa")
    sys_command("cp ./release.plist ./AriaMaestosa.app/Contents/info.plist")
    #sys_command("cp -r ./Resources ./AriaMaestosa.app/Contents/")
    
    # these stupid commands are a way to remove invisible SVN files from resources.
    # there must be a better way - unix gurus, help me!
    sys_command("tar cj --exclude '.svn' --exclude '.DS_Store' --exclude '.sconsign' -f ./rsrc.tar.bz2 Resources")
    sys_command("mv ./rsrc.tar.bz2 ./AriaMaestosa.app/Contents/")
    sys_command("cd ./AriaMaestosa.app/Contents/ && bzip2 -d ./rsrc.tar.bz2 && tar -xf ./rsrc.tar && rm ./rsrc.tar")
    print "** Done. (The application icon will eventually appear)"
    sys.exit(0)
    
def install_Aria_linux():
    
    # check if user defined his own prefix, else use defaults
    prefix = ARGUMENTS.get('prefix', 0)
    
    if prefix == 0:
        print "*** No prefix specified, defaulting to /usr/local/"
        prefix = '/usr/local/'
    else:
         print "*** Installing to prefix " + prefix
    
    if prefix[-1] != "/":
        prefix += "/"
        
    resource_path = prefix + "share/Aria/"
    app_path = prefix + "bin/Aria"
    locale_path = prefix + "share/locale/"
    
    # copy resources
    os.system("sudo mkdir -p " + resource_path)
    os.system("sudo cp -r --remove-destination Resources/* " + resource_path)
    
    # copy executable
    os.system("sudo cp --remove-destination ./Aria " + app_path)
    
    #copy translations
    os.system("sudo cp ./Resources/fr.lproj/aria_maestosa.mo " + locale_path + "fr/LC_MESSAGES/aria_maestosa.mo")
    
    #copy docs
    os.system("sudo cp -r ./../docs " + resource_path)

    os.system("echo Installation done")
    sys.exit(0)


def compile_Aria():

    # check if user defined his own WXCONFIG, else use defaults
    WXCONFIG = ARGUMENTS.get('WXCONFIG', 0)

    if WXCONFIG == 0:
        print "No wx-config specified, using default"
        WXCONFIG = 'wx-config'

    libjdk_include_path = ['./libjdkmidi/include']
    libjdk_lib_path = ['./libjdkmidi/tmp/build/lib'] 

    #  --------------------- headers, libs, etc.  ---------------------

    header_search_path = ['wxAdditions','.'] + libjdk_include_path
    libpath = ['.'] + libjdk_lib_path
    libs = ['libjdkmidi']

    # **********************************************************************************************
    # *********************************** COMMON SOURCES *******************************************
    # **********************************************************************************************

    print "*** Adding common sources"

    sources = Split("""
    Actions/AddControlEvent.cpp
    Actions/AddControllerSlide.cpp
    Actions/AddNote.cpp
    Actions/DeleteSelected.cpp
    Actions/EditAction.cpp
    Actions/InsertEmptyMeasures.cpp
    Actions/MoveNotes.cpp
    Actions/NumberPressed.cpp
    Actions/Paste.cpp
    Actions/RearrangeNotes.cpp
    Actions/RemoveMeasures.cpp
    Actions/RemoveOverlapping.cpp
    Actions/ResizeNotes.cpp
    Actions/ScaleSong.cpp
    Actions/ScaleTrack.cpp
    Actions/SetNoteVolume.cpp
    Actions/ShiftFrets.cpp
    Actions/ShiftString.cpp
    Actions/SnapNotesToGrid.cpp
    Actions/UpdateGuitarTuning.cpp
    Clipboard.cpp
    Dialogs/About.cpp
    Dialogs/CopyrightWindow.cpp
    Dialogs/CustomNoteSelectDialog.cpp
    Dialogs/Preferences.cpp
    Dialogs/ScalePicker.cpp
    Dialogs/WaitWindow.cpp
    Editors/ControllerEditor.cpp
    Editors/DrumEditor.cpp
    Editors/Editor.cpp
    Editors/GuitarEditor.cpp
    Editors/KeyboardEditor.cpp
    Editors/ScoreAnalyser.cpp
    Editors/ScoreEditor.cpp
    Editors/RelativeXCoord.cpp
    GUI/GLPane.cpp
    GUI/GraphicalTrack.cpp
    GUI/MainFrame.cpp
    GUI/MeasureBar.cpp
    Images/Drawable.cpp
    Images/Image.cpp
    Images/ImageProvider.cpp
    Images/wxImageLoader.cpp
    IO/AriaFileWriter.cpp
    IO/IOUtils.cpp
    IO/MidiFileReader.cpp
    IO/MidiToMemoryStream.cpp
    IO/TablatureExporter.cpp
    IO/NotationExport.cpp
    irrXML/irrXML.cpp
    LeakCheck.cpp
    main.cpp
    Midi/ControllerEvent.cpp
    Midi/Note.cpp
    Midi/Sequence.cpp
    Midi/Track.cpp
    Midi/CommonMidiUtils.cpp
    Midi/TimeSigChange.cpp
    Pickers/ControllerChoice.cpp
    Pickers/DrumChoice.cpp
    Pickers/KeyPicker.cpp
    Pickers/InstrumentChoice.cpp
    Pickers/MagneticGrid.cpp
    Pickers/TuningPicker.cpp
    Pickers/VolumeSlider.cpp
    wxAdditions/bsizer.cpp
    """)

    # *************** retrieve info from arguments ************

    player_arg = ARGUMENTS.get('player', 0)
    platform_arg = ARGUMENTS.get('platform', 0)
    release_arg = ARGUMENTS.get('release', 0)

    # **********************************************************************************************
    # *************************************** PLAYERS **********************************************
    # ********************************************************************************************** 
    
    # this part just checks the user entered something, and if not pick a default value
    if player_arg and 'macframeworks' in player_arg:
        print "*** Player: Mac Frameworks"
    elif player_arg and 'pmidialsa' in player_arg:
        print "*** Player: pmidi/Alsa"
    else:
    
        # default values
        if platform_arg and 'mac' in platform_arg:
            print "*** Player: unspecified. Defaulting to Mac Frameworks."
            player_arg = 'macframeworks'
    
        elif platform_arg and 'linux' in platform_arg:
            print "*** Player: unspecified. Defaulting to pMidi/Alsa."
            player_arg = 'pmidialsa'
    
        else:
            print"\n\n/!\\ Please specify a platform ",message
            sys.exit(0)
    
    
    #  --------------------- add platform native sources if necessary  ---------------------
    
    if 'macframeworks' in player_arg:
        source_mac_native = Split("""Midi/Players/Mac/AudioToolboxPlayer.cpp
        Midi/Players/Mac/CoreAudioNotePlayer.cpp
        Midi/Players/Mac/MacPlayerInterface.cpp
        """)
    
        print "*** Adding mac sources"
        c_flags = ['-D_MAC_QUICKTIME_COREAUDIO']
        sources = sources + source_mac_native
        header_search_path = header_search_path + ['Midi/Players/Mac']
    
    # --------------------- add pmidi sources if necessary ----------------------
    
    if 'pmidialsa' in player_arg:
    
        source_pmidi = Split("""
        Midi/Players/pmidi/elements.cpp
        Midi/Players/pmidi/alsaNotePlayer.cpp
        Midi/Players/pmidi/AlsaPort.cpp
        Midi/Players/pmidi/mdutil.cpp
        Midi/Players/pmidi/MidiDataProvider.cpp
        Midi/Players/pmidi/midiread.cpp
        Midi/Players/pmidi/pmidi.cpp
        Midi/Players/pmidi/seqlib.cpp
        Midi/Players/pmidi/seqmidi.cpp
        """)
        
        print "*** Adding pMidi/Alsa sources"
        c_flags = ['-DwxUSE_GLCANVAS=1','-D_PMIDI_ALSA']
        sources = sources + source_pmidi
        #header_search_path = header_search_path + ['/usr/include/glib-2.0/','/usr/include/glib-2.0/glib','/usr/lib/glib-2.0/include']
        
    # ***********************************************************************************************
    # ************************************* PLATFORM ************************************************
    # ***********************************************************************************************
    
    print "*** Adding libs"
    
    if platform_arg and 'mac' in platform_arg:
    
        linkflags =  ['-framework','OpenGL','-framework','GLUT','-framework','AGL',
        '-framework','QTKit','-framework', 'Quicktime','-framework','CoreAudio',
        '-framework','AudioToolbox','-framework','AudioUnit','-framework','AppKit',
        '-framework','Carbon','-framework','Cocoa','-framework','IOKit','-framework','System']
    
    elif platform_arg and 'linux' in platform_arg:
    
        linkflags = ['-Iusr/include','-Wl,--rpath,/usr/local/lib/']
        libpath = libpath + ['usr/local/lib/','usr/lib/', '/opt/gnome/lib']
        
        libs = libs + Split("""
        GL
        GLU
        glut
        asound
        z
        dl
        m
        """)
    
    else:
    
        print "\n\n*** /!\\ Platform must be either mac or linux ",message
        sys.exit(0) 
    
    # ***************************************************************************************************
    # *********************************** DEBUG/RELEASE *************************************************
    # ***************************************************************************************************
    
    if release_arg and 'debug' in release_arg:
    
        print '*** Release: debug'
        c_flags = c_flags + ['-g','-D_MORE_DEBUG_CHECKS','-D_CHECK_FOR_LEAKS','-Wfatal-errors']
        
    elif release_arg and 'beta' in release_arg:
    
        print '*** Release: beta'
        c_flags = c_flags + ['-O3','-g','-D_CHECK_FOR_LEAKS']
        
    elif release_arg and 'final' in release_arg:
    
        print '*** Release: final'
        c_flags = c_flags + ['-O3']
    
    else:
    
        print '*** Release: Unspecified. Defaulting to final'
        c_flags = c_flags + ['-O3']
    
    
    # **************************************************************************************************
    # ******************************************* COMPILE **********************************************
    # **************************************************************************************************
    
    env = Environment()
    env.ParseConfig( [WXCONFIG] + ['--cppflags','--libs','core,base,gl'])
    
    if 'pmidialsa' in player_arg:
        env.ParseConfig( 'pkg-config --cflags glib-2.0' )
        
    object_list = env.Object(
    source = sources,
    CPPPATH = env['CPPPATH'] + header_search_path,
    CCFLAGS = c_flags + env['CCFLAGS']#['`'] + [WXCONFIG] + ['--cppflags`']
    )
    
    if platform_arg and 'mac' in platform_arg:
        print "Building QTKitPlayer.mm..."
        os.system("gcc -x objective-c++ -c -O3 -D_MAC_QUICKTIME_COREAUDIO Midi/Players/Mac/QTKitPlayer.mm -I. -IMidi -IMidi/Players -IMidi/Players/Mac")
        object_list = object_list + ['QTKitPlayer.o']
    
    # check if we are using a LIBPATH before trying to add it to build params
    # this is because depending on platform and player there is not always one
    # and we don't want to trigger errors
    try:
        env['LIBPATH']
    except KeyError:
        print "No libpath in config"
    else:
        libpath = libpath + env['LIBPATH']
    
    env.Program(
    target = 'Aria',
    source = object_list,
    LIBPATH = libpath,
    LIBS = env['LIBS'] + libs,
    LINKFLAGS = linkflags)

main_Aria_func()
