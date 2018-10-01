# Microsoft Developer Studio Project File - Name="chaseTrack" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=chaseTrack - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "chaseTrack.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "chaseTrack.mak" CFG="chaseTrack - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "chaseTrack - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "chaseTrack - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "chaseTrack - Win32 Release"

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
# ADD BASE CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD CPP /nologo /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "NDEBUG"
# ADD RSC /l 0x409 /d "NDEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386
# ADD LINK32 cv.lib highgui.lib libc.lib msvcrt.lib libcmt.lib ws2_32.lib winmm.lib ilib_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /machine:I386 /out:"CTracker.exe"

!ELSEIF  "$(CFG)" == "chaseTrack - Win32 Debug"

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
# ADD BASE CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /YX /FD /GZ /c
# ADD CPP /nologo /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x409 /d "_DEBUG"
# ADD RSC /l 0x409 /d "_DEBUG"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 cv.lib cxcore.lib highgui.lib libc.lib msvcrt.lib libcmt.lib ws2_32.lib winmm.lib ilib_32.lib kernel32.lib user32.lib gdi32.lib winspool.lib comdlg32.lib advapi32.lib shell32.lib ole32.lib oleaut32.lib uuid.lib odbc32.lib odbccp32.lib /nologo /subsystem:windows /debug /machine:I386 /nodefaultlib:"libcd.lib" /out:"Debug/CTracker.exe" /pdbtype:sept

!ENDIF 

# Begin Target

# Name "chaseTrack - Win32 Release"
# Name "chaseTrack - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Group "MeanShift"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\meanshift\globaldata.c
# End Source File
# Begin Source File

SOURCE=.\meanshift\globaldata.h
# End Source File
# Begin Source File

SOURCE=.\meanshift\imageProcessing.c
# End Source File
# Begin Source File

SOURCE=.\meanshift\imageProcessing.h
# End Source File
# Begin Source File

SOURCE=.\meanshift\meanshift_tracker.c
# End Source File
# Begin Source File

SOURCE=.\meanshift\meanshift_tracker.h
# End Source File
# End Group
# Begin Group "Klt"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\klt\convolve.c
# End Source File
# Begin Source File

SOURCE=.\klt\error.c
# End Source File
# Begin Source File

SOURCE=.\klt\klt.c
# End Source File
# Begin Source File

SOURCE=.\klt\klt_util.c
# End Source File
# Begin Source File

SOURCE=.\klt\pnmio.c
# End Source File
# Begin Source File

SOURCE=.\klt\pyramid.c
# End Source File
# Begin Source File

SOURCE=.\klt\selectGoodFeatures.c
# End Source File
# Begin Source File

SOURCE=.\klt\storeFeatures.c
# End Source File
# Begin Source File

SOURCE=.\klt\trackFeatures.c
# End Source File
# Begin Source File

SOURCE=.\klt\writeFeatures.c
# End Source File
# End Group
# Begin Group "AffineTransform"

# PROP Default_Filter ""
# Begin Source File

SOURCE=.\AffineTransform\affprojNR.c
# End Source File
# Begin Source File

SOURCE=.\AffineTransform\affprojNR.h
# End Source File
# Begin Source File

SOURCE=.\AffineTransform\nrutil.c
# End Source File
# Begin Source File

SOURCE=.\AffineTransform\pythag.c
# End Source File
# Begin Source File

SOURCE=.\AffineTransform\svbksb.c
# End Source File
# Begin Source File

SOURCE=.\AffineTransform\svdcmp.c
# End Source File
# End Group
# Begin Source File

SOURCE=.\Histogram.c
# End Source File
# Begin Source File

SOURCE=.\ImageIO.c
# End Source File
# Begin Source File

SOURCE=.\KltWrap.c
# End Source File
# Begin Source File

SOURCE=.\Main.c
# End Source File
# Begin Source File

SOURCE=.\MeanShiftWrap.c
# End Source File
# Begin Source File

SOURCE=.\Msfeature.c
# End Source File
# Begin Source File

SOURCE=.\TemplateMatch.c
# End Source File
# Begin Source File

SOURCE=.\Utility.c
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\Histogram.h
# End Source File
# Begin Source File

SOURCE=.\ImageIO.h
# End Source File
# Begin Source File

SOURCE=.\KltWrap.h
# End Source File
# Begin Source File

SOURCE=.\MeanShiftWrap.h
# End Source File
# Begin Source File

SOURCE=.\Msfeature.h
# End Source File
# Begin Source File

SOURCE=.\scalePyramid.h
# End Source File
# Begin Source File

SOURCE=.\TemplateMatch.h
# End Source File
# Begin Source File

SOURCE=.\utility.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\chaseTrack.rc
# End Source File
# Begin Source File

SOURCE=.\Imagepxc.ico
# End Source File
# End Group
# End Target
# End Project
