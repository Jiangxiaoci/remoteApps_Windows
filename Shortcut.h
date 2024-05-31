#ifndef SHORTCUT_H
#define SHORTCUT_H

#include <QString>
#include <windows.h>
#include <shlobj.h>
#include <objbase.h>

bool createWindowsShortcut(const QString &filePath, const QString &shortcutPath);

#endif // SHORTCUT_H
