# WinUtils Installer
# Copyright (C) 2015 Renato Silva

Name "WinUtils"
!define VERSION "16.1"
RequestExecutionLevel admin
InstallDirRegKey HKLM Software\WinUtils Install_Dir
!if ${NSIS_PTR_SIZE} = 8
    InstallDir "$PROGRAMFILES64\WinUtils"
    OutFile "WinUtils ${VERSION} x64 Setup.exe"
    !define BITNESS "64-bit"
!else
    InstallDir "$PROGRAMFILES32\WinUtils"
    OutFile "WinUtils ${VERSION} x86 Setup.exe"
    !define BITNESS "32-bit"
!endif

VIAddVersionKey "ProductName" "WinUtils"
VIAddVersionKey "ProductVersion" "${VERSION}"
VIAddVersionKey "LegalCopyright" "Renato Silva"
VIAddVersionKey "FileDescription" "WinUtils Installer"
VIAddVersionKey "FileVersion" "${VERSION}"
VIProductVersion "0.0.0.0"

!include "MUI2.nsh"
!define MUI_ABORTWARNING
!define MUI_ICON "resource\winutils.ico"
!define MUI_UNICON "resource\winutils.ico"
!define MUI_FINISHPAGE_SHOWREADME "documents\README.html"
!define MUI_FINISHPAGE_SHOWREADME_CHECKED
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "LICENSE"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_WELCOME
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH
!insertmacro MUI_LANGUAGE "English"
!insertmacro MUI_LANGUAGE "PortugueseBR"

Section "WinUtils" SecMain
    SectionIn RO
    SetOutPath "$INSTDIR"
    File /r "build\*.*"

    CreateDirectory "$SMPROGRAMS\WinUtils"
    CreateShortcut "$SMPROGRAMS\WinUtils\Uninstall.lnk"     "$INSTDIR\uninstall.exe"    "" "$INSTDIR\do_uninstall.exe" 0
    CreateShortcut "$SMPROGRAMS\WinUtils\Notes Hider.lnk"   "$INSTDIR\noteshider.exe"   "" "$INSTDIR\noteshider.exe"   0
    CreateShortcut "$SMPROGRAMS\WinUtils\Toogle Hidden.lnk" "$INSTDIR\togglehidden.exe" "" "$INSTDIR\togglehidden.exe" 0
    CreateShortcut "$SMPROGRAMS\WinUtils\Send Tray.lnk"     "$INSTDIR\sendtray.exe"     "" "$INSTDIR\sendtray.exe"     0

    WriteRegStr HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Run SendTray     "$INSTDIR\sendtray.exe"
    WriteRegStr HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Run NotesHider   "$INSTDIR\noteshider.exe"
    WriteRegStr HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Run ToggleHidden "$INSTDIR\togglehidden.exe"

    WriteRegStr HKLM SOFTWARE\WinUtils Install_Dir "$INSTDIR"
    WriteRegStr HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WinUtils DisplayName "WinUtils (${BITNESS})"
    WriteRegStr HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WinUtils DisplayVersion "${VERSION}"
    WriteRegStr HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WinUtils DisplayIcon "$INSTDIR\do_uninstall.exe"
    WriteRegStr HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WinUtils UninstallString "$INSTDIR\uninstall.exe"
    WriteRegDWORD HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WinUtils NoModify 1
    WriteRegDWORD HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WinUtils NoRepair 1
    WriteUninstaller do_uninstall.exe
SectionEnd

Section "Uninstall"
    RMDir /r "$SMPROGRAMS\WinUtils"
    RMDir /r "$INSTDIR"
    DeleteRegKey HKLM SOFTWARE\WinUtils
    DeleteRegKey HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Uninstall\WinUtils
    DeleteRegValue HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Run SendTray
    DeleteRegValue HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Run NotesHider
    DeleteRegValue HKLM SOFTWARE\Microsoft\Windows\CurrentVersion\Run ToggleHidden
SectionEnd
