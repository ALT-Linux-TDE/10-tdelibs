#ifndef KDOCKWIDGETDEMO_H
#define KDOCKWIDGETDEMO_H

#include <kdockwidget.h>

#include <tqdialog.h>
#include <tqlistview.h>
#include <tqstring.h>
#include <tqfile.h>
#include <tqfileinfo.h>
#include <tqtimer.h>
#include <tqscrollview.h>
#include <tqfiledialog.h>
#include <tqwidgetstack.h>
#include <tqvbox.h>
#include <tqurl.h>
#include <tqpixmap.h>

class TQMultiLineEdit;
class TQTextView;
class TQToolButton;
class TQSpinBox;
class TQShowEvent;
class TQPopupMenu;

class DirectoryView;
class CustomFileDialog;
class Preview;
class DirectoryView;

class SFileDialog : public TQDialog
{TQ_OBJECT

public:
	SFileDialog( TQString initially = TQString::null,
                                  const TQStringList& filter = "All Files ( * )", const char* name = 0 );
	~SFileDialog();

  static TQString getOpenFileName( TQString initially = TQString::null,
                                  const TQStringList& filter = "All Files ( * )",
                                  const TQString caption = TQString::null, const char* name = 0 );

  static TQStringList getOpenFileNames( TQString initially = TQString::null,
                                  const TQStringList& filter = "All Files ( * )",
                                  const TQString caption = TQString::null, const char* name = 0 );


protected:
  void showEvent( TQShowEvent *e );

protected slots:
  void dockChange();
  void setDockDefaultPos( KDockWidget* );
  void changeDir( const TQString& );

private:
  DirectoryView* dirView;
  CustomFileDialog* fd;
  Preview* preview;

  KDockManager* dockManager;
  KDockWidget* d_dirView;
  KDockWidget* d_preview;
  KDockWidget* d_fd;

  TQToolButton *b_tree;
  TQToolButton *b_preview;
};
/******************************************************************************************************/
class Directory : public TQListViewItem
{
public:
    Directory( TQListView * parent, const TQString& filename );
    Directory( Directory * parent, const TQString& filename );

    TQString text( int column ) const;

    TQString fullName();

    void setOpen( bool );
    void setup();

private:
    TQFile f;
    Directory * p;
    bool readable;
};

class DirectoryView : public TQListView
{TQ_OBJECT
public:
  DirectoryView( TQWidget *parent = 0, const char *name = 0 );
  virtual void setOpen ( TQListViewItem *, bool );

  TQString selectedDir();

public slots:
  void setDir( const TQString & );

signals:
  void folderSelected( const TQString & );

protected slots:
  void slotFolderSelected( TQListViewItem * );

private:
  TQString fullPath(TQListViewItem* item);
};
/******************************************************************************************************/
class PixmapView : public TQScrollView
{TQ_OBJECT
public:
  PixmapView( TQWidget *parent );
  void setPixmap( const TQPixmap &pix );
  void drawContents( TQPainter *p, int, int, int, int );

private:
  TQPixmap pixmap;
};

class Preview : public TQWidgetStack
{TQ_OBJECT
public:
  Preview( TQWidget *parent );

public slots:
  void showPreview( const TQString& );

private:
  TQMultiLineEdit *normalText;
  TQTextView *html;
  PixmapView *pixmap;
};

class CustomFileDialog : public TQFileDialog
{TQ_OBJECT
public:
  CustomFileDialog( TQWidget* parent );
  ~CustomFileDialog();

  void addToolButton( TQButton * b, bool separator = false ){ TQFileDialog::addToolButton(b,separator); }
  void setBookmark( TQStringList& );
  TQStringList getBookmark(){ return bookmarkList; }

public slots:
  void setDir2( const TQString & );

signals:
  void signalDone( int );

protected slots:
  void bookmarkChosen( int i );
  void goHome();
  virtual void done( int );

private:
  TQPopupMenu *bookmarkMenu;
  TQStringList bookmarkList;
  int addId, clearId;
};

#endif


