# Microsoft Developer Studio Project File - Name="MercReckoning" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO EDIT **
# TARGTYPE "Win32 (x86) Console Application" 0x0103
# TARGTYPE "Win32 (x86) External Target" 0x0106
CFG=MercReckoning - Win32 Debug
!MESSAGE NMAKE /f "MercReckoning.mak".
!MESSAGE NMAKE /f "MercReckoning.mak" CFG="MercReckoning - Win32 Debug"
!MESSAGE "MercReckoning - Win32 Release" (based on  "Win32 (x86) Console Application")
!MESSAGE "MercReckoning - Win32 Debug" (based on  "Win32 (x86) Console Application")
!MESSAGE "MercReckoning - NACL Release" (basierend auf  "Win32 (x86) External Target")
!MESSAGE "MercReckoning - NACL Debug" (basierend auf  "Win32 (x86) External Target")
!MESSAGE "MercReckoning - Emscripten Release" (basierend auf  "Win32 (x86) External Target")
!MESSAGE "MercReckoning - Emscripten Debug" (basierend auf  "Win32 (x86) External Target")
# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "MercReckoning - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release-vc6"
# PROP Intermediate_Dir "Release-vc6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /MT /W3 /O2 /I "../ZillaLib/Include" /D "WIN32" /D "NDEBUG" /D "_MBCS" /FD /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x807 /d "NDEBUG"
# ADD RSC /l 0x807 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 "../ZillaLib/Release-vc6/ZillaLib.lib" /nologo /subsystem:windows /pdb:"Release-vc6/MercReckoning.pdb" /map:"Release-vc6/MercReckoning.map" /machine:I386 /out:"Release-vc6/MercReckoning.exe"
# SUBTRACT LINK32 /pdb:none

!ELSEIF  "$(CFG)" == "MercReckoning - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug-vc6"
# PROP Intermediate_Dir "Debug-vc6"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /MTd /W3 /Gm /ZI /Od /I "../ZillaLib/Include" /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /D "ZILLALOG" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x807 /d "_DEBUG"
# ADD RSC /l 0x807 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 "../ZillaLib/Debug-vc6/ZillaLib.lib" /nologo /subsystem:console /incremental:yes /pdb:"Debug-vc6/MercReckoning.pdb" /debug /machine:I386 /out:"Debug-vc6/MercReckoning.exe" /pdbtype:sept

!ELSEIF  "$(CFG)" == "MercReckoning - NACL Release"

# PROP Output_Dir "Release-nacl"
# PROP Intermediate_Dir "Release-nacl"
# PROP Cmd_Line "python ../ZillaLib/NACL/ZillaLibNACL.py build -rel -vc MercReckoning"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Release-nacl/MercReckoning_x86_64.nexe.gz.exe"

!ELSEIF  "$(CFG)" == "MercReckoning - NACL Debug"

# PROP Output_Dir "Debug-nacl"
# PROP Intermediate_Dir "Debug-nacl"
# PROP Cmd_Line "python ../ZillaLib/NACL/ZillaLibNACL.py build -vc MercReckoning"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Debug-nacl/MercReckoning_x86_64.nexe.gz.exe"

!ELSEIF  "$(CFG)" == "MercReckoning - Emscripten Release"

# PROP Output_Dir "Release-emscripten"
# PROP Intermediate_Dir "Release-emscripten"
# PROP Cmd_Line "python ../ZillaLib/Emscripten/ZillaLibEmscripten.py build -rel -vc MercReckoning"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Release-emscripten/MercReckoning.js.exe"

!ELSEIF  "$(CFG)" == "MercReckoning - Emscripten Debug"

# PROP Output_Dir "Debug-emscripten"
# PROP Intermediate_Dir "Debug-emscripten"
# PROP Cmd_Line "python ../ZillaLib/Emscripten/ZillaLibEmscripten.py build -vc MercReckoning"
# PROP Rebuild_Opt "-clean"
# PROP Target_File "Debug-emscripten/MercReckoning.js.exe"

!ENDIF

# Begin Target
# Name "MercReckoning - Win32 Release"
# Name "MercReckoning - Win32 Debug"
# Name "MercReckoning - NACL Release"
# Name "MercReckoning - NACL Debug"
# Name "MercReckoning - Emscripten Release"
# Name "MercReckoning - Emscripten Debug"
# Begin Source File
SOURCE=./include.h
# End Source File
# Begin Source File
SOURCE=./main.cpp
# End Source File
# Begin Source File
SOURCE=./animals.cpp
# End Source File
# Begin Source File
SOURCE=./animals.h
# End Source File
# Begin Source File
SOURCE=./Bugs.txt
# End Source File
# Begin Source File
SOURCE=./SceneEditor.cpp
# End Source File
# Begin Source File
SOURCE=./SceneGame.cpp
# End Source File
# Begin Source File
SOURCE=./SceneMenu.cpp
# End Source File
# Begin Source File
SOURCE=./SceneIntro.cpp
# End Source File
# Begin Source File
SOURCE=./SceneTeam.cpp
# End Source File
# Begin Source File
SOURCE=./SceneCleared.cpp
# End Source File
# Begin Source File
SOURCE=./ToDo.txt
# End Source File
# Begin Source File
SOURCE=./world.cpp
# End Source File
# Begin Source File
SOURCE=./world.h
# End Source File
# Begin Source File
SOURCE=./MercReckoning.rc
# End Source File
# End Target
# End Project
