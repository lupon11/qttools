/****************************************************************************
**
** Copyright (C) 2013 Digia Plc and/or its subsidiary(-ies).
** Contact: http://www.qt-project.org/legal
**
** This file is part of the tools applications of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** Commercial License Usage
** Licensees holding valid commercial Qt licenses may use this file in
** accordance with the commercial license agreement provided with the
** Software or, alternatively, in accordance with the terms contained in
** a written agreement between you and Digia.  For licensing terms and
** conditions see http://qt.digia.com/licensing.  For further information
** use the contact form at http://qt.digia.com/contact-us.
**
** GNU Lesser General Public License Usage
** Alternatively, this file may be used under the terms of the GNU Lesser
** General Public License version 2.1 as published by the Free Software
** Foundation and appearing in the file LICENSE.LGPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU Lesser General Public License version 2.1 requirements
** will be met: http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Digia gives you certain additional
** rights.  These rights are described in the Digia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU
** General Public License version 3.0 as published by the Free Software
** Foundation and appearing in the file LICENSE.GPL included in the
** packaging of this file.  Please review the following information to
** ensure the GNU General Public License version 3.0 requirements will be
** met: http://www.gnu.org/copyleft/gpl.html.
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef UTILS_H
#define UTILS_H

#include <QStringList>
#include <QMap>
#include <QtCore/QFile>
#include <QtCore/QDir>
#include <QtCore/QDateTime>

enum Platform { Windows, WinRt };

QString findInPath(const QString &file);
QString normalizeFileName(const QString &name);
QString findSdkTool(const QString &tool);
QMap<QString, QString> queryQMakeAll(QString *errorMessage);
QString queryQMake(const QString &variable, QString *errorMessage);
QStringList findDependentLibs(const QString &binary, QString *errorMessage);

enum QtModule {
    GuiModule = 0x1,
    SqlModule = 0x4,
    NetworkModule = 0x8,
    MultimediaModule = 0x10,
    PrintSupportModule = 0x20,
    Quick1Module = 0x40,
    Quick2Module = 0x80,
    ScriptModule = 0x100,
    XmlPatternsModule = 0x200,
    HelpModule = 0x400,
    SensorsModule = 0x800,
    WebKitModule = 0x1000
};

bool updateFile(const QString &sourceFileName, const QStringList &nameFilters,
                const QString &targetDirectory, QString *errorMessage);
bool runProcess(const QString &commandLine, const QString &workingDirectory = QString(),
                unsigned long *exitCode = 0, QByteArray *stdOut = 0, QByteArray *stdErr = 0,
                QString *errorMessage = 0);
QString winErrorMessage(unsigned long error);

bool readPeExecutable(const QString &peExecutableFileName, QString *errorMessage,
                      QStringList *dependentLibraries = 0, unsigned *wordSize = 0,
                      bool *isDebug = 0);
// Return dependent modules of a PE executable files.

inline QStringList findDependentLibraries(const QString &peExecutableFileName, QString *errorMessage)
{
    QStringList result;
    readPeExecutable(peExecutableFileName, errorMessage, &result);
    return result;
}

extern int optVerboseLevel;

// Recursively update a file or directory, matching DirectoryFileEntryFunction against the QDir
// to obtain the files.
template <class DirectoryFileEntryFunction>
bool updateFile(const QString &sourceFileName,
                DirectoryFileEntryFunction directoryFileEntryFunction,
                const QString &targetDirectory,
                QString *errorMessage)
{
    const QFileInfo sourceFileInfo(sourceFileName);
    const QString targetFileName = targetDirectory + QLatin1Char('/') + sourceFileInfo.fileName();
    if (optVerboseLevel > 1)
        std::fprintf(stderr, "Checking %s, %s\n", qPrintable(sourceFileName), qPrintable(targetFileName));

    if (!sourceFileInfo.exists()) {
        *errorMessage = QString::fromLatin1("%1 does not exist.").arg(QDir::toNativeSeparators(sourceFileName));
        return false;
    }

    if (sourceFileInfo.isSymLink()) {
        *errorMessage = QString::fromLatin1("Symbolic links are not supported (%1).")
                        .arg(QDir::toNativeSeparators(sourceFileName));
        return false;
    }

    const QFileInfo targetFileInfo(targetFileName);

    if (sourceFileInfo.isDir()) {
        if (targetFileInfo.exists()) {
            if (!targetFileInfo.isDir()) {
                *errorMessage = QString::fromLatin1("%1 already exists and is not a directory.")
                                .arg(QDir::toNativeSeparators(targetFileName));
                return false;
            } // Not a directory.
        } else { // exists.
            QDir d(targetDirectory);
            std::printf("Creating %s.\n", qPrintable(targetFileName));
            if (!d.mkdir(sourceFileInfo.fileName())) {
                *errorMessage = QString::fromLatin1("Cannot create directory %1 under %2.")
                                .arg(sourceFileInfo.fileName(), QDir::toNativeSeparators(targetDirectory));
                return false;
            }
        }
        // Recurse into directory
        QDir dir(sourceFileName);

        const QStringList allEntries = directoryFileEntryFunction(dir) + dir.entryList(QDir::Dirs | QDir::NoDotAndDotDot);
        foreach (const QString &entry, allEntries)
            if (!updateFile(sourceFileName + QLatin1Char('/') + entry, directoryFileEntryFunction, targetFileName, errorMessage))
                return false;
        return true;
    } // Source is directory.

    if (targetFileInfo.exists()) {
        if (targetFileInfo.lastModified() >= sourceFileInfo.lastModified()) {
            if (optVerboseLevel)
                std::printf("%s is up to date.\n", qPrintable(sourceFileInfo.fileName()));
            return true;
        }
        QFile targetFile(targetFileName);
        if (!targetFile.remove()) {
            *errorMessage = QString::fromLatin1("Cannot remove existing file %1: %2")
                            .arg(QDir::toNativeSeparators(targetFileName), targetFile.errorString());
            return false;
        }
    } // target exists
    QFile file(sourceFileName);
    if (optVerboseLevel)
        std::printf("Updating %s.\n", qPrintable(sourceFileInfo.fileName()));
    if (!file.copy(targetFileName)) {
        *errorMessage = QString::fromLatin1("Cannot copy %1 to %2: %3")
                .arg(QDir::toNativeSeparators(sourceFileName),
                     QDir::toNativeSeparators(targetFileName),
                     file.errorString());
        return false;
    }
    return true;
}

// Base class to filter files by name filters functions to be passed to updateFile().
class NameFilterFileEntryFunction {
public:
    explicit NameFilterFileEntryFunction(const QStringList &nameFilters) : m_nameFilters(nameFilters) {}
    QStringList operator()(const QDir &dir) const { return dir.entryList(m_nameFilters, QDir::Files); }

private:
    const QStringList m_nameFilters;
};

// Convenience for all files.
inline bool updateFile(const QString &sourceFileName, const QString &targetDirectory, QString *errorMessage)
{
    return updateFile(sourceFileName, NameFilterFileEntryFunction(QStringList()), targetDirectory, errorMessage);
}

#endif // UTILS_H
