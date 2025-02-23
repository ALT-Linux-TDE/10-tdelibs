/* This file is part of the KDE project
   Copyright (c) 2001 David Faure <david@mandrakesoft.com>
   Copyright (c) 2001 Laurent Montel <lmontel@mandrakesoft.com>

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

#include "tdefilesharedlg.h"
#include <tqvbox.h>
#include <tqlabel.h>
#include <tqdir.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <kprocess.h>
#include <kprocio.h>
#include <tdelocale.h>
#include <tdeglobalsettings.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <tdeio/tdefileshare.h>
#include <kseparator.h>
#include <tqpushbutton.h>
#include <tdeapplication.h>
#include <ksimpleconfig.h>
#include <tdemessagebox.h>

class KFileSharePropsPlugin::Private
{
public:
    TQVBox *m_vBox;
    TDEProcess *m_configProc;
    bool m_bAllShared;
    bool m_bAllUnshared;
    bool m_bAllReadOnly;
};

KFileSharePropsPlugin::KFileSharePropsPlugin( KPropertiesDialog *_props )
    : KPropsDlgPlugin( _props )
{
    d = new Private;
    d->m_vBox = _props->addVBoxPage( i18n("&Share") );
    d->m_configProc = 0;
    properties->setFileSharingPage(d->m_vBox);
    m_widget = 0L;
    init();
}

KFileSharePropsPlugin::~KFileSharePropsPlugin()
{
    if (d->m_configProc)
        d->m_configProc->detach(); // Detach to prevent that we kill the process
    delete d;
}

bool KFileSharePropsPlugin::supports( const KFileItemList& items )
{
    // Do not show dialog if in advanced mode,
    // because the advanced dialog is shown already.
    if (KFileShare::shareMode() == KFileShare::Advanced) {
        kdDebug() << "KFileSharePropsPlugin::supports: false because sharemode is advanced" << endl;
        return false;
    }

    KFileItemListIterator it( items );
    for ( ; it.current(); ++it )
    {
        bool isLocal = ( *it )->isLocalFile();
        // We only support local dirs
        if ( !(*it)->isDir() || !isLocal )
            return false;
        // And sharing the trash doesn't make sense
        if ( isLocal && (*it)->url().path( 1 ) == TDEGlobalSettings::trashPath() )
            return false;
    }
    return true;
}

void KFileSharePropsPlugin::init()
{
    // We store the main widget, so that it's possible (later) to call init()
    // more than once, to update the page if something changed (e.g. after
    // the user has been authorized)
    delete m_widget;
    m_rbShare = 0L;
    m_rbUnShare = 0L;
    m_rbSharerw = 0L;
    m_widget = new TQWidget( d->m_vBox );
    TQVBoxLayout * vbox = new TQVBoxLayout( m_widget );
    //TQHBoxLayout * hbox = new TQHBoxLayout( vbox );

    switch ( KFileShare::authorization() ) {
    case KFileShare::Authorized:
    {
        // Check if all selected dirs are in $HOME
        TQString home = TQDir::homeDirPath();
        if ( home[home.length()-1] != '/' )
            home += '/';
        bool ok = true;
        KFileItemList items = properties->items();
        // We have 3 possibilities: all shared, all unshared (ro,rw), or mixed.
        d->m_bAllShared = true;
        d->m_bAllUnshared = true;
        d->m_bAllReadOnly = true;
        KFileItemListIterator it( items );
        for ( ; it.current() && ok; ++it ) {
            TQString path = (*it)->url().path();
            // 0 => not shared
            // 1 => shared read only
            // 3 => shared writeable
            int dirStatus = KFileShare::isDirectoryShared( path );
            if ( !path.startsWith( home ) )
                ok = false;
            if ( dirStatus == 1 ) {
                d->m_bAllUnshared = false;
            }
            else if ( dirStatus == 3 ) {
                d->m_bAllUnshared = false;
                d->m_bAllReadOnly = false;
            }
            else {
                d->m_bAllReadOnly = false;
            }
        }
        if ( !ok )
        {
            vbox->addWidget( new TQLabel( i18n( "Only folders in your home folder can be shared."),
                                         m_widget ), 0 );
        }
        else
        {
            // Everything ok, show the share/unshare GUI
            vbox->setSpacing( KDialog::spacingHint() );
            vbox->setMargin( KDialog::marginHint() );

            TQButtonGroup *rbGroup = new TQButtonGroup( m_widget );
            rbGroup->hide();
            m_rbUnShare = new TQRadioButton( i18n("Not shared"), m_widget );
            connect( m_rbUnShare, TQ_SIGNAL( toggled(bool) ), TQ_SIGNAL( changed() ) );
            vbox->addWidget( m_rbUnShare, 0 );
            rbGroup->insert( m_rbUnShare );

            m_rbShare = new TQRadioButton( i18n("Shared - read only for others"), m_widget );
            connect( m_rbShare, TQ_SIGNAL( toggled(bool) ), TQ_SIGNAL( changed() ) );
            vbox->addWidget( m_rbShare, 0 );
            rbGroup->insert( m_rbShare );

            m_rbSharerw = new TQRadioButton( i18n("Shared - writeable for others"), m_widget );
            connect( m_rbSharerw, TQ_SIGNAL( toggled(bool) ), TQ_SIGNAL( changed() ) );
            vbox->addWidget( m_rbSharerw, 0 );
            rbGroup->insert( m_rbSharerw );

            //TQLabel *testlabel1 = new TQLabel(i18n("Enter Samba Share Name here"),m_widget);
            //m_leSmbShareName = new TQLineEdit(m_widget);
            //m_leSmbShareName->setMaxLength(12);

            //hbox->addWidget( testlabel1, 0 );
            //hbox->addWidget( m_leSmbShareName );
            //vbox->addLayout( hbox );

            // Activate depending on status
            if ( d->m_bAllShared )
                m_rbSharerw->setChecked(true);
            if ( d->m_bAllUnshared )
                m_rbUnShare->setChecked(true);
            if ( d->m_bAllReadOnly )
                m_rbShare->setChecked(true);

            // Some help text
            TQLabel *label = new TQLabel( i18n("Sharing this folder makes it available under Linux/UNIX (NFS) and Windows (Samba).") , m_widget );
            label->setAlignment( TQt::AlignAuto | TQt::AlignVCenter | TQt::WordBreak );
            vbox->addWidget( label, 0 );

	    KSeparator* sep=new KSeparator(m_widget);
	    vbox->addWidget( sep, 0 );
	    label = new TQLabel( i18n("You can also reconfigure file sharing authorization.") , m_widget );
            label->setAlignment( TQt::AlignAuto | TQt::AlignVCenter | TQt::WordBreak );
	    vbox->addWidget( label, 0 );
	    m_pbConfig = new TQPushButton( i18n("Configure File Sharing..."), m_widget );
	    connect( m_pbConfig, TQ_SIGNAL( clicked() ), TQ_SLOT( slotConfigureFileSharing() ) );
	    vbox->addWidget( m_pbConfig, 0, TQt::AlignHCenter );

            vbox->addStretch( 10 );
            
            if( !KFileShare::sambaActive() && !KFileShare::nfsActive())
                m_widget->setEnabled( false );
        }
    }
    break;
    case KFileShare::ErrorNotFound:
        vbox->addWidget( new TQLabel( i18n("Error running 'filesharelist'. Check if installed and in $PATH or /usr/sbin."),
                    m_widget ), 0 );
        break;
    case KFileShare::UserNotAllowed:
    {
        vbox->setSpacing( 10 );
        if (KFileShare::sharingEnabled()) {
          vbox->addWidget( new TQLabel( i18n("You need to be authorized to share folders."),
                    m_widget ), 0 );
        } else {
          vbox->addWidget( new TQLabel( i18n("File sharing is disabled."),
                    m_widget ), 0 );
        }
        TQHBoxLayout* hBox = new TQHBoxLayout( (TQWidget *)0L );
        vbox->addLayout( hBox, 0 );
        m_pbConfig = new TQPushButton( i18n("Configure File Sharing..."), m_widget );
        connect( m_pbConfig, TQ_SIGNAL( clicked() ), TQ_SLOT( slotConfigureFileSharing() ) );
        hBox->addWidget( m_pbConfig, 0, TQt::AlignHCenter );
        vbox->addStretch( 10 ); // align items on top
        break;
    }
    case KFileShare::NotInitialized:
        kdWarning() << "KFileShare Authorization still NotInitialized after calling authorization() - impossible" << endl;
        break;
    }
    m_widget->show(); // In case the dialog was shown already.
}

void KFileSharePropsPlugin::slotConfigureFileSharing()
{
    if (d->m_configProc) return;

    d->m_configProc = new TDEProcess(this);
    (*d->m_configProc) << TDEStandardDirs::findExe("tdesu") << locate("exe", "tdecmshell") << "fileshare";
    if (!d->m_configProc->start( TDEProcess::NotifyOnExit ))
    {
       delete d->m_configProc;
       d->m_configProc = 0;
       return;
    }
    connect(d->m_configProc, TQ_SIGNAL(processExited(TDEProcess *)),
            this, TQ_SLOT(slotConfigureFileSharingDone()));
    m_pbConfig->setEnabled(false);
}

void KFileSharePropsPlugin::slotConfigureFileSharingDone()
{
    delete d->m_configProc;
    d->m_configProc = 0;
    KFileShare::readConfig();
    KFileShare::readShareList();
    init();
}

void KFileSharePropsPlugin::applyChanges()
{
    kdDebug() << "KFileSharePropsPlugin::applyChanges" << endl;
    if ( m_rbShare && m_rbUnShare && m_rbSharerw )
    {
        bool share = m_rbShare->isChecked();

        if (share && d->m_bAllShared)
           return; // Nothing to do
        if (!share && d->m_bAllUnshared)
           return; // Nothing to do
          
        KFileItemList items = properties->items();
        KFileItemListIterator it( items );
        bool ok = true;
        for ( ; it.current() && ok; ++it ) {
             TQString path = (*it)->url().path();
             ok = SuSEsetShared( path, share, m_rbSharerw->isChecked() );
             if (!ok) {
                if (share)
                  KMessageBox::detailedError(properties,
                    i18n("Sharing folder '%1' failed.").arg(path),
                    i18n("An error occurred while trying to share folder '%1'. "
                         "Make sure that the Perl script 'fileshareset' is set suid root.")
                         .arg(path));
                else
                  KMessageBox::error(properties,
                    i18n("Unsharing folder '%1' failed.").arg(path),
                    i18n("An error occurred while trying to unshare folder '%1'. "
                         "Make sure that the Perl script 'fileshareset' is set suid root.")
                         .arg(path));

                properties->abortApplying();
                break;
             }
        }

        // Get the change back into our cached info
        KFileShare::readShareList();
    }
}

bool KFileSharePropsPlugin::setShared( const TQString& path, bool shared )
{
   return SuSEsetShared( path, shared, true );
}

bool KFileSharePropsPlugin::SuSEsetShared( const TQString& path, bool shared, bool readonly )
{
    kdDebug() << "KFileSharePropsPlugin::setShared " << path << ","
              << shared << readonly << endl;
    return KFileShare::SuSEsetShared( path, shared, readonly );
}

TQWidget* KFileSharePropsPlugin::page() const
{
    return d->m_vBox;
}

#include "tdefilesharedlg.moc"

//TODO: do we need to monitor /etc/security/fileshare.conf ?
// if the user is added to the 'fileshare' group, we wouldn't be notified
// Of course the config module can notify us.
// TODO: listen to such notifications ;)
