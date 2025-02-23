#ifndef _EDITOR_CHOOSER_H_
#define  _EDITOR_CHOOSER_H_

#include <tdetexteditor/editor.h>
#include <tdetexteditor/document.h>

#include <tqwidget.h>

class TDEConfig;
class TQString;

namespace KTextEditor
{

class KTEXTEDITOR_EXPORT EditorChooser: public TQWidget
{                    
  friend class PrivateEditorChooser;

  TQ_OBJECT

  public:
    EditorChooser(TQWidget *parent=0,const char *name=0);
    virtual ~EditorChooser();
    
   /* void writeSysDefault();*/

    void readAppSetting(const TQString& postfix=TQString::null);
    void writeAppSetting(const TQString& postfix=TQString::null);

    static KTextEditor::Document *createDocument(TQObject* parent=0,const char *name=0,const TQString& postfix=TQString::null, bool fallBackToKatePart=true);
    static KTextEditor::Editor *createEditor(TQWidget *parentWidget,TQObject *parent,const char* widgetName=0,const char* name=0,const TQString& postfix=TQString::null,bool fallBackToKatePart=true);
  private:
    class PrivateEditorChooser *d;
};

/*
class EditorChooserBackEnd: public ComponentChooserPlugin {

TQ_OBJECT
public:
	EditorChooserBackEnd(TQObject *parent=0, const char *name=0);
	virtual ~EditorChooserBackEnd();

	virtual TQWidget *widget(TQWidget *);
	virtual const TQStringList &choices();
	virtual void saveSettings();

	void readAppSetting(TDEConfig *cfg,const TQString& postfix);
	void writeAppSetting(TDEConfig *cfg,const TQString& postfix);

public slots:
	virtual void madeChoice(int pos,const TQString &choice);

};
*/

}
#endif
