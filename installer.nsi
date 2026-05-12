; 二十四节气壁纸安装脚本
; NSIS 安装脚本

;--------------------------------
; 包含头文件
!include "MUI2.nsh"
!include "LogicLib.nsh"
!include "FileFunc.nsh"
!include "WinVer.nsh"

;--------------------------------
; 产品信息
!define PRODUCT_NAME "二十四节气壁纸"
!define PRODUCT_SHORT_NAME "SolarTermsWallpaper" ; 不要更改这一项
!define PRODUCT_VERSION "v0.2.0"
!define PRODUCT_PUBLISHER "AIR-Kevin"
!define PRODUCT_WEB_SITE "https://github.com/pbw-Kevin/${PRODUCT_SHORT_NAME}"

;--------------------------------
; 常规属性
Name "${PRODUCT_NAME} ${PRODUCT_VERSION}"
OutFile "${PRODUCT_SHORT_NAME}_Setup_${PRODUCT_VERSION}.exe"
Icon "favicon.ico"
UninstallIcon "favicon.ico"
InstallDir "$PROGRAMFILES32\${PRODUCT_SHORT_NAME}"
RequestExecutionLevel admin
SetCompressor /SOLID lzma

;--------------------------------
; MUI 设置
!define MUI_ABORTWARNING
!define MUI_ICON "favicon.ico"
!define MUI_UNICON "favicon.ico"
!define MUI_WELCOMEPAGE_TITLE "${PRODUCT_NAME} 安装向导"
!define MUI_WELCOMEPAGE_TEXT "本向导将引导您完成 ${PRODUCT_NAME} ${PRODUCT_VERSION} 的安装。$\r$\n$\r$\n二十四节气壁纸 - 让您的桌面跟随节气变换。"

; 完成页面选项
!define MUI_FINISHPAGE_NOAUTOCLOSE
!define MUI_FINISHPAGE_RUN ""                      ; 不使用默认运行方式，改用自定义函数
!define MUI_FINISHPAGE_RUN_FUNCTION "RunAppNow"   ; 自定义运行函数
!define MUI_FINISHPAGE_RUN_TEXT "立即启动 ${PRODUCT_NAME}"
!define MUI_FINISHPAGE_SHOWREADME "$INSTDIR\${PRODUCT_SHORT_NAME}.exe"
!define MUI_FINISHPAGE_SHOWREADME_TEXT "设置开机自启动"
!define MUI_FINISHPAGE_SHOWREADME_FUNCTION "SetAutoStart"

!define MUI_UNFINISHPAGE_NOAUTOCLOSE

; 页面定义
!insertmacro MUI_PAGE_WELCOME
!insertmacro MUI_PAGE_LICENSE "NSISLicense.txt"
!insertmacro MUI_PAGE_DIRECTORY
!insertmacro MUI_PAGE_INSTFILES
!insertmacro MUI_PAGE_FINISH

!insertmacro MUI_UNPAGE_CONFIRM
!insertmacro MUI_UNPAGE_INSTFILES
!insertmacro MUI_UNPAGE_FINISH

; 语言
!insertmacro MUI_LANGUAGE "SimpChinese"
!insertmacro MUI_LANGUAGE "English"

;--------------------------------
; 函数：立即启动程序
Function RunAppNow
    Exec '"$INSTDIR\${PRODUCT_SHORT_NAME}.exe"'
    ${If} ${Errors}
        MessageBox MB_OK|MB_ICONEXCLAMATION "无法自动启动程序，请手动运行：$\n$INSTDIR\${PRODUCT_SHORT_NAME}.exe"
    ${EndIf}
FunctionEnd

; 函数：设置开机自启动
Function SetAutoStart
    WriteRegStr HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${PRODUCT_SHORT_NAME}" '"$INSTDIR\${PRODUCT_SHORT_NAME}.exe"'
    MessageBox MB_OK|MB_ICONINFORMATION "已添加开机自启动项。"
FunctionEnd

;--------------------------------
; 检测进程是否运行（卸载版）
Function un.IsAppRunning
    nsExec::ExecToStack 'tasklist /NH /FI "IMAGENAME eq ${PRODUCT_SHORT_NAME}.exe"'
    Pop $0
    Pop $1
    Push $1
    Call un.StrContainsSolarTerm
    Pop $2
    ${If} $2 == "true"
        Push 1
    ${Else}
        Push 0
    ${EndIf}
FunctionEnd

Function un.StrContainsSolarTerm
    Exch $0
    Push $1
    Push $2
    StrCpy $1 "${PRODUCT_SHORT_NAME}.exe"
    StrCpy $2 0
loop:
    StrLen $3 $1
    StrCpy $4 $0 $3 $2
    StrCmp $4 "" done
    StrCmp $4 $1 found
    IntOp $2 $2 + 1
    Goto loop
found:
    StrCpy $0 "true"
    Goto done2
done:
    StrCpy $0 "false"
done2:
    Pop $2
    Pop $1
    Exch $0
FunctionEnd

;--------------------------------
; 安装部分
Section "Install"
    SetOverwrite ifnewer
    SetOutPath "$INSTDIR"

    CreateDirectory "$APPDATA\${PRODUCT_SHORT_NAME}"

    WriteUninstaller "$INSTDIR\uninstall.exe"

    File "build\Release\${PRODUCT_SHORT_NAME}.exe"
    File "kill.bat"

    SetOutPath "$INSTDIR\wallpapers"
    File /r "wallpapers\*.*"

    SetOutPath "$INSTDIR\assets"
    File /r "assets\*.*"

    SetOutPath "$INSTDIR"

    CreateDirectory "$SMPROGRAMS\${PRODUCT_SHORT_NAME}"
    CreateShortCut "$SMPROGRAMS\${PRODUCT_SHORT_NAME}\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_SHORT_NAME}.exe" "" "" "" SW_SHOWMINIMIZED
    CreateShortCut "$SMPROGRAMS\${PRODUCT_SHORT_NAME}\卸载 ${PRODUCT_NAME}.lnk" "$INSTDIR\uninstall.exe" 

    CreateShortCut "$DESKTOP\${PRODUCT_NAME}.lnk" "$INSTDIR\${PRODUCT_SHORT_NAME}.exe" "" "" "" SW_SHOWMINIMIZED

    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "DisplayName" "${PRODUCT_NAME} ${PRODUCT_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "UninstallString" "$INSTDIR\uninstall.exe"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "QuietUninstallString" "$INSTDIR\uninstall.exe /S"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "DisplayVersion" "${PRODUCT_VERSION}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "Publisher" "${PRODUCT_PUBLISHER}"
    WriteRegStr HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "URLInfoAbout" "${PRODUCT_WEB_SITE}"
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "NoModify" 1
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "NoRepair" 1

    ${GetSize} "$INSTDIR" "/S=0K" $0 $1 $2
    IntFmt $0 "0x%08X" $0
    WriteRegDWORD HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}" \
                 "EstimatedSize" "$0"
SectionEnd

;--------------------------------
; 卸载区块
Section "Uninstall"
    Call un.IsAppRunning
    Pop $0
    ${If} $0 == 1
        MessageBox MB_YESNO|MB_ICONQUESTION "程序正在运行。$\r$\n是否关闭程序并继续卸载？" IDYES close_app IDNO abort_uninstall
        close_app:
            nsExec::ExecToStack 'taskkill /IM ${PRODUCT_SHORT_NAME}.exe /F'
            Pop $1
            Sleep 1000
            Goto continue_uninstall
        abort_uninstall:
            Abort "请先关闭程序后再运行卸载程序。"
        continue_uninstall:
    ${EndIf}

    DeleteRegValue HKCU "Software\Microsoft\Windows\CurrentVersion\Run" "${PRODUCT_SHORT_NAME}"
    DeleteRegKey HKLM "Software\Microsoft\Windows\CurrentVersion\Uninstall\${PRODUCT_SHORT_NAME}"
    RMDir /r "$APPDATA\${PRODUCT_SHORT_NAME}"
    RMDir /r "$SMPROGRAMS\${PRODUCT_SHORT_NAME}"
    Delete "$DESKTOP\${PRODUCT_NAME}.lnk"
    RMDir /r "$INSTDIR"
    RMDir "$INSTDIR"
SectionEnd

;--------------------------------
Function un.onInit
    MessageBox MB_YESNO|MB_ICONQUESTION|MB_DEFBUTTON2 "您确定要完全移除 ${PRODUCT_NAME} 及其所有组件吗？" IDYES +2
    Abort
FunctionEnd

Function .onInit
    ; 检测 Windows 版本，低于 Windows 10 则退出安装
    ${IfNot} ${AtLeastWin10}
        MessageBox MB_OK|MB_ICONSTOP "抱歉，${PRODUCT_NAME} 需要 Windows 10 或更高版本才能运行。$\n您的系统版本过低，安装即将中止。"
        Abort
    ${EndIf}

    UserInfo::GetAccountType
    Pop $0
    ${If} $0 != "admin"
        MessageBox MB_OK|MB_ICONEXCLAMATION "您没有管理员权限。安装到 Program Files 可能需要管理员权限，推荐以管理员身份运行安装程序。"
    ${EndIf}
FunctionEnd
