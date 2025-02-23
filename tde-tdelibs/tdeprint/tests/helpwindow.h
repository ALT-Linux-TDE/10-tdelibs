/****************************************************************************
** $Id$
**
** Copyright (C) 1992-2000 Trolltech AS.  All rights reserved.
**
** This file is part of an example program for Qt.  This example
** program may be used, distributed and modified without limitation.
**
*****************************************************************************/

#ifndef HELPWINDOW_H
#define HELPWINDOW_H

#include <tdemainwindow.h>
#include <tqtextbrowser.h>
#include <tqstringlist.h>
#include <tqmap.h>
#include <tqdir.h>

class TQComboBox;
class TQPopupMenu;

class HelpWindow : public TDEMainWindow
{
    TQ_OBJECT
public:
    HelpWindow( const TQString& home_,  const TQString& path, TQWidget* parent = 0, const char *name=0 );
    ~HelpWindow();

private slots:
    void setBackwardAvailable( bool );
    void setForwardAvailable( bool );

    void textChanged();
    void about();
    void aboutQt();
    void openFile();
    void newWindow();
    void print();

    void pathSelected( const TQString & );
    void histChosen( int );
    void bookmChosen( int );
    void addBookmark();
    
private:
    void readHistory();
    void readBookmarks();
    
    TQTextBrowser* browser;
    TQComboBox *pathCombo;
    int backwardId, forwardId;
    TQString selectedURL;
    TQStringList history, bookmarks;
    TQMap<int, TQString> mHistory, mBookmarks;
    TQPopupMenu *hist, *bookm;

};





#endif

