/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#include <tqhbuttongroup.h>
#include <tqpushbutton.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqmultilineedit.h>
#include <tqradiobutton.h>
#include <tqwhatsthis.h>
#include <tqregexp.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <kprocess.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <kurl.h>
#include <kurllabel.h>

#include "kbugreport.h"

#include <stdio.h>
#include <pwd.h>
#include <unistd.h>

#include <sys/utsname.h>

#include "kdepackages.h"
#include <kcombobox.h>
#include <config.h>
#include <tdetempfile.h>
#include <tqtextstream.h>
#include <tqfile.h>

class KBugReportPrivate {
public:
    KComboBox *appcombo;
    TQString lastError;
    TQString tde_version;
    TQString appname;
    TQString os;
    TQPushButton *submitBugButton;
    KURL url;
};

KBugReport::KBugReport( TQWidget * parentw, bool modal, const TDEAboutData *aboutData )
  : KDialogBase( Plain,
                 i18n("Submit Bug Report"),
                 Ok | Cancel,
                 Ok,
                 parentw,
                 "KBugReport",
                 modal, // modal
                 true // separator
                 )
{
  d = new KBugReportPrivate;

  // Use supplied aboutdata, otherwise the one from the active instance
  // otherwise the TDEGlobal one. _activeInstance should neved be 0L in theory.
  m_aboutData = aboutData
    ? aboutData
    : ( TDEGlobal::_activeInstance ? TDEGlobal::_activeInstance->aboutData()
                                 : TDEGlobal::instance()->aboutData() );
  m_process = 0;
  TQWidget * parent = plainPage();
  d->submitBugButton = 0;

  //if ( m_aboutData->bugAddress() == TQString::fromLatin1("submit@bugs.trinitydesktop.org") )
  //{
  //  // This is a core KDE application -> redirect to the web form
    // Always redirect to the Web form for Trinity
    d->submitBugButton = new TQPushButton( parent );
    setButtonCancel( KStdGuiItem::close() );
  //}

  TQLabel * tmpLabel;
  TQVBoxLayout * lay = new TQVBoxLayout( parent, 0, spacingHint() );

  TQGridLayout *glay = new TQGridLayout( lay, 4, 3 );
  glay->setColStretch( 1, 10 );
  glay->setColStretch( 2, 10 );

  int row = 0;

  if ( !d->submitBugButton )
  {
    // From
    TQString qwtstr = i18n( "Your email address. If incorrect, use the Configure Email button to change it" );
    tmpLabel = new TQLabel( i18n("From:"), parent );
    glay->addWidget( tmpLabel, row,0 );
    TQWhatsThis::add( tmpLabel, qwtstr );
    m_from = new TQLabel( parent );
    glay->addWidget( m_from, row, 1 );
    TQWhatsThis::add( m_from, qwtstr );


    // Configure email button
    m_configureEmail = new TQPushButton( i18n("Configure Email..."),
                                        parent );
    connect( m_configureEmail, TQ_SIGNAL( clicked() ), this,
             TQ_SLOT( slotConfigureEmail() ) );
    glay->addMultiCellWidget( m_configureEmail, 0, 2, 2, 2, TQt::AlignTop|TQt::AlignRight );

    // To
    qwtstr = i18n( "The email address this bug report is sent to." );
    tmpLabel = new TQLabel( i18n("To:"), parent );
    glay->addWidget( tmpLabel, ++row,0 );
    TQWhatsThis::add( tmpLabel, qwtstr );
    tmpLabel = new TQLabel( m_aboutData->bugAddress(), parent );
    glay->addWidget( tmpLabel, row, 1 );
    TQWhatsThis::add( tmpLabel, qwtstr );

    setButtonOK( KGuiItem( i18n("&Send"), "mail-send", i18n( "Send bug report." ),
                    i18n( "Send this bug report to %1." ).arg( m_aboutData->bugAddress() ) ) );

  }
  else
  {
    m_configureEmail = 0;
    m_from = 0;
    showButtonOK( false );
  }

  // Program name
  TQString qwtstr = i18n( "The application for which you wish to submit a bug report - if incorrect, please use the Report Bug menu item of the correct application" );
  tmpLabel = new TQLabel( i18n("Application: "), parent );
  glay->addWidget( tmpLabel, ++row, 0 );
  TQWhatsThis::add( tmpLabel, qwtstr );
  d->appcombo = new KComboBox( false, parent, "app");
  TQWhatsThis::add( d->appcombo, qwtstr );
  d->appcombo->insertStrList((const char**)packages);
  connect(d->appcombo, TQ_SIGNAL(activated(int)), TQ_SLOT(appChanged(int)));
  d->appname = TQString::fromLatin1( m_aboutData
                                    ? m_aboutData->productName()
                                    : tqApp->name() );
  glay->addWidget( d->appcombo, row, 1 );
  int index = 0;
  for (; index < d->appcombo->count(); index++) {
      if (d->appcombo->text(index) == d->appname) {
          break;
      }
  }
  if (index == d->appcombo->count()) { // not present
      d->appcombo->insertItem(d->appname);
  }
  d->appcombo->setCurrentItem(index);

  TQWhatsThis::add( tmpLabel, qwtstr );

  // Version
  qwtstr = i18n( "The version of this application - please make sure that no newer version is available before sending a bug report" );
  tmpLabel = new TQLabel( i18n("Version:"), parent );
  glay->addWidget( tmpLabel, ++row, 0 );
  TQWhatsThis::add( tmpLabel, qwtstr );
  if (m_aboutData)
      m_strVersion = m_aboutData->version();
  else
      m_strVersion = i18n("no version set (programmer error!)");
  d->tde_version = TQString::fromLatin1( TDE_VERSION_STRING );
  if (TQString::fromLatin1( KDE_DISTRIBUTION_TEXT ) != "")
      d->tde_version += ", " + TQString::fromLatin1( KDE_DISTRIBUTION_TEXT );
  if ( !d->submitBugButton )
      m_strVersion += " " + d->tde_version;
  m_version = new TQLabel( m_strVersion, parent );
  //glay->addWidget( m_version, row, 1 );
  glay->addMultiCellWidget( m_version, row, row, 1, 2 );
  TQWhatsThis::add( m_version, qwtstr );

  tmpLabel = new TQLabel(i18n("OS:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );

  struct utsname unameBuf;
  uname( &unameBuf );
  d->os = TQString::fromLatin1( unameBuf.sysname ) +
          " (" + TQString::fromLatin1( unameBuf.machine ) + ") "
          "release " + TQString::fromLatin1( unameBuf.release );

  tmpLabel = new TQLabel(d->os, parent);
  glay->addMultiCellWidget( tmpLabel, row, row, 1, 2 );

  tmpLabel = new TQLabel(i18n("Compiler:"), parent);
  glay->addWidget( tmpLabel, ++row, 0 );
  tmpLabel = new TQLabel(TQString::fromLatin1(KDE_COMPILER_VERSION), parent);
  glay->addMultiCellWidget( tmpLabel, row, row, 1, 2 );

  if ( !d->submitBugButton )
  {
    // Severity
    m_bgSeverity = new TQHButtonGroup( i18n("Se&verity"), parent );
    static const char * const sevNames[5] = { "critical", "grave", "normal", "wishlist", "i18n" };
    const TQString sevTexts[5] = { i18n("Critical"), i18n("Grave"), i18n("normal severity","Normal"), i18n("Wishlist"), i18n("Translation") };

    for (int i = 0 ; i < 5 ; i++ )
    {
      // Store the severity string as the name
      TQRadioButton *rb = new TQRadioButton( sevTexts[i], m_bgSeverity, sevNames[i] );
      if (i==2) rb->setChecked(true); // default : "normal"
    }

    lay->addWidget( m_bgSeverity );

    // Subject
    TQHBoxLayout * hlay = new TQHBoxLayout( lay );
    tmpLabel = new TQLabel( i18n("S&ubject: "), parent );
    hlay->addWidget( tmpLabel );
    m_subject = new KLineEdit( parent );
    m_subject->setFocus();
    tmpLabel->setBuddy(m_subject);
    hlay->addWidget( m_subject );

    TQString text = i18n("Enter the text (in English if possible) that you wish to submit for the "
                        "bug report.\n"
                        "If you press \"Send\", a mail message will be sent to the maintainer of "
                        "this program.\n");
    TQLabel * label = new TQLabel( parent, "label" );

    label->setText( text );
    lay->addWidget( label );

    // The multiline-edit
    m_lineedit = new TQMultiLineEdit( parent, "TQMultiLineEdit" );
    m_lineedit->setMinimumHeight( 180 ); // make it big
    m_lineedit->setWordWrap(TQMultiLineEdit::WidgetWidth);
    lay->addWidget( m_lineedit, 10 /*stretch*/ );

    slotSetFrom();
  } else {
    // Point to the web form

    lay->addSpacing(10);
    TQString text = i18n("Reporting bugs and requesting enhancements are maintained using the Bugzilla reporting system.\n"
                        "You'll need a login account and password to use the reporting system.\n"
                        "To control spam and rogue elements the login requires a valid email address.\n"
                        "Consider using any large email service if you want to avoid using your private email address.\n"
                        "\n"
                        "Selecting the button below opens your web browser to http://bugs.trinitydesktop.org,\n"
                        "where you will find the report form.\n"
                        "The information displayed above will be transferred to the reporting system.\n"
                        "Session cookies must be enabled to use the reporting system.\n"
                        "\n"
                        "Thank you for helping!");
    TQLabel * label = new TQLabel( text, parent, "label");
    lay->addWidget( label );
    lay->addSpacing(10);

    updateURL();
    d->submitBugButton->setText( i18n("&Launch Bug Report Wizard") );
    d->submitBugButton->setSizePolicy(TQSizePolicy::Fixed,TQSizePolicy::Fixed);
    lay->addWidget( d->submitBugButton );
    lay->addSpacing(10);

    connect( d->submitBugButton, TQ_SIGNAL(clicked()),
             this, TQ_SLOT(slotOk()));
  }
}

KBugReport::~KBugReport()
{
    delete d;
}

void KBugReport::updateURL()
{
    KURL url ( "http://bugs.trinitydesktop.org/enter_bug.cgi" );
    url.addQueryItem( "product", "TDE" );
    url.addQueryItem( "op_sys", d->os );
    url.addQueryItem( "cf_kde_compiler", KDE_COMPILER_VERSION );
    url.addQueryItem( "cf_tde_version", d->tde_version );
    url.addQueryItem( "cf_kde_appversion", m_strVersion );
    url.addQueryItem( "cf_kde_package", d->appcombo->currentText() );
    url.addQueryItem( "cf_kde_kbugreport", "1" );
    d->url = url;
}

void KBugReport::appChanged(int i)
{
    TQString appName = d->appcombo->text(i);
    int index = appName.find( '/' );
    if ( index > 0 )
        appName = appName.left( index );
    kdDebug() << "appName " << appName << endl;

    if (d->appname == appName && m_aboutData)
        m_strVersion = m_aboutData->version();
    else
        m_strVersion = i18n("unknown program name", "unknown");

    if ( !d->submitBugButton )
        m_strVersion += d->tde_version;

    m_version->setText(m_strVersion);
    if ( d->submitBugButton )
        updateURL();
}

void KBugReport::slotConfigureEmail()
{
  if (m_process) return;
  m_process = new TDEProcess;
  *m_process << TQString::fromLatin1("tdecmshell") << TQString::fromLatin1("kcm_useraccount");
  connect(m_process, TQ_SIGNAL(processExited(TDEProcess *)), TQ_SLOT(slotSetFrom()));
  if (!m_process->start())
  {
    kdDebug() << "Couldn't start tdecmshell.." << endl;
    delete m_process;
    m_process = 0;
    return;
  }
  m_configureEmail->setEnabled(false);
}

void KBugReport::slotSetFrom()
{
  delete m_process;
  m_process = 0;
  m_configureEmail->setEnabled(true);

  // ### KDE4: why oh why is KEmailSettings in tdeio?
  TDEConfig emailConf( TQString::fromLatin1("emaildefaults") );

  // find out the default profile
  emailConf.setGroup( TQString::fromLatin1("Defaults") );
  TQString profile = TQString::fromLatin1("PROFILE_");
  profile += emailConf.readEntry( TQString::fromLatin1("Profile"),
                                  TQString::fromLatin1("Default") );

  emailConf.setGroup( profile );
  TQString fromaddr = emailConf.readEntry( TQString::fromLatin1("EmailAddress") );
  if (fromaddr.isEmpty()) {
     struct passwd *p;
     p = getpwuid(getuid());
     fromaddr = TQString::fromLatin1(p->pw_name);
  } else {
     TQString name = emailConf.readEntry( TQString::fromLatin1("FullName"));
     if (!name.isEmpty())
        fromaddr = name + TQString::fromLatin1(" <") + fromaddr + TQString::fromLatin1(">");
  }
  m_from->setText( fromaddr );
}

void KBugReport::slotUrlClicked(const TQString &urlText)
{
  if ( kapp )
    kapp->invokeBrowser( urlText );

  // When using the web form, a click can also close the window, as there's
  // not much to do. It also gives the user a direct response to his click:
  if ( d->submitBugButton )
      KDialogBase::slotCancel();
}


void KBugReport::slotOk( void )
{
    if ( d->submitBugButton ) {
        if ( kapp )
            kapp->invokeBrowser( d->url.url() );
        return;
    }

    if( m_lineedit->text().isEmpty() ||
        m_subject->text().isEmpty() )
    {
        TQString msg = i18n("You must specify both a subject and a description "
                           "before the report can be sent.");
        KMessageBox::error(this,msg);
        return;
    }

    switch ( m_bgSeverity->id( m_bgSeverity->selected() ) )
    {
        case 0: // critical
            if ( KMessageBox::questionYesNo( this, i18n(
                "<p>You chose the severity <b>Critical</b>. "
                "Please note that this severity is intended only for bugs that</p>"
                "<ul><li>break unrelated software on the system (or the whole system)</li>"
                "<li>cause serious data loss</li>"
                "<li>introduce a security hole on the system where the affected package is installed</li></ul>\n"
                "<p>Does the bug you are reporting cause any of the above damage? "
                "If it does not, please select a lower severity. Thank you!</p>" ),TQString::null,KStdGuiItem::cont(),KStdGuiItem::cancel() ) == KMessageBox::No )
                return;
            break;
        case 1: // grave
            if ( KMessageBox::questionYesNo( this, i18n(
                "<p>You chose the severity <b>Grave</b>. "
                "Please note that this severity is intended only for bugs that</p>"
                "<ul><li>make the package in question unusable or mostly so</li>"
                "<li>cause data loss</li>"
                "<li>introduce a security hole allowing access to the accounts of users who use the affected package</li></ul>\n"
                "<p>Does the bug you are reporting cause any of the above damage? "
                "If it does not, please select a lower severity. Thank you!</p>" ),TQString::null,KStdGuiItem::cont(),KStdGuiItem::cancel() ) == KMessageBox::No )
                return;
            break;
    }
    if( !sendBugReport() )
    {
        TQString msg = i18n("Unable to send the bug report.\n"
                           "Please submit a bug report manually...\n"
                           "See http://bugs.trinitydesktop.org/ for instructions.");
        KMessageBox::error(this, msg + "\n\n" + d->lastError);
        return;
    }

    KMessageBox::information(this,
                             i18n("Bug report sent, thank you for your input."));
    accept();
}

void KBugReport::slotCancel()
{
  if( !d->submitBugButton && ( m_lineedit->edited() || m_subject->edited() ) )
  {
    int rc = KMessageBox::warningYesNo( this,
             i18n( "Close and discard\nedited message?" ),
             i18n( "Close Message" ), KStdGuiItem::discard(), KStdGuiItem::cont() );
    if( rc == KMessageBox::No )
      return;
  }
  KDialogBase::slotCancel();
}


TQString KBugReport::text() const
{
    kdDebug() << m_bgSeverity->selected()->name() << endl;
    // Prepend the pseudo-headers to the contents of the mail
  TQString severity = TQString::fromLatin1(m_bgSeverity->selected()->name());
  TQString appname = d->appcombo->currentText();
  TQString os = TQString::fromLatin1("OS: %1 (%2)\n").
               arg(KDE_COMPILING_OS).
               arg(KDE_DISTRIBUTION_TEXT);
  TQString bodyText;
  for(int i = 0; i < m_lineedit->numLines(); i++)
  {
     TQString line = m_lineedit->textLine(i);
     if (!line.endsWith("\n"))
        line += '\n';
     bodyText += line;
  }

  if (severity == TQString::fromLatin1("i18n") && TDEGlobal::locale()->language() != TDELocale::defaultLanguage()) {
      // Case 1 : i18n bug
      TQString package = TQString::fromLatin1("i18n_%1").arg(TDEGlobal::locale()->language());
      package = package.replace(TQString::fromLatin1("_"), TQString::fromLatin1("-"));
      return TQString::fromLatin1("Package: %1").arg(package) +
          TQString::fromLatin1("\n"
                              "Application: %1\n"
                              // not really i18n's version, so better here IMHO
                              "Version: %2\n").arg(appname).arg(m_strVersion)+
          os+TQString::fromLatin1("\n")+bodyText;
  } else {
      appname = appname.replace(TQString::fromLatin1("_"), TQString::fromLatin1("-"));
      // Case 2 : normal bug
      return TQString::fromLatin1("Package: %1\n"
                                 "Version: %2\n"
                                 "Severity: %3\n")
          .arg(appname).arg(m_strVersion).arg(severity)+
          TQString::fromLatin1("Compiler: %1\n").arg(KDE_COMPILER_VERSION)+
          os+TQString::fromLatin1("\n")+bodyText;
  }
}

bool KBugReport::sendBugReport()
{
  TQString recipient ( m_aboutData ?
    m_aboutData->bugAddress() :
    TQString::fromLatin1("submit@bugs.trinitydesktop.org") );

  TQString command;
  command = locate("exe", "tdesendbugmail");
  if (command.isEmpty())
      command = TDEStandardDirs::findExe( TQString::fromLatin1("tdesendbugmail") );

  KTempFile outputfile;
  outputfile.close();

  TQString subject = m_subject->text();
  command += " --subject ";
  command += TDEProcess::quote(subject);
  command += " --recipient ";
  command += TDEProcess::quote(recipient);
  command += " > ";
  command += TDEProcess::quote(outputfile.name());

  fflush(stdin);
  fflush(stderr);

  FILE * fd = popen(TQFile::encodeName(command), "w");
  if (!fd)
  {
    kdError() << "Unable to open a pipe to " << command << endl;
    return false;
  }

  TQString btext = text();
  fwrite(btext.ascii(),btext.length(),1,fd);
  fflush(fd);

  int error = pclose(fd);
  kdDebug() << "exit status1 " << error << " " << (WIFEXITED(error)) << " " <<  WEXITSTATUS(error) << endl;

  if ((WIFEXITED(error)) && WEXITSTATUS(error) == 1) {
      TQFile of(outputfile.name());
      if (of.open(IO_ReadOnly )) {
          TQTextStream is(&of);
          is.setEncoding(TQTextStream::UnicodeUTF8);
          TQString line;
          while (!is.eof())
              line = is.readLine();
          d->lastError = line;
      } else {
          d->lastError = TQString::null;
      }
      outputfile.unlink();
      return false;
  }
  outputfile.unlink();
  return true;
}

TQString KBugReport::compilerVersion() {
	return KDE_COMPILER_VERSION;
}

TQString KBugReport::operatingSystem() {
	return d->os;
}

TQString KBugReport::tdeVersion() {
	return d->tde_version;
}

void KBugReport::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kbugreport.moc"
