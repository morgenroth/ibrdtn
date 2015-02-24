; ibrdtn-inst.nsi
;
;--------------------------------
;Include Modern UI

  !include "MUI2.nsh"
  !include "TextFunc.nsh"
  !include "LogicLib.nsh"
  !include "StrFunc.nsh"
  
!include "EnvVarUpdate.nsh"

;--------------------------------
;General

!define APP_NAME "IBR-DTN"
!define VERSION "1.0.1"
!define DLL_VERSION "1-0-1"
!define MUI_ICON ibrdtn.ico
!define MUI_UNICON ibrdtn.ico
!define SERVICE_NAME "${APP_NAME}"
!define WEB_SITE "http://www.ibr.cs.tu-bs.de/projects/ibr-dtn"

; The name of the installer
Name "${APP_NAME}"

; The default installation directory
InstallDir $PROGRAMFILES\${APP_NAME}

; The file to write
OutFile "ibrdtn-installer-${VERSION}.exe"

; Registry key to check for directory (so if you install again, it will
; overwrite the old one automatically)
InstallDirRegKey HKLM "SOFTWARE\${APP_NAME}" "Install_Dir"

; Request application privileges for Windows Vista
RequestExecutionLevel admin

;--------------------------------
;Variables

Var StartMenuFolder
Var DtnDataDir
Var ret

;--------------------------------
;Interface Settings

!define MUI_ABORTWARNING

;--------------------------------
; Pages

  !insertmacro MUI_PAGE_COMPONENTS
  !insertmacro MUI_PAGE_DIRECTORY
  
  ;Start Menu Folder Page Configuration
  !define MUI_STARTMENUPAGE_REGISTRY_ROOT "HKCU"
  !define MUI_STARTMENUPAGE_REGISTRY_KEY "Software\${APP_NAME}"
  !define MUI_STARTMENUPAGE_REGISTRY_VALUENAME "Start Menu Folder"
  
  !insertmacro MUI_PAGE_STARTMENU Application $StartMenuFolder
  
  ;second directory selection
  !define MUI_PAGE_HEADER_SUBTEXT "Choose the folder to use for persistent data."
  !define MUI_DIRECTORYPAGE_TEXT_TOP "The daemon will use this directory to store the configuration, logs and bundles. To install in a differenct folder, click Browse and select another folder. Click Next to continue."
  !define MUI_DIRECTORYPAGE_VARIABLE $DtnDataDir ; <= the other directory will be stored into that variable
  !insertmacro MUI_PAGE_DIRECTORY
  
  !insertmacro MUI_PAGE_INSTFILES
  !insertmacro MUI_PAGE_FINISH
  
  !insertmacro MUI_UNPAGE_CONFIRM
  !insertmacro MUI_UNPAGE_INSTFILES

;--------------------------------
; Languages
 
  !insertmacro MUI_LANGUAGE "English"

;--------------------------------
; The stuff to install
Section "${APP_NAME} Daemon (required)" SecDaemon

  SectionIn RO
  SetOverwrite ifnewer
  SetOutPath "$INSTDIR"
  
  ; daemon plus libraries
  File dtnd.exe
  File "libgcc_s_dw2-1.dll"
  File "libibrcommon-${DLL_VERSION}.dll"
  File "libibrdtn-${DLL_VERSION}.dll"
  File "libstdc++-6.dll"
  File "pthreadGC2.dll"
  File "ibrdtnd.conf.dist"
  File "ibrdtn.ico"
  File "show_interfaces.bat"
  
  ; Add website
  WriteIniStr "$INSTDIR\${APP_NAME} website.url" "InternetShortcut" "URL" "${WEB_SITE}"
  
  ; Write the installation path into the registry
  WriteRegStr HKLM SOFTWARE\${APP_NAME} "Install_Dir" "$INSTDIR"
  
  ; Write the uninstall keys for Windows
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayName" "${APP_NAME}"
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "UninstallString" '"$INSTDIR\uninstall.exe"'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "Publisher" 'IBR, TU Braunschweig'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayVersion" '${VERSION}'
  WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "DisplayIcon" '"$INSTDIR\ibrdtn.ico"'
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoModify" 1
  WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" "NoRepair" 1
  WriteUninstaller "uninstall.exe"
  
  ; Add install path to $PATH
  ${EnvVarUpdate} $0 "PATH" "A" "HKLM" "$INSTDIR"
  
  ; Create data dir
  CreateDirectory "$DtnDataDir"
  
  ; Delete old file
  Delete "$DtnDataDir\ibrdtnd.conf"
  
  ; create config file
  FileOpen  $0 "$DtnDataDir\ibrdtnd.conf" w
  FileClose $0
  
  ; Write config file
  ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "routing = " "prophet" $ret
  ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "storage_path = " "$DtnDataDir\bundles" $ret
  ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "blob_path = " "$DtnDataDir\tmp" $ret

  ; Get interfaces
  nsExec::ExecToStack '$INSTDIR\dtnd.exe --interfaces'
  Pop $0
  Pop $1

  ; Walk through all interfaces
  ; $R3 list of all interfaces
  ; $2 is the ID of the interface
  ; $3 is the name of the interface
  ; $R2 is the interface index
  ;
  StrCpy $R3 " "
  StrCpy $R1 1
  ${StrTok} $2 $1 "$\r$\n$\t" "$R1" "1"

  ${While} $2 != ""
    ; get the interface name
    IntOp $R1 $R1 + 1
    ${StrTok} $3 $1 "$\r$\n$\t" "$R1" "1"

    ; calculate index
    IntOp $R2 $R1 / 2

    ; write interface to the config file
    ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "# interface $R2: " "$3" $ret
    ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "net_if$R2_type = " "tcp" $ret
    ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "net_if$R2_port = " "4556" $ret
    ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "net_if$R2_interface = " "$2" $ret

    ; add to interface list
    StrCpy $R3 "$R3 if$R2"

    ; select the next item
    IntOp $R1 $R1 + 1
    ${StrTok} $2 $1 "$\r$\n$\t" "$R1" "1"
  ${EndWhile}

  ; set active interfaces
  ${ConfigWrite} "$DtnDataDir\ibrdtnd.conf" "net_interfaces = " "$R3" $ret

SectionEnd

; Windows NT service
Section "Windows service" SecService

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  
  ; install DTN service
  File "dtnserv.exe"
  
  ; register the service
  ;SimpleSC::InstallService "${SERVICE_NAME}" "16" "2" "$INSTDIR\dtnserv.exe" "" "" ""
  nsExec::ExecToLog 'sc create "${SERVICE_NAME}" binPath= "$INSTDIR\dtnserv.exe" start= auto depend= Tcpip'
  SimpleSC::SetServiceDescription "${SERVICE_NAME}" "Delay tolerant networking stack"
  
  ; Write daemon configuration
  WriteRegStr HKLM "Software\${APP_NAME}" "logfile" "$DtnDataDir\ibrdtn.log"
  WriteRegStr HKLM "Software\${APP_NAME}" "configfile" "$DtnDataDir\ibrdtnd.conf"
  WriteRegDWORD HKLM "Software\${APP_NAME}" "debugLevel" 0
  WriteRegDWORD HKLM "Software\${APP_NAME}" "logLevel" 2
  
  ; start the service
  SimpleSC::StartService "${SERVICE_NAME}"
  
SectionEnd

; Tools
Section "Tools" SecTools

  SetOutPath "$INSTDIR"
  SetOverwrite ifnewer
  
  ; install tools
  File "dtnconvert.exe"
  File "dtnping.exe"
  File "dtnrecv.exe"
  File "dtnsend.exe"
  File "dtnstream.exe"
  File "dtntracepath.exe"
  File "dtntrigger.exe"
  
SectionEnd

; Optional section (can be disabled by the user)
Section "Start Menu Shortcuts"

  !insertmacro MUI_STARTMENU_WRITE_BEGIN Application

    ;Create shortcuts
    CreateDirectory "$SMPROGRAMS\$StartMenuFolder"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\DTN Console.lnk" "$SYSDIR\cmd.exe"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Show Interfaces.lnk" "$INSTDIR\show_interfaces.bat"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Website.lnk" "$INSTDIR\${APP_NAME} website.url"
    CreateShortCut "$SMPROGRAMS\$StartMenuFolder\Uninstall.lnk" "$INSTDIR\Uninstall.exe"
    
  !insertmacro MUI_STARTMENU_WRITE_END
  
SectionEnd

;--------------------------------
;Descriptions

  ;Language strings
  LangString DESC_SecDaemon ${LANG_ENGLISH} "DTN daemon and library files"

  ;Assign language strings to sections
  !insertmacro MUI_FUNCTION_DESCRIPTION_BEGIN
    !insertmacro MUI_DESCRIPTION_TEXT ${SecDaemon} $(DESC_SecDaemon)
  !insertmacro MUI_FUNCTION_DESCRIPTION_END

;--------------------------------

; Uninstaller

Section "Uninstall"

  ; stop and remove DTN service
  SimpleSC::StopService "${SERVICE_NAME}" 1 60
  SimpleSC::RemoveService "${SERVICE_NAME}"
  
  ; Delete all installed files
  Delete /REBOOTOK "$INSTDIR\*.*"
  RMDir /REBOOTOK "$INSTDIR"
  
  ; Remove Path entry
  ${un.EnvVarUpdate} $0 "PATH" "R" "HKLM" "$INSTDIR"

  ; Get start menu folder variable
  !insertmacro MUI_STARTMENU_GETFOLDER Application $StartMenuFolder
  
  ; Remove start menu entries
  Delete "$SMPROGRAMS\$StartMenuFolder\*.*"
  RMDir "$SMPROGRAMS\$StartMenuFolder"
  
  ; Remove registry keys
  DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}"
  DeleteRegKey /ifempty HKLM "Software\${APP_NAME}"

SectionEnd

Function UninstallPrevious

  ; Check previous installation
  ReadRegStr $R0 HKLM \
    "Software\Microsoft\Windows\CurrentVersion\Uninstall\${APP_NAME}" \
    "UninstallString"
  StrCmp $R0 "" done
 
  MessageBox MB_OKCANCEL|MB_ICONEXCLAMATION \
    "${APP_NAME} is already installed. $\n$\nClick `OK` to remove the \
    previous version or `Cancel` to cancel this upgrade." \
    IDOK uninst
    Abort
 
; Run the uninstaller
uninst:
  ClearErrors
  ExecWait '$R0' ;Do not copy the uninstaller to a temp file
 
  IfErrors no_remove_uninstaller done
    ; You can either use Delete /REBOOTOK in the uninstaller or add some code
    ; here to remove the uninstaller. Use a registry key to check
    ; whether the user has chosen to uninstall. If you are using an uninstaller
    ; components page, make sure all sections are uninstalled.
  no_remove_uninstaller:
 
done:

FunctionEnd

Function .onInit
  ; uninstall previous installation
  Call UninstallPrevious

  ; define default data directory
  StrCpy $DtnDataDir "C:\dtndata"

FunctionEnd
