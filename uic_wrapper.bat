@echo off
SetLocal EnableDelayedExpansion
(set PATH=C:\Qt\Qt5.15.3\qt-everywhere-src-5.15.3\qtbase\bin;!PATH!)
if defined QT_PLUGIN_PATH (
    set QT_PLUGIN_PATH=C:\Qt\Qt5.15.3\qt-everywhere-src-5.15.3\qtbase\plugins;!QT_PLUGIN_PATH!
) else (
    set QT_PLUGIN_PATH=C:\Qt\Qt5.15.3\qt-everywhere-src-5.15.3\qtbase\plugins
)
C:\Qt\Qt5.15.3\qt-everywhere-src-5.15.3\qtbase\bin\uic.exe %*
EndLocal
