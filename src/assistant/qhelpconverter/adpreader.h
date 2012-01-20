/****************************************************************************
**
** Copyright (C) 2012 Nokia Corporation and/or its subsidiary(-ies).
** All rights reserved.
** Contact: http://www.qt-project.org/
**
** This file is part of the Qt Assistant of the Qt Toolkit.
**
** $QT_BEGIN_LICENSE:LGPL$
** GNU Lesser General Public License Usage
** This file may be used under the terms of the GNU Lesser General Public
** License version 2.1 as published by the Free Software Foundation and
** appearing in the file LICENSE.LGPL included in the packaging of this
** file. Please review the following information to ensure the GNU Lesser
** General Public License version 2.1 requirements will be met:
** http://www.gnu.org/licenses/old-licenses/lgpl-2.1.html.
**
** In addition, as a special exception, Nokia gives you certain additional
** rights. These rights are described in the Nokia Qt LGPL Exception
** version 1.1, included in the file LGPL_EXCEPTION.txt in this package.
**
** GNU General Public License Usage
** Alternatively, this file may be used under the terms of the GNU General
** Public License version 3.0 as published by the Free Software Foundation
** and appearing in the file LICENSE.GPL included in the packaging of this
** file. Please review the following information to ensure the GNU General
** Public License version 3.0 requirements will be met:
** http://www.gnu.org/copyleft/gpl.html.
**
** Other Usage
** Alternatively, this file may be used in accordance with the terms and
** conditions contained in a signed written agreement between you and Nokia.
**
**
**
**
**
** $QT_END_LICENSE$
**
****************************************************************************/

#ifndef ADPREADER_H
#define ADPREADER_H

#include <QtCore/QMap>
#include <QtCore/QSet>
#include <QtCore/QXmlStreamReader>

QT_BEGIN_NAMESPACE

struct ContentItem {
    ContentItem(const QString &t, const QString &r, int d)
       : title(t), reference(r), depth(d) {}
    QString title;
    QString reference;
    int depth;
};

struct KeywordItem {
    KeywordItem(const QString &k, const QString &r)
       : keyword(k), reference(r) {}
    QString keyword;
    QString reference;
};

class AdpReader : public QXmlStreamReader
{
public:
    void readData(const QByteArray &contents);
    QList<ContentItem> contents() const;
    QList<KeywordItem> keywords() const;
    QSet<QString> files() const;

    QMap<QString, QString> properties() const;

private:
    void readProject();
    void readProfile();
    void readDCF();
    void addFile(const QString &file);

    QMap<QString, QString> m_properties;
    QList<ContentItem> m_contents;
    QList<KeywordItem> m_keywords;
    QSet<QString> m_files;
};

QT_END_NAMESPACE

#endif
