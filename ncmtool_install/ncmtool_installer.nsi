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

  ; Copy all files from installer directory
  File /r "..\out\build-release\installer\*.*"

  ; Create Start Menu and Desktop shortcuts
  CreateDirectory "$SMPROGRAMS\${APPNAME}"
  CreateShortcut "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk" "$INSTDIR\ncmtool.exe"
  CreateShortcut "$DESKTOP\${APPNAME}.lnk" "$INSTDIR\ncmtool.exe"

  ; Associate .ncm files with this application
  WriteRegStr HKCR "SystemFileAssociations\.ncm\shell\${APPNAME}" "" "Open with ${APPNAME}"
  WriteRegStr HKCR "SystemFileAssociations\.ncm\shell\${APPNAME}\command" "" '"$INSTDIR\ncmtool.exe" "%1"'

  ; Save uninstaller
  WriteUninstaller "$INSTDIR\Uninstall.exe"
SectionEnd

Section "Uninstall"
  ; Remove created files
  Delete "$INSTDIR\Uninstall.exe"
  Delete "$INSTDIR\ncmtool.exe"
  Delete "$DESKTOP\${APPNAME}.lnk"
  Delete "$SMPROGRAMS\${APPNAME}\${APPNAME}.lnk"

  ; Remove directory
  RMDir /r "$INSTDIR"
  RMDir "$SMPROGRAMS\${APPNAME}"

  ; Remove .ncm file association
  DeleteRegKey HKCR "SystemFileAssociations\.ncm\shell\${APPNAME}"

SectionEnd