# Microsoft Developer Studio Project File - Name="viewbutton" - Package Owner=<4>
# Microsoft Developer Studio Generated Build File, Format Version 6.00
# ** DO NOT EDIT **

# TARGTYPE "Win32 (x86) Application" 0x0101

CFG=viewbutton - Win32 Debug
!MESSAGE This is not a valid makefile. To build this project using NMAKE,
!MESSAGE use the Export Makefile command and run
!MESSAGE 
!MESSAGE NMAKE /f "viewbutton.mak".
!MESSAGE 
!MESSAGE You can specify a configuration when running NMAKE
!MESSAGE by defining the macro CFG on the command line. For example:
!MESSAGE 
!MESSAGE NMAKE /f "viewbutton.mak" CFG="viewbutton - Win32 Debug"
!MESSAGE 
!MESSAGE Possible choices for configuration are:
!MESSAGE 
!MESSAGE "viewbutton - Win32 Release" (based on "Win32 (x86) Application")
!MESSAGE "viewbutton - Win32 Debug" (based on "Win32 (x86) Application")
!MESSAGE 

# Begin Project
# PROP AllowPerConfigDependencies 0
# PROP Scc_ProjName ""
# PROP Scc_LocalPath ""
CPP=cl.exe
MTL=midl.exe
RSC=rc.exe

!IF  "$(CFG)" == "viewbutton - Win32 Release"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 0
# PROP BASE Output_Dir "Release"
# PROP BASE Intermediate_Dir "Release"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 0
# PROP Output_Dir "Release"
# PROP Intermediate_Dir "Release"
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /c
# ADD CPP /nologo /MD /W3 /GX /O2 /D "WIN32" /D "NDEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /Yu"stdafx.h" /FD /c
# ADD BASE MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "NDEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "NDEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /machine:I386
# ADD LINK32 /nologo /subsystem:windows /machine:I386

!ELSEIF  "$(CFG)" == "viewbutton - Win32 Debug"

# PROP BASE Use_MFC 6
# PROP BASE Use_Debug_Libraries 1
# PROP BASE Output_Dir "Debug"
# PROP BASE Intermediate_Dir "Debug"
# PROP BASE Target_Dir ""
# PROP Use_MFC 6
# PROP Use_Debug_Libraries 1
# PROP Output_Dir "Debug"
# PROP Intermediate_Dir "Debug"
# PROP Ignore_Export_Lib 0
# PROP Target_Dir ""
# ADD BASE CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /Yu"stdafx.h" /FD /GZ /c
# ADD CPP /nologo /MDd /W3 /Gm /GX /ZI /Od /D "WIN32" /D "_DEBUG" /D "_WINDOWS" /D "_AFXDLL" /D "_MBCS" /FR /FD /GZ /c
# SUBTRACT CPP /YX /Yc /Yu
# ADD BASE MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD MTL /nologo /D "_DEBUG" /mktyplib203 /win32
# ADD BASE RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
# ADD RSC /l 0x804 /d "_DEBUG" /d "_AFXDLL"
BSC32=bscmake.exe
# ADD BASE BSC32 /nologo
# ADD BSC32 /nologo
LINK32=link.exe
# ADD BASE LINK32 /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept
# ADD LINK32 strmiids.lib ddraw.lib CxImage\Lib\CxImage.lib CxImage\Lib\Jpeg.lib /nologo /subsystem:windows /debug /machine:I386 /pdbtype:sept

!ENDIF 

# Begin Target

# Name "viewbutton - Win32 Release"
# Name "viewbutton - Win32 Debug"
# Begin Group "Source Files"

# PROP Default_Filter "cpp;c;cxx;rc;def;r;odl;idl;hpj;bat"
# Begin Source File

SOURCE=.\BtnST.cpp
# End Source File
# Begin Source File

SOURCE=.\EnumDeviceDlg.cpp
# End Source File
# Begin Source File

SOURCE=.\ISampleGrabberCB.cpp
# End Source File
# Begin Source File

SOURCE=.\ISampleRendererCB.cpp
# End Source File
# Begin Source File

SOURCE=.\libvisca.c
# End Source File
# Begin Source File

SOURCE=.\MainFrm.cpp
# End Source File
# Begin Source File

SOURCE=.\MYButton.cpp
# End Source File
# Begin Source File

SOURCE=.\MYStatic.cpp
# End Source File
# Begin Source File

SOURCE=.\PTZCtrl.cpp
# End Source File
# Begin Source File

SOURCE=.\savePTZ.cpp
# End Source File
# Begin Source File

SOURCE=.\StdAfx.cpp
# ADD CPP /Yc"stdafx.h"
# End Source File
# Begin Source File

SOURCE=.\VideoDirectShow.cpp
# End Source File
# Begin Source File

SOURCE=.\viewbutton.cpp
# End Source File
# Begin Source File

SOURCE=.\viewbutton.rc
# End Source File
# Begin Source File

SOURCE=.\viewbuttonDoc.cpp
# End Source File
# Begin Source File

SOURCE=.\viewbuttonView.cpp
# End Source File
# Begin Source File

SOURCE=.\XPReBar.cpp
# End Source File
# Begin Source File

SOURCE=.\XPSliderCtrl.cpp
# End Source File
# End Group
# Begin Group "Header Files"

# PROP Default_Filter "h;hpp;hxx;hm;inl"
# Begin Source File

SOURCE=.\BtnST.h
# End Source File
# Begin Source File

SOURCE=.\EnumDeviceDlg.h
# End Source File
# Begin Source File

SOURCE=.\Interface.h
# End Source File
# Begin Source File

SOURCE=.\ISampleGrabberCB.h
# End Source File
# Begin Source File

SOURCE=.\ISampleRendererCB.h
# End Source File
# Begin Source File

SOURCE=.\libvisca.h
# End Source File
# Begin Source File

SOURCE=.\MainFrm.h
# End Source File
# Begin Source File

SOURCE=.\MYButton.h
# End Source File
# Begin Source File

SOURCE=.\MYStatic.h
# End Source File
# Begin Source File

SOURCE=.\PTZCtrl.h
# End Source File
# Begin Source File

SOURCE=.\Resource.h
# End Source File
# Begin Source File

SOURCE=.\savePTZ.h
# End Source File
# Begin Source File

SOURCE=.\StdAfx.h
# End Source File
# Begin Source File

SOURCE=.\VideoDirectShow.h
# End Source File
# Begin Source File

SOURCE=.\viewbutton.h
# End Source File
# Begin Source File

SOURCE=.\viewbuttonDoc.h
# End Source File
# Begin Source File

SOURCE=.\viewbuttonView.h
# End Source File
# Begin Source File

SOURCE=.\XPReBar.h
# End Source File
# Begin Source File

SOURCE=.\XPSliderCtrl.h
# End Source File
# End Group
# Begin Group "Resource Files"

# PROP Default_Filter "ico;cur;bmp;dlg;rc2;rct;bin;rgs;gif;jpg;jpeg;jpe"
# Begin Source File

SOURCE=.\res\1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\BACKGROUND.BMP
# End Source File
# Begin Source File

SOURCE=.\res\background1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap1.bmp
# End Source File
# Begin Source File

SOURCE=.\res\bitmap2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\BK.BMP
# End Source File
# Begin Source File

SOURCE=.\res\bmp00001.bmp
# End Source File
# Begin Source File

SOURCE=.\res\down.BMP
# End Source File
# Begin Source File

SOURCE=.\res\down_after.BMP
# End Source File
# Begin Source File

SOURCE=.\res\kY2Yl1Y.jpg
# End Source File
# Begin Source File

SOURCE=.\res\left.BMP
# End Source File
# Begin Source File

SOURCE=.\res\left.gif
# End Source File
# Begin Source File

SOURCE=.\res\left_after.BMP
# End Source File
# Begin Source File

SOURCE=.\res\ptzright.bmp
# End Source File
# Begin Source File

SOURCE=.\res\right.BMP
# End Source File
# Begin Source File

SOURCE=.\res\right_after.BMP
# End Source File
# Begin Source File

SOURCE=".\res\S5-0862.BMP"
# End Source File
# Begin Source File

SOURCE=.\res\Toolbar.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolBar2.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ToolBar3.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbar4.bmp
# End Source File
# Begin Source File

SOURCE=.\res\toolbarback.bmp
# End Source File
# Begin Source File

SOURCE=.\res\Transformer.BMP
# End Source File
# Begin Source File

SOURCE=.\res\up.BMP
# End Source File
# Begin Source File

SOURCE=.\res\up_after.BMP
# End Source File
# Begin Source File

SOURCE=.\res\ViewBack.bmp
# End Source File
# Begin Source File

SOURCE=.\res\viewbutton.ico
# End Source File
# Begin Source File

SOURCE=.\res\viewbutton.rc2
# End Source File
# Begin Source File

SOURCE=.\res\viewbuttonDoc.ico
# End Source File
# Begin Source File

SOURCE=.\res\ZOOMIN.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ZOOMIN1.BMP
# End Source File
# Begin Source File

SOURCE=.\res\ZOOMOUT.bmp
# End Source File
# Begin Source File

SOURCE=.\res\ZOOMOUT1.BMP
# End Source File
# End Group
# Begin Source File

SOURCE=.\ReadMe.txt
# End Source File
# End Target
# End Project
