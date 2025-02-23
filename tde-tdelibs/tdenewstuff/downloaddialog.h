/*
    This file is part of TDENewStuff.
    Copyright (c) 2003 Josef Spillner <spillner@kde.org>

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
#ifndef KNEWSTUFF_DOWNLOADDIALOG_H
#define KNEWSTUFF_DOWNLOADDIALOG_H

#include <kdialogbase.h>
#include <tdenewstuff/provider.h>

namespace TDEIO
{
  class Job;
}

class TDEListView;
class TQTextBrowser;
class TQFrame;
class TDENewStuffGeneric;

namespace KNS
{

class ProviderLoader;
class Entry;
class Provider;
class Engine;

/**
 * @short Common download dialog for data browsing and installation.
 *
 * It provides an easy-to-use convenience method named open() which does all
 * the work, unless a more complex operation is needed.
 * \code
 * TDENewStuff::DownloadDialog::open("kdesktop/wallpapers");
 * \endcode
 *
 * @author Josef Spillner (spillner@kde.org)
 * \par Maintainer:
 * Josef Spillner (spillner@kde.org)
 */
class TDE_EXPORT DownloadDialog : public KDialogBase
{
    TQ_OBJECT
    struct Private;
  public:
    /**
      Constructor.

      @param engine a pre-built engine object, or NULL if the download
                    dialog should create an engine on its own
      @param parent the parent window
      @param caption the dialog caption
    */
    DownloadDialog(Engine *engine, TQWidget *parent, const TQString& caption);

    /**
      Alternative constructor.
      Always uses an internal engine.

      @param parent the parent window
      @param caption the dialog caption
    */
    DownloadDialog(TQWidget *parent, const TQString& caption);

    /**
      Destructor.
    */
    ~DownloadDialog();

    /**
      Restricts the display of available data to a certain data type.

      @param type a Hotstuff data type such as "korganizer/calendar"
    */
    void setType(TQString type);
	// ### KDE 4.0: use const TQString&

    /**
      Fetches descriptions of all available data, optionally considering
      a previously set type.
    */
    void load();

    /**
      Explicitly uses this provider list instead of the one read from
      the application configuration.

      @param providerList the URL of the provider list

      @since 3.4
    */
    void setProviderList(const TQString& providerList);

    /**
      Fetches descriptions of all available data, optionally considering
      a previously set type.

      @param providerList the URl to the list of providers; if empty
             we first try the ProvidersUrl from TDEGlobal::config, then we
             fall back to a hardcoded value.
    */
    void load(TQString providerList); // KDE4: merge with load() above

    /**
      Adds another provider to the download dialog.
      This is normally done internally.

      @param p the Hotstuff provider to be added
    */
    void addProvider(Provider *p);

    /**
      Adds an additional entry to the current provider.
      This is normally done internally.

      @param entry a Hotstuff data entry to be added
    */
    void addEntry(Entry *entry);

    /**
      Adds an additional entry to the current provider.
      This is normally done internal.
      This version takes into accounts the download variant.

      @param entry a Hotstuff data entry to be added
      @param variants all variants this entry is intended for
    */
    void addEntry(Entry *entry, const TQStringList& variants);

    /**
      Clears the entry list of the current provider.
      This is normally done internally.
    */
    void clear();

    /**
      Constructor.

      @param engine a pre-built engine object, or NULL if the download
                    dialog should create an engine on its own
      @param parent the parent window
    */
    DownloadDialog(Engine *engine, TQWidget *parent = 0);
    // ### KDE 4.0: remove and make caption/parent argument optional

    /**
      Alternative constructor.
      Always uses an internal engine.

      @param parent the parent window
    */
    DownloadDialog(TQWidget *parent = 0);
    // ### KDE 4.0: remove and make caption/parent argument optional

    /**
      Opens the download dialog.
      This is a convenience method which automatically sets up the dialog.
      @see setType()
      @see load()

      @param type a data type such as "korganizer/calendar"
      @param caption the dialog caption
    */
    static void open(const TQString& type, const TQString& caption);

    /**
      Opens the download dialog.
      This is a convenience method which automatically sets up the dialog.
      @see setType()
      @see load()

      @param type a data type such as "korganizer/calendar"
      @deprecated use open( const TQString& type, const TQString& caption );
    */
    static void open(TQString type) TDE_DEPRECATED; // ### KDE 4.0: remove and make caption/parent argument optional

    /**
      Returns the list of installed data entries.

      @return list of data entries which have been installed
    */
    TQPtrList<Entry> installedEntries();
    // ### KDE 4.0: the open() method should return this

  public slots:
    /**
      Availability of the provider list.

      @param list list of Hotstuff providers
    */
    void slotProviders(Provider::List *list);

  protected slots:
    void slotApply();
    void slotOk();

  private slots:
    void slotResult(TDEIO::Job *job);
    void slotData(TDEIO::Job *job, const TQByteArray &a);
    void slotJobData( TDEIO::Job *, const TQByteArray & );
    void slotJobResult( TDEIO::Job * );
    void slotInstall();
    void slotInstallPhase2();
    void slotDetails();
    void slotInstalled(TDEIO::Job *job);
    void slotTab();
    void slotSelected();
    void slotPage(TQWidget *w);
    void slotFinish();
    void slotEmail(const TQString& link);

  private:
    void init(Engine *e);
    Entry *getEntry();
    void loadProvider(Provider *p);
    void install(Entry *e);
    int installStatus(Entry *e);
    TQListViewItem *currentEntryItem();

    ProviderLoader *m_loader;
    TQString m_entryname;
    TDEListView *lv_r, *lv_d, *lv_l;
    TQTextBrowser *m_rt;
    TQFrame *m_frame;
    TQListViewItem *m_entryitem;
    TQPtrList<Entry> m_entries;
    Entry *m_entry;
    TDENewStuffGeneric *m_s;
    int m_curtab;
    TQMap<TQWidget*, TQValueList<TDEListView*>* > m_map;
    TQMap<TQWidget*, Provider*> m_providers;
    TQMap<TQWidget*, TQTextBrowser*> m_rts;
    TQMap<TQWidget*, TQValueList<TQPushButton*>* > m_buttons;
    TQMap<TDEIO::Job*, Provider*> m_jobs;
    TQMap<TDEIO::Job*, TQString> m_data;
    TQString m_filter;
    TQString mJobData;
    Engine *m_engine;
    Private *d;
};

}

#endif

