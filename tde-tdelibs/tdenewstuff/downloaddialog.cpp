/*
    This file is part of TDENewStuff.
    Copyright (c) 2003 Josef Spillner <spillner@kde.org>
    Copyright (c) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "downloaddialog.h"
#include "downloaddialog.moc"

#include <tdelocale.h>
#include <tdelistview.h>
#include <kdebug.h>
#include <tdeio/job.h>
#include <tdeio/netaccess.h>
#include <tdemessagebox.h>
#include <kurl.h>
#include <tdeconfig.h>
#include <tdeapplication.h>
#include <kiconloader.h>

#include <tdenewstuff/entry.h>
#include <tdenewstuff/knewstuffgeneric.h>
#include <tdenewstuff/engine.h>

#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqdom.h>
#include <tqlabel.h>
#include <tqtextbrowser.h>
#include <tqtabwidget.h>
#include <tqtimer.h> // hack

#define OPENDESKTOP_REDIRECT_URL "opendesktop.org/content/download.php?content="
#define OPENDESKTOP_REDIRECT_TEXT "If the download does not start in 3 seconds:</span><span class=\"defaulttext\">&nbsp;<a href=\""

using namespace KNS;

struct DownloadDialog::Private
{
    TQString m_providerlist;
    TQWidget *m_page;
    TQFrame *m_loadingFrame;
    TQLabel *m_loadingLabel;
    TDEListView *m_lvtmp_r, *m_lvtmp_d, *m_lvtmp_l;
    TQPtrList<Entry> m_installlist;
    TQMap<TDEIO::Job*, Provider*> m_variantjobs;
    TQMap<TDEIO::Job*, TQStringList> m_variants;
    TQMap<Provider*, Provider*> m_newproviders;
};

class NumSortListViewItem : public TDEListViewItem
{
  public:
  NumSortListViewItem( TQListView * parent, TQString label1, TQString label2 = TQString::null, TQString label3 = TQString::null, TQString label4 = TQString::null, TQString label5 = TQString::null, TQString label6 = TQString::null, TQString label7 = TQString::null, TQString label8 = TQString::null )  :
  TDEListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 )
  {
  }

  TQString key(int col, bool asc) const {
    if (col == 2)
    {
      TQString s;
      s.sprintf("%08d", text(col).toInt());
      return s;
    }
    return TDEListViewItem::key( col, asc );
  }
};

class DateSortListViewItem : public TDEListViewItem
{
  public:
  DateSortListViewItem( TQListView * parent, TQString label1, TQString label2 = TQString::null, TQString label3 = TQString::null, TQString label4 = TQString::null, TQString label5 = TQString::null, TQString label6 = TQString::null, TQString label7 = TQString::null, TQString label8 = TQString::null )  :
  TDEListViewItem( parent, label1, label2, label3, label4, label5, label6, label7, label8 )
  {
  }

  TQString key(int col, bool asc) const {
    if (col == 2)
    {
      TQString s;
      TQDate date = TDEGlobal::locale()->readDate(text(col));
      s.sprintf("%08d", date.year() * 366 + date.dayOfYear());
      return s;
    }
    return TDEListViewItem::key( col, asc );
  }
};

// BEGIN deprecated for KDE 4
DownloadDialog::DownloadDialog(Engine *engine, TQWidget *)
: KDialogBase(KDialogBase::IconList, i18n("Get Hot New Stuff"),
  KDialogBase::Close, KDialogBase::Close)
{
  init(engine);
}

DownloadDialog::DownloadDialog(TQWidget *)
: KDialogBase(KDialogBase::IconList, i18n("Get Hot New Stuff"),
  KDialogBase::Close, KDialogBase::Close)
{
  init(0);
}

void DownloadDialog::open(TQString type)
{
  DownloadDialog d;
  d.setType(type);
  d.load();
  d.exec();
}
// END deprecated for KDE 4

DownloadDialog::DownloadDialog(Engine *engine, TQWidget *, const TQString& caption)
: KDialogBase(KDialogBase::IconList, (caption.isNull() ? i18n("Get Hot New Stuff") : caption),
  KDialogBase::Close, KDialogBase::Close)
{
  init(engine);
}

DownloadDialog::DownloadDialog(TQWidget *, const TQString& caption)
: KDialogBase(KDialogBase::IconList, (caption.isNull() ? i18n("Get Hot New Stuff") : caption),
  KDialogBase::Close, KDialogBase::Close)
{
  init(0);
}

void DownloadDialog::init(Engine *engine)
{
  resize(700, 400);
  d = new Private();

  m_engine = engine;
  d->m_page = NULL;

  // Provide graphical feedback to the user while the intial provider load is taking place
  d->m_loadingFrame = addPage(i18n("Welcome"), i18n("Welcome"), TQPixmap(TQString("")));
  d->m_loadingLabel = new TQLabel(d->m_loadingFrame);
  d->m_loadingLabel->setText(i18n("Loading data providers..."));
  TQVBoxLayout *box = new TQVBoxLayout(d->m_loadingFrame);
  box->add(d->m_loadingLabel);
  box->addItem(new TQSpacerItem(0, 0, TQSizePolicy::MinimumExpanding, TQSizePolicy::MinimumExpanding));

  connect(this, TQ_SIGNAL(aboutToShowPage(TQWidget*)), TQ_SLOT(slotPage(TQWidget*)));

  if(!engine)
  {
    m_loader = new ProviderLoader(this);
    connect(m_loader, TQ_SIGNAL(providersLoaded(Provider::List*)), TQ_SLOT(slotProviders(Provider::List*)));
  }

  m_entries.setAutoDelete(true);
}

DownloadDialog::~DownloadDialog()
{
    for (TQMap<TQWidget *, TQValueList<TQPushButton *>* >::const_iterator it = m_buttons.constBegin(); it != m_buttons.constEnd(); ++it)
        delete it.data();
    for (TQMap<TQWidget *, TQValueList<TDEListView *>* >::const_iterator it = m_map.constBegin(); it != m_map.constEnd(); ++it)
        delete it.data();
    delete d;
}

void DownloadDialog::load()
{
  m_loader->load(m_filter, d->m_providerlist);
}

void DownloadDialog::load(TQString providerList)
{
  m_loader->load(m_filter, providerList);
}

void DownloadDialog::clear()
{
  TQMap<TQWidget*, TQValueList<TDEListView*>* >::Iterator it;
  TQMap<TQWidget*, TQValueList<TDEListView*>* >::Iterator end(m_map.end());
  for(it = m_map.begin(); it != end; ++it)
  {
    TQValueList<TDEListView*> *v = it.data();
    kdDebug() << "clear listviews in " << v << endl;
    if(v)
    {
      (*(v->at(0)))->clear();
      (*(v->at(1)))->clear();
      (*(v->at(2)))->clear();

      //delete (*it);
    }

    delete it.key();
  }
  m_map.clear();
}

void DownloadDialog::slotProviders(Provider::List *list)
{
  d->m_loadingLabel->setText(i18n("Loading data listings..."));

  Provider *p;
  /*TQFrame *frame;*/

  for(p = list->first(); p; p = list->next())
  {
    kdDebug() << "++ provider ++ " << p->name() << endl;

    if(!m_filter.isEmpty())
      loadProvider(p);
    else
      addProvider(p);
    /*if(p == list->getFirst())
      slotPage(m_frame);*/ // only if !qtbug
  }
}

void DownloadDialog::addProvider(Provider *p)
{
  TQFrame *frame;
  TQTabWidget *ctl;
  TQWidget *w_d, *w_r, *w_l;
  TQTextBrowser *rt;
  TQString tmp;
  int ret;
  TQPixmap pix;

  kdDebug() << "addProvider()/begin" << endl;

  ret = true;
  if(p->icon().path().isEmpty()) ret = false;
  else
  {
    if(!p->icon().protocol().isEmpty())
    {
      ret = TDEIO::NetAccess::download(p->icon(), tmp, this);
      if(ret) pix = TQPixmap(tmp);
    }
    else
    {
      pix = TDEGlobal::iconLoader()->loadIcon(p->icon().path(), TDEIcon::Panel);
      ret = true;
    }
  }
  if(!ret) pix = TDEGlobal::iconLoader()->loadIcon("knewstuff", TDEIcon::Panel);
  if (d->m_loadingFrame) {
    delete d->m_loadingFrame;
    d->m_loadingFrame = NULL;
  }
  frame = addPage(p->name(), p->name(), pix);
  m_frame = frame;

  w_d = new TQWidget(frame);
  w_r = new TQWidget(frame);
  w_l = new TQWidget(frame);

  ctl = new TQTabWidget(frame);
  ctl->addTab(w_r, i18n("Highest Rated"));
  ctl->addTab(w_d, i18n("Most Downloads"));
  ctl->addTab(w_l, i18n("Latest"));

  m_curtab = 0;
  connect(ctl, TQ_SIGNAL(currentChanged(TQWidget *)), TQ_SLOT(slotTab()));

  TQHBoxLayout *box = new TQHBoxLayout(frame);
  box->add(ctl);

  d->m_lvtmp_r = new TDEListView(w_r);
  d->m_lvtmp_r->addColumn(i18n("Name"));
  d->m_lvtmp_r->addColumn(i18n("Version"));
  d->m_lvtmp_r->addColumn(i18n("Rating"));
  d->m_lvtmp_r->setSorting(2, false);

  d->m_lvtmp_d = new TDEListView(w_d);
  d->m_lvtmp_d->addColumn(i18n("Name"));
  d->m_lvtmp_d->addColumn(i18n("Version"));
  d->m_lvtmp_d->addColumn(i18n("Downloads"));
  d->m_lvtmp_d->setSorting(2, false);

  d->m_lvtmp_l = new TDEListView(w_l);
  d->m_lvtmp_l->addColumn(i18n("Name"));
  d->m_lvtmp_l->addColumn(i18n("Version"));
  d->m_lvtmp_l->addColumn(i18n("Release Date"));
  d->m_lvtmp_l->setSorting(2, false);

  connect(d->m_lvtmp_r, TQ_SIGNAL(clicked(TQListViewItem*)), TQ_SLOT(slotSelected()));
  connect(d->m_lvtmp_d, TQ_SIGNAL(clicked(TQListViewItem*)), TQ_SLOT(slotSelected()));
  connect(d->m_lvtmp_l, TQ_SIGNAL(clicked(TQListViewItem*)), TQ_SLOT(slotSelected()));

  rt = new TQTextBrowser(frame);
  rt->setMinimumWidth(150);

  TQPushButton *in = new TQPushButton(i18n("Install"), frame);
  TQPushButton *de = new TQPushButton(i18n("Details"), frame);
  in->setEnabled(false);
  de->setEnabled(false);

  box->addSpacing(spacingHint());
  TQVBoxLayout *vbox = new TQVBoxLayout(box);
  vbox->add(rt);
  vbox->addSpacing(spacingHint());
  vbox->add(de);
  vbox->add(in);

  connect(rt, TQ_SIGNAL(linkClicked(const TQString&)), TQ_SLOT(slotEmail(const TQString&)));

  connect(in, TQ_SIGNAL(clicked()), TQ_SLOT(slotInstall()));
  connect(de, TQ_SIGNAL(clicked()), TQ_SLOT(slotDetails()));

  TQVBoxLayout *box2 = new TQVBoxLayout(w_r);
  box2->add(d->m_lvtmp_r);
  TQVBoxLayout *box3 = new TQVBoxLayout(w_d);
  box3->add(d->m_lvtmp_d);
  TQVBoxLayout *box4 = new TQVBoxLayout(w_l);
  box4->add(d->m_lvtmp_l);

  TQValueList<TDEListView*> *v = new TQValueList<TDEListView*>;
  *v << d->m_lvtmp_r << d->m_lvtmp_d << d->m_lvtmp_l;
  m_map[frame] = v;
  m_rts[frame] = rt;
  TQValueList<TQPushButton*> *vb = new TQValueList<TQPushButton*>;
  *vb << in << de;
  m_buttons[frame] = vb;
  m_providers[frame] = p;

  kdDebug() << "addProvider()/end; d->m_lvtmp_r = " << d->m_lvtmp_r << endl;

  if(m_engine) slotPage(frame);

  TQTimer::singleShot(100, this, TQ_SLOT(slotFinish()));
}

void DownloadDialog::slotResult(TDEIO::Job *job)
{
  TQDomDocument dom;
  TQDomElement knewstuff;
  TQDomElement content;

  kdDebug() << "got data: " << m_data[job] << endl;

  kapp->config()->setGroup("TDENewStuffStatus");

  dom.setContent(m_data[job]);
  knewstuff = dom.documentElement();

  for(TQDomNode pn = knewstuff.firstChild(); !pn.isNull(); pn = pn.nextSibling())
  {
    TQDomElement stuff = pn.toElement();

    if(stuff.tagName() == "data")
    {
      content = pn.toElement();
    }
  }

  for(TQDomNode pn = content.firstChild(); !pn.isNull(); pn = pn.nextSibling())
  {
    TQDomElement stuff = pn.toElement();

    kdDebug() << "element: " << stuff.tagName() << endl;

    if(stuff.tagName() == "content")
    {
      Entry *entry = new Entry(stuff);
      kdDebug() << "TYPE::" << entry->type() << " FILTER::" << m_filter << endl;
      if(!entry->type().isEmpty())
      {
        if((!m_filter.isEmpty()) && (entry->type() != m_filter)) continue;
      }

      /*if((!m_filter.isEmpty()) && (m_jobs[job]))
      {
        Provider *p = m_jobs[job];
        if(d->m_newproviders[p])
        {
          addProvider(p);
          slotPage(m_frame);
          d->m_newproviders[p] = 0;
        }
      }*/
      if((!m_filter.isEmpty()) && (d->m_variantjobs[job]))
      {
        Provider *p = d->m_variantjobs[job];

        /*bool jobsActive = false;
        TQMap<TDEIO::Job*, Provider*>::Iterator it;
        for ( it = d->m_variantjobs.begin(); it != d->m_variantjobs.end(); ++it ) {
          if (it.data() == p) {
            if (it.key() != job) {
              jobsActive = true;
            }
          }
        }*/

        if(d->m_newproviders[p])
        {
          addProvider(p);
          slotPage(m_frame);
          d->m_newproviders[p] = 0;
        }
      }

      /*if(m_jobs[job]) addEntry(entry);
      else*/
      if(d->m_variantjobs[job]) {
        addEntry(entry, d->m_variants[job]);
      }
    }
  }

  if(d->m_variantjobs[job]) {
    d->m_variantjobs.remove(job);
  }
}

int DownloadDialog::installStatus(Entry *entry)
{
  TQDate date;
  TQString datestring;
  int installed;

  TQString lang = TDEGlobal::locale()->language();

  kapp->config()->setGroup("TDENewStuffStatus");
  datestring = kapp->config()->readEntry(entry->name(lang));
  if(datestring.isNull()) installed = 0;
  else
  {
    date = TQDate::fromString(datestring, TQt::ISODate);
    if(!date.isValid()) installed = 0;
    else if(date < entry->releaseDate()) installed = -1;
    else installed = 1;
  }

  return installed;
}

void DownloadDialog::addEntry(Entry *entry, const TQStringList& variants)
{
  TQPixmap pix;
  int installed;

  installed = installStatus(entry);

  if(installed > 0) pix = TDEGlobal::iconLoader()->loadIcon("ok", TDEIcon::Small);
  else if(installed < 0) pix = TDEGlobal::iconLoader()->loadIcon("history", TDEIcon::Small);
  else pix = TQPixmap();

  TQString lang = TDEGlobal::locale()->language();

  if(variants.contains("score"))
  {
    TDEListViewItem *tmp_r = new NumSortListViewItem(lv_r,
      entry->name(lang), entry->version(), TQString("%1").arg(entry->rating()));
    tmp_r->setPixmap(0, pix);
  }
  if(variants.contains("downloads"))
  {
    TDEListViewItem *tmp_d = new NumSortListViewItem(lv_d,
      entry->name(lang), entry->version(), TQString("%1").arg(entry->downloads()));
    tmp_d->setPixmap(0, pix);
  }
  if(variants.contains("latest"))
  {
    TDEListViewItem *tmp_l = new DateSortListViewItem(lv_l,
      entry->name(lang), entry->version(), TDEGlobal::locale()->formatDate(entry->releaseDate()));
    tmp_l->setPixmap(0, pix);
  }

  m_entries.append(entry);

  kdDebug() << "added entry " << entry->name() << " for variants " << variants << endl;
}

void DownloadDialog::addEntry(Entry *entry)
{
  TQStringList variants;

  variants << "score";
  variants << "downloads";
  variants << "latest";

  addEntry(entry, variants);

  // not used anymore due to variants (but still used by engine)
  kdDebug() << "added entry " << entry->name() << endl;
}

void DownloadDialog::slotData(TDEIO::Job *job, const TQByteArray &a)
{
  TQCString tmp(a, a.size() + 1);
  m_data[job].append(TQString::fromUtf8(tmp));
}

void DownloadDialog::slotDetails()
{
  Entry *e = getEntry();
  if(!e) return;

  TQString lang = TDEGlobal::locale()->language();

  TQString info = i18n
  (
    "Name: %1\n"
    "Author: %2\n"
    "License: %3\n"
    "Version: %4\n"
    "Release: %5\n"
    "Rating: %6\n"
    "Downloads: %7\n"
    "Release date: %8\n"
    "Summary: %9\n"
    ).arg(e->name(lang)
    ).arg(e->author()
    ).arg(e->license()
    ).arg(e->version()
    ).arg(e->release()
    ).arg(e->rating()
    ).arg(e->downloads()
    ).arg(TDEGlobal::locale()->formatDate(e->releaseDate())
    ).arg(e->summary(lang)
  );

  info.append(i18n
  (
    "Preview: %1\n"
    "Payload: %2\n"
    ).arg(e->preview().url()
    ).arg(e->payload().url()
  ));

  KMessageBox::information(this, info, i18n("Details"));
}

TQListViewItem *DownloadDialog::currentEntryItem()
{
  if((m_curtab == 0) && (lv_r->selectedItem())) return lv_r->selectedItem();
  if((m_curtab == 1) && (lv_d->selectedItem())) return lv_d->selectedItem();
  if((m_curtab == 2) && (lv_l->selectedItem())) return lv_l->selectedItem();

  return 0;
}

void DownloadDialog::slotInstall()
{
  Entry *e = getEntry();
  if(!e) return;

  TQPushButton *de, *in;
  in = *(m_buttons[d->m_page]->at(0));
  de = *(m_buttons[d->m_page]->at(1));

  if(in) in->setEnabled(false);
  if(de) de->setEnabled(false);

  d->m_lvtmp_r->setEnabled( false );
  d->m_lvtmp_l->setEnabled( false );
  d->m_lvtmp_d->setEnabled( false );

  kdDebug() << "download entry now" << endl;

  // OpenDesktop.org broke the basic functionality of TDEHNS by forcing
  // downloads though an advertising display page.  This in turn forces
  // the user to download and manually install the wallpaper, which
  // is relatively complex and negates much of the benefit of TDEHNS.
  // Therefore, if the download URL contains OPENDESKTOP_REDIRECT_URL
  // then download the raw HTML page and extract the real download URL for use below.
  // In the future we may want to figure out how to display unobtrusive ads
  // during the download process, but OpenDesktop.org would need to add back
  // in the direct download ability for this to even be considered.
  if (e->payload().url().contains(OPENDESKTOP_REDIRECT_URL)) {
    TDEIO::TransferJob *job = TDEIO::get( KURL( e->payload() ), false, false );
    connect( job, TQ_SIGNAL( result( TDEIO::Job * ) ),
             TQ_SLOT( slotJobResult( TDEIO::Job * ) ) );
    connect( job, TQ_SIGNAL( data( TDEIO::Job *, const TQByteArray & ) ),
             TQ_SLOT( slotJobData( TDEIO::Job *, const TQByteArray & ) ) );
  }
  else {
    slotInstallPhase2();
  }
}

void DownloadDialog::slotInstallPhase2()
{
  Entry *e = getEntry();
  if(!e) return;

  m_entryitem = currentEntryItem();
  m_entryname = m_entryitem->text(0);

  kdDebug() << "download entry now (phase 2)" << endl;

  if(m_engine)
  {
    m_engine->download(e);
    install(e);
  }
  else
  {
    m_s = new TDENewStuffGeneric(m_filter, this);
    m_entry = e;
    KURL source = e->payload();
    KURL dest = KURL(m_s->downloadDestination(e));

    TDEIO::FileCopyJob *job = TDEIO::file_copy(source, dest, -1, true);
    connect(job, TQ_SIGNAL(result(TDEIO::Job*)), TQ_SLOT(slotInstalled(TDEIO::Job*)));
  }
}

void DownloadDialog::slotJobData( TDEIO::Job *, const TQByteArray &data )
{
  kdDebug() << "DownloadDialog::slotJobData()" << endl;

  if ( data.size() == 0 ) return;

  TQCString str( data, data.size() + 1 );

  mJobData.append( TQString::fromUtf8( str ) );
}

void DownloadDialog::slotJobResult( TDEIO::Job *job )
{
  if ( job->error() ) {
    job->showErrorDialog( this );
    return;
  }

  Entry *e = getEntry();
  if(!e) return;

  // See previous note regarding OpenDesktop.org
  if (e->payload().url().contains(OPENDESKTOP_REDIRECT_URL)) {
    int pos = mJobData.find("<a href=\"/CONTENT/content-files/");
    if (pos >= 0) {
      TQString realURL = mJobData.mid(pos);
      realURL = realURL.mid(0, realURL.find("\">"));
      realURL = realURL.mid(strlen("<a href=\""));
      realURL = e->payload().protocol() + "://opendesktop.org" + realURL;
      e->setPayload(realURL);
    }
    else if ((pos = mJobData.find(OPENDESKTOP_REDIRECT_TEXT)) > 0) {
      pos = pos + strlen(OPENDESKTOP_REDIRECT_TEXT);
      TQString realURL = mJobData.mid(pos);
      realURL = realURL.mid(0, realURL.find("\">"));
      e->setPayload(realURL);
    }
  }

  // Reset for next load
  mJobData = TQString::null;

  slotInstallPhase2();
}

void DownloadDialog::install(Entry *e)
{
  kapp->config()->setGroup("TDENewStuffStatus");
  kapp->config()->writeEntry(m_entryname, TQString(e->releaseDate().toString(TQt::ISODate)));
  kapp->config()->sync();

  TQPixmap pix = TDEGlobal::iconLoader()->loadIcon("ok", TDEIcon::Small);

  TQString lang = TDEGlobal::locale()->language();

  if(m_entryitem)
  {
    m_entryitem->setPixmap(0, pix);

    TQListViewItem *item;
    item = lv_r->findItem(e->name(lang), 0);
    if(item) item->setPixmap(0, pix);
    item = lv_d->findItem(e->name(lang), 0);
    if(item) item->setPixmap(0, pix);
    item = lv_l->findItem(e->name(lang), 0);
    if(item) item->setPixmap(0, pix);
  }

  if(currentEntryItem() == m_entryitem)
  {
    TQPushButton *in;
    in = *(m_buttons[d->m_page]->at(0));
    if(in) in->setEnabled(false);
  }

  d->m_installlist.append(e);
  d->m_lvtmp_r->setEnabled( true );
  d->m_lvtmp_l->setEnabled( true );
  d->m_lvtmp_d->setEnabled( true );
}

void DownloadDialog::slotInstalled(TDEIO::Job *job)
{
  TQPushButton *de, *in;
  in = *(m_buttons[d->m_page]->at(0));
  de = *(m_buttons[d->m_page]->at(1));

  if(in) in->setEnabled(true);
  if(de) de->setEnabled(true);

  bool ret = job && (job->error() == 0);
  if(ret)
  {
    TDEIO::FileCopyJob *cjob = ::tqt_cast<TDEIO::FileCopyJob*>(job);
    if(cjob)
    {
      ret = m_s->install(cjob->destURL().path());
    }
    else ret = false;
  }

  if(ret)
  {
    install(m_entry);

    KMessageBox::information(this, i18n("Installation successful."), i18n("Installation"));
  }
  else KMessageBox::error(this, i18n("Installation failed."), i18n("Installation"));
  d->m_lvtmp_r->setEnabled( true );
  d->m_lvtmp_l->setEnabled( true );
  d->m_lvtmp_d->setEnabled( true );

  delete m_s;
}

void DownloadDialog::slotTab()
{
  int tab = static_cast<const TQTabWidget *>(sender())->currentPageIndex();
  kdDebug() << "switch tab to: " << tab << endl;

  Entry *eold = getEntry();
  m_curtab = tab;
  Entry *e = getEntry();

  if(e == eold) return;

  if(e)
  {
    slotSelected();
  }
  else
  {
    TQPushButton *de, *in;
    in = *(m_buttons[d->m_page]->at(0));
    de = *(m_buttons[d->m_page]->at(1));

    if(in) in->setEnabled(false);
    if(de) de->setEnabled(false);

    m_rt->clear();
  }
}

void DownloadDialog::slotSelected()
{
  TQString tmp;
  bool enabled;
  Entry *e = getEntry();
  TQString lang = TDEGlobal::locale()->language();
  bool ret;

  if(e)
  {
    TQString lang = TDEGlobal::locale()->language();

    TQListViewItem *item;
    if(m_curtab != 0)
    {
      lv_r->clearSelection();
      item = lv_r->findItem(e->name(lang), 0);
      if(item) lv_r->setSelected(item, true);
    }
    if(m_curtab != 1)
    {
      lv_d->clearSelection();
      item = lv_d->findItem(e->name(lang), 0);
      if(item) lv_d->setSelected(item, true);
    }
    if(m_curtab != 2)
    {
      lv_l->clearSelection();
      item = lv_l->findItem(e->name(lang), 0);
      if(item) lv_l->setSelected(item, true);
    }

    if(!e->preview(lang).isValid())
    {
      ret = 0;
    }
    else
    {
      ret = TDEIO::NetAccess::download(e->preview(lang), tmp, this);
    }

    TQString desc = TQString("<b>%1</b><br>").arg(e->name(lang));
    if(!e->authorEmail().isNull())
    {
      desc += TQString("<a href='mailto:" + e->authorEmail() + "'>" + e->author() + "</a>");
    }
    else
    {
      desc += TQString("%1").arg(e->author());
    }
    desc += TQString("<br>%1").arg(TDEGlobal::locale()->formatDate(e->releaseDate()));
    desc += TQString("<br><br>");
    if(ret)
    {
      desc += TQString("<img src='%1'>").arg(tmp);
    }
    else
    {
      desc += i18n("Preview not available.");
    }
    desc += TQString("<br><i>%1</i>").arg(e->summary(lang));
    desc += TQString("<br>(%1)").arg(e->license());

    m_rt->clear();
    m_rt->setText(desc);

    if(installStatus(e) == 1) enabled = false;
    else enabled = true;

    TQPushButton *de, *in;
    in = *(m_buttons[d->m_page]->at(0));
    de = *(m_buttons[d->m_page]->at(1));
    if(in) in->setEnabled(enabled);
    if(de) de->setEnabled(true);
  }
}

void DownloadDialog::slotEmail(const TQString& link)
{
  kdDebug() << "EMAIL: " << link << endl;
  kapp->invokeMailer(link);
  slotSelected(); // TQTextBrowser oddity workaround as it cannot handle mailto: URLs
}

Entry *DownloadDialog::getEntry()
{
  TQListViewItem *entryItem = currentEntryItem();

  if(!entryItem)
    return 0;

  TQString entryName = entryItem->text(0);

  TQString lang = TDEGlobal::locale()->language();

  for(Entry *e = m_entries.first(); e; e = m_entries.next())
    if(e->name(lang) == entryName)
      return e;

  return 0;
}

void DownloadDialog::slotPage(TQWidget *w)
{
  Provider *p;

  kdDebug() << "changed widget!!!" << endl;

  if(m_map.find(w) == m_map.end()) return;

  d->m_page = w;

  lv_r = *(m_map[w]->at(0));
  lv_d = *(m_map[w]->at(1));
  lv_l = *(m_map[w]->at(2));
  p = m_providers[w];
  m_rt = m_rts[w];

  kdDebug() << "valid change!!!; lv_r = " << lv_r << endl;

  if(m_engine) return;

  if(!m_filter.isEmpty()) return;

  lv_r->clear();
  lv_d->clear();
  lv_l->clear();

  kdDebug() << "-- fetch -- " << p->downloadUrl() << endl;

  loadProvider(p);
}

void DownloadDialog::loadProvider(Provider *p)
{
  TQMap<TDEIO::Job*, Provider*>::Iterator it;

  for(it = d->m_variantjobs.begin(); it != d->m_variantjobs.end(); it++)
  {
    if(it.data() == p)
    {
      kdDebug() << "-- found provider data in cache" << endl;
      slotResult(it.key());
      return;
    }
  }

  TQStringList variants;
  variants << "score";
  variants << "downloads";
  variants << "latest";

  // Optimise URLs so each unique URL only gets fetched once

  TQMap<TQString, TQStringList> urls;

  for(TQStringList::Iterator it = variants.begin(); it != variants.end(); it++)
  {
    TQString url = p->downloadUrlVariant((*it)).url();
    if(!urls.contains(url))
    {
      urls[url] = TQStringList();
    }
    urls[url] << (*it);

    it = variants.remove(it);
  }

  // Now fetch the URLs while keeping the variant list for each attached

  for(TQMap<TQString, TQStringList>::Iterator it = urls.begin(); it != urls.end(); it++)
  {
    TQString url = it.key();
    TQStringList urlvariants = it.data();

    TDEIO::TransferJob *variantjob = TDEIO::get(url, false, false);
    d->m_newproviders[p] = p;
    d->m_variantjobs[variantjob] = p;
    d->m_variants[variantjob] = urlvariants;
    m_data[variantjob] = "";

    connect(variantjob, TQ_SIGNAL(result(TDEIO::Job*)), TQ_SLOT(slotResult(TDEIO::Job*)));
    connect(variantjob, TQ_SIGNAL(data(TDEIO::Job*, const TQByteArray&)),
      TQ_SLOT(slotData(TDEIO::Job*, const TQByteArray&)));
  }

  if(variants.count() == 0) return;

  // If not all variants are given, use default URL for those

  kdDebug() << "-- reached old downloadurl section; variants left: " << variants.count() << endl;

  TDEIO::TransferJob *job = TDEIO::get(p->downloadUrl());

  d->m_newproviders[p] = p;
  d->m_variantjobs[job] = p;
  d->m_variants[job] = variants;
  //m_jobs[job] = p; // not used anymore due to variants
  m_data[job] = "";

  connect(job, TQ_SIGNAL(result(TDEIO::Job*)), TQ_SLOT(slotResult(TDEIO::Job*)));
  connect(job, TQ_SIGNAL(data(TDEIO::Job*, const TQByteArray&)),
    TQ_SLOT(slotData(TDEIO::Job*, const TQByteArray&)));
}

void DownloadDialog::setType(TQString type)
{
  m_filter = type;
}

void DownloadDialog::setProviderList(const TQString& providerList)
{
  d->m_providerlist = providerList;
}

void DownloadDialog::slotOk()
{
}

void DownloadDialog::slotApply()
{
}

void DownloadDialog::open(const TQString& type, const TQString& caption)
{
  DownloadDialog d(0, caption);
  d.setType(type);
  d.load();
  d.exec();
}

void DownloadDialog::slotFinish()
{
  showPage(1);
  //updateBackground();
}

TQPtrList<Entry> DownloadDialog::installedEntries()
{
  return d->m_installlist;
}
