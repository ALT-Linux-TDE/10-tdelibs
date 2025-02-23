/* This file is part of the KDE libraries
   Copyright (C) 2000 Kurt Granroth <granroth@kde.org>

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
#include <kedittoolbar.h>

#include <tqdom.h>
#include <tqlayout.h>
#include <tqdir.h>
#include <tqfile.h>
#include <tqheader.h>
#include <tqcombobox.h>
#include <tqdragobject.h>
#include <tqtoolbutton.h>
#include <tqlabel.h>
#include <tqvaluelist.h>
#include <tqapplication.h>
#include <tqtextstream.h>

#include <tdeaction.h>
#include <kstandarddirs.h>
#include <tdelocale.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kinstance.h>
#include <tdemessagebox.h>
#include <kxmlguifactory.h>
#include <kseparator.h>
#include <tdeconfig.h>
#include <tdelistview.h>
#include <kdebug.h>
#include <kpushbutton.h>
#include <kprocio.h>

static const char * const lineseparatorstring = I18N_NOOP("--- line separator ---");
static const char * const separatorstring = I18N_NOOP("--- separator ---");

#define LINESEPARATORSTRING i18n(lineseparatorstring)
#define SEPARATORSTRING i18n(separatorstring)

static void dump_xml(const TQDomDocument& doc)
{
    TQString str;
    TQTextStream ts(&str, IO_WriteOnly);
    ts << doc;
    kdDebug() << str << endl;
}

typedef TQValueList<TQDomElement> ToolbarList;

namespace KEditToolbarInternal
{
class XmlData
{
public:
  enum XmlType { Shell = 0, Part, Local, Merged };
  XmlData()
  {
    m_isModified = false;
    m_actionCollection = 0;
  }

  TQString      m_xmlFile;
  TQDomDocument m_document;
  XmlType      m_type;
  bool         m_isModified;
  TDEActionCollection* m_actionCollection;

  ToolbarList  m_barList;
};

typedef TQValueList<XmlData> XmlDataList;

class ToolbarItem : public TQListViewItem
{
public:
  ToolbarItem(TDEListView *parent, const TQString& tag = TQString::null, const TQString& name = TQString::null, const TQString& statusText = TQString::null)
    : TQListViewItem(parent),
      m_tag(tag),
      m_name(name),
      m_statusText(statusText)
  {
  }

  ToolbarItem(TDEListView *parent, TQListViewItem *item, const TQString &tag = TQString::null, const TQString& name = TQString::null, const TQString& statusText = TQString::null)
    : TQListViewItem(parent, item),
      m_tag(tag),
      m_name(name),
      m_statusText(statusText)
  {
  }

  virtual TQString key(int column, bool) const
  {
    TQString s = text( column );
    if ( s == LINESEPARATORSTRING )
      return "0";
    if ( s == SEPARATORSTRING )
      return "1";
    return "2" + s;
  }

  void setInternalTag(const TQString &tag) { m_tag = tag; }
  void setInternalName(const TQString &name) { m_name = name; }
  void setStatusText(const TQString &text) { m_statusText = text; }
  TQString internalTag() const { return m_tag; }
  TQString internalName() const { return m_name; }
  TQString statusText() const { return m_statusText; }
private:
  TQString m_tag;
  TQString m_name;
  TQString m_statusText;
};

#define TOOLBARITEMMIMETYPE "data/x-kde.toolbar.item"
class ToolbarItemDrag : public TQStoredDrag
{
public:
  ToolbarItemDrag(ToolbarItem *toolbarItem,
                    TQWidget *dragSource = 0, const char *name = 0)
    : TQStoredDrag( TOOLBARITEMMIMETYPE, dragSource, name )
  {
    if (toolbarItem) {
      TQByteArray data;
      TQDataStream out(data, IO_WriteOnly);
      out << toolbarItem->internalTag();
      out << toolbarItem->internalName();
      out << toolbarItem->statusText();
      out << toolbarItem->text(1); // separators need this.
      setEncodedData(data);
    }
  }

  static bool canDecode(TQMimeSource* e)
  {
    return e->provides(TOOLBARITEMMIMETYPE);
  }

  static bool decode( const TQMimeSource* e, KEditToolbarInternal::ToolbarItem& item )
  {
    if (!e)
      return false;

    TQByteArray data = e->encodedData(TOOLBARITEMMIMETYPE);
    if ( data.isEmpty() )
      return false;

    TQString internalTag, internalName, statusText, text;
    TQDataStream in(data, IO_ReadOnly);
    in >> internalTag;
    in >> internalName;
    in >> statusText;
    in >> text;

    item.setInternalTag( internalTag );
    item.setInternalName( internalName );
    item.setStatusText( statusText );
    item.setText(1, text);

    return true;
  }
};

class ToolbarListView : public TDEListView
{
public:
  ToolbarListView(TQWidget *parent=0, const char *name=0)
    : TDEListView(parent, name)
  {
  }
protected:
  virtual TQDragObject *dragObject()
  {
    KEditToolbarInternal::ToolbarItem *item = dynamic_cast<KEditToolbarInternal::ToolbarItem*>(selectedItem());
    if ( item ) {
      KEditToolbarInternal::ToolbarItemDrag *obj = new KEditToolbarInternal::ToolbarItemDrag(item,
                                 this, "ToolbarAction drag item");
      const TQPixmap *pm = item->pixmap(0);
      if( pm )
        obj->setPixmap( *pm );
      return obj;
    }
    return 0;
  }

  virtual bool acceptDrag(TQDropEvent *event) const
  {
    return KEditToolbarInternal::ToolbarItemDrag::canDecode( event );
  }
};
} // namespace

class KEditToolbarWidgetPrivate
{
public:
    /**
     * @param instance The instance.
     * @param collection In a non-KParts application, this is the collection passed
     * to the KEditToolbar constructor.
     * In a KParts application we let create a KXMLGUIClient create a dummy one,
     * but it probably isn't used.
     */
  KEditToolbarWidgetPrivate(TDEInstance *instance, TDEActionCollection* collection)
      : m_collection( collection )
  {
    m_instance = instance;
    m_isPart   = false;
    m_helpArea = 0L;
    m_kdialogProcess = 0;
  }
  ~KEditToolbarWidgetPrivate()
  {
  }

  TQString xmlFile(const TQString& xml_file)
  {
    return xml_file.isNull() ? TQString(m_instance->instanceName()) + "ui.rc" :
                               xml_file;
  }

  /**
   * Load in the specified XML file and dump the raw xml
   */
  TQString loadXMLFile(const TQString& _xml_file)
  {
    TQString raw_xml;
    TQString xml_file = xmlFile(_xml_file);
    //kdDebug() << "loadXMLFile xml_file=" << xml_file << endl;

    if ( !TQDir::isRelativePath(xml_file) )
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file);
    else
      raw_xml = KXMLGUIFactory::readConfigFile(xml_file, m_instance);

    return raw_xml;
  }

  /**
   * Return a list of toolbar elements given a toplevel element
   */
  ToolbarList findToolbars(TQDomNode n)
  {
    static const TQString &tagToolbar = TDEGlobal::staticQString( "ToolBar" );
    static const TQString &attrNoEdit = TDEGlobal::staticQString( "noEdit" );
    ToolbarList list;

    for( ; !n.isNull(); n = n.nextSibling() )
    {
      TQDomElement elem = n.toElement();
      if (elem.isNull())
        continue;

      if (elem.tagName() == tagToolbar && elem.attribute( attrNoEdit ) != "true" )
        list.append(elem);

      list += findToolbars(elem.firstChild());
    }

    return list;
  }

  /**
   * Return the name of a given toolbar
   */
  TQString toolbarName( const KEditToolbarInternal::XmlData& xmlData, const TQDomElement& it ) const
  {
      static const TQString &tagText = TDEGlobal::staticQString( "text" );
      static const TQString &tagText2 = TDEGlobal::staticQString( "Text" );
      static const TQString &attrName = TDEGlobal::staticQString( "name" );

      TQString name;
      TQCString txt( it.namedItem( tagText ).toElement().text().utf8() );
      if ( txt.isEmpty() )
          txt = it.namedItem( tagText2 ).toElement().text().utf8();
      if ( txt.isEmpty() )
          name = it.attribute( attrName );
      else
          name = i18n( txt );

      // the name of the toolbar might depend on whether or not
      // it is in tdeparts
      if ( ( xmlData.m_type == KEditToolbarInternal::XmlData::Shell ) ||
           ( xmlData.m_type == KEditToolbarInternal::XmlData::Part ) )
      {
        TQString doc_name(xmlData.m_document.documentElement().attribute( attrName ));
        name += " <" + doc_name + ">";
      }
      return name;
  }
  /**
   * Look for a given item in the current toolbar
   */
  TQDomElement findElementForToolbarItem( const KEditToolbarInternal::ToolbarItem* item ) const
  {
    static const TQString &attrName    = TDEGlobal::staticQString( "name" );
    for(TQDomNode n = m_currentToolbarElem.firstChild(); !n.isNull(); n = n.nextSibling())
    {
      TQDomElement elem = n.toElement();
      if ((elem.attribute(attrName) == item->internalName()) &&
          (elem.tagName() == item->internalTag()))
        return elem;
    }
    return TQDomElement();
  }

#ifndef NDEBUG
  void dump()
  {
    static const char* s_XmlTypeToString[] = { "Shell", "Part", "Local", "Merged" };
    KEditToolbarInternal::XmlDataList::Iterator xit = m_xmlFiles.begin();
    for ( ; xit != m_xmlFiles.end(); ++xit )
    {
        kdDebug(240) << "KEditToolbarInternal::XmlData type " << s_XmlTypeToString[(*xit).m_type] << " xmlFile: " << (*xit).m_xmlFile << endl;
        for( TQValueList<TQDomElement>::Iterator it = (*xit).m_barList.begin();
             it != (*xit).m_barList.end(); ++it ) {
            kdDebug(240) << "    Toolbar: " << toolbarName( *xit, *it ) << endl;
        }
        if ( (*xit).m_actionCollection )
            kdDebug(240) << "    " << (*xit).m_actionCollection->count() << " actions in the collection." << endl;
        else
            kdDebug(240) << "    no action collection." << endl;
    }
  }
#endif

  //TQValueList<TDEAction*> m_actionList;
  TDEActionCollection* m_collection;
  TDEInstance         *m_instance;

  KEditToolbarInternal::XmlData*     m_currentXmlData;
  TQDomElement m_currentToolbarElem;

  TQString            m_xmlFile;
  TQString            m_globalFile;
  TQString            m_rcFile;
  TQDomDocument       m_localDoc;
  bool               m_isPart;

  ToolbarList        m_barList;

  KEditToolbarInternal::XmlDataList m_xmlFiles;

  TQLabel     *m_comboLabel;
  KSeparator *m_comboSeparator;
  TQLabel * m_helpArea;
  KPushButton* m_changeIcon;
  KProcIO* m_kdialogProcess;
  bool m_hasKDialog;
};

class KEditToolbarPrivate {
public:
    bool m_accept;

    // Save parameters for recreating widget after resetting toolbar
    bool m_global;
    TDEActionCollection* m_collection;
    TQString m_file;
    KXMLGUIFactory* m_factory;
};

const char *KEditToolbar::s_defaultToolbar = 0L;

KEditToolbar::KEditToolbar(TDEActionCollection *collection, const TQString& file,
                           bool global, TQWidget* parent, const char* name)
  : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent, name),
    m_widget(new KEditToolbarWidget(TQString::fromLatin1(s_defaultToolbar), collection, file, global, this))
{
    init();
    d->m_global = global;
    d->m_collection = collection;
    d->m_file = file;
}

KEditToolbar::KEditToolbar(const TQString& defaultToolbar, TDEActionCollection *collection,
                           const TQString& file, bool global,
                           TQWidget* parent, const char* name)
  : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent, name),
    m_widget(new KEditToolbarWidget(defaultToolbar, collection, file, global, this))
{
    init();
    d->m_global = global;
    d->m_collection = collection;
    d->m_file = file;
}

KEditToolbar::KEditToolbar(KXMLGUIFactory* factory, TQWidget* parent, const char* name)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent, name),
      m_widget(new KEditToolbarWidget(TQString::fromLatin1(s_defaultToolbar), factory, this))
{
    init();
    d->m_factory = factory;
}

KEditToolbar::KEditToolbar(const TQString& defaultToolbar,KXMLGUIFactory* factory,
                           TQWidget* parent, const char* name)
    : KDialogBase(Swallow, i18n("Configure Toolbars"), Default|Ok|Apply|Cancel, Ok, parent, name),
      m_widget(new KEditToolbarWidget(defaultToolbar, factory, this))
{
    init();
    d->m_factory = factory;
}

void KEditToolbar::init()
{
    d = new KEditToolbarPrivate();
    d->m_accept = false;
    d->m_factory = 0;

    setMainWidget(m_widget);

    connect(m_widget, TQ_SIGNAL(enableOk(bool)), TQ_SLOT(acceptOK(bool)));
    connect(m_widget, TQ_SIGNAL(enableOk(bool)), TQ_SLOT(enableButtonApply(bool)));
    enableButtonApply(false);

    setMinimumSize(sizeHint());
    s_defaultToolbar = 0L;
}

KEditToolbar::~KEditToolbar()
{
    delete d;
}

void KEditToolbar::acceptOK(bool b)
{
    enableButtonOK(b);
    d->m_accept = b;
}

void KEditToolbar::slotDefault()
{
    if ( KMessageBox::warningContinueCancel(this, i18n("Do you really want to reset all toolbars of this application to their default? The changes will be applied immediately."), i18n("Reset Toolbars"),i18n("Reset"))!=KMessageBox::Continue )
        return;

    delete m_widget;
    d->m_accept = false;

    if ( d->m_factory )
    {
        const TQString localPrefix = locateLocal("data", "");
        TQPtrList<KXMLGUIClient> clients(d->m_factory->clients());
        TQPtrListIterator<KXMLGUIClient> it( clients );

        for( ; it.current(); ++it)
        {
            KXMLGUIClient *client = it.current();
            TQString file = client->xmlFile();

            if (file.isNull())
                continue;

            if (TQDir::isRelativePath(file))
            {
                const TDEInstance *instance = client->instance() ? client->instance() : TDEGlobal::instance();
                file = locateLocal("data", TQString::fromLatin1( instance->instanceName() + '/' ) + file);
            }
            else
            {
                if (!file.startsWith(localPrefix))
                    continue;
            }

            if ( TQFile::exists( file ) )
                if ( !TQFile::remove( file ) )
                    kdWarning() << "Could not delete " << file << endl;
        }

        m_widget = new KEditToolbarWidget(TQString::null, d->m_factory, this);
        m_widget->rebuildKXMLGUIClients();
    }
    else
    {
        int slash = d->m_file.findRev('/')+1;
        if (slash)
            d->m_file = d->m_file.mid(slash);
        TQString xml_file = locateLocal("data", TQString::fromLatin1( TDEGlobal::instance()->instanceName() + '/' ) + d->m_file);

        if ( TQFile::exists( xml_file ) )
            if ( !TQFile::remove( xml_file ) )
                kdWarning() << "Could not delete " << xml_file << endl;

        m_widget = new KEditToolbarWidget(TQString::null, d->m_collection, d->m_file, d->m_global, this);
    }

    setMainWidget(m_widget);
    m_widget->show();

    connect(m_widget, TQ_SIGNAL(enableOk(bool)), TQ_SLOT(acceptOK(bool)));
    connect(m_widget, TQ_SIGNAL(enableOk(bool)), TQ_SLOT(enableButtonApply(bool)));

    enableButtonApply(false);
    emit newToolbarConfig();
}

void KEditToolbar::slotOk()
{
  if (!d->m_accept) {
      reject();
      return;
  }

  if (!m_widget->save())
  {
    // some error box here is needed
  }
  else
  {
    emit newToolbarConfig();
    accept();
  }
}

void KEditToolbar::slotApply()
{
    (void)m_widget->save();
    enableButtonApply(false);
    emit newToolbarConfig();
}

void KEditToolbar::setDefaultToolbar(const char *toolbarName)
{
    s_defaultToolbar = toolbarName;
}

KEditToolbarWidget::KEditToolbarWidget(TDEActionCollection *collection,
                                       const TQString& file,
                                       bool global, TQWidget *parent)
  : TQWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), collection))
{
  initNonKPart(collection, file, global);
  // now load in our toolbar combo box
  loadToolbarCombo();
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::KEditToolbarWidget(const TQString& defaultToolbar,
                                       TDEActionCollection *collection,
                                       const TQString& file, bool global,
                                       TQWidget *parent)
  : TQWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), collection))
{
  initNonKPart(collection, file, global);
  // now load in our toolbar combo box
  loadToolbarCombo(defaultToolbar);
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::KEditToolbarWidget( KXMLGUIFactory* factory,
                                        TQWidget *parent)
  : TQWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), KXMLGUIClient::actionCollection() /*create new one*/))
{
  initKPart(factory);
  // now load in our toolbar combo box
  loadToolbarCombo();
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::KEditToolbarWidget( const TQString& defaultToolbar,
                                        KXMLGUIFactory* factory,
                                        TQWidget *parent)
  : TQWidget(parent),
    d(new KEditToolbarWidgetPrivate(instance(), KXMLGUIClient::actionCollection() /*create new one*/))
{
  initKPart(factory);
  // now load in our toolbar combo box
  loadToolbarCombo(defaultToolbar);
  adjustSize();
  setMinimumSize(sizeHint());
}

KEditToolbarWidget::~KEditToolbarWidget()
{
    delete d;
}

void KEditToolbarWidget::initNonKPart(TDEActionCollection *collection,
                                      const TQString& file, bool global)
{
  //d->m_actionList = collection->actions();

  // handle the merging
  if (global)
    setXMLFile(locate("config", "ui/ui_standards.rc"));
  TQString localXML = d->loadXMLFile(file);
  setXML(localXML, true);

  // reusable vars
  TQDomElement elem;

  // first, get all of the necessary info for our local xml
  KEditToolbarInternal::XmlData local;
  local.m_xmlFile = d->xmlFile(file);
  local.m_type    = KEditToolbarInternal::XmlData::Local;
  local.m_document.setContent(localXML);
  elem = local.m_document.documentElement().toElement();
  local.m_barList = d->findToolbars(elem);
  local.m_actionCollection = collection;
  d->m_xmlFiles.append(local);

  // then, the merged one (ui_standards + local xml)
  KEditToolbarInternal::XmlData merge;
  merge.m_xmlFile  = TQString::null;
  merge.m_type     = KEditToolbarInternal::XmlData::Merged;
  merge.m_document = domDocument();
  elem = merge.m_document.documentElement().toElement();
  merge.m_barList  = d->findToolbars(elem);
  merge.m_actionCollection = collection;
  d->m_xmlFiles.append(merge);

#ifndef NDEBUG
  //d->dump();
#endif

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();
}

void KEditToolbarWidget::initKPart(KXMLGUIFactory* factory)
{
  // reusable vars
  TQDomElement elem;

  setFactory( factory );
  actionCollection()->setWidget( this );

  // add all of the client data
  TQPtrList<KXMLGUIClient> clients(factory->clients());
  TQPtrListIterator<KXMLGUIClient> it( clients );
  for( ; it.current(); ++it)
  {
    KXMLGUIClient *client = it.current();

    if (client->xmlFile().isNull())
      continue;

    KEditToolbarInternal::XmlData data;
    data.m_xmlFile = client->localXMLFile();
    if ( it.atFirst() )
      data.m_type = KEditToolbarInternal::XmlData::Shell;
    else
      data.m_type = KEditToolbarInternal::XmlData::Part;
    data.m_document.setContent( KXMLGUIFactory::readConfigFile( client->xmlFile(), client->instance() ) );
    elem = data.m_document.documentElement().toElement();
    data.m_barList = d->findToolbars(elem);
    data.m_actionCollection = client->actionCollection();
    d->m_xmlFiles.append(data);

    //d->m_actionList += client->actionCollection()->actions();
  }

#ifndef NDEBUG
  //d->dump();
#endif

  // okay, that done, we concern ourselves with the GUI aspects
  setupLayout();
}

bool KEditToolbarWidget::save()
{
  //kdDebug(240) << "KEditToolbarWidget::save" << endl;
  KEditToolbarInternal::XmlDataList::Iterator it = d->m_xmlFiles.begin();
  for ( ; it != d->m_xmlFiles.end(); ++it)
  {
    // let's not save non-modified files
    if ( !((*it).m_isModified) )
      continue;

    // let's also skip (non-existent) merged files
    if ( (*it).m_type == KEditToolbarInternal::XmlData::Merged )
      continue;

    dump_xml((*it).m_document);

    kdDebug(240) << "Saving " << (*it).m_xmlFile << endl;
    // if we got this far, we might as well just save it
    KXMLGUIFactory::saveConfigFile((*it).m_document, (*it).m_xmlFile);
  }

  if ( !factory() )
    return true;

  rebuildKXMLGUIClients();

  return true;
}

void KEditToolbarWidget::rebuildKXMLGUIClients()
{
  if ( !factory() )
    return;

  TQPtrList<KXMLGUIClient> clients(factory()->clients());
  //kdDebug(240) << "factory: " << clients.count() << " clients" << endl;

  // remove the elements starting from the last going to the first
  KXMLGUIClient *client = clients.last();
  while ( client )
  {
    //kdDebug(240) << "factory->removeClient " << client << endl;
    factory()->removeClient( client );
    client = clients.prev();
  }

  KXMLGUIClient *firstClient = clients.first();

  // now, rebuild the gui from the first to the last
  //kdDebug(240) << "rebuilding the gui" << endl;
  TQPtrListIterator<KXMLGUIClient> cit( clients );
  for( ; cit.current(); ++cit)
  {
    KXMLGUIClient* client = cit.current();
    //kdDebug(240) << "updating client " << client << " " << client->instance()->instanceName() << "  xmlFile=" << client->xmlFile() << endl;
    TQString file( client->xmlFile() ); // before setting ui_standards!
    if ( !file.isEmpty() )
    {
        // passing an empty stream forces the clients to reread the XML
        client->setXMLGUIBuildDocument( TQDomDocument() );

        // for the shell, merge in ui_standards.rc
        if ( client == firstClient ) // same assumption as in the ctor: first==shell
            client->setXMLFile(locate("config", "ui/ui_standards.rc"));

        // and this forces it to use the *new* XML file
        client->setXMLFile( file, client == firstClient /* merge if shell */ );
    }
  }

  // Now we can add the clients to the factory
  // We don't do it in the loop above because adding a part automatically
  // adds its plugins, so we must make sure the plugins were updated first.
  cit.toFirst();
  for( ; cit.current(); ++cit)
    factory()->addClient( cit.current() );
}

void KEditToolbarWidget::setupLayout()
{
  // the toolbar name combo
  d->m_comboLabel = new TQLabel(i18n("&Toolbar:"), this);
  m_toolbarCombo = new TQComboBox(this);
  m_toolbarCombo->setEnabled(false);
  d->m_comboLabel->setBuddy(m_toolbarCombo);
  d->m_comboSeparator = new KSeparator(this);
  connect(m_toolbarCombo, TQ_SIGNAL(activated(const TQString&)),
          this,           TQ_SLOT(slotToolbarSelected(const TQString&)));

//  TQPushButton *new_toolbar = new TQPushButton(i18n("&New"), this);
//  new_toolbar->setPixmap(BarIcon("document-new", TDEIcon::SizeSmall));
//  new_toolbar->setEnabled(false); // disabled until implemented
//  TQPushButton *del_toolbar = new TQPushButton(i18n("&Delete"), this);
//  del_toolbar->setPixmap(BarIcon("edit-delete", TDEIcon::SizeSmall));
//  del_toolbar->setEnabled(false); // disabled until implemented

  // our list of inactive actions
  TQLabel *inactive_label = new TQLabel(i18n("A&vailable actions:"), this);
  m_inactiveList = new KEditToolbarInternal::ToolbarListView(this);
  m_inactiveList->setDragEnabled(true);
  m_inactiveList->setAcceptDrops(true);
  m_inactiveList->setDropVisualizer(false);
  m_inactiveList->setAllColumnsShowFocus(true);
  m_inactiveList->setMinimumSize(180, 250);
  m_inactiveList->header()->hide();
  m_inactiveList->addColumn(""); // icon
  int column2 = m_inactiveList->addColumn(""); // text
  m_inactiveList->setSorting( column2 );
  inactive_label->setBuddy(m_inactiveList);
  connect(m_inactiveList, TQ_SIGNAL(selectionChanged(TQListViewItem *)),
          this,           TQ_SLOT(slotInactiveSelected(TQListViewItem *)));
  connect(m_inactiveList, TQ_SIGNAL( doubleClicked( TQListViewItem *, const TQPoint &, int  )),
          this,           TQ_SLOT(slotInsertButton()));

  // our list of active actions
  TQLabel *active_label = new TQLabel(i18n("Curr&ent actions:"), this);
  m_activeList = new KEditToolbarInternal::ToolbarListView(this);
  m_activeList->setDragEnabled(true);
  m_activeList->setAcceptDrops(true);
  m_activeList->setDropVisualizer(true);
  m_activeList->setAllColumnsShowFocus(true);
  m_activeList->setMinimumWidth(m_inactiveList->minimumWidth());
  m_activeList->header()->hide();
  m_activeList->addColumn(""); // icon
  m_activeList->addColumn(""); // text
  m_activeList->setSorting(-1);
  active_label->setBuddy(m_activeList);

  connect(m_inactiveList, TQ_SIGNAL(dropped(TDEListView*,TQDropEvent*,TQListViewItem*)),
          this,              TQ_SLOT(slotDropped(TDEListView*,TQDropEvent*,TQListViewItem*)));
  connect(m_activeList, TQ_SIGNAL(dropped(TDEListView*,TQDropEvent*,TQListViewItem*)),
          this,            TQ_SLOT(slotDropped(TDEListView*,TQDropEvent*,TQListViewItem*)));
  connect(m_activeList, TQ_SIGNAL(selectionChanged(TQListViewItem *)),
          this,         TQ_SLOT(slotActiveSelected(TQListViewItem *)));
  connect(m_activeList, TQ_SIGNAL( doubleClicked( TQListViewItem *, const TQPoint &, int  )),
          this,           TQ_SLOT(slotRemoveButton()));

  // "change icon" button
  d->m_changeIcon = new KPushButton( i18n( "Change &Icon..." ), this );
  TQString kdialogExe = TDEStandardDirs::findExe(TQString::fromLatin1("kdialog"));
  d->m_hasKDialog = !kdialogExe.isEmpty();
  d->m_changeIcon->setEnabled( d->m_hasKDialog );

  connect( d->m_changeIcon, TQ_SIGNAL( clicked() ),
           this, TQ_SLOT( slotChangeIcon() ) );

  // The buttons in the middle
  TQIconSet iconSet;

  m_upAction     = new TQToolButton(this);
  iconSet = SmallIconSet( "go-up" );
  m_upAction->setIconSet( iconSet );
  m_upAction->setEnabled(false);
  m_upAction->setAutoRepeat(true);
  connect(m_upAction, TQ_SIGNAL(clicked()), TQ_SLOT(slotUpButton()));

  m_insertAction = new TQToolButton(this);
  iconSet = TQApplication::reverseLayout() ? SmallIconSet( "back" ) : SmallIconSet( "forward" );
  m_insertAction->setIconSet( iconSet );
  m_insertAction->setEnabled(false);
  connect(m_insertAction, TQ_SIGNAL(clicked()), TQ_SLOT(slotInsertButton()));

  m_removeAction = new TQToolButton(this);
  iconSet = TQApplication::reverseLayout() ? SmallIconSet( "forward" ) : SmallIconSet( "back" );
  m_removeAction->setIconSet( iconSet );
  m_removeAction->setEnabled(false);
  connect(m_removeAction, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveButton()));

  m_downAction   = new TQToolButton(this);
  iconSet = SmallIconSet( "go-down" );
  m_downAction->setIconSet( iconSet );
  m_downAction->setEnabled(false);
  m_downAction->setAutoRepeat(true);
  connect(m_downAction, TQ_SIGNAL(clicked()), TQ_SLOT(slotDownButton()));

  d->m_helpArea = new TQLabel(this);
  d->m_helpArea->setAlignment( TQt::WordBreak );

  // now start with our layouts
  TQVBoxLayout *top_layout = new TQVBoxLayout(this, 0, KDialog::spacingHint());

  TQVBoxLayout *name_layout = new TQVBoxLayout(KDialog::spacingHint());
  TQHBoxLayout *list_layout = new TQHBoxLayout(KDialog::spacingHint());

  TQVBoxLayout *inactive_layout = new TQVBoxLayout(KDialog::spacingHint());
  TQVBoxLayout *active_layout = new TQVBoxLayout(KDialog::spacingHint());
  TQHBoxLayout *changeIcon_layout = new TQHBoxLayout(KDialog::spacingHint());

  TQGridLayout *button_layout = new TQGridLayout(5, 3, 0);

  name_layout->addWidget(d->m_comboLabel);
  name_layout->addWidget(m_toolbarCombo);
//  name_layout->addWidget(new_toolbar);
//  name_layout->addWidget(del_toolbar);

  button_layout->setRowStretch( 0, 10 );
  button_layout->addWidget(m_upAction, 1, 1);
  button_layout->addWidget(m_removeAction, 2, 0);
  button_layout->addWidget(m_insertAction, 2, 2);
  button_layout->addWidget(m_downAction, 3, 1);
  button_layout->setRowStretch( 4, 10 );

  inactive_layout->addWidget(inactive_label);
  inactive_layout->addWidget(m_inactiveList, 1);

  active_layout->addWidget(active_label);
  active_layout->addWidget(m_activeList, 1);
  active_layout->addLayout(changeIcon_layout);

  changeIcon_layout->addStretch( 1 );
  changeIcon_layout->addWidget( d->m_changeIcon );
  changeIcon_layout->addStretch( 1 );

  list_layout->addLayout(inactive_layout);
  list_layout->addLayout(button_layout);
  list_layout->addLayout(active_layout);

  top_layout->addLayout(name_layout);
  top_layout->addWidget(d->m_comboSeparator);
  top_layout->addLayout(list_layout,10);
  top_layout->addWidget(d->m_helpArea);
  top_layout->addWidget(new KSeparator(this));
}

void KEditToolbarWidget::loadToolbarCombo(const TQString& defaultToolbar)
{
  static const TQString &attrName = TDEGlobal::staticQString( "name" );
  // just in case, we clear our combo
  m_toolbarCombo->clear();

  int defaultToolbarId = -1;
  int count = 0;
  // load in all of the toolbar names into this combo box
  KEditToolbarInternal::XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    // skip the local one in favor of the merged
    if ( (*xit).m_type == KEditToolbarInternal::XmlData::Local )
      continue;

    // each xml file may have any number of toolbars
    ToolbarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      TQString name = d->toolbarName( *xit, *it );
      m_toolbarCombo->setEnabled( true );
      m_toolbarCombo->insertItem( name );
      if (defaultToolbarId == -1 && (name == defaultToolbar || defaultToolbar == (*it).attribute( attrName )))
          defaultToolbarId = count;
      count++;
    }
  }
  bool showCombo = (count > 1);
  d->m_comboLabel->setShown(showCombo);
  d->m_comboSeparator->setShown(showCombo);
  m_toolbarCombo->setShown(showCombo);
  if (defaultToolbarId == -1)
      defaultToolbarId = 0;
  // we want to the specified item selected and its actions loaded
  m_toolbarCombo->setCurrentItem(defaultToolbarId);
  slotToolbarSelected(m_toolbarCombo->currentText());
}

void KEditToolbarWidget::loadActionList(TQDomElement& elem)
{
  static const TQString &tagSeparator = TDEGlobal::staticQString( "Separator" );
  static const TQString &tagMerge     = TDEGlobal::staticQString( "Merge" );
  static const TQString &tagActionList= TDEGlobal::staticQString( "ActionList" );
  static const TQString &attrName     = TDEGlobal::staticQString( "name" );
  static const TQString &attrLineSeparator = TDEGlobal::staticQString( "lineSeparator" );

  int     sep_num = 0;
  TQString sep_name("separator_%1");

  // clear our lists
  m_inactiveList->clear();
  m_activeList->clear();
  m_insertAction->setEnabled(false);
  m_removeAction->setEnabled(false);
  m_upAction->setEnabled(false);
  m_downAction->setEnabled(false);

  // We'll use this action collection
  TDEActionCollection* actionCollection = d->m_currentXmlData->m_actionCollection;

  // store the names of our active actions
  TQMap<TQString, bool> active_list;

  // see if our current action is in this toolbar
  TDEIconLoader *loader = TDEGlobal::instance()->iconLoader();
  TQDomNode n = elem.lastChild();
  for( ; !n.isNull(); n = n.previousSibling() )
  {
    TQDomElement it = n.toElement();
    if (it.isNull()) continue;
    if (it.tagName() == tagSeparator)
    {
      KEditToolbarInternal::ToolbarItem *act = new KEditToolbarInternal::ToolbarItem(m_activeList, tagSeparator, sep_name.arg(sep_num++), TQString::null);
      bool isLineSep = ( it.attribute(attrLineSeparator, "true").lower() == TQString::fromLatin1("true") );
      if(isLineSep)
        act->setText(1, LINESEPARATORSTRING);
      else
        act->setText(1, SEPARATORSTRING);
      it.setAttribute( attrName, act->internalName() );
      continue;
    }

    if (it.tagName() == tagMerge)
    {
      // Merge can be named or not - use the name if there is one
      TQString name = it.attribute( attrName );
      KEditToolbarInternal::ToolbarItem *act = new KEditToolbarInternal::ToolbarItem(m_activeList, tagMerge, name, i18n("This element will be replaced with all the elements of an embedded component."));
      if ( name.isEmpty() )
          act->setText(1, i18n("<Merge>"));
      else
          act->setText(1, i18n("<Merge %1>").arg(name));
      continue;
    }

    if (it.tagName() == tagActionList)
    {
      KEditToolbarInternal::ToolbarItem *act = new KEditToolbarInternal::ToolbarItem(m_activeList, tagActionList, it.attribute(attrName), i18n("This is a dynamic list of actions. You can move it, but if you remove it you won't be able to re-add it.") );
      act->setText(1, i18n("ActionList: %1").arg(it.attribute(attrName)));
      continue;
    }

    // iterate through this client's actions
    // This used to iterate through _all_ actions, but we don't support
    // putting any action into any client...
    for (unsigned int i = 0;  i < actionCollection->count(); i++)
    {
      TDEAction *action = actionCollection->action( i );

      // do we have a match?
      if (it.attribute( attrName ) == action->name())
      {
        // we have a match!
        KEditToolbarInternal::ToolbarItem *act = new KEditToolbarInternal::ToolbarItem(m_activeList, it.tagName(), action->name(), action->toolTip());
        act->setText(1, action->plainText());
        if (action->hasIcon()) {
          if (!action->icon().isEmpty()) {
            act->setPixmap(0, loader->loadIcon(action->icon(), TDEIcon::Toolbar, 16, TDEIcon::DefaultState, 0, true) );
          }
          else { // Has iconset
            act->setPixmap(0, action->iconSet(TDEIcon::Toolbar).pixmap());
          }
        }

        active_list.insert(action->name(), true);
        break;
      }
    }
  }

  // go through the rest of the collection
  for (int i = actionCollection->count() - 1; i > -1; --i)
  {
    TDEAction *action = actionCollection->action( i );

    // skip our active ones
    if (active_list.contains(action->name())) {
      continue;
    }

    KEditToolbarInternal::ToolbarItem *act = new KEditToolbarInternal::ToolbarItem(m_inactiveList, tagActionList, action->name(), action->toolTip());
    act->setText(1, action->plainText());
    if (action->hasIcon()) {
      if (!action->icon().isEmpty()) {
        act->setPixmap(0, loader->loadIcon(action->icon(), TDEIcon::Toolbar, 16, TDEIcon::DefaultState, 0, true) );
      }
      else { // Has iconset
        act->setPixmap(0, action->iconSet(TDEIcon::Toolbar).pixmap());
      }
    }
  }

  // finally, add default separators to the inactive list
  KEditToolbarInternal::ToolbarItem *act = new KEditToolbarInternal::ToolbarItem(m_inactiveList, tagSeparator, sep_name.arg(sep_num++), TQString::null);
  act->setText(1, LINESEPARATORSTRING);
  act = new KEditToolbarInternal::ToolbarItem(m_inactiveList, tagSeparator, sep_name.arg(sep_num++), TQString::null);
  act->setText(1, SEPARATORSTRING);
}

TDEActionCollection *KEditToolbarWidget::actionCollection() const
{
  return d->m_collection;
}

void KEditToolbarWidget::slotToolbarSelected(const TQString& _text)
{
  // iterate through everything
  KEditToolbarInternal::XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    // each xml file may have any number of toolbars
    ToolbarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      TQString name = d->toolbarName( *xit, *it );
      // is this our toolbar?
      if ( name == _text )
      {
        // save our current settings
        d->m_currentXmlData     = & (*xit);
        d->m_currentToolbarElem = (*it);

        // load in our values
        loadActionList(d->m_currentToolbarElem);

        if ((*xit).m_type == KEditToolbarInternal::XmlData::Part || (*xit).m_type == KEditToolbarInternal::XmlData::Shell)
          setDOMDocument( (*xit).m_document );
        return;
      }
    }
  }
}

void KEditToolbarWidget::slotInactiveSelected(TQListViewItem *item)
{
  KEditToolbarInternal::ToolbarItem* toolitem = static_cast<KEditToolbarInternal::ToolbarItem *>(item);
  if (item)
  {
    m_insertAction->setEnabled(true);
    TQString statusText = toolitem->statusText();
    d->m_helpArea->setText( statusText );
  }
  else
  {
    m_insertAction->setEnabled(false);
    d->m_helpArea->setText( TQString::null );
  }
}

void KEditToolbarWidget::slotActiveSelected(TQListViewItem *item)
{
  KEditToolbarInternal::ToolbarItem* toolitem = static_cast<KEditToolbarInternal::ToolbarItem *>(item);
  m_removeAction->setEnabled( item );

  static const TQString &tagAction = TDEGlobal::staticQString( "Action" );
  d->m_changeIcon->setEnabled( item &&
                               d->m_hasKDialog &&
                               toolitem->internalTag() == tagAction );

  if (item)
  {
    if (item->itemAbove())
      m_upAction->setEnabled(true);
    else
      m_upAction->setEnabled(false);

    if (item->itemBelow())
      m_downAction->setEnabled(true);
    else
      m_downAction->setEnabled(false);
    TQString statusText = toolitem->statusText();
    d->m_helpArea->setText( statusText );
  }
  else
  {
    m_upAction->setEnabled(false);
    m_downAction->setEnabled(false);
    d->m_helpArea->setText( TQString::null );
  }
}

void KEditToolbarWidget::slotDropped(TDEListView *list, TQDropEvent *e, TQListViewItem *after)
{
  KEditToolbarInternal::ToolbarItem *item = new KEditToolbarInternal::ToolbarItem(m_inactiveList); // needs parent, use inactiveList temporarily
  if(!KEditToolbarInternal::ToolbarItemDrag::decode(e, *item)) {
    delete item;
    return;
  }

  if (list == m_activeList) {
    if (e->source() == m_activeList) {
      // has been dragged within the active list (moved).
      moveActive(item, after);
    }
    else
      insertActive(item, after, true);
  } else if (list == m_inactiveList) {
    // has been dragged to the inactive list -> remove from the active list.
    removeActive(item);
  }

  delete item; item = 0; // not neded anymore

  // we're modified, so let this change
  emit enableOk(true);

  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotInsertButton()
{
  KEditToolbarInternal::ToolbarItem *item = (KEditToolbarInternal::ToolbarItem*)m_inactiveList->currentItem();
  insertActive(item, m_activeList->currentItem(), false);

  // we're modified, so let this change
  emit enableOk(true);

  // TODO: #### this causes #97572.
  // It would be better to just "delete item; loadActions( ... , ActiveListOnly );" or something.
  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::slotRemoveButton()
{
  removeActive( dynamic_cast<KEditToolbarInternal::ToolbarItem*>(m_activeList->currentItem()) );

  // we're modified, so let this change
  emit enableOk(true);

  slotToolbarSelected( m_toolbarCombo->currentText() );
}

void KEditToolbarWidget::insertActive(KEditToolbarInternal::ToolbarItem *item, TQListViewItem *before, bool prepend)
{
  if (!item)
    return;

  static const TQString &tagAction    = TDEGlobal::staticQString( "Action" );
  static const TQString &tagSeparator = TDEGlobal::staticQString( "Separator" );
  static const TQString &attrName     = TDEGlobal::staticQString( "name" );
  static const TQString &attrLineSeparator = TDEGlobal::staticQString( "lineSeparator" );
  static const TQString &attrNoMerge  = TDEGlobal::staticQString( "noMerge" );

  TQDomElement new_item;
  // let's handle the separator specially
  if (item->text(1) == LINESEPARATORSTRING) {
    new_item = domDocument().createElement(tagSeparator);
  } else if (item->text(1) == SEPARATORSTRING) {
    new_item = domDocument().createElement(tagSeparator);
    new_item.setAttribute(attrLineSeparator, "false");
  } else
    new_item = domDocument().createElement(tagAction);
  new_item.setAttribute(attrName, item->internalName());

  if (before)
  {
    // we have the item in the active list which is before the new
    // item.. so let's try our best to add our new item right after it
    KEditToolbarInternal::ToolbarItem *act_item = (KEditToolbarInternal::ToolbarItem*)before;
    TQDomElement elem = d->findElementForToolbarItem( act_item );
    Q_ASSERT( !elem.isNull() );
    d->m_currentToolbarElem.insertAfter(new_item, elem);
  }
  else
  {
    // simply put it at the beginning or the end of the list.
    if (prepend)
      d->m_currentToolbarElem.insertBefore(new_item, d->m_currentToolbarElem.firstChild());
    else
      d->m_currentToolbarElem.appendChild(new_item);
  }

  // and set this container as a noMerge
  d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

  // update the local doc
  updateLocal(d->m_currentToolbarElem);
}

void KEditToolbarWidget::removeActive(KEditToolbarInternal::ToolbarItem *item)
{
  if (!item)
    return;

  static const TQString &attrNoMerge = TDEGlobal::staticQString( "noMerge" );

  // we're modified, so let this change
  emit enableOk(true);

  // now iterate through to find the child to nuke
  TQDomElement elem = d->findElementForToolbarItem( item );
  if ( !elem.isNull() )
  {
    // nuke myself!
    d->m_currentToolbarElem.removeChild(elem);

    // and set this container as a noMerge
    d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

    // update the local doc
    updateLocal(d->m_currentToolbarElem);
  }
}

void KEditToolbarWidget::slotUpButton()
{
  KEditToolbarInternal::ToolbarItem *item = (KEditToolbarInternal::ToolbarItem*)m_activeList->currentItem();

  // make sure we're not the top item already
  if (!item->itemAbove())
    return;

  // we're modified, so let this change
  emit enableOk(true);

  moveActive( item, item->itemAbove()->itemAbove() );
  delete item;
}

void KEditToolbarWidget::moveActive( KEditToolbarInternal::ToolbarItem* item, TQListViewItem* before )
{
  TQDomElement e = d->findElementForToolbarItem( item );

  if ( e.isNull() )
    return;

  // cool, i found me.  now clone myself
  KEditToolbarInternal::ToolbarItem *clone = new KEditToolbarInternal::ToolbarItem(m_activeList,
                                       before,
                                       item->internalTag(),
                                       item->internalName(),
                                       item->statusText());

  clone->setText(1, item->text(1));

  // only set new pixmap if exists
  if( item->pixmap(0) )
    clone->setPixmap(0, *item->pixmap(0));

  // select my clone
  m_activeList->setSelected(clone, true);

  // make clone visible
  m_activeList->ensureItemVisible(clone);

  // and do the real move in the DOM
  if ( !before )
    d->m_currentToolbarElem.insertBefore(e, d->m_currentToolbarElem.firstChild() );
  else
    d->m_currentToolbarElem.insertAfter(e, d->findElementForToolbarItem( (KEditToolbarInternal::ToolbarItem*)before ));

  // and set this container as a noMerge
  static const TQString &attrNoMerge = TDEGlobal::staticQString( "noMerge" );
  d->m_currentToolbarElem.setAttribute( attrNoMerge, "1");

  // update the local doc
  updateLocal(d->m_currentToolbarElem);
}

void KEditToolbarWidget::slotDownButton()
{
  KEditToolbarInternal::ToolbarItem *item = (KEditToolbarInternal::ToolbarItem*)m_activeList->currentItem();

  // make sure we're not the bottom item already
  if (!item->itemBelow())
    return;

  // we're modified, so let this change
  emit enableOk(true);

  moveActive( item, item->itemBelow() );
  delete item;
}

void KEditToolbarWidget::updateLocal(TQDomElement& elem)
{
  static const TQString &attrName = TDEGlobal::staticQString( "name" );

  KEditToolbarInternal::XmlDataList::Iterator xit = d->m_xmlFiles.begin();
  for ( ; xit != d->m_xmlFiles.end(); ++xit)
  {
    if ( (*xit).m_type == KEditToolbarInternal::XmlData::Merged )
      continue;

    if ( (*xit).m_type == KEditToolbarInternal::XmlData::Shell ||
         (*xit).m_type == KEditToolbarInternal::XmlData::Part )
    {
      if ( d->m_currentXmlData->m_xmlFile == (*xit).m_xmlFile )
      {
        (*xit).m_isModified = true;
        return;
      }

      continue;
    }

    (*xit).m_isModified = true;

    ToolbarList::Iterator it = (*xit).m_barList.begin();
    for ( ; it != (*xit).m_barList.end(); ++it)
    {
      TQString name( (*it).attribute( attrName ) );
      TQString tag( (*it).tagName() );
      if ( (tag != elem.tagName()) || (name != elem.attribute(attrName)) )
        continue;

      TQDomElement toolbar = (*xit).m_document.documentElement().toElement();
      toolbar.replaceChild(elem, (*it));
      return;
    }

    // just append it
    TQDomElement toolbar = (*xit).m_document.documentElement().toElement();
    toolbar.appendChild(elem);
  }
}

void KEditToolbarWidget::slotChangeIcon()
{
  // We can't use TDEIconChooser here, since it's in libtdeio
  // ##### KDE4: reconsider this, e.g. move KEditToolbar to libtdeio
  
  //if the process is already running (e.g. when somebody clicked the change button twice (see #127149)) - do nothing... 
  //otherwise m_kdialogProcess will be overwritten and set to zero in slotProcessExited()...crash!
  if ( d->m_kdialogProcess && d->m_kdialogProcess->isRunning() )
        return;
  
  d->m_kdialogProcess = new KProcIO;
  TQString kdialogExe = TDEStandardDirs::findExe(TQString::fromLatin1("kdialog"));
  (*d->m_kdialogProcess) << kdialogExe;
  (*d->m_kdialogProcess) << "--embed";
  (*d->m_kdialogProcess) << TQString::number( (ulong)topLevelWidget()->winId() );
  (*d->m_kdialogProcess) << "--geticon";
  (*d->m_kdialogProcess) << "Toolbar";
  (*d->m_kdialogProcess) << "Actions";
  if ( !d->m_kdialogProcess->start( TDEProcess::NotifyOnExit ) ) {
    kdError(240) << "Can't run " << kdialogExe << endl;
    delete d->m_kdialogProcess;
    d->m_kdialogProcess = 0;
    return;
  }

  m_activeList->setEnabled( false ); // don't change the current item
  m_toolbarCombo->setEnabled( false ); // don't change the current toolbar

  connect( d->m_kdialogProcess, TQ_SIGNAL( processExited( TDEProcess* ) ),
           this, TQ_SLOT( slotProcessExited( TDEProcess* ) ) );
}

void KEditToolbarWidget::slotProcessExited( TDEProcess* )
{
  m_activeList->setEnabled( true );
  m_toolbarCombo->setEnabled( true );

  TQString icon;

  if (!d->m_kdialogProcess) {
         kdError(240) << "Something is wrong here! m_kdialogProcess is zero!" << endl;
         return;
  }

  if ( !d->m_kdialogProcess->normalExit() ||
       d->m_kdialogProcess->exitStatus() ||
       d->m_kdialogProcess->readln(icon, true) <= 0 ) {
    delete d->m_kdialogProcess;
    d->m_kdialogProcess = 0;
    return;
  }

  KEditToolbarInternal::ToolbarItem *item = (KEditToolbarInternal::ToolbarItem*)m_activeList->currentItem();
  if(item){
    item->setPixmap(0, BarIcon(icon, 16));

    Q_ASSERT( d->m_currentXmlData->m_type != KEditToolbarInternal::XmlData::Merged );

    d->m_currentXmlData->m_isModified = true;

    // Get hold of ActionProperties tag
    TQDomElement elem = KXMLGUIFactory::actionPropertiesElement( d->m_currentXmlData->m_document );
    // Find or create an element for this action
    TQDomElement act_elem = KXMLGUIFactory::findActionByName( elem, item->internalName(), true /*create*/ );
    Q_ASSERT( !act_elem.isNull() );
    act_elem.setAttribute( "icon", icon );

    // we're modified, so let this change
    emit enableOk(true);
  }

  delete d->m_kdialogProcess;
  d->m_kdialogProcess = 0;
}

void KEditToolbar::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

void KEditToolbarWidget::virtual_hook( int id, void* data )
{ KXMLGUIClient::virtual_hook( id, data ); }

#include "kedittoolbar.moc"
