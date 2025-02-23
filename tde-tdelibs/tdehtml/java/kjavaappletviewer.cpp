/* This file is part of the KDE project
 *
 * Copyright (C) 2003 Koos Vriezen <koos.vriezen@xs4all.nl>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#include <stdio.h>

#ifdef KDE_USE_FINAL
#undef Always
#endif
#include <tqdir.h>
#include <tqtable.h>
#include <tqpair.h>
#include <tqtimer.h>
#include <tqguardedptr.h>
#include <tqlabel.h>

#include <klibloader.h>
#include <tdeaboutdata.h>
#include <kstaticdeleter.h>
#include <tdelocale.h>
#include <kstatusbar.h>
#include <kiconloader.h>
#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeconfig.h>
#include <tdeio/authinfo.h>
#include <dcopclient.h>

#include "kjavaappletwidget.h"
#include "kjavaappletviewer.h"
#include "kjavaappletserver.h"


K_EXPORT_COMPONENT_FACTORY (kjavaappletviewer, KJavaAppletViewerFactory)

TDEInstance *KJavaAppletViewerFactory::s_instance = 0;

KJavaAppletViewerFactory::KJavaAppletViewerFactory () {
    s_instance = new TDEInstance ("kjava");
}

KJavaAppletViewerFactory::~KJavaAppletViewerFactory () {
    delete s_instance;
}

KParts::Part *KJavaAppletViewerFactory::createPartObject
  (TQWidget *wparent, const char *wname,
   TQObject *parent, const char * name, const char *, const TQStringList & args) {
    return new KJavaAppletViewer (wparent, wname, parent, name, args);
}

//-----------------------------------------------------------------------------

class KJavaServerMaintainer;
static KJavaServerMaintainer * serverMaintainer = 0;

class KJavaServerMaintainer {
public:
    KJavaServerMaintainer () { }
    ~KJavaServerMaintainer ();

    KJavaAppletContext * getContext (TQObject*, const TQString &);
    void releaseContext (TQObject*, const TQString &);
    void setServer (KJavaAppletServer * s);
    TQGuardedPtr <KJavaAppletServer> server;
private:
    typedef TQMap <TQPair <TQObject*, TQString>, TQPair <KJavaAppletContext*, int> >
            ContextMap;
    ContextMap m_contextmap;
};

KJavaServerMaintainer::~KJavaServerMaintainer () {
    delete server;
}

KJavaAppletContext * KJavaServerMaintainer::getContext (TQObject * w, const TQString & doc) {
    ContextMap::key_type key = qMakePair (w, doc);
    ContextMap::iterator it = m_contextmap.find (key);
    if (it != m_contextmap.end ()) {
        ++((*it).second);
        return (*it).first;
    }
    KJavaAppletContext* const context = new KJavaAppletContext ();
    m_contextmap.insert (key, qMakePair(context, 1));
    return context;
}

void KJavaServerMaintainer::releaseContext (TQObject * w, const TQString & doc) {
    ContextMap::iterator it = m_contextmap.find (qMakePair (w, doc));
    if (it != m_contextmap.end () && --(*it).second <= 0) {
        kdDebug(6100) << "KJavaServerMaintainer::releaseContext" << endl;
        (*it).first->deleteLater ();
        m_contextmap.remove (it);
    }
}

inline void KJavaServerMaintainer::setServer (KJavaAppletServer * s) {
    if (!server)
        server = s;
}

static KStaticDeleter <KJavaServerMaintainer> serverMaintainerDeleter;

//-----------------------------------------------------------------------------

AppletParameterDialog::AppletParameterDialog (KJavaAppletWidget * parent)
    : KDialogBase (parent, "paramdialog", true, i18n ("Applet Parameters"),
                   KDialogBase::Close, KDialogBase::Close, true),
      m_appletWidget (parent) {
    KJavaApplet* const applet = parent->applet ();
    table = new TQTable (30, 2, this);
    table->setMinimumSize (TQSize (600, 400));
    table->setColumnWidth (0, 200);
    table->setColumnWidth (1, 340);
    TQHeader* const header = table->horizontalHeader();
    header->setLabel (0, i18n ("Parameter"));
    header->setLabel (1, i18n ("Value"));
    TQTableItem * tit = new TQTableItem (table, TQTableItem::Never, i18n("Class"));
    table->setItem (0, 0, tit);
    tit = new TQTableItem(table, TQTableItem::Always, applet->appletClass());
    table->setItem (0, 1, tit);
    tit = new TQTableItem (table, TQTableItem::Never, i18n ("Base URL"));
    table->setItem (1, 0, tit);
    tit = new TQTableItem(table, TQTableItem::Always, applet->baseURL());
    table->setItem (1, 1, tit);
    tit = new TQTableItem (table, TQTableItem::Never, i18n ("Archives"));
    table->setItem (2, 0, tit);
    tit = new TQTableItem(table, TQTableItem::Always, applet->archives());
    table->setItem (2, 1, tit);
    TQMap<TQString,TQString>::const_iterator it = applet->getParams().begin();
    const TQMap<TQString,TQString>::const_iterator itEnd = applet->getParams().end();
    for (int count = 2; it != itEnd; ++it) {
        tit = new TQTableItem (table, TQTableItem::Always, it.key ());
        table->setItem (++count, 0, tit);
        tit = new TQTableItem(table, TQTableItem::Always, it.data ());
        table->setItem (count, 1, tit);
    }
    setMainWidget (table);
}

void AppletParameterDialog::slotClose () {
    table->selectCells (0, 0, 0, 0);
    KJavaApplet* const applet = m_appletWidget->applet ();
    applet->setAppletClass (table->item (0, 1)->text ());
    applet->setBaseURL (table->item (1, 1)->text ());
    applet->setArchives (table->item (2, 1)->text ());
    const int lim = table->numRows();
    for (int i = 3; i < lim; ++i) {
        if (table->item (i, 0) && table->item (i, 1) && !table->item (i, 0)->text ().isEmpty ())
            applet->setParameter (table->item (i, 0)->text (),
                                  table->item (i, 1)->text ());
    }
    hide ();
}
//-----------------------------------------------------------------------------

class CoverWidget : public TQWidget {
    KJavaAppletWidget * m_appletwidget;
public:
    CoverWidget (TQWidget *);
    ~CoverWidget () {}
    KJavaAppletWidget * appletWidget () const;
protected:
    void resizeEvent (TQResizeEvent * e);
};

inline CoverWidget::CoverWidget (TQWidget * parent)
 : TQWidget (parent, "KJavaAppletViewer Widget")
{
    m_appletwidget = new KJavaAppletWidget (this);
    setFocusProxy (m_appletwidget);
}

inline KJavaAppletWidget * CoverWidget::appletWidget () const {
    return m_appletwidget;
}

void CoverWidget::resizeEvent (TQResizeEvent * e) {
    m_appletwidget->resize (e->size().width(), e->size().height());
}

//-----------------------------------------------------------------------------

class StatusBarIcon : public TQLabel {
public:
    StatusBarIcon (TQWidget * parent) : TQLabel (parent) {
        setPixmap (SmallIcon (TQString ("java"), KJavaAppletViewerFactory::instance ()));
    }
protected:
    void mousePressEvent (TQMouseEvent *) {
        serverMaintainer->server->showConsole ();
    }
};

//-----------------------------------------------------------------------------

KJavaAppletViewer::KJavaAppletViewer (TQWidget * wparent, const char *,
                 TQObject * parent, const char * name, const TQStringList & args)
 : KParts::ReadOnlyPart (parent, name),
   m_browserextension (new KJavaAppletViewerBrowserExtension (this)),
   m_liveconnect (new KJavaAppletViewerLiveConnectExtension (this)),
   m_statusbar (new KParts::StatusBarExtension (this)),
   m_statusbar_icon (0L),
   m_closed (true)
{
    if (!serverMaintainer) {
        serverMaintainerDeleter.setObject (serverMaintainer,
                                           new KJavaServerMaintainer);
    }
    m_view = new CoverWidget (wparent);
    TQString classname, classid, codebase, tdehtml_codebase, src_param;
    int width = -1;
    int height = -1;
    KJavaApplet* const applet = m_view->appletWidget()->applet ();
    TQStringList::const_iterator it = args.begin();
    const TQStringList::const_iterator itEnd = args.end();
    for ( ; it != itEnd; ++it) {
        const int equalPos = (*it).find("=");
        if (equalPos > 0) {
            const TQString name = (*it).left (equalPos).upper ();
            TQString value = (*it).right ((*it).length () - equalPos - 1);
            if (value.at(0)=='\"')
                value = value.right (value.length () - 1);
            if (value.at (value.length () - 1) == '\"')
                value.truncate (value.length () - 1);
            kdDebug(6100) << "name=" << name << " value=" << value << endl;
            if (!name.isEmpty()) {
                const TQString name_lower = name.lower ();
                if (name == "__TDEHTML__PLUGINBASEURL") {
                    baseurl = KURL (KURL (value), TQString (".")).url ();
                } else if (name == "__TDEHTML__CODEBASE")
                    tdehtml_codebase = value;
                else if (name_lower == TQString::fromLatin1("codebase") ||
                         name_lower == TQString::fromLatin1("java_codebase")) {
                    if (!value.isEmpty ())
                        codebase = value;
                } else if (name == "__TDEHTML__CLASSID")
                //else if (name.lower()==TQString::fromLatin1("classid"))
                    classid = value;
                else if (name_lower == TQString::fromLatin1("code") ||
                         name_lower == TQString::fromLatin1("java_code"))
                    classname = value;
                else if (name_lower == TQString::fromLatin1("src"))
                    src_param = value;
                else if (name_lower == TQString::fromLatin1("archive") ||
                         name_lower == TQString::fromLatin1("java_archive") ||
                         name_lower.startsWith ("cache_archive"))
                    applet->setArchives (value);
                else if (name_lower == TQString::fromLatin1("name"))
                    applet->setAppletName (value);
                else if (name_lower == TQString::fromLatin1("width"))
                    width = value.toInt();
                else if (name_lower == TQString::fromLatin1("height"))
                    height = value.toInt();
                if (!name.startsWith ("__TDEHTML__")) {
                    applet->setParameter (name, value);
                }
            }
        }
    }
    if (!classid.isEmpty ()) {
        applet->setParameter ("CLSID", classid);
        kdDebug(6100) << "classid=" << classid << classid.startsWith("clsid:")<< endl;
        if (classid.startsWith ("clsid:"))
            // codeBase contains the URL to the plugin page
            tdehtml_codebase = baseurl;
        else if (classname.isEmpty () && classid.startsWith ("java:"))
            classname = classid.mid(5);
    }
    if (classname.isEmpty ())
        classname = src_param;
    else if (!src_param.isEmpty ())
        applet->setParameter (TQString ("SRC"), src_param);
    if (codebase.isEmpty ())
        codebase = tdehtml_codebase;
    if (baseurl.isEmpty ()) {
        // not embeded in tdehtml
        TQString pwd = TQDir().absPath ();
        if (!pwd.endsWith (TQChar (TQDir::separator ())))
            pwd += TQDir::separator ();
        baseurl = KURL (KURL (pwd), codebase).url ();
    }
    if (width > 0 && height > 0) {
        m_view->resize (width, height);
        applet->setSize( TQSize( width, height ) );
    }
    applet->setBaseURL (baseurl);
    // check codebase first
    const KURL kbaseURL( baseurl );
    const KURL newURL(kbaseURL, codebase);
    if (kapp->authorizeURLAction("redirect", KURL(baseurl), newURL))
        applet->setCodeBase (newURL.url());
    applet->setAppletClass (classname);
    KJavaAppletContext* const cxt = serverMaintainer->getContext (parent, baseurl);
    applet->setAppletContext (cxt);

    KJavaAppletServer* const server = cxt->getServer ();

    serverMaintainer->setServer (server);

    if (!server->usingKIO ()) {
        /* if this page needs authentication */
        TDEIO::AuthInfo info;
        TQString errorMsg;
        TQCString replyType;
        TQByteArray params;
        TQByteArray reply;
        TDEIO::AuthInfo authResult;

        //(void) dcopClient(); // Make sure to have a dcop client.
        info.url = baseurl;
        info.verifyPath = true;

        TQDataStream stream(params, IO_WriteOnly);
        stream << info << m_view->topLevelWidget()->winId();

        if (!kapp->dcopClient ()->call( "kded", "kpasswdserver", "checkAuthInfo(TDEIO::AuthInfo, long int)", params, replyType, reply ) ) {
            kdWarning() << "Can't communicate with kded_kpasswdserver!" << endl;
        } else if ( replyType == "TDEIO::AuthInfo" ) {
            TQDataStream stream2( reply, IO_ReadOnly );
            stream2 >> authResult;
            applet->setUser (authResult.username);
            applet->setPassword (authResult.password);
            applet->setAuthName (authResult.realmValue);
        }
    }

    /* install event filter for close events */
    if (wparent)
        wparent->topLevelWidget ()->installEventFilter (this);

    setInstance (KJavaAppletViewerFactory::instance ());
    KParts::Part::setWidget (m_view);

    connect (applet->getContext(), TQ_SIGNAL(appletLoaded()), this, TQ_SLOT(appletLoaded()));
    connect (applet->getContext(), TQ_SIGNAL(showDocument(const TQString&, const TQString&)), m_browserextension, TQ_SLOT(showDocument(const TQString&, const TQString&)));
    connect (applet->getContext(), TQ_SIGNAL(showStatus(const TQString &)), this, TQ_SLOT(infoMessage(const TQString &)));
    connect (applet, TQ_SIGNAL(jsEvent (const TQStringList &)), m_liveconnect, TQ_SLOT(jsEvent (const TQStringList &)));
}

bool KJavaAppletViewer::eventFilter (TQObject *o, TQEvent *e) {
    if (m_liveconnect->jsSessions () > 0) {
        switch (e->type()) {
            case TQEvent::Destroy:
            case TQEvent::Close:
            case TQEvent::Quit:
                return true;
            default:
                break;
        }
    }
    return KParts::ReadOnlyPart::eventFilter(o,e);
}

KJavaAppletViewer::~KJavaAppletViewer () {
    m_view = 0L;
    serverMaintainer->releaseContext (parent(), baseurl);
    if (m_statusbar_icon) {
        m_statusbar->removeStatusBarItem (m_statusbar_icon);
        delete m_statusbar_icon;
    }
}

bool KJavaAppletViewer::openURL (const KURL & url) {
    if (!m_view) return false;
    m_closed = false;
    KJavaAppletWidget* const w = m_view->appletWidget ();
    KJavaApplet* const applet = w->applet ();
    if (applet->isCreated ())
        applet->stop ();
    if (applet->appletClass ().isEmpty ()) {
        // preview without setting a class?
        if (applet->baseURL ().isEmpty ()) {
            applet->setAppletClass (url.fileName ());
            applet->setBaseURL (url.upURL ().url ());
        } else
            applet->setAppletClass (url.url ());
        AppletParameterDialog (w).exec ();
        applet->setSize (w->sizeHint());
    }
    if (!m_statusbar_icon) {
        KStatusBar *sb = m_statusbar->statusBar();
        if (sb) {
            m_statusbar_icon = new StatusBarIcon (sb);
            m_statusbar->addStatusBarItem (m_statusbar_icon, 0, false);
        }
    }
    // delay showApplet if size is unknown and m_view not shown
    if (applet->size().width() > 0 || m_view->isVisible())
        w->showApplet ();
    else
        TQTimer::singleShot (10, this, TQ_SLOT (delayedCreateTimeOut ()));
    if (!applet->failed ())
        emit started (0L);
    return url.isValid ();
}

bool KJavaAppletViewer::closeURL () {
    kdDebug(6100) << "closeURL" << endl;
    m_closed = true;
    KJavaApplet* const applet = m_view->appletWidget ()->applet ();
    if (applet->isCreated ())
        applet->stop ();
    applet->getContext()->getServer()->endWaitForReturnData();
    return true;
}

bool KJavaAppletViewer::appletAlive () const {
    return !m_closed && m_view &&
           m_view->appletWidget ()->applet () &&
           m_view->appletWidget ()->applet ()->isAlive ();
}

bool KJavaAppletViewer::openFile () {
    return false;
}

void KJavaAppletViewer::delayedCreateTimeOut () {
    KJavaAppletWidget* const w = m_view->appletWidget ();
    if (!w->applet ()->isCreated () && !m_closed)
        w->showApplet ();
}

void KJavaAppletViewer::appletLoaded () {
    if (!m_view) return;
    KJavaApplet* const applet = m_view->appletWidget ()->applet ();
    if (applet->isAlive() || applet->failed())
        emit completed();
}

void KJavaAppletViewer::infoMessage (const TQString & msg) {
    m_browserextension->infoMessage(msg);
}

TDEAboutData* KJavaAppletViewer::createAboutData () {
    return new TDEAboutData("KJavaAppletViewer", I18N_NOOP("TDE Java Applet Plugin"), "1.0");
}

//---------------------------------------------------------------------

KJavaAppletViewerBrowserExtension::KJavaAppletViewerBrowserExtension (KJavaAppletViewer * parent)
  : KParts::BrowserExtension (parent, "KJavaAppletViewer Browser Extension") {
}

void KJavaAppletViewerBrowserExtension::urlChanged (const TQString & url) {
    emit setLocationBarURL (url);
}

void KJavaAppletViewerBrowserExtension::setLoadingProgress (int percentage) {
    emit loadingProgress (percentage);
}

void KJavaAppletViewerBrowserExtension::setURLArgs (const KParts::URLArgs & /*args*/) {
}

void KJavaAppletViewerBrowserExtension::saveState (TQDataStream & stream) {
    KJavaApplet* const applet = static_cast<KJavaAppletViewer*>(parent())->view()->appletWidget ()->applet ();
    stream << applet->appletClass();
    stream << applet->baseURL();
    stream << applet->archives();
    stream << applet->getParams().size ();
    TQMap<TQString,TQString>::const_iterator it = applet->getParams().begin();
    const TQMap<TQString,TQString>::const_iterator itEnd = applet->getParams().end();
    for ( ; it != itEnd; ++it) {
        stream << it.key ();
        stream << it.data ();
    }
}

void KJavaAppletViewerBrowserExtension::restoreState (TQDataStream & stream) {
    KJavaAppletWidget* const w = static_cast<KJavaAppletViewer*>(parent())->view()->appletWidget();
    KJavaApplet* const applet = w->applet ();
    TQString key, val;
    int paramcount;
    stream >> val;
    applet->setAppletClass (val);
    stream >> val;
    applet->setBaseURL (val);
    stream >> val;
    applet->setArchives (val);
    stream >> paramcount;
    for (int i = 0; i < paramcount; ++i) {
        stream >> key;
        stream >> val;
        applet->setParameter (key, val);
        kdDebug(6100) << "restoreState key:" << key << " val:" << val << endl;
    }
    applet->setSize (w->sizeHint ());
    if (w->isVisible())
        w->showApplet ();
}

void KJavaAppletViewerBrowserExtension::showDocument (const TQString & doc,
                                                      const TQString & frame) {
    const KURL url (doc);
    KParts::URLArgs args;
    args.frameName = frame;
    emit openURLRequest (url, args);
}

//-----------------------------------------------------------------------------

KJavaAppletViewerLiveConnectExtension::KJavaAppletViewerLiveConnectExtension(KJavaAppletViewer * parent)
    : KParts::LiveConnectExtension (parent, "KJavaAppletViewer LiveConnect Extension"), m_viewer (parent) {
}

bool KJavaAppletViewerLiveConnectExtension::get (
        const unsigned long objid, const TQString & name,
        KParts::LiveConnectExtension::Type & type,
        unsigned long & rid, TQString & value)
{
    if (!m_viewer->appletAlive ())
        return false;
    TQStringList args, ret_args;
    KJavaApplet* const applet = m_viewer->view ()->appletWidget ()->applet ();
    args.append (TQString::number (applet->appletId ()));
    args.append (TQString::number ((int) objid));
    args.append (name);
    m_jssessions++;
    const bool ret = applet->getContext()->getMember (args, ret_args);
    m_jssessions--;
    if (!ret || ret_args.count() != 3) return false;
    bool ok;
    int itype = ret_args[0].toInt (&ok);
    if (!ok || itype < 0) return false;
    type = (KParts::LiveConnectExtension::Type) itype;
    rid = ret_args[1].toInt (&ok);
    if (!ok) return false;
    value = ret_args[2];
    return true;
}

bool KJavaAppletViewerLiveConnectExtension::put(const unsigned long objid, const TQString & name, const TQString & value)
{
    if (!m_viewer->appletAlive ())
        return false;
    TQStringList args;
    KJavaApplet* const applet = m_viewer->view ()->appletWidget ()->applet ();
    args.append (TQString::number (applet->appletId ()));
    args.append (TQString::number ((int) objid));
    args.append (name);
    args.append (value);
    ++m_jssessions;
    const bool ret = applet->getContext()->putMember (args);
    --m_jssessions;
    return ret;
}

bool KJavaAppletViewerLiveConnectExtension::call( const unsigned long objid, const TQString & func, const TQStringList & fargs, KParts::LiveConnectExtension::Type & type, unsigned long & retobjid, TQString & value )
{
    if (!m_viewer->appletAlive ())
        return false;
    KJavaApplet* const applet = m_viewer->view ()->appletWidget ()->applet ();
    TQStringList args, ret_args;
    args.append (TQString::number (applet->appletId ()));
    args.append (TQString::number ((int) objid));
    args.append (func);
    args.append (TQString::number ((int) fargs.size ()));
    {
        TQStringList::const_iterator it = fargs.begin();
        const TQStringList::const_iterator itEnd = fargs.end();
	for ( ; it != itEnd; ++it)
            args.append(*it);
    }

    ++m_jssessions;
    const bool ret = applet->getContext()->callMember (args, ret_args);
    --m_jssessions;
    if (!ret || ret_args.count () != 3) return false;
    bool ok;
    const int itype = ret_args[0].toInt (&ok);
    if (!ok || itype < 0) return false;
    type = (KParts::LiveConnectExtension::Type) itype;
    retobjid = ret_args[1].toInt (&ok);
    if (!ok) return false;
    value = ret_args[2];
    return true;
}

void KJavaAppletViewerLiveConnectExtension::unregister(const unsigned long objid)
{
    if (!m_viewer->view () || !m_viewer->view ())
        return;
    KJavaApplet* const applet = m_viewer->view ()->appletWidget ()->applet ();
    if (!applet || objid == 0) {
        // typically a gc after a function call on the applet,
        // no need to send to the jvm
        return;
    }
    TQStringList args;
    args.append (TQString::number (applet->appletId ()));
    args.append (TQString::number ((int) objid));
    applet->getContext()->derefObject (args);
}

void KJavaAppletViewerLiveConnectExtension::jsEvent (const TQStringList & args) {
    if (args.count () < 2 || !m_viewer->appletAlive ())
        return;
    bool ok;
    TQStringList::ConstIterator it = args.begin();
    const TQStringList::ConstIterator itEnd = args.end();
    const unsigned long objid = (*it).toInt(&ok);
    ++it;
    const TQString event = (*it);
    ++it;
    KParts::LiveConnectExtension::ArgList arglist;

    for (; it != itEnd; ++it) {
        // take a deep breath here
        const TQStringList::ConstIterator prev = it++;
	arglist.push_back(KParts::LiveConnectExtension::ArgList::value_type((KParts::LiveConnectExtension::Type) (*prev).toInt(), (*it)));
    }
    emit partEvent (objid, event, arglist);
}

int KJavaAppletViewerLiveConnectExtension::m_jssessions = 0;

//-----------------------------------------------------------------------------

#include "kjavaappletviewer.moc"
