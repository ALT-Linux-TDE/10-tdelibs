/* This file is part of the KDE libraries
   Copyright (C) 2002, 2003 Anders Lund <anders.lund@lund.tdcadsl.dk>
   Copyright (C) 2003 Christoph Cullmann <cullmann@kde.org>
   Copyright (C) 2001 Joseph Wenninger <jowenn@kde.org>

   Based on work of:
     Copyright (C) 1999 Jochen Wilhelmy <digisnap@cs.tu-berlin.de>

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

#ifndef __KATE_DIALOGS_H__
#define __KATE_DIALOGS_H__

#include "katehighlight.h"
#include "kateattribute.h"

#include "../interfaces/document.h"

#include <tdelistview.h>
#include <kdialogbase.h>
#include <kmimetype.h>

#include <tqstringlist.h>
#include <tqcolor.h>
#include <tqintdict.h>
#include <tqvbox.h>
#include <tqtabwidget.h>

class KatePartPluginListItem;

struct syntaxContextData;

class KateDocument;
class KateView;

namespace TDEIO
{
  class Job;
  class TransferJob;
}

class TDEAccel;
class KColorButton;
class KComboBox;
class KIntNumInput;
class KKeyButton;
class KKeyChooser;
class TDEMainWindow;
class KPushButton;
class KRegExpDialog;
class KIntNumInput;

class TQButtonGroup;
class TQCheckBox;
class TQHBoxLayout;
class TQLabel;
class TQLineEdit;
class TQPushButton;
class TQRadioButton;
class TQSpinBox;
class TQListBoxItem;
class TQWidgetStack;
class TQVBox;
class TQListViewItem;
class TQCheckBox;

class KateConfigPage : public Kate::ConfigPage
{
  TQ_OBJECT

  public:
    KateConfigPage ( TQWidget *parent=0, const char *name=0 );
    virtual ~KateConfigPage ();

  public:
    bool changed () { return m_changed; }

  private slots:
    void somethingHasChanged ();

  protected:
    bool m_changed;
};

class KateGotoLineDialog : public KDialogBase
{
  TQ_OBJECT

  public:

    KateGotoLineDialog(TQWidget *parent, int line, int max);
    int getLine();

  protected:

    KIntNumInput *e1;
    TQPushButton *btnOK;
};

class KateIndentConfigTab : public KateConfigPage
{
  TQ_OBJECT

  public:
    KateIndentConfigTab(TQWidget *parent);

  protected slots:
    void somethingToggled();
    void indenterSelected (int);

  protected:
    enum { numFlags = 8 };
    static const int flags[numFlags];
    TQCheckBox *opt[numFlags];
    KIntNumInput *indentationWidth;
    TQButtonGroup *m_tabs;
    KComboBox *m_indentMode;
    TQPushButton *m_configPage;

  public slots:
    void configPage();

    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};
};

class KateSelectConfigTab : public KateConfigPage
{
  TQ_OBJECT

  public:
    KateSelectConfigTab(TQWidget *parent);

  protected:
    enum { numFlags = 2 };
    static const int flags[numFlags];
    TQCheckBox *opt[numFlags];

    TQButtonGroup *m_tabs;
    KIntNumInput *e4;
    TQCheckBox *e6;

  public slots:
    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};
};

class KateEditConfigTab : public KateConfigPage
{
    TQ_OBJECT

  public:
    KateEditConfigTab(TQWidget *parent);

  protected:
    enum { numFlags = 5 };
    static const int flags[numFlags];
    TQCheckBox *opt[numFlags];

    KIntNumInput *e1;
    KIntNumInput *e2;
    KIntNumInput *e3;
    KComboBox *e5;
    TQCheckBox *m_wwmarker;

  public slots:
    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};
};

class KateViewDefaultsConfig : public KateConfigPage
{
  TQ_OBJECT

  public:
    KateViewDefaultsConfig( TQWidget *parent );
    ~KateViewDefaultsConfig();

  private:
    TQCheckBox *m_line;
    TQCheckBox *m_folding;
    TQCheckBox *m_collapseTopLevel;
    TQCheckBox *m_icons;
    TQCheckBox *m_scrollBarMarks;
    TQCheckBox *m_dynwrap;
	TQCheckBox *m_showIndentLines;
    KIntNumInput *m_dynwrapAlignLevel;
    TQLabel *m_dynwrapIndicatorsLabel;
    KComboBox *m_dynwrapIndicatorsCombo;
    TQButtonGroup *m_bmSort;

  public slots:
  void apply ();
  void reload ();
  void reset ();
  void defaults ();
};

class KateEditKeyConfiguration: public KateConfigPage
{
  TQ_OBJECT

  public:
    KateEditKeyConfiguration( TQWidget* parent, KateDocument* doc );

  public slots:
    void apply();
    void reload()   {};
    void reset()    {};
    void defaults() {};

  protected:
    void showEvent ( TQShowEvent * );

  private:
    bool m_ready;
    class KateDocument *m_doc;
    KKeyChooser* m_keyChooser;
    class TDEActionCollection *m_ac;
};

class KateSaveConfigTab : public KateConfigPage
{
  TQ_OBJECT
  public:
  KateSaveConfigTab( TQWidget *parent );

  public slots:
  void apply();
  void reload();
  void reset();
  void defaults();

  protected:
  KComboBox *m_encoding, *m_eol;
  TQCheckBox *cbLocalFiles, *cbRemoteFiles;
  TQCheckBox *replaceTabs, *removeSpaces, *allowEolDetection;
  TQLineEdit *leBuPrefix;
  TQLineEdit *leBuSuffix;
  KIntNumInput *dirSearchDepth;
  class TQSpinBox *blockCount;
  class TQLabel *blockCountLabel;
};

class KatePartPluginListItem;

class KatePartPluginListView : public TDEListView
{
  TQ_OBJECT

  friend class KatePartPluginListItem;

  public:
    KatePartPluginListView (TQWidget *parent = 0, const char *name = 0);

  signals:
    void stateChange(KatePartPluginListItem *, bool);

  private:
    void stateChanged(KatePartPluginListItem *, bool);
};

class TQListViewItem;
class KatePartPluginConfigPage : public KateConfigPage
{
  TQ_OBJECT

  public:
    KatePartPluginConfigPage (TQWidget *parent);
    ~KatePartPluginConfigPage ();

  public slots:
    void apply ();
    void reload () {};
    void reset () {};
    void defaults () {};

  private slots:
    void slotCurrentChanged( TQListViewItem * );
    void slotConfigure();
    void slotStateChanged( KatePartPluginListItem *, bool );

  private:
    KatePartPluginListView *listView;
    TQPtrList<KatePartPluginListItem> m_items;
    class TQPushButton *btnConfigure;
};

class KateHlConfigPage : public KateConfigPage
{
  TQ_OBJECT

  public:
    KateHlConfigPage (TQWidget *parent, KateDocument *doc);
    ~KateHlConfigPage ();

  public slots:
    void apply ();
    void reload ();
    void reset () {};
    void defaults () {};

  protected slots:
    void hlChanged(int);
    void hlDownload();
    void showMTDlg();

  private:
    void writeback ();

    TQComboBox *hlCombo;
    TQLineEdit *wildcards;
    TQLineEdit *mimetypes;
    class KIntNumInput *priority;
    class TQLabel *author, *license;

    TQIntDict<KateHlData> hlDataDict;
    KateHlData *hlData;

	KateDocument *m_doc;
};

class KateHlDownloadDialog: public KDialogBase
{
  TQ_OBJECT

  public:
    KateHlDownloadDialog(TQWidget *parent, const char *name, bool modal);
    ~KateHlDownloadDialog();

  private:
    class TQListView  *list;
    class TQString listData;
    TDEIO::TransferJob *transferJob;

  private slots:
    void listDataReceived(TDEIO::Job *, const TQByteArray &data);

  public slots:
    void slotUser1();
};

class KProcIO;
class TDEProcess;
/**
 * This dialog will prompt the user for what do with a file that is
 * modified on disk.
 * If the file wasn't deleted, it has a 'diff' button, which will create
 * a diff file (uing diff(1)) and launch that using KRun.
 */
class KateModOnHdPrompt : public KDialogBase
{
  TQ_OBJECT
  public:
    enum Status {
      Reload=1, // 0 is KDialogBase::Cancel
      Save,
      Overwrite,
      Ignore
    };
    KateModOnHdPrompt( KateDocument *doc, int modtype, const TQString &reason, TQWidget *parent  );
    ~KateModOnHdPrompt();

  public slots:
    /**
     * Show a diff between the document text and the disk file.
     * This will not close the dialog, since we still need a
     * decision from the user.
     */
    void slotDiff();

    void slotOk();
    void slotApply();
    void slotUser1();

  private slots:
    void slotPRead(KProcIO*); ///< Read from the diff process
    void slotPDone(TDEProcess*); ///< Runs the diff file when done

  private:
    KateDocument *m_doc;
    int m_modtype;
    class KTempFile *m_tmpfile; ///< The diff file. Deleted by KRun when the viewer is exited.

};

#endif
