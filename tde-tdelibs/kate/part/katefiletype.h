/* This file is part of the KDE libraries
   Copyright (C) 2001-2003 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __kate_filetype_h__
#define __kate_filetype_h__

#include <tqstringlist.h>
#include <tqptrlist.h>
#include <tqpopupmenu.h> // for TQPtrList<TQPopupMenu>, compile with gcc 3.4
#include <tqguardedptr.h>

#include "katedialogs.h"

class KateDocument;

class KateFileType
{
  public:
    int number;
    TQString name;
    TQString section;
    TQStringList wildcards;
    TQStringList mimetypes;
    int priority;
    TQString varLine;
};

class KateFileTypeManager
{
  public:
    KateFileTypeManager ();
    ~KateFileTypeManager ();

    /**
     * File Type Config changed, update all docs (which will take care of views/renderers)
     */
    void update ();

    void save (TQPtrList<KateFileType> *v);

    /**
     * get the right fileType for the given document
     * -1 if none found !
     */
    int fileType (KateDocument *doc);

    /**
     * Don't store the pointer somewhere longer times, won't be valid after the next update()
     */
    const KateFileType *fileType (uint number);

    /**
     * Don't modify
     */
    TQPtrList<KateFileType> *list () { return &m_types; }

  private:
    int wildcardsFind (const TQString &fileName);

  private:
    TQPtrList<KateFileType> m_types;
};

class KateFileTypeConfigTab : public KateConfigPage
{
  TQ_OBJECT

  public:
    KateFileTypeConfigTab( TQWidget *parent );

  public slots:
    void apply();
    void reload();
    void reset();
    void defaults();

  private slots:
    void update ();
    void deleteType ();
    void newType ();
    void typeChanged (int type);
    void showMTDlg();
    void save ();

  private:
    class TQGroupBox *gbProps;
    class TQPushButton *btndel;
    class TQComboBox *typeCombo;
    class TQLineEdit *wildcards;
    class TQLineEdit *mimetypes;
    class KIntNumInput *priority;
    class TQLineEdit *name;
    class TQLineEdit *section;
    class TQLineEdit *varLine;

    TQPtrList<KateFileType> m_types;
    KateFileType *m_lastType;
};

class KateViewFileTypeAction : public Kate::ActionMenu
{
  TQ_OBJECT

  public:
    KateViewFileTypeAction(const TQString& text, TQObject* parent = 0, const char* name = 0)
       : Kate::ActionMenu(text, parent, name) { init(); };

    ~KateViewFileTypeAction(){;};

    void updateMenu (Kate::Document *doc);

  private:
    void init();

    TQGuardedPtr<KateDocument> m_doc;
    TQStringList subMenusName;
    TQStringList names;
    TQPtrList<TQPopupMenu> subMenus;

  public  slots:
    void slotAboutToShow();

  private slots:
    void setType (int mode);
};

#endif
