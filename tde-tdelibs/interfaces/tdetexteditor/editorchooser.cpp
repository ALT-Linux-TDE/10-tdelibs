#include <editorchooser.h>
#include <editorchooser.moc>

#include <tqcombobox.h>
#include <ktrader.h>
#include <tdeconfig.h>
#include <tqstringlist.h>
#include <kservice.h>
#include <tdelocale.h>
#include <tqlabel.h>
#include <tdeapplication.h>
#include <tqlayout.h>

#include "editorchooser_ui.h"

using namespace KTextEditor;

namespace KTextEditor
{
  class PrivateEditorChooser
  {
  public:
    PrivateEditorChooser()
    {
    }
    ~PrivateEditorChooser(){}
  // Data Members
  EditorChooser_UI *chooser;
  TQStringList ElementNames;
  TQStringList elements;
  };

}

EditorChooser::EditorChooser(TQWidget *parent,const char *name) :
	TQWidget (parent,name)
  {
  d = new PrivateEditorChooser ();

  // sizemanagment
  TQGridLayout *grid = new TQGridLayout( this, 1, 1 );


  d->chooser = new EditorChooser_UI (this, name);

  grid->addWidget( d->chooser, 0, 0);


	TDETrader::OfferList offers = TDETrader::self()->query("text/plain", "'KTextEditor/Document' in ServiceTypes");
	TDEConfig *config=new TDEConfig("default_components");
  	config->setGroup("KTextEditor");
  	TQString editor = config->readPathEntry("embeddedEditor");

        if (editor.isEmpty()) editor="katepart";

	for (TDETrader::OfferList::Iterator it = offers.begin(); it != offers.end(); ++it)
	{
    		if ((*it)->desktopEntryName().contains(editor))
		{
			d->chooser->editorCombo->insertItem(TQString(i18n("System Default (%1)").arg((*it)->name())));
			break;
		}
  	}

  	for (TDETrader::OfferList::Iterator it = offers.begin(); it != offers.end(); ++it)
  	{
    		d->chooser->editorCombo->insertItem((*it)->name());
		d->elements.append((*it)->desktopEntryName());
  	}
    	d->chooser->editorCombo->setCurrentItem(0);
}

EditorChooser:: ~EditorChooser(){
  delete d;
}

void EditorChooser::readAppSetting(const TQString& postfix){
	TDEConfig *cfg=kapp->config();
	TQString previousGroup=cfg->group();
	cfg->setGroup("KTEXTEDITOR:"+postfix);
	TQString editor=cfg->readPathEntry("editor");
	if (editor.isEmpty()) d->chooser->editorCombo->setCurrentItem(0);
	else
	{
		int idx=d->elements.findIndex(editor);
		idx=idx+1;
		d->chooser->editorCombo->setCurrentItem(idx);
	}
	cfg->setGroup(previousGroup);
}

void EditorChooser::writeAppSetting(const TQString& postfix){
	TDEConfig *cfg=kapp->config();
	TQString previousGroup=cfg->group();
	cfg->setGroup("KTEXTEDITOR:"+postfix);
	cfg->writeEntry("DEVELOPER_INFO","NEVER TRY TO USE VALUES FROM THAT GROUP, THEY ARE SUBJECT TO CHANGES");
	cfg->writePathEntry("editor", (d->chooser->editorCombo->currentItem()==0) ? 
		TQString::null : (*d->elements.at(d->chooser->editorCombo->currentItem()-1)));
	cfg->sync();
	cfg->setGroup(previousGroup);

}

KTextEditor::Document *EditorChooser::createDocument(TQObject *parent,const char* name, const TQString& postfix,bool fallBackToKatePart){

	KTextEditor::Document *tmpDoc=0;

	TDEConfig *cfg=kapp->config();
        TQString previousGroup=cfg->group();
        cfg->setGroup("KTEXTEDITOR:"+postfix);
        TQString editor=cfg->readPathEntry("editor");
	cfg->setGroup(previousGroup);
	if (editor.isEmpty())
	{
		TDEConfig *config=new TDEConfig("default_components");
  		config->setGroup("KTextEditor");
	  	editor = config->readPathEntry("embeddedEditor", "katepart");
		delete config;
	}

	KService::Ptr serv=KService::serviceByDesktopName(editor);
	if (serv)
	{
		tmpDoc=KTextEditor::createDocument(serv->library().latin1(),parent,name);
		if (tmpDoc) return tmpDoc;
	}
	if (fallBackToKatePart)
		return KTextEditor::createDocument("libkatepart",parent,name);

	return 0;
}

KTextEditor::Editor *EditorChooser::createEditor(TQWidget *parentWidget,TQObject *parent,const char* widgetName,
	const char* name,const TQString& postfix,bool fallBackToKatePart){

        KTextEditor::Editor *tmpEd=0;

        TDEConfig *cfg=kapp->config();
        TQString previousGroup=cfg->group();
        cfg->setGroup("KTEXTEDITOR:"+postfix);
        TQString editor=cfg->readPathEntry("editor");
        cfg->setGroup(previousGroup);
        if (editor.isEmpty())
        {
                TDEConfig *config=new TDEConfig("default_components");
                config->setGroup("KTextEditor");
                editor = config->readPathEntry("embeddedEditor", "katepart");
                delete config;
        }

        KService::Ptr serv=KService::serviceByDesktopName(editor);
        if (serv)
        {
                tmpEd=KTextEditor::createEditor(serv->library().latin1(),parentWidget,widgetName,parent,name);
                if (tmpEd) return tmpEd;
        }
        if (fallBackToKatePart)
                return KTextEditor::createEditor("libkatepart",parentWidget,widgetName,parent,name);

        return 0;
}

