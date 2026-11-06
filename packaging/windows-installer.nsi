; Invoked by package-windows.sh from its temporary payload directory.
Unicode true
RequestExecutionLevel user
SetCompressor /SOLID lzma

!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "x64.nsh"

!define PRODUCT_NAME "Wing Commander II (WC2-re)"
!define UNINSTALL_KEY "Software\Microsoft\Windows\CurrentVersion\Uninstall\wc2-re"

Name "${PRODUCT_NAME}"
Caption "${PRODUCT_NAME} ${WC2_VERSION} Setup"
OutFile "installer.exe"
InstallDir "$LOCALAPPDATA\Programs\wc2-re"
InstallDirRegKey HKCU "${UNINSTALL_KEY}" "InstallLocation"
BrandingText "WC2-re"
ShowInstDetails hide
ShowUninstDetails hide

VIProductVersion "${WC2_NUMERIC_VERSION}"
VIAddVersionKey /LANG=1033 "ProductName" "${PRODUCT_NAME}"
VIAddVersionKey /LANG=1033 "FileDescription" "Wing Commander II SDL2 port installer"
VIAddVersionKey /LANG=1033 "FileVersion" "${WC2_VERSION}"
VIAddVersionKey /LANG=1033 "LegalCopyright" "WC2-re contributors; see LICENSE.txt"

!define MUI_ABORTWARNING
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!define MUI_FINISHPAGE_RUN "$INSTDIR\wc2-modern.exe"
!define MUI_FINISHPAGE_RUN_TEXT "Open Wing Commander II"
!define MUI_FINISHPAGE_RUN_UNCHECKED
!insertmacro MUI_PAGE_FINISH
!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_LANGUAGE "English"

Var PreviousInstallDir

Function .onInit
    SetShellVarContext current
    ${IfNot} ${RunningX64}
        MessageBox MB_OK|MB_ICONSTOP "Wing Commander II requires 64-bit Windows." /SD IDOK
        SetErrorLevel 1633
        Abort
    ${EndIf}
    ReadRegStr $PreviousInstallDir HKCU "${UNINSTALL_KEY}" "InstallLocation"
FunctionEnd

Function un.onInit
    SetShellVarContext current
FunctionEnd

Section "Wing Commander II"
    ; Use the previous installer's own file list to clean up superseded DLLs.
    ${If} $PreviousInstallDir != ""
    ${AndIf} ${FileExists} "$PreviousInstallDir\Uninstall.exe"
        ClearErrors
        ExecWait '"$PreviousInstallDir\Uninstall.exe" /S _?=$PreviousInstallDir' $0
        ${If} ${Errors}
        ${OrIf} $0 != 0
            MessageBox MB_OK|MB_ICONSTOP "The previous installation could not be removed. Close Wing Commander II and run Setup again." /SD IDOK
            SetErrorLevel 1
            Abort
        ${EndIf}
        Delete "$PreviousInstallDir\Uninstall.exe"
        RMDir "$PreviousInstallDir"
    ${EndIf}

    SetOutPath "$INSTDIR"
    SetOverwrite on
    !include "install-files.nsh"
    WriteUninstaller "$INSTDIR\Uninstall.exe"

    CreateDirectory "$SMPROGRAMS\WC2-re"
    CreateShortcut "$SMPROGRAMS\WC2-re\Wing Commander II.lnk" "$INSTDIR\wc2-modern.exe"

    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayName" "${PRODUCT_NAME}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayVersion" "${WC2_VERSION}"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "Publisher" "WC2-re contributors"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "URLInfoAbout" "https://github.com/neuromancer/wc2-re"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "InstallLocation" "$INSTDIR"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "DisplayIcon" "$INSTDIR\wc2-modern.exe,0"
    WriteRegStr HKCU "${UNINSTALL_KEY}" "UninstallString" '"$INSTDIR\Uninstall.exe"'
    WriteRegStr HKCU "${UNINSTALL_KEY}" "QuietUninstallString" '"$INSTDIR\Uninstall.exe" /S'
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "EstimatedSize" ${WC2_INSTALLED_SIZE}
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoModify" 1
    WriteRegDWORD HKCU "${UNINSTALL_KEY}" "NoRepair" 1
SectionEnd

Section "Uninstall"
    SetOutPath "$TEMP"
    ClearErrors
    !include "uninstall-files.nsh"
    ${If} ${Errors}
        MessageBox MB_OK|MB_ICONSTOP "Some application files could not be removed. Close Wing Commander II and run the uninstaller again." /SD IDOK
        SetErrorLevel 1
        Abort
    ${EndIf}
    Delete "$SMPROGRAMS\WC2-re\Wing Commander II.lnk"
    RMDir "$SMPROGRAMS\WC2-re"
    DeleteRegKey HKCU "${UNINSTALL_KEY}"
    Delete "$INSTDIR\Uninstall.exe"
    ; Remove the directory only if empty, preserving anything added by the user.
    RMDir "$INSTDIR"
    SetErrorLevel 0
SectionEnd
