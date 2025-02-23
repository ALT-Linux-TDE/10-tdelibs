/*
    This library is free software you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.

    ---
    file: autobookmarker.h

    KTextEditor plugin to add bookmarks to documents.
    Copyright Anders Lund <anders.lund@lund.tdcadsl.dk>, 2003
*/

#ifndef _AUTOBOOKMARKER_H_
#define _AUTOBOOKMARKER_H_

#include <tdetexteditor/plugin.h>
#include <tdetexteditor/configinterfaceextension.h>

#include <kdialogbase.h>

#include <tqptrlist.h>
#include <tqvbox.h>

class AutoBookmarkEnt
{
  public:
  enum REFlags { CaseSensitive=1, MinimalMatching=2 };
  AutoBookmarkEnt(const TQString &p=TQString::null,
                  const TQStringList &f=TQStringList(),
                  const TQStringList &m=TQStringList(),
                  int flags=1 );
  ~AutoBookmarkEnt(){};
  TQString pattern;
  TQStringList filemask;
  TQStringList mimemask;
  int flags;
};

class AutoBookmarker
  : public KTextEditor::Plugin, public KTextEditor::PluginViewInterface,
    public KTextEditor::ConfigInterfaceExtension
{
  TQ_OBJECT
  public:
    AutoBookmarker( TQObject *parent = 0,
                        const char* name = 0,
                        const TQStringList &args = TQStringList() );
    virtual ~AutoBookmarker() {};

    void addView (KTextEditor::View *view);
    void removeView (KTextEditor::View *view);

    // ConfigInterfaceExtention
    uint configPages() const { return 1; };
    KTextEditor::ConfigPage * configPage( uint number, TQWidget *parent, const char *name );
    TQString configPageName( uint ) const;
    TQString configPageFullName( uint ) const;
    TQPixmap configPagePixmap( uint, int ) const;
    bool abDone;

  private slots:
    void slotCompleted();
    void applyEntity( AutoBookmarkEnt *e );
};

typedef TQPtrList<AutoBookmarkEnt> ABEntityList;
typedef TQPtrListIterator<AutoBookmarkEnt> ABEntityListIterator;

/* singleton to keep the data */
class ABGlobal
{
  public:
    ABGlobal();
    ~ABGlobal();

    static ABGlobal* self();

    ABEntityList* entities() { return m_ents; };
    void readConfig();
    void writeConfig();

  private:
    ABEntityList *m_ents;
    static ABGlobal *s_self;
};

class AutoBookmarkerConfigPage : public KTextEditor::ConfigPage
{
  TQ_OBJECT
  public:
    AutoBookmarkerConfigPage( TQWidget *parent, const char *name );
    virtual ~AutoBookmarkerConfigPage() {};

    virtual void apply();
    virtual void reset();
    virtual void defaults();

  private slots:
    void slotNew();
    void slotDel();
    void slotEdit();

  private:
    class TDEListView *lvPatterns;
    class TQPushButton *btnNew, *btnDel, *btnEdit;
    ABEntityList *m_ents;
};

class AutoBookmarkerEntEditor : public KDialogBase
{
  TQ_OBJECT
  public:
    AutoBookmarkerEntEditor( TQWidget *parent, AutoBookmarkEnt *e );
    ~AutoBookmarkerEntEditor(){};

    void apply();

  private slots:
  void showMTDlg();
    void slotPatternChanged( const TQString& );
  private:
    class TQLineEdit *lePattern, *leMimeTypes, *leFileMask;
    class TQCheckBox *cbCS, *cbMM;
    AutoBookmarkEnt *e;
};

#endif //_AUTOBOOKMARKER_H_
