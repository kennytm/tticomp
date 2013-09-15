# Microsoft Developer Studio Project File - Name="NewOTFont" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Console Application" 0x0103

CFG=NewOTFont - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "OTFontTest.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "OTFontTest.mak" CFG="NewOTFont - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "NewOTFont - Win32 Release" (based on "Win32 (x86) Console Application")
!MESSAGE "NewOTFont - Win32 Debug" (based on "Win32 (x86) Console Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
RSC=rc.exe

!IF  "$(CFG)" == "NewOTFont - Win32 Release"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /c
# ADD BASE RSC /l 0x809 /d "NDEBUG"
# ADD RSC /l 0x809 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386
# ADD LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /machine:I386 /nodefaultlib:"LIBCMT"

!ELSEIF  "$(CFG)" == "NewOTFont - Win32 Debug"

# PROP BASE Use_MFC 0
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 0
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_CONSOLE" /D "_MBCS" /FD /GZ /c
# SUBTRACT CPP /YX
# ADD BASE RSC /l 0x809 /d "_DEBUG"
# ADD RSC /l 0x809 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:console /debug /machine:I386 /pdbtype:sept
# ADD LINK32 Util.lib /nologo /subsystem:console /debug /machine:I386 /nodefaultlib:"LIBCMT" /pdbtype:sept /libpath:"../Util/Debug"

!ENDIF 

# Begin Target

# Name "NewOTFont - Win32 Release"
# Name "NewOTFont - Win32 Debug"
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\CompositeOTText.h
# End Source File
# Begin Source File

SOURCE=.\OpenType.h
# End Source File
# Begin Source File

SOURCE=.\OpenTypeFile.h
# End Source File
# Begin Source File

SOURCE=.\OpenTypeFont.h
# End Source File
# Begin Source File

SOURCE=.\OpenTypeText.h
# End Source File
# Begin Source File

SOURCE=.\OTcmapTable.h
# End Source File
# Begin Source File

SOURCE=.\OTException.h
# End Source File
# Begin Source File

SOURCE=.\OTgaspTable.h
# End Source File
# Begin Source File

SOURCE=.\OTGlyph.h
# End Source File
# Begin Source File

SOURCE=.\OTheadTable.h
# End Source File
# Begin Source File

SOURCE=.\OThheaTable.h
# End Source File
# Begin Source File

SOURCE=.\OThmtxTable.h
# End Source File
# Begin Source File

SOURCE=.\OTInstructionProcessor.h
# End Source File
# Begin Source File

SOURCE=.\OTInstructions.h
# End Source File
# Begin Source File

SOURCE=.\OTLayoutInternal.h
# End Source File
# Begin Source File

SOURCE=.\OTLayoutTable.h
# End Source File
# Begin Source File

SOURCE=.\OTlocaTable.h
# End Source File
# Begin Source File

SOURCE=.\OTmaxpTable.h
# End Source File
# Begin Source File

SOURCE=.\OTMemoryBlock.h
# End Source File
# Begin Source File

SOURCE=.\OTOS2Table.h
# End Source File
# Begin Source File

SOURCE=.\OTpostTable.h
# End Source File
# Begin Source File

SOURCE=.\OTTable.h
# End Source File
# Begin Source File

SOURCE=.\OTTags.h
# End Source File
# End Group
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\CompositeOTText.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenTypeFile.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenTypeFont.cpp
# End Source File
# Begin Source File

SOURCE=.\OpenTypeText.cpp
# End Source File
# Begin Source File

SOURCE=.\OTcmapTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTException.cpp
# End Source File
# Begin Source File

SOURCE=.\OTFontTestcpp\OTFontTest.cpp
# End Source File
# Begin Source File

SOURCE=.\OTgaspTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTGDEFTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTGlyph.cpp
# End Source File
# Begin Source File

SOURCE=.\OTGPOSTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTGSUBTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTheadTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OThheaTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OThmtxTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTInstructionProcessor.cpp
# End Source File
# Begin Source File

SOURCE=.\OTInstructions.cpp
# End Source File
# Begin Source File

SOURCE=.\OTLayoutTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTlocaTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTmaxpTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTMemoryBlock.cpp
# End Source File
# Begin Source File

SOURCE=.\OTOS2Table.cpp
# End Source File
# Begin Source File

SOURCE=.\OTpostTable.cpp
# End Source File
# Begin Source File

SOURCE=.\OTTable.cpp
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# End Group
# End Target
# End Project
