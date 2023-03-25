Name "Aodix"
OutFile "aodixsetup.exe"
Unicode True
SetCompressor LZMA
InstallDir "$PROGRAMFILES\Arguru Software\Aodix"
InstallDirRegKey HKLM "Software\Arguru Software\Aodix" Installdir

!include MUI2.nsh

!define REG_UNINST_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\Aodix"

!define MUI_WELCOMEPAGE_TEXT "Aodix is an ASIO-only audio software. You must install ASIO audio \
drivers on your system so Aodix can operate.$\r$\n$\r$\n\
Click Next to continue."

!define MUI_STARTMENUPAGE_DEFAULTFOLDER "Arguru Software\Aodix"
!define MUI_STARTMENUPAGE_REGISTRY_ROOT HKLM
!define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\Arguru Software\Aodix"
!define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "StartMenuFolder"

!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_DIRECTORY
Var StartMenuFolder
!insertmacro MUI_PAGE_STARTMENU "PageStartMenu" $StartMenuFolder
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_LANGUAGE "English"

Section "Aodix" SecBase
    SetOutPath $INSTDIR
    WriteUninstaller "$INSTDIR\uninst.exe"

    WriteRegStr HKLM "${REG_UNINST_KEY}" "DisplayName" "Arguru Software Aodix"
    WriteRegStr HKLM "${REG_UNINST_KEY}" "DisplayIcon" "$INSTDIR\Aodix.exe,0"
    WriteRegStr HKLM "${REG_UNINST_KEY}" "UninstallString" '"$INSTDIR\uninst.exe"'

    WriteRegStr HKLM "Software\Arguru Software\Aodix" Installdir "$INSTDIR"
    WriteRegStr HKCR ".adx" "" "Aodix.Document"
    WriteRegStr HKCR "Aodix.Document\shell\open\command" "" '"$INSTDIR\Aodix.exe" "%1"'
    WriteRegStr HKCR "Aodix.Document\DefaultIcon" "" "$INSTDIR\Aodix.exe,0"

    File ..\Aodix\Release\Aodix.exe
    File /oname=Aodix.chm ..\HTMLHelp\aodixv4helpv1.chm
    File /oname=LICENSE.txt ..\LICENSE

    SetShellVarContext all
    !insertmacro MUI_STARTMENU_WRITE_BEGIN "PageStartMenu"
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Aodix.lnk" "$INSTDIR\Aodix.exe"
    CreateShortcut "$SMPROGRAMS\$StartMenuFolder\Aodix Help.lnk" "$INSTDIR\Aodix.chm"
    !insertmacro MUI_STARTMENU_WRITE_END

    SetOutPath $INSTDIR\Skins\Blue
    File ..\Aodix\Skins\Blue\*.bmp
    SetOutPath $INSTDIR\Skins\Night
    File ..\Aodix\Skins\Night\*.bmp

    ; remove old uninstall files
    Delete $INSTDIR\uninst.bmp
    Delete $INSTDIR\uninst.dat
    Delete $INSTDIR\uninst.ini
    Delete $INSTDIR\uninst.lng
    Delete "$SMPROGRAMS\$StartMenuFolder\Uninstall Aodix.lnk"
SectionEnd

Section "un.Uninstall"
    Delete $INSTDIR\Aodix.exe
    Delete $INSTDIR\Aodix.chm
    Delete $INSTDIR\Aodix.cfg
    Delete $INSTDIR\LICENSE.txt
    Delete $INSTDIR\Skins\Blue\*.bmp
    RMDir $INSTDIR\Skins\Blue
    Delete $INSTDIR\Skins\Night\*.bmp
    RMDir $INSTDIR\Skins\Night
    RMDir $INSTDIR\Skins

    SetShellVarContext all
    !insertmacro MUI_STARTMENU_GETFOLDER "PageStartMenu" $StartMenuFolder
    Delete "$SMPROGRAMS\$StartMenuFolder\Aodix.lnk"
    Delete "$SMPROGRAMS\$StartMenuFolder\Aodix Help.lnk"
    RMDir "$SMPROGRAMS\$StartMenuFolder"

    DeleteRegKey HKLM "Software\Arguru Software\Aodix"
    DeleteRegKey HKCR ".adx"
    DeleteRegKey HKCR "Aodix.Document"
    DeleteRegKey HKLM "${REG_UNINST_KEY}"

    Delete $INSTDIR\uninst.exe
    RMDir $INSTDIR
SectionEnd
