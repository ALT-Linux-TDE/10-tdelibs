/*
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

    ---
    file: docwordcompletion.h

    KTextEditor plugin to autocompletion with document words.
    Copyright Anders Lund <anders.lund@lund.tdcadsl.dk>, 2003

    The following completion methods are supported:
    * Completion with bigger matching words in
      either direction (backward/forward).
    * NOT YET Pop up a list of all bigger matching words in document

*/

#ifndef _DocWordCompletionPlugin_h_
#define _DocWordCompletionPlugin_h_

#include <tdetexteditor/plugin.h>
#include <tdetexteditor/view.h>
#include <tdetexteditor/codecompletioninterface.h>
#include <tdetexteditor/configinterfaceextension.h>
#include <kxmlguiclient.h>

#include <tqevent.h>
#include <tqobject.h>
#include <tqvaluelist.h>

class DocWordCompletionPlugin
  : public KTextEditor::Plugin
  , public KTextEditor::PluginViewInterface
  , public KTextEditor::ConfigInterfaceExtension
{
  TQ_OBJECT

  public:
    DocWordCompletionPlugin( TQObject *parent = 0,
                            const char* name = 0,
                            const TQStringList &args = TQStringList() );
    virtual ~DocWordCompletionPlugin() {};

    void addView (KTextEditor::View *view);
    void removeView (KTextEditor::View *view);

    void readConfig();
    void writeConfig();

    // ConfigInterfaceExtention
    uint configPages() const { return 1; };
    KTextEditor::ConfigPage * configPage( uint number, TQWidget *parent, const char *name );
    TQString configPageName( uint ) const;
    TQString configPageFullName( uint ) const;
    TQPixmap configPagePixmap( uint, int ) const;

    uint treshold() const { return m_treshold; };
    void setTreshold( uint t ) { m_treshold = t; };
    bool autoPopupEnabled() const { return m_autopopup; };
    void setAutoPopupEnabled( bool enable ) { m_autopopup = enable; };


  private:
    TQPtrList<class DocWordCompletionPluginView> m_views;
    uint m_treshold;
    bool m_autopopup;

};

class DocWordCompletionPluginView
   : public TQObject, public KXMLGUIClient
{
  TQ_OBJECT

  public:
    DocWordCompletionPluginView( uint treshold=3, bool autopopup=true, KTextEditor::View *view=0,
                               const char *name=0 );
    ~DocWordCompletionPluginView() {};

    void settreshold( uint treshold );

  private slots:
    void completeBackwards();
    void completeForwards();
    void shellComplete();

    void popupCompletionList( TQString word=TQString::null );
    void autoPopupCompletionList();
    void toggleAutoPopup();

    void slotVariableChanged( const TQString &, const TQString & );

  private:
    void complete( bool fw=true );

    TQString word();
    TQValueList<KTextEditor::CompletionEntry> allMatches( const TQString &word );
    TQString findLongestUnique(const TQValueList < KTextEditor::CompletionEntry > &matches);
    KTextEditor::View *m_view;
    struct DocWordCompletionPluginViewPrivate *d;
};

class DocWordCompletionConfigPage : public KTextEditor::ConfigPage
{
  TQ_OBJECT
  public:
    DocWordCompletionConfigPage( DocWordCompletionPlugin *completion, TQWidget *parent, const char *name );
    virtual ~DocWordCompletionConfigPage() {};

    virtual void apply();
    virtual void reset();
    virtual void defaults();

  private:
    DocWordCompletionPlugin *m_completion;
    class TQCheckBox *cbAutoPopup;
    class TQSpinBox *sbAutoPopup;
    class TQLabel *lSbRight;
};

#endif // _DocWordCompletionPlugin_h_
