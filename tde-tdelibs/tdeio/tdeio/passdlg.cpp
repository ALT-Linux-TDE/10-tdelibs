/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include "passdlg.h"

#include <tqapplication.h>
#include <tqcheckbox.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqsimplerichtext.h>
#include <tqstylesheet.h>

#include <kcombobox.h>
#include <tdeconfig.h>
#include <kiconloader.h>
#include <klineedit.h>
#include <tdelocale.h>
#include <kstandarddirs.h>

using namespace TDEIO;

struct PasswordDialog::PasswordDialogPrivate
{
    TQGridLayout *layout;
    TQLineEdit* userEdit;
    KLineEdit* passEdit;
    TQLabel* userNameLabel;
    TQLabel* prompt;
    TQCheckBox* keepCheckBox;
    TQMap<TQString,TQString> knownLogins;
    KComboBox* userEditCombo;
    TQHBox* userNameHBox;

    bool keep;
    short unsigned int nRow;
};

PasswordDialog::PasswordDialog( const TQString& prompt, const TQString& user,
                                bool enableKeep, bool modal, TQWidget* parent,
                                const char* name )
               :KDialogBase( parent, name, modal, i18n("Password"), Ok|Cancel, Ok, true)
{
    init ( prompt, user, enableKeep );
}

PasswordDialog::~PasswordDialog()
{
    delete d;
}

void PasswordDialog::init( const TQString& prompt, const TQString& user,
                           bool enableKeep  )
{
    TQWidget *main = makeMainWidget();

    d = new PasswordDialogPrivate;
    d->keep = false;
    d->nRow = 0;
    d->keepCheckBox = 0;

    TDEConfig* cfg = TDEGlobal::config();
    TDEConfigGroupSaver saver( cfg, "Passwords" );

    d->layout = new TQGridLayout( main, 9, 3, spacingHint(), marginHint());
    d->layout->addColSpacing(1, 5);

    // Row 0: pixmap  prompt
    TQLabel* lbl;
    TQPixmap pix( TDEGlobal::iconLoader()->loadIcon( "password", TDEIcon::NoGroup, TDEIcon::SizeHuge, 0, 0, true));
    if ( !pix.isNull() )
    {
        lbl = new TQLabel( main );
        lbl->setPixmap( pix );
        lbl->setAlignment( TQt::AlignLeft|TQt::AlignVCenter );
        lbl->setFixedSize( lbl->sizeHint() );
        d->layout->addWidget( lbl, 0, 0, TQt::AlignLeft );
    }
    d->prompt = new TQLabel( main );
    d->prompt->setAlignment( TQt::AlignLeft|TQt::AlignVCenter|TQt::WordBreak );
    d->layout->addWidget( d->prompt, 0, 2, TQt::AlignLeft );
    if ( prompt.isEmpty() )
        setPrompt( i18n( "You need to supply a username and a password" ) );
    else
        setPrompt( prompt );

    // Row 1: Row Spacer
    d->layout->addRowSpacing( 1, 7 );

    // Row 2-3: Reserved for an additional comment

    // Row 4: Username field
    d->userNameLabel = new TQLabel( i18n("&Username:"), main );
    d->userNameLabel->setAlignment( TQt::AlignVCenter | TQt::AlignLeft );
    d->userNameLabel->setFixedSize( d->userNameLabel->sizeHint() );
    d->userNameHBox = new TQHBox( main );

    d->userEdit = new KLineEdit( d->userNameHBox );
    TQSize s = d->userEdit->sizeHint();
    d->userEdit->setFixedHeight( s.height() );
    d->userEdit->setMinimumWidth( s.width() );
    d->userNameLabel->setBuddy( d->userEdit );
    d->layout->addWidget( d->userNameLabel, 4, 0 );
    d->layout->addWidget( d->userNameHBox, 4, 2 );

    // Row 5: Row spacer
    d->layout->addRowSpacing( 5, 4 );

    // Row 6: Password field
    lbl = new TQLabel( i18n("&Password:"), main );
    lbl->setAlignment( TQt::AlignVCenter | TQt::AlignLeft );
    lbl->setFixedSize( lbl->sizeHint() );
    TQHBox* hbox = new TQHBox( main );
    d->passEdit = new KLineEdit( hbox );
    if ( cfg->readEntry("EchoMode", "OneStar") == "NoEcho" )
        d->passEdit->setEchoMode( TQLineEdit::NoEcho );
    else
        d->passEdit->setEchoMode( TQLineEdit::Password );
    s = d->passEdit->sizeHint();
    d->passEdit->setFixedHeight( s.height() );
    d->passEdit->setMinimumWidth( s.width() );
    lbl->setBuddy( d->passEdit );
    d->layout->addWidget( lbl, 6, 0 );
    d->layout->addWidget( hbox, 6, 2 );

    if ( enableKeep )
    {
        // Row 7: Add spacer
        d->layout->addRowSpacing( 7, 4 );
        // Row 8: Keep Password
        hbox = new TQHBox( main );
        d->keepCheckBox = new TQCheckBox( i18n("&Keep password"), hbox );
        d->keepCheckBox->setFixedSize( d->keepCheckBox->sizeHint() );
        d->keep = cfg->readBoolEntry("Keep", false );
        d->keepCheckBox->setChecked( d->keep );
        connect(d->keepCheckBox, TQ_SIGNAL(toggled( bool )), TQ_SLOT(slotKeep( bool )));
        d->layout->addWidget( hbox, 8, 2 );
    }

    // Configure necessary key-bindings and connect necessar slots and signals
    connect( d->userEdit, TQ_SIGNAL(returnPressed()), d->passEdit, TQ_SLOT(setFocus()) );
    connect( d->passEdit, TQ_SIGNAL(returnPressed()), TQ_SLOT(slotOk()) );

    if ( !user.isEmpty() )
    {
        d->userEdit->setText( user );
        d->passEdit->setFocus();
    }
    else
        d->userEdit->setFocus();

    d->userEditCombo = 0;
//    setFixedSize( sizeHint() );
}

TQString PasswordDialog::username() const
{
    return d->userEdit->text();
}

TQString PasswordDialog::password() const
{
    return d->passEdit->text();
}

void PasswordDialog::setKeepPassword( bool b )
{
    if ( d->keepCheckBox )
        d->keepCheckBox->setChecked( b );
}

bool PasswordDialog::keepPassword() const
{
    return d->keep;
}

static void calculateLabelSize(TQLabel *label)
{
   TQString qt_text = label->text();

   int pref_width = 0;
   int pref_height = 0;
   // Calculate a proper size for the text.
   {
       TQSimpleRichText rt(qt_text, label->font());
       TQRect d = TDEGlobalSettings::desktopGeometry(label->topLevelWidget());

       pref_width = d.width() / 4;
       rt.setWidth(pref_width-10);
       int used_width = rt.widthUsed();
       pref_height = rt.height();
       if (used_width <= pref_width)
       {
          while(true)
          {
             int new_width = (used_width * 9) / 10;
             rt.setWidth(new_width-10);
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
    label->setFixedSize(TQSize(pref_width+10, pref_height));
}

void PasswordDialog::addCommentLine( const TQString& label,
                                     const TQString comment )
{
    if (d->nRow > 0)
        return;

    TQWidget *main = mainWidget();

    TQLabel* lbl = new TQLabel( label, main);
    lbl->setAlignment( TQt::AlignVCenter|TQt::AlignRight );
    lbl->setFixedSize( lbl->sizeHint() );
    d->layout->addWidget( lbl, d->nRow+2, 0, TQt::AlignLeft );
    lbl = new TQLabel( comment, main);
    lbl->setAlignment( TQt::AlignVCenter|TQt::AlignLeft|TQt::WordBreak );
    calculateLabelSize(lbl);
    d->layout->addWidget( lbl, d->nRow+2, 2, TQt::AlignLeft );
    d->layout->addRowSpacing( 3, 10 ); // Add a spacer
    d->nRow++;
}

void PasswordDialog::slotKeep( bool keep )
{
    d->keep = keep;
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

void PasswordDialog::setPrompt(const TQString& prompt)
{
    TQString text = qrichtextify(prompt);
    d->prompt->setText(text);
    calculateLabelSize(d->prompt);
}

void PasswordDialog::setPassword(const TQString &p)
{
    d->passEdit->setText(p);
}

void PasswordDialog::setUserReadOnly( bool readOnly )
{
    d->userEdit->setReadOnly( readOnly );
    if ( readOnly && d->userEdit->hasFocus() )
        d->passEdit->setFocus();
}

void PasswordDialog::setKnownLogins( const TQMap<TQString, TQString>& knownLogins )
{
    const int nr = knownLogins.count();
    if ( nr == 0 )
        return;
    if ( nr == 1 ) {
        d->userEdit->setText( knownLogins.begin().key() );
        setPassword( knownLogins.begin().data() );
        return;
    }

    Q_ASSERT( !d->userEdit->isReadOnly() );
    if ( !d->userEditCombo ) {
        delete d->userEdit;
        d->userEditCombo = new KComboBox( true, d->userNameHBox );
        d->userEdit = d->userEditCombo->lineEdit();
        TQSize s = d->userEditCombo->sizeHint();
        d->userEditCombo->setFixedHeight( s.height() );
        d->userEditCombo->setMinimumWidth( s.width() );
        d->userNameLabel->setBuddy( d->userEditCombo );
        d->layout->addWidget( d->userNameHBox, 4, 2 );
    }

    d->knownLogins = knownLogins;
    d->userEditCombo->insertStringList( knownLogins.keys() );
    d->userEditCombo->setFocus();

    connect( d->userEditCombo, TQ_SIGNAL( activated( const TQString& ) ),
             this, TQ_SLOT( slotActivated( const TQString& ) ) );
}

void PasswordDialog::slotActivated( const TQString& userName )
{
    TQMap<TQString, TQString>::ConstIterator it = d->knownLogins.find( userName );
    if ( it != d->knownLogins.end() )
        setPassword( it.data() );
}


int PasswordDialog::getNameAndPassword( TQString& user, TQString& pass, bool* keep,
                                        const TQString& prompt, bool readOnly,
                                        const TQString& caption,
                                        const TQString& comment,
                                        const TQString& label )
{
    PasswordDialog* dlg;
    if( keep )
        dlg = new PasswordDialog( prompt, user, (*keep) );
    else
        dlg = new PasswordDialog( prompt, user );

    if ( !caption.isEmpty() )
        dlg->setPlainCaption( caption );
    else
        dlg->setPlainCaption( i18n("Authorization Dialog") );

    if ( !comment.isEmpty() )
        dlg->addCommentLine( label, comment );

    if ( readOnly )
        dlg->setUserReadOnly( readOnly );

    int ret = dlg->exec();
    if ( ret == Accepted )
    {
        user = dlg->username();
        pass = dlg->password();
        if ( keep ) { (*keep) = dlg->keepPassword(); }
    }
    delete dlg;
    return ret;
 }

void PasswordDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "passdlg.moc"
