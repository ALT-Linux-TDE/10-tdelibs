/*
   Copyright (C) 2000, 2001, 2002 Dawit Alemayehu <adawit@kde.org>

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
#include <unistd.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <errno.h>
#include <sys/time.h>
#include <sys/resource.h>
#include <sys/ioctl.h>
#include <sys/stat.h>
#include <fcntl.h>

#define HAVE_TERMIOS_H 1
#define HAVE_GRANTPT 1

#include <stdlib.h>
#ifdef HAVE_PTY_H
#include <pty.h>
#endif
#ifdef HAVE_TERMIOS_H
#include <termios.h>
#endif
#ifdef HAVE_STROPTS
#include <stropts.h>
#endif
#ifdef HAVE_SYS_IOCTL_H
#include <sys/ioctl.h>
#endif
#ifdef HAVE_LIBUTIL_H
#include <libutil.h>
#endif
#ifdef HAVE_UTIL_H
#include <util.h>
#endif

#include <tqfile.h>
#include <tqtimer.h>
#include <tqapplication.h>
#include <tqpushbutton.h>
#include <tqhbox.h>
#include <tqwhatsthis.h>
#include <tqiconview.h>
#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqstring.h>
#include <tqregexp.h>
#include <tqstyle.h>
#include <tqtimer.h>
#include <tqgroupbox.h>
#include <tqradiobutton.h>
#include <tqbuttongroup.h>

#include <kdebug.h>
#include <tdelocale.h>
#include <kinstance.h>

#include <twin.h>
#include <tdeaction.h>
#include <tdepopupmenu.h>
#include <tdemessagebox.h>
#include <kiconloader.h>
#include <kprogressbox.h>
#include <kpassdlg.h>
#include <tdelistview.h>
#include <tdeapplication.h>
#include <tdeconfigdialog.h>

#include <kdirlister.h>
#include <kstandarddirs.h>
#include <tdelistviewsearchline.h>
#include <kiconviewsearchline.h>
#include <kstaticdeleter.h>
#include <kgenericfactory.h>
#include <tdeparts/browserextension.h>

#include "rsyncconfigdialog.h"

/*
 * RsyncConfigDialog implementation
 */
RsyncConfigDialog::RsyncConfigDialog(TQWidget* parent, const char* name,
                                 const TQString& caption, const TQString& text,
                                 const TQString& localfolder, const TQString& remotefolder,
                                 int syncmode, bool modal)
    : KDialogBase(KDialogBase::Plain, caption, KDialogBase::Cancel | KDialogBase::Ok,
                  KDialogBase::Ok, parent, name, modal),
      mAutoClose(true),
      mAutoReset(false),
      mCancelled(false),
      mAllowCancel(true),
      mAllowTextEdit(false),
      mShown(false),
      mSyncAutoLogout(false)
{
#ifdef TQ_WS_X11
    KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
#endif
    mShowTimer = new TQTimer(this);

    showButton(KDialogBase::Close, false);
    mCancelText = actionButton(KDialogBase::Cancel)->text();

    TQFrame* mainWidget = plainPage();
    TQVBoxLayout* layout = new TQVBoxLayout(mainWidget, 10);
    mLabel = new TQLabel(TQString("<b>") + text + TQString("</b><br>")+i18n("Setting up synchronization for local folder")+TQString("<br><i>") + localfolder, mainWidget);
    layout->addWidget(mLabel);

    // Create an exclusive button group
    TQButtonGroup *layoutg = new TQButtonGroup( 1, TQt::Horizontal, i18n("Synchronization Method")+TQString(":"), mainWidget);
    layout->addWidget( layoutg );
    layoutg->setExclusive( TRUE );

    // Insert radiobuttons
    rsync_rb1 = new TQRadioButton(i18n("&Utilize rsync + ssh for upload to remote server\nExample: servername:/path/to/remote/folder"), layoutg);
    rsync_rb2 = new TQRadioButton(i18n("&Utilize rsync + ssh for download from remote server\nExample: servername:/path/to/remote/folder"), layoutg);
    rsync_rb3 = new TQRadioButton(i18n("&Utilize unison + ssh for bidirectional synchronization with remote server\nExample: ssh://servername//path/to/remote/folder"), layoutg);

    if (syncmode == 1)
        rsync_rb1->setChecked( TRUE );
    else if (syncmode == 2)
        rsync_rb2->setChecked( TRUE );
    else if (syncmode == 3)
        rsync_rb3->setChecked( TRUE );

    //(void)new TQRadioButton( "R&adiobutton 2", layoutg );
    //(void)new TQRadioButton( "Ra&diobutton 3", layoutg );

    // Create an exclusive button group
    TQButtonGroup *layoutm = new TQButtonGroup( 1, TQt::Horizontal, i18n("Remote Folder")+TQString(":"), mainWidget);
    layout->addWidget( layoutm );
    layoutg->setExclusive( TRUE );

    m_rsync_txt = new TQLineEdit(layoutm);
    if (remotefolder.isEmpty() == false) {
        m_rsync_txt->setText(remotefolder);
    }

    // Create an exclusive button group
    TQButtonGroup *layouta = new TQButtonGroup( 1, TQt::Horizontal, i18n("Automatic Synchronization")+TQString(":"), mainWidget);
    layout->addWidget( layouta );
    layouta->setExclusive( FALSE );

    m_sync_auto_logout_cb = new TQCheckBox(layouta);
    m_sync_auto_logout_cb->setText(i18n("Synchronize on logout"));
    m_sync_auto_logout_cb->setChecked(mSyncAutoLogout);

    setFixedSize( sizeHint() );

    m_rsync_txt->setFocus();
}

int RsyncConfigDialog::getSyncMode()
{
    if (rsync_rb1->isChecked() == true)
        return 1;
    else if (rsync_rb2->isChecked() == true)
        return 2;
    else if (rsync_rb3->isChecked() == true)
        return 3;
    else
        return 0;
}

int RsyncConfigDialog::getAutoSyncFlags()
{
    int flags = 0;
    if (m_sync_auto_logout_cb->isChecked() == true)
        flags = flags | 0x1;

    return flags;
}

void RsyncConfigDialog::setAutoSyncFlags(int flags) {
    m_sync_auto_logout_cb->setChecked((flags & 0x1) != 0);
}

TQLineEdit* RsyncConfigDialog::lineEdit()
{
    return m_rsync_txt;
}

const TQLineEdit* RsyncConfigDialog::lineEdit() const
{
    return m_rsync_txt;
}

#include "rsyncconfigdialog.moc"
