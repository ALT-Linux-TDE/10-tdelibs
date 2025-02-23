// $Id$

#include <sys/types.h>
#include "main.h"
#include <pwd.h>
#include <stdlib.h>
#include <unistd.h>

#include <tqtextstream.h>

#include <tdeapplication.h>
#include <tdeemailsettings.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>
#include <kdebug.h>
#include <tdeconfig.h>

#include "smtp.h"

static TDECmdLineOptions options[] = {
    { "subject <argument>", I18N_NOOP("Subject line"), 0 },
    { "recipient <argument>", I18N_NOOP("Recipient"), "submit@bugs.trinitydesktop.org" },
    TDECmdLineLastOption
};

void BugMailer::slotError(int errornum) {
    kdDebug() << "slotError\n";
    TQString str, lstr;

    switch(errornum) {
        case SMTP::CONNECTERROR:
            lstr = i18n("Error connecting to server.");
            break;
        case SMTP::NOTCONNECTED:
            lstr = i18n("Not connected.");
            break;
        case SMTP::CONNECTTIMEOUT:
            lstr = i18n("Connection timed out.");
            break;
        case SMTP::INTERACTTIMEOUT:
            lstr = i18n("Time out waiting for server interaction.");
            break;
        default:
            lstr = sm->getLastLine().stripWhiteSpace();
            lstr = i18n("Server said: \"%1\"").arg(lstr);
    }
    fputs(lstr.utf8().data(), stdout);
    fflush(stdout);

    ::exit(1);
}

void BugMailer::slotSend() {
    kdDebug() << "slotSend\n";
    ::exit(0);
}

int main(int argc, char **argv) {

    TDELocale::setMainCatalogue("tdelibs");
    TDEAboutData d("tdesendbugmail", I18N_NOOP("KSendBugMail"), "1.0",
                 I18N_NOOP("Sends a short bug report to submit@bugs.trinitydesktop.org"),
                 TDEAboutData::License_GPL, "(c) 2000 Stephan Kulow");
    d.addAuthor("Stephan Kulow", I18N_NOOP("Author"), "coolo@kde.org");

    TDECmdLineArgs::init(argc, argv, &d);
    TDECmdLineArgs::addCmdLineOptions(options);
    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

    TDEApplication a(false, false);

    TQCString recipient = args->getOption("recipient");
    if (recipient.isEmpty())
        recipient = "submit@bugs.trinitydesktop.org";
    else {
        if (recipient.at(0) == '\'') {
            recipient = recipient.mid(1).left(recipient.length() - 2);
        }
    }
    kdDebug() << "recp \"" << recipient << "\"\n";

    TQCString subject = args->getOption("subject");
    if (subject.isEmpty())
        subject = "(no subject)";
    else {
        if (subject.at(0) == '\'')
            subject = subject.mid(1).left(subject.length() - 2);
    }
    TQTextIStream input(stdin);
    TQString text, line;
    while (!input.eof()) {
        line = input.readLine();
        text += line + "\r\n";
    }
    kdDebug() << text << endl;

    KEMailSettings emailConfig;
    emailConfig.setProfile(emailConfig.defaultProfileName());
    TQString fromaddr = emailConfig.getSetting(KEMailSettings::EmailAddress);
    if (!fromaddr.isEmpty()) {
        TQString name = emailConfig.getSetting(KEMailSettings::RealName);
        if (!name.isEmpty())
            fromaddr = name + TQString::fromLatin1(" <") + fromaddr + TQString::fromLatin1(">");
    } else {
        struct passwd *p;
        p = getpwuid(getuid());
        fromaddr = TQString::fromLatin1(p->pw_name);
        fromaddr += "@";
        char buffer[256];
	buffer[0] = '\0';
        if(!gethostname(buffer, sizeof(buffer)))
	    buffer[sizeof(buffer)-1] = '\0';
        fromaddr += buffer;
    }
    kdDebug() << "fromaddr \"" << fromaddr << "\"" << endl;

    TQString  server = emailConfig.getSetting(KEMailSettings::OutServer);
    if (server.isEmpty())
        server=TQString::fromLatin1("bugs.trinitydesktop.org");

    SMTP *sm = new SMTP;
    BugMailer bm(sm);

    TQObject::connect(sm, TQ_SIGNAL(messageSent()), &bm, TQ_SLOT(slotSend()));
    TQObject::connect(sm, TQ_SIGNAL(error(int)), &bm, TQ_SLOT(slotError(int)));
    sm->setServerHost(server);
    sm->setPort(25);
    sm->setSenderAddress(fromaddr);
    sm->setRecipientAddress(recipient);
    sm->setMessageSubject(subject);
    sm->setMessageHeader(TQString::fromLatin1("From: %1\r\nTo: %2\r\n").arg(fromaddr).arg(recipient.data()));
    sm->setMessageBody(text);
    sm->sendMessage();

    int r = a.exec();
    kdDebug() << "execing " << r << endl;
    delete sm;
    return r;
}

#include "main.moc"
