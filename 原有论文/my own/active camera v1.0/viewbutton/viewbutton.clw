; CLW file contains information for the MFC ClassWizard

[General Info]
Version=1
LastClass=CViewbuttonView
LastTemplate=CDialog
NewFileInclude1=#include "stdafx.h"
NewFileInclude2=#include "viewbutton.h"
LastPage=0

ClassCount=9
Class1=CViewbuttonApp
Class2=CViewbuttonDoc
Class3=CViewbuttonView
Class4=CMainFrame

ResourceCount=4
Resource1=IDD_SAVEPTZ
Class5=CAboutDlg
Class6=CMYButton
Resource2=IDD_ABOUTBOX
Class7=CsavePTZ
Class8=CMYStatic
Resource3=IDD_ENUMDEVICE
Class9=CEnumDeviceDlg
Resource4=IDR_MAINFRAME

[CLS:CViewbuttonApp]
Type=0
HeaderFile=viewbutton.h
ImplementationFile=viewbutton.cpp
Filter=N
BaseClass=CWinApp
VirtualFilter=AC
LastObject=CViewbuttonApp

[CLS:CViewbuttonDoc]
Type=0
HeaderFile=viewbuttonDoc.h
ImplementationFile=viewbuttonDoc.cpp
Filter=N

[CLS:CViewbuttonView]
Type=0
HeaderFile=viewbuttonView.h
ImplementationFile=viewbuttonView.cpp
Filter=W
BaseClass=CView
VirtualFilter=VWC
LastObject=CViewbuttonView


[CLS:CMainFrame]
Type=0
HeaderFile=MainFrm.h
ImplementationFile=MainFrm.cpp
Filter=T
BaseClass=CFrameWnd
VirtualFilter=fWC
LastObject=IDM_READPTZSTATE




[CLS:CAboutDlg]
Type=0
HeaderFile=viewbutton.cpp
ImplementationFile=viewbutton.cpp
Filter=D
LastObject=IDC_CHECK1
BaseClass=CDialog
VirtualFilter=dWC

[DLG:IDD_ABOUTBOX]
Type=1
Class=CAboutDlg
ControlCount=5
Control1=IDC_STATIC,static,1342177283
Control2=IDC_STATIC,static,1342308480
Control3=IDC_STATIC,static,1342308352
Control4=IDOK,button,1342373889
Control5=IDC_CHECK1,button,1342242819

[MNU:IDR_MAINFRAME]
Type=1
Class=?
Command1=IDM_OPEN
Command2=IDM_CAPTURE
Command3=IDM_PURGE
Command4=IDM_RECORD
Command5=ID_APP_EXIT
Command6=IDM_ADJUST
Command7=IDM_TOGGLEALGO
Command8=IDM_RULE
Command9=IDM_SETTING
Command10=IDM_SAVEPTZSTATE
Command11=IDM_READPTZSTATE
Command12=IDM_PLAY
Command13=IDM_PAUSE
Command14=IDM_STOP
Command15=IDM_STEP
Command16=ID_VIEW_TOOLBAR
Command17=IDM_VIEW_SLIDER
Command18=ID_VIEW_STATUS_BAR
CommandCount=18

[ACL:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_FILE_PRINT
Command5=ID_EDIT_UNDO
Command6=ID_EDIT_CUT
Command7=ID_EDIT_COPY
Command8=ID_EDIT_PASTE
Command9=ID_EDIT_UNDO
Command10=ID_EDIT_CUT
Command11=ID_EDIT_COPY
Command12=ID_EDIT_PASTE
Command13=ID_NEXT_PANE
Command14=ID_PREV_PANE
CommandCount=14

[TB:IDR_MAINFRAME]
Type=1
Class=?
Command1=ID_FILE_NEW
Command2=ID_FILE_OPEN
Command3=ID_FILE_SAVE
Command4=ID_EDIT_CUT
Command5=ID_EDIT_COPY
Command6=ID_EDIT_PASTE
Command7=ID_FILE_PRINT
Command8=ID_APP_ABOUT
CommandCount=8

[CLS:CMYButton]
Type=0
HeaderFile=MYButton.h
ImplementationFile=MYButton.cpp
BaseClass=CButton
Filter=W
VirtualFilter=BWC
LastObject=CMYButton

[DLG:IDD_SAVEPTZ]
Type=1
Class=CsavePTZ
ControlCount=4
Control1=IDOK,button,1342275595
Control2=IDCANCEL,button,1342275595
Control3=IDC_SAVESTATE,edit,1342242816
Control4=IDC_STATIC,static,1342308352

[CLS:CsavePTZ]
Type=0
HeaderFile=savePTZ.h
ImplementationFile=savePTZ.cpp
BaseClass=CDialog
Filter=D
LastObject=CsavePTZ
VirtualFilter=dWC

[CLS:CMYStatic]
Type=0
HeaderFile=MYStatic.h
ImplementationFile=MYStatic.cpp
BaseClass=CStatic
Filter=W
VirtualFilter=WC
LastObject=CMYStatic

[DLG:IDD_ENUMDEVICE]
Type=1
Class=CEnumDeviceDlg
ControlCount=4
Control1=IDOK,button,1476460545
Control2=IDCANCEL,button,1342242816
Control3=IDC_COMBODEVICE,combobox,1344340226
Control4=IDC_STATIC,static,1342308352

[CLS:CEnumDeviceDlg]
Type=0
HeaderFile=EnumDeviceDlg.h
ImplementationFile=EnumDeviceDlg.cpp
BaseClass=CDialog
Filter=D
VirtualFilter=dWC
LastObject=CEnumDeviceDlg

