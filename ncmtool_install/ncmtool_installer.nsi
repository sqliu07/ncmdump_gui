!define APPNAME "ncmtool"
!define VERSION "1.0.0"
!define INSTALLDIR "$PROGRAMFILES64\${APPNAME}"

Name "${APPNAME} ${VERSION}"
OutFile "ncmtool_setup.exe"
InstallDir "${INSTALLDIR}"
RequestExecutionLevel admin

Page directory
Page instfiles
UninstPage uninstConfirm
UninstPage instfiles

Section "Install"
SetOutPath "$INSTDIR"
File /r "../installer\*.*"


  CreateShortcut "$SMPROGRAMS\${APPNAME}.lnk" "$INSTDIR\ncmtool.exe"
  CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\ncmtool.exe"

  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$SMPROGRAMS\${APPNAME}.lnk"
  Delete "$DESKTOP\${APPNAME}.lnk"
  RMDir /r "$INSTDIR"
SectionEnd
