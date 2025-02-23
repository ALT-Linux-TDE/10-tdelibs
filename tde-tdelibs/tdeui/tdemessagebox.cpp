/*  This file is part of the KDE libraries
    Copyright (C) 1999 Waldo Bastian (bastian@kde.org)

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; version 2
    of the License.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqcheckbox.h>
#include <tqguardedptr.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlineedit.h>
#include <tqmessagebox.h>
#include <tqstringlist.h>
#include <tqvbox.h>
#include <tqvgroupbox.h>
#include <tqstylesheet.h>
#include <tqsimplerichtext.h>
#include <tqpushbutton.h>
#include <tqlayout.h>

#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <kdialogbase.h>
#include <kguiitem.h>
#include <tdelistbox.h>
#include <tdelocale.h>
#include <tdemessagebox.h>
#include <knotifyclient.h>
#include <kstdguiitem.h>
#include <kactivelabel.h>
#include <kiconloader.h>
#include <tdeglobalsettings.h>

#ifdef TQ_WS_X11
#include <X11/Xlib.h>
#endif

 /**
  * Easy MessageBox Dialog.
  *
  * Provides convenience functions for some i18n'ed standard dialogs,
  * as well as audible notification via @ref KNotifyClient
  *
  * @author Waldo Bastian (bastian@kde.org)
  */

static bool KMessageBox_queue = false;

static TQPixmap themedMessageBoxIcon(TQMessageBox::Icon icon)
{
    TQString icon_name;

    switch(icon)
    {
    case TQMessageBox::NoIcon:
        return TQPixmap();
        break;
    case TQMessageBox::Information:
        icon_name = "messagebox_info";
        break;
    case TQMessageBox::Warning:
        icon_name = "messagebox_warning";
        break;
    case TQMessageBox::Critical:
        icon_name = "messagebox_critical";
        break;
    default:
        break;
    }

   TQPixmap ret = TDEGlobal::iconLoader()->loadIcon(icon_name, TDEIcon::NoGroup, TDEIcon::SizeMedium, TDEIcon::DefaultState, 0, true);

   if (ret.isNull())
       return TQMessageBox::standardIcon(icon);
   else
       return ret;
}

static void sendNotification( TQString message,
                              const TQStringList& strlist,
                              TQMessageBox::Icon icon,
                              WId parent_id )
{
    // create the message for KNotify
    TQString messageType;
    switch ( icon )
    {
        case TQMessageBox::Warning:
            messageType = "messageWarning";
            break;
        case TQMessageBox::Critical:
            messageType = "messageCritical";
            break;
        case TQMessageBox::Question:
            messageType = "messageQuestion";
            break;
        default:
            messageType = "messageInformation";
            break;
    }

    if ( !strlist.isEmpty() )
    {
        for ( TQStringList::ConstIterator it = strlist.begin(); it != strlist.end(); ++it )
            message += "\n" + *it;
    }

    if ( !message.isEmpty() )
        KNotifyClient::event( (int)parent_id, messageType, message );
}

static TQString qrichtextify( const TQString& text )
{
  if ( text.isEmpty() || text[0] == '<' )
    return text;

  TQStringList lines = TQStringList::split('\n', text);
  for(TQStringList::Iterator it = lines.begin(); it != lines.end(); ++it)
  {
    *it = TQStyleSheet::convertFromPlainText( *it, TQStyleSheetItem::WhiteSpaceNormal );
  }

  return lines.join(TQString::null);
}

int KMessageBox::createKMessageBox(KDialogBase *dialog, TQMessageBox::Icon icon,
                             const TQString &text, const TQStringList &strlist,
                             const TQString &ask, bool *checkboxReturn,
                             int options, const TQString &details)
{
    return createKMessageBox(dialog, themedMessageBoxIcon(icon), text, strlist,
                      ask, checkboxReturn, options, details, icon);
}

int KMessageBox::createKMessageBox(KDialogBase *dialog, TQPixmap icon,
                             const TQString &text, const TQStringList &strlist,
                             const TQString &ask, bool *checkboxReturn, int options,
                             const TQString &details, TQMessageBox::Icon notifyType)
{
    TQVBox *topcontents = new TQVBox (dialog);
    topcontents->setSpacing(KDialog::spacingHint()*2);
    topcontents->setMargin(KDialog::marginHint());

    TQWidget *contents = new TQWidget(topcontents);
    TQHBoxLayout * lay = new TQHBoxLayout(contents);
    lay->setSpacing(KDialog::spacingHint());

    TQLabel *label1 = new TQLabel( contents);

    if (!icon.isNull())
       label1->setPixmap(icon);

    lay->addWidget( label1, 0, TQt::AlignCenter );
    lay->addSpacing(KDialog::spacingHint());
    // Enforce <p>text</p> otherwise the word-wrap doesn't work well
    TQString qt_text = qrichtextify( text );

    int pref_width = 0;
    int pref_height = 0;
    // Calculate a proper size for the text.
    {
       TQSimpleRichText rt(qt_text, dialog->font());
       TQRect d = TDEGlobalSettings::desktopGeometry(dialog);

       pref_width = d.width() / 3;
       rt.setWidth(pref_width);
       int used_width = rt.widthUsed();
       pref_height = rt.height();
       if (3*pref_height > 2*d.height())
       {
          // Very high dialog.. make it wider
          pref_width = d.width() / 2;
          rt.setWidth(pref_width);
          used_width = rt.widthUsed();
          pref_height = rt.height();
       }
       if (used_width <= pref_width)
       {
          while(true)
          {
             int new_width = (used_width * 9) / 10;
             rt.setWidth(new_width);
             int new_height = rt.height();
             if (new_height > pref_height)
                break;
             used_width = rt.widthUsed();
             if (used_width > new_width)
                break;
          }
          pref_width = used_width;
       }
       else
       {
          if (used_width > (pref_width *2))
             pref_width = pref_width *2;
          else
             pref_width = used_width;
       }
    }
    KActiveLabel *label2 = new KActiveLabel( qt_text, contents );
    if (!(options & KMessageBox::AllowLink))
    {
       TQObject::disconnect(label2, TQ_SIGNAL(linkClicked(const TQString &)),
                  label2, TQ_SLOT(openLink(const TQString &)));
    }

    // We add 10 pixels extra to compensate for some KActiveLabel margins.
    // TODO: find out why this is 10.
    label2->setFixedSize(TQSize(pref_width+10, pref_height));
    lay->addWidget( label2 );
    lay->addStretch();

    TDEListBox *listbox = 0;
    if (!strlist.isEmpty())
    {
       listbox=new TDEListBox( topcontents );
       listbox->insertStringList( strlist );
       listbox->setSelectionMode( TQListBox::NoSelection );
       topcontents->setStretchFactor(listbox, 1);
    }

    TQGuardedPtr<TQCheckBox> checkbox = 0;
    if (!ask.isEmpty())
    {
       checkbox = new TQCheckBox(ask, topcontents);
       if (checkboxReturn)
         checkbox->setChecked(*checkboxReturn);
    }

    if (!details.isEmpty())
    {
       TQVGroupBox *detailsGroup = new TQVGroupBox( i18n("Details"), dialog);
       if ( details.length() < 512 ) {
         KActiveLabel *label3 = new KActiveLabel(qrichtextify(details),
                                                 detailsGroup);
         label3->setMinimumSize(label3->sizeHint());
         if (!(options & KMessageBox::AllowLink))
         {
           TQObject::disconnect(label3, TQ_SIGNAL(linkClicked(const TQString &)),
                               label3, TQ_SLOT(openLink(const TQString &)));
         }
       } else {
         TQTextEdit* te = new TQTextEdit(details, TQString::null, detailsGroup);
         te->setReadOnly( true );
         te->setMinimumHeight( te->fontMetrics().lineSpacing() * 11 );
       }
       dialog->setDetailsWidget(detailsGroup);
    }

    dialog->setMainWidget(topcontents);
    dialog->enableButtonSeparator(false);
    if (!listbox)
       dialog->disableResize();

    const KDialogBase::ButtonCode buttons[] = {
        KDialogBase::Help,
        KDialogBase::Default,
        KDialogBase::Ok,
        KDialogBase::Apply,
        KDialogBase::Try,
        KDialogBase::Cancel,
        KDialogBase::Close,
        KDialogBase::User1,
        KDialogBase::User2,
        KDialogBase::User3,
        KDialogBase::No,
        KDialogBase::Yes,
        KDialogBase::Details };
    for( unsigned int i = 0;
	 i < sizeof( buttons )/sizeof( buttons[ 0 ] );
	 ++i )
	if( TQPushButton* btn = dialog->actionButton( buttons[ i ] ))
	    if( btn->isDefault())
		btn->setFocus();

    if ( (options & KMessageBox::Notify) )
        sendNotification( text, strlist, notifyType, dialog->topLevelWidget()->winId());

    if (KMessageBox_queue)
    {
       KDialogQueue::queueDialog(dialog);
       return KMessageBox::Cancel; // We have to return something.
    }

    if ( (options & KMessageBox::NoExec) )
    {
       return KMessageBox::Cancel; // We have to return something.
    }

    // We use a TQGuardedPtr because the dialog may get deleted
    // during exec() if the parent of the dialog gets deleted.
    // In that case the guarded ptr will reset to 0.
    TQGuardedPtr<KDialogBase> guardedDialog = dialog;

    int result = guardedDialog->exec();
    if (checkbox && checkboxReturn)
       *checkboxReturn = checkbox->isChecked();
    delete (KDialogBase *) guardedDialog;
    return result;
}

int
KMessageBox::questionYesNo(TQWidget *parent, const TQString &text,
                           const TQString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const TQString &dontAskAgainName,
                           int options)
{
   return questionYesNoList(parent, text, TQStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::questionYesNoWId(WId parent_id, const TQString &text,
                           const TQString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const TQString &dontAskAgainName,
                           int options)
{
   return questionYesNoListWId(parent_id, text, TQStringList(), caption,
                            buttonYes, buttonNo, dontAskAgainName, options);
}

bool
KMessageBox::shouldBeShownYesNo(const TQString &dontShowAgainName,
                                ButtonCode &result)
{
    if ( dontShowAgainName.isEmpty() ) return true;
    TQString grpNotifMsgs = TQString::fromLatin1("Notification Messages");
    TDEConfig *config = againConfig ? againConfig : TDEGlobal::config();
    TDEConfigGroupSaver saver( config, grpNotifMsgs );
    TQString dontAsk = config->readEntry(dontShowAgainName).lower();
    if (dontAsk == "yes") {
        result = Yes;
        return false;
    }
    if (dontAsk == "no") {
        result = No;
        return false;
    }
    return true;
}

bool
KMessageBox::shouldBeShownContinue(const TQString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) return true;
    TQString grpNotifMsgs = TQString::fromLatin1("Notification Messages");
    TDEConfig *config = againConfig ? againConfig : TDEGlobal::config();
    TDEConfigGroupSaver saver( config, grpNotifMsgs );
    return config->readBoolEntry(dontShowAgainName,  true);
}

void
KMessageBox::saveDontShowAgainYesNo(const TQString &dontShowAgainName,
                                    ButtonCode result)
{
    if ( dontShowAgainName.isEmpty() ) return;
    TQString grpNotifMsgs = TQString::fromLatin1("Notification Messages");
    TDEConfig *config = againConfig ? againConfig : TDEGlobal::config();
    TDEConfigGroupSaver saver( config, grpNotifMsgs );
    config->writeEntry( dontShowAgainName, result==Yes ? "yes" : "no", true, (dontShowAgainName[0] == ':'));
    config->sync();
}

void
KMessageBox::saveDontShowAgainContinue(const TQString &dontShowAgainName)
{
    if ( dontShowAgainName.isEmpty() ) return;
    TQString grpNotifMsgs = TQString::fromLatin1("Notification Messages");
    TDEConfig *config = againConfig ? againConfig : TDEGlobal::config();
    TDEConfigGroupSaver saver( config, grpNotifMsgs );
    config->writeEntry( dontShowAgainName, false, true, (dontShowAgainName[0] == ':'));
    config->sync();
}

TDEConfig* KMessageBox::againConfig = NULL;
void
KMessageBox::setDontShowAskAgainConfig(TDEConfig* cfg)
{
  againConfig = cfg;
}

int
KMessageBox::questionYesNoList(TQWidget *parent, const TQString &text,
                           const TQStringList &strlist,
                           const TQString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const TQString &dontAskAgainName,
                           int options)
{ // in order to avoid code duplication, convert to WId, it will be converted back
    return questionYesNoListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::questionYesNoListWId(WId parent_id, const TQString &text,
                           const TQStringList &strlist,
                           const TQString &caption,
                           const KGuiItem &buttonYes,
                           const KGuiItem &buttonNo,
                           const TQString &dontAskAgainName,
                           int options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Question") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "questionYesNo", true, true,
                       buttonYes, buttonNo);
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, TQMessageBox::Information, text, strlist,
                       dontAskAgainName.isEmpty() ? TQString::null : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::questionYesNoCancel(TQWidget *parent,
                          const TQString &text,
                          const TQString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const TQString &dontAskAgainName,
                          int options)
{
    return questionYesNoCancelWId( parent ? parent->winId() : 0, text, caption, buttonYes, buttonNo,
        dontAskAgainName, options );
}

int
KMessageBox::questionYesNoCancelWId(WId parent_id,
                          const TQString &text,
                          const TQString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const TQString &dontAskAgainName,
                          int options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Question") : caption,
                       KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                       KDialogBase::Yes, KDialogBase::Cancel,
                       parent, "questionYesNoCancel", true, true,
                       buttonYes, buttonNo);
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, TQMessageBox::Information,
                       text, TQStringList(),
                       dontAskAgainName.isEmpty() ? TQString::null : i18n("Do not ask again"),
                       &checkboxResult, options);
    if ( result==KDialogBase::Cancel ) return Cancel;
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::warningYesNo(TQWidget *parent, const TQString &text,
                          const TQString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const TQString &dontAskAgainName,
                          int options)
{
   return warningYesNoList(parent, text, TQStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoWId(WId parent_id, const TQString &text,
                          const TQString &caption,
                          const KGuiItem &buttonYes,
                          const KGuiItem &buttonNo,
                          const TQString &dontAskAgainName,
                          int options)
{
   return warningYesNoListWId(parent_id, text, TQStringList(), caption,
                       buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoList(TQWidget *parent, const TQString &text,
                              const TQStringList &strlist,
                              const TQString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const TQString &dontAskAgainName,
                              int options)
{
    return warningYesNoListWId( parent ? parent->winId() : 0, text, strlist, caption,
        buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::warningYesNoListWId(WId parent_id, const TQString &text,
                              const TQStringList &strlist,
                              const TQString &caption,
                              const KGuiItem &buttonYes,
                              const KGuiItem &buttonNo,
                              const TQString &dontAskAgainName,
                              int options)
{
    // warningYesNo and warningYesNoList are always "dangerous"
    // ### Remove this line for KDE 4, when the 'options' default parameter
    // takes effects.
    options |= Dangerous;

    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::No, KDialogBase::No,
                       parent, "warningYesNoList", true, true,
                       buttonYes, buttonNo);
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, TQMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? TQString::null : i18n("Do not ask again"),
                       &checkboxResult, options);
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

int
KMessageBox::warningContinueCancel(TQWidget *parent,
                                   const TQString &text,
                                   const TQString &caption,
                                   const KGuiItem &buttonContinue,
                                   const TQString &dontAskAgainName,
                                   int options)
{
   return warningContinueCancelList(parent, text, TQStringList(), caption,
                                buttonContinue, dontAskAgainName, options);
}

int
KMessageBox::warningContinueCancelWId(WId parent_id,
                                   const TQString &text,
                                   const TQString &caption,
                                   const KGuiItem &buttonContinue,
                                   const TQString &dontAskAgainName,
                                   int options)
{
   return warningContinueCancelListWId(parent_id, text, TQStringList(), caption,
                                buttonContinue, dontAskAgainName, options);
}

int
KMessageBox::warningContinueCancelList(TQWidget *parent, const TQString &text,
                             const TQStringList &strlist,
                             const TQString &caption,
                             const KGuiItem &buttonContinue,
                             const TQString &dontAskAgainName,
                             int options)
{
    return warningContinueCancelListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonContinue, dontAskAgainName, options );
}

int
KMessageBox::warningContinueCancelListWId(WId parent_id, const TQString &text,
                             const TQStringList &strlist,
                             const TQString &caption,
                             const KGuiItem &buttonContinue,
                             const TQString &dontAskAgainName,
                             int options)
{
    if ( !shouldBeShownContinue(dontAskAgainName) )
        return Continue;

    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No,
                       KDialogBase::Yes, KDialogBase::No,
                       parent, "warningYesNo", true, true,
                       buttonContinue, KStdGuiItem::cancel() );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, TQMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? TQString::null : i18n("Do not ask again"),
                       &checkboxResult, options);

    if ( result==KDialogBase::No )
        return Cancel;
    if (checkboxResult)
        saveDontShowAgainContinue(dontAskAgainName);
    return Continue;
}

int
KMessageBox::warningYesNoCancel(TQWidget *parent, const TQString &text,
                                const TQString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const TQString &dontAskAgainName,
                                int options)
{
   return warningYesNoCancelList(parent, text, TQStringList(), caption,
                      buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoCancelWId(WId parent_id, const TQString &text,
                                const TQString &caption,
                                const KGuiItem &buttonYes,
                                const KGuiItem &buttonNo,
                                const TQString &dontAskAgainName,
                                int options)
{
   return warningYesNoCancelListWId(parent_id, text, TQStringList(), caption,
                      buttonYes, buttonNo, dontAskAgainName, options);
}

int
KMessageBox::warningYesNoCancelList(TQWidget *parent, const TQString &text,
                                    const TQStringList &strlist,
                                    const TQString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const TQString &dontAskAgainName,
                                    int options)
{
    return warningYesNoCancelListWId( parent ? parent->winId() : 0, text, strlist,
        caption, buttonYes, buttonNo, dontAskAgainName, options );
}

int
KMessageBox::warningYesNoCancelListWId(WId parent_id, const TQString &text,
                                    const TQStringList &strlist,
                                    const TQString &caption,
                                    const KGuiItem &buttonYes,
                                    const KGuiItem &buttonNo,
                                    const TQString &dontAskAgainName,
                                    int options)
{
    ButtonCode res;
    if ( !shouldBeShownYesNo(dontAskAgainName, res) )
        return res;

    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Warning") : caption,
                       KDialogBase::Yes | KDialogBase::No | KDialogBase::Cancel,
                       KDialogBase::Yes, KDialogBase::Cancel,
                       parent, "warningYesNoCancel", true, true,
                       buttonYes, buttonNo);
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;
    int result = createKMessageBox(dialog, TQMessageBox::Warning, text, strlist,
                       dontAskAgainName.isEmpty() ? TQString::null : i18n("Do not ask again"),
                       &checkboxResult, options);
    if ( result==KDialogBase::Cancel ) return Cancel;
    res = (result==KDialogBase::Yes ? Yes : No);

    if (checkboxResult)
        saveDontShowAgainYesNo(dontAskAgainName, res);
    return res;
}

void
KMessageBox::error(TQWidget *parent,  const TQString &text,
                   const TQString &caption, int options)
{
    return errorListWId( parent ? parent->winId() : 0, text, TQStringList(), caption, options );
}

void
KMessageBox::errorWId(WId parent_id, const TQString &text,
                      const TQString &caption, int options)
{
    errorListWId( parent_id, text, TQStringList(), caption, options );
}

void
KMessageBox::errorList(TQWidget *parent, const TQString &text, const TQStringList &strlist,
                       const TQString &caption, int options)
{
    return errorListWId( parent ? parent->winId() : 0, text, strlist, caption, options );
}

void
KMessageBox::errorListWId(WId parent_id,  const TQString &text, const TQStringList &strlist,
                   const TQString &caption, int options)
{
    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, TQMessageBox::Critical, text, strlist, TQString::null, 0, options);
}

void
KMessageBox::detailedError(TQWidget *parent,  const TQString &text,
                   const TQString &details,
                   const TQString &caption, int options)
{
    return detailedErrorWId( parent ? parent->winId() : 0, text, details, caption, options );
}

void
KMessageBox::detailedErrorWId(WId parent_id,  const TQString &text,
                   const TQString &details,
                   const TQString &caption, int options)
{
    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Error") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "error", true, true,
                       KStdGuiItem::ok() );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, TQMessageBox::Critical, text, TQStringList(), TQString::null, 0, options, details);
}

void
KMessageBox::queuedDetailedError(TQWidget *parent,  const TQString &text,
                   const TQString &details,
                   const TQString &caption)
{
    return queuedDetailedErrorWId( parent ? parent->winId() : 0, text, details, caption );
}

void
KMessageBox::queuedDetailedErrorWId(WId parent_id,  const TQString &text,
                   const TQString &details,
                   const TQString &caption)
{
   KMessageBox_queue = true;
   (void) detailedErrorWId(parent_id, text, details, caption);
   KMessageBox_queue = false;
}


void
KMessageBox::sorry(TQWidget *parent, const TQString &text,
                   const TQString &caption, int options)
{
    return sorryWId( parent ? parent->winId() : 0, text, caption, options );
}

void
KMessageBox::sorryWId(WId parent_id, const TQString &text,
                   const TQString &caption, int options)
{
    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, TQMessageBox::Warning, text, TQStringList(), TQString::null, 0, options);
}

void
KMessageBox::detailedSorry(TQWidget *parent, const TQString &text,
                   const TQString &details,
                   const TQString &caption, int options)
{
    return detailedSorryWId( parent ? parent->winId() : 0, text, details, caption, options );
}

void
KMessageBox::detailedSorryWId(WId parent_id, const TQString &text,
                   const TQString &details,
                   const TQString &caption, int options)
{
    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Sorry") : caption,
                       KDialogBase::Yes | KDialogBase::Details,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "sorry", true, true,
                       KStdGuiItem::ok() );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    createKMessageBox(dialog, TQMessageBox::Warning, text, TQStringList(), TQString::null, 0, options, details);
}

void
KMessageBox::information(TQWidget *parent,const TQString &text,
			 const TQString &caption, const TQString &dontShowAgainName, int options)
{
  informationList(parent, text, TQStringList(), caption, dontShowAgainName, options);
}

void
KMessageBox::informationWId(WId parent_id,const TQString &text,
			 const TQString &caption, const TQString &dontShowAgainName, int options)
{
  informationListWId(parent_id, text, TQStringList(), caption, dontShowAgainName, options);
}

void
KMessageBox::informationList(TQWidget *parent,const TQString &text, const TQStringList & strlist,
                         const TQString &caption, const TQString &dontShowAgainName, int options)
{
    return informationListWId( parent ? parent->winId() : 0, text, strlist, caption,
        dontShowAgainName, options );
}

void
KMessageBox::informationListWId(WId parent_id,const TQString &text, const TQStringList & strlist,
                         const TQString &caption, const TQString &dontShowAgainName, int options)
{
    if ( !shouldBeShownContinue(dontShowAgainName) )
        return;

    TQWidget* parent = TQWidget::find( parent_id );
    KDialogBase *dialog= new KDialogBase(
                       caption.isEmpty() ? i18n("Information") : caption,
                       KDialogBase::Yes,
                       KDialogBase::Yes, KDialogBase::Yes,
                       parent, "information", true, true,
                       KStdGuiItem::ok() );
    if( options & PlainCaption )
        dialog->setPlainCaption( caption );
#ifdef TQ_WS_X11
    if( parent == NULL && parent_id )
        XSetTransientForHint( tqt_xdisplay(), dialog->winId(), parent_id );
#endif

    bool checkboxResult = false;

    createKMessageBox(dialog, TQMessageBox::Information, text, strlist,
		dontShowAgainName.isEmpty() ? TQString::null : i18n("Do not show this message again"),
                &checkboxResult, options);

    if (checkboxResult)
        saveDontShowAgainContinue(dontShowAgainName);
}

void
KMessageBox::enableAllMessages()
{
   TDEConfig *config = againConfig ? againConfig : TDEGlobal::config();
   TQString grpNotifMsgs = TQString::fromLatin1("Notification Messages");
   if (!config->hasGroup(grpNotifMsgs))
      return;

   TDEConfigGroupSaver saver( config, grpNotifMsgs );

   typedef TQMap<TQString, TQString> configMap;

   configMap map = config->entryMap(grpNotifMsgs);

   configMap::Iterator it;
   for (it = map.begin(); it != map.end(); ++it)
      config->deleteEntry( it.key() );
   config->sync();
}

void
KMessageBox::enableMessage(const TQString &dontShowAgainName)
{
   TDEConfig *config = againConfig ? againConfig : TDEGlobal::config();
   TQString grpNotifMsgs = TQString::fromLatin1("Notification Messages");
   if (!config->hasGroup(grpNotifMsgs))
      return;

   TDEConfigGroupSaver saver( config, grpNotifMsgs );

   config->deleteEntry(dontShowAgainName);
   config->sync();
}

void
KMessageBox::about(TQWidget *parent, const TQString &text,
                   const TQString &caption, int options)
{
    TQString _caption = caption;
    if (_caption.isEmpty())
        _caption = i18n("About %1").arg(kapp->caption());

    KDialogBase *dialog = new KDialogBase(
                                caption,
                                KDialogBase::Yes,
                                KDialogBase::Yes, KDialogBase::Yes,
                                parent, "about", true, true,
                                KStdGuiItem::ok() );
    
    TQPixmap ret = TDEApplication::kApplication()->icon();
    if (ret.isNull())
        ret = TQMessageBox::standardIcon(TQMessageBox::Information);
    dialog->setIcon(ret);

    createKMessageBox(dialog, ret, text, TQStringList(), TQString::null, 0, options);
    
    return;
}

int KMessageBox::messageBox( TQWidget *parent, DialogType type, const TQString &text,
                             const TQString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const TQString &dontShowAskAgainName,
                             int options )
{
    return messageBoxWId( parent ? parent->winId() : 0, type, text, caption,
        buttonYes, buttonNo, dontShowAskAgainName, options );
}

int KMessageBox::messageBox( TQWidget *parent, DialogType type, const TQString &text,
                             const TQString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, int options )
{
    return messageBoxWId( parent ? parent->winId() : 0, type, text, caption,
        buttonYes, buttonNo, TQString::null, options );
}

int KMessageBox::messageBoxWId( WId parent_id, DialogType type, const TQString &text,
                             const TQString &caption, const KGuiItem &buttonYes,
                             const KGuiItem &buttonNo, const TQString &dontShow,
                             int options )
{
    switch (type) {
        case QuestionYesNo:
            return KMessageBox::questionYesNoWId( parent_id,
                                               text, caption, buttonYes, buttonNo, dontShow, options );
        case QuestionYesNoCancel:
            return KMessageBox::questionYesNoCancelWId( parent_id,
                                               text, caption, buttonYes, buttonNo, dontShow, options );
        case WarningYesNo:
            return KMessageBox::warningYesNoWId( parent_id,
                                              text, caption, buttonYes, buttonNo, dontShow, options );
        case WarningContinueCancel:
            return KMessageBox::warningContinueCancelWId( parent_id,
                                              text, caption, buttonYes.text(), dontShow, options );
        case WarningYesNoCancel:
            return KMessageBox::warningYesNoCancelWId( parent_id,
                                              text, caption, buttonYes, buttonNo, dontShow, options );
        case Information:
            KMessageBox::informationWId( parent_id,
                                      text, caption, dontShow, options );
            return KMessageBox::Ok;

        case Error:
            KMessageBox::errorWId( parent_id, text, caption, options );
            return KMessageBox::Ok;

        case Sorry:
            KMessageBox::sorryWId( parent_id, text, caption, options );
            return KMessageBox::Ok;
    }
    return KMessageBox::Cancel;
}

void KMessageBox::queuedMessageBox( TQWidget *parent, DialogType type, const TQString &text, const TQString &caption, int options )
{
    return queuedMessageBoxWId( parent ? parent->winId() : 0, type, text, caption, options );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const TQString &text, const TQString &caption, int options )
{
   KMessageBox_queue = true;
   (void) messageBoxWId(parent_id, type, text, caption, KStdGuiItem::yes(),
                     KStdGuiItem::no(), TQString::null, options);
   KMessageBox_queue = false;
}

void KMessageBox::queuedMessageBox( TQWidget *parent, DialogType type, const TQString &text, const TQString &caption )
{
    return queuedMessageBoxWId( parent ? parent->winId() : 0, type, text, caption );
}

void KMessageBox::queuedMessageBoxWId( WId parent_id, DialogType type, const TQString &text, const TQString &caption )
{
   KMessageBox_queue = true;
   (void) messageBoxWId(parent_id, type, text, caption);
   KMessageBox_queue = false;
}
