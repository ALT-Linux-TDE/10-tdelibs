/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __kbookmarkimporter_h
#define __kbookmarkimporter_h

#include <tqdom.h>
#include <tqcstring.h>
#include <tqstringlist.h>
#include <ksimpleconfig.h>

#include "kbookmark.h"

/**
 * A class for importing NS bookmarks
 * KEditBookmarks uses it to insert bookmarks into its DOM tree,
 * and TDEActionMenu uses it to create actions directly.
 * @since 3.2
 */
class TDEIO_EXPORT KBookmarkImporterBase : public TQObject
{
    TQ_OBJECT
public:
    KBookmarkImporterBase() {}
    virtual ~KBookmarkImporterBase() {}

    void setFilename(const TQString &filename) { m_fileName = filename; }

    virtual void parse() = 0;
    virtual TQString findDefaultLocation(bool forSaving = false) const = 0;

    // TODO - make this static?
    void setupSignalForwards(TQObject *src, TQObject *dst);
    static KBookmarkImporterBase *factory(const TQString &type);

signals:
    /**
     * Notify about a new bookmark
     * Use "html" for the icon
     */
    void newBookmark(const TQString & text, const TQCString & url, const TQString & additionalInfo);

    /**
     * Notify about a new folder
     * Use "bookmark_folder" for the icon
     */
    void newFolder(const TQString & text, bool open, const TQString & additionalInfo);

    /**
     * Notify about a new separator
     */
    void newSeparator();

    /**
     * Tell the outside world that we're going down
     * one menu
     */
    void endFolder();

protected:
    TQString m_fileName;

private:
    class KBookmarkImporterBasePrivate *d;
};

/**
 * A class for importing XBEL files
 */
class TDEIO_EXPORT KXBELBookmarkImporterImpl : public KBookmarkImporterBase, protected KBookmarkGroupTraverser
{
    TQ_OBJECT
public:
    KXBELBookmarkImporterImpl() {}
    virtual void parse();
    virtual TQString findDefaultLocation(bool = false) const { return TQString::null; }
protected:
    virtual void visit(const KBookmark &);
    virtual void visitEnter(const KBookmarkGroup &);
    virtual void visitLeave(const KBookmarkGroup &);
private:
    class KXBELBookmarkImporterImplPrivate *d;
};

// for SC
#include "kbookmarkimporter_ns.h"
#include "kbookmarkimporter_kde1.h"

#endif
