/*
    Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

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

#include "tdefontrequester.h"

#include <tqlabel.h>
#include <tqpushbutton.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqwhatsthis.h>

#include <tdefontdialog.h>
#include <tdelocale.h>

TDEFontRequester::TDEFontRequester( TQWidget *parent, const char *name,
    bool onlyFixed ) : TQWidget( parent, name ),
    m_onlyFixed( onlyFixed )
{
  TQHBoxLayout *layout = new TQHBoxLayout( this, 0, KDialog::spacingHint() );

  m_sampleLabel = new TQLabel( this, "m_sampleLabel" );
  m_button = new TQPushButton( i18n( "Choose..." ), this, "m_button" );

  m_sampleLabel->setFrameStyle( TQFrame::StyledPanel | TQFrame::Sunken );
  setFocusProxy( m_button );

  layout->addWidget( m_sampleLabel, 1 );
  layout->addWidget( m_button );

  connect( m_button, TQ_SIGNAL( clicked() ), TQ_SLOT( buttonClicked() ) );

  displaySampleText();
  setToolTip();
}

void TDEFontRequester::setFont( const TQFont &font, bool onlyFixed )
{
  m_selFont = font;
  m_onlyFixed = onlyFixed;

  displaySampleText();
  emit fontSelected( m_selFont );
}

void TDEFontRequester::setSampleText( const TQString &text )
{
  m_sampleText = text;
  displaySampleText();
}

void TDEFontRequester::setTitle( const TQString &title )
{
  m_title = title;
  setToolTip();
}

void TDEFontRequester::buttonClicked()
{
  int result = TDEFontDialog::getFont( m_selFont, m_onlyFixed, parentWidget() );

  if ( result == KDialog::Accepted )
  {
    displaySampleText();
    emit fontSelected( m_selFont );
  }
}

void TDEFontRequester::displaySampleText()
{
  m_sampleLabel->setFont( m_selFont );

  int size = m_selFont.pointSize();
  if(size == -1)
    size = m_selFont.pixelSize();

  if ( m_sampleText.isEmpty() )
    m_sampleLabel->setText( TQString( "%1 %2" ).arg( m_selFont.family() )
      .arg( size ) );
  else
    m_sampleLabel->setText( m_sampleText );
}

void TDEFontRequester::setToolTip()
{
  TQToolTip::remove( m_button );
  TQToolTip::add( m_button, i18n( "Click to select a font" ) );

  TQToolTip::remove( m_sampleLabel );
  TQWhatsThis::remove( m_sampleLabel );

  if ( m_title.isNull() )
  {
    TQToolTip::add( m_sampleLabel, i18n( "Preview of the selected font" ) );
    TQWhatsThis::add( m_sampleLabel, 
        i18n( "This is a preview of the selected font. You can change it"
        " by clicking the \"Choose...\" button." ) );
  }
  else
  {
    TQToolTip::add( m_sampleLabel, 
        i18n( "Preview of the \"%1\" font" ).arg( m_title ) );
    TQWhatsThis::add( m_sampleLabel, 
        i18n( "This is a preview of the \"%1\" font. You can change it"
        " by clicking the \"Choose...\" button." ).arg( m_title ) );
  }
}

#include "tdefontrequester.moc"
