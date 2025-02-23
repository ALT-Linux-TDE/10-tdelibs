/*
 * This file is part of the KDE Libraries
 * Copyright (C) 2000 Waldo Bastian (bastian@kde.org) and
 * Espen Sand (espen@kde.org)
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
 *
 */

// I (espen) prefer that header files are included alphabetically

#include <tqlabel.h>
#include <tdeaboutapplication.h>
#include <kaboutdialog_private.h>
#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kurllabel.h>
#include <kactivelabel.h>
#include "ktextedit.h"

TDEAboutApplication::TDEAboutApplication( TQWidget *parent, const char *name,
              bool modal )
  :TDEAboutDialog( AbtTabbed|AbtProduct,
                 kapp ? kapp->caption() : TQString::null,
                 Close, Close,
     parent, name, modal )
{
  buildDialog(TDEGlobal::instance()->aboutData());
}

TDEAboutApplication::TDEAboutApplication( const TDEAboutData *aboutData, TQWidget *parent,
                                      const char *name, bool modal )
  :TDEAboutDialog( AbtTabbed|AbtProduct, aboutData->programName(), Close, Close,
     parent, name, modal )
{
  buildDialog(aboutData);
}

void TDEAboutApplication::buildDialog( const TDEAboutData *aboutData )
{
  if( !aboutData )
  {
    //
    // Recovery
    //

    //i18n "??" is displayed as (pseudo-)version when no data is known about the application
    setProduct( kapp ? kapp->caption() : TQString::null, i18n("??"), TQString::null, TQString::null );
    TDEAboutContainer *appPage = addContainerPage( i18n("&About"));

    TQString appPageText =
      i18n("No information available.\n"
     "The supplied TDEAboutData object does not exist.");
    TQLabel *appPageLabel = new TQLabel( "\n\n\n\n"+appPageText+"\n\n\n\n", 0 );
    appPage->addWidget( appPageLabel );
    return;
  }

  setProduct( aboutData->programName(), aboutData->version(),
        TQString::null, TQString::null );

  if (!aboutData->programLogo().isNull())
    setProgramLogo( aboutData->programLogo() );

  TQString appPageText = aboutData->shortDescription() + "\n";

  if (!aboutData->otherText().isEmpty())
    appPageText += "\n" + aboutData->otherText()+"\n";

  if (!aboutData->copyrightStatement().isEmpty())
    appPageText += "\n" + aboutData->copyrightStatement()+"\n";

  TDEAboutContainer *appPage = addContainerPage( i18n("&About"));

  TQLabel *appPageLabel = new TQLabel( appPageText, 0 );
  appPage->addWidget( appPageLabel );

  if (!aboutData->homepage().isEmpty())
  {
    KURLLabel *url = new KURLLabel();
    url->setText(aboutData->homepage());
    url->setURL(aboutData->homepage());
    appPage->addWidget( url );
    connect( url, TQ_SIGNAL(leftClickedURL(const TQString &)),
             this, TQ_SLOT(openURLSlot(const TQString &)));
  }

  int authorCount = aboutData->authors().count();
  if (authorCount)
  {
    TQString authorPageTitle = authorCount == 1 ?
      i18n("A&uthor") : i18n("A&uthors");
    TDEAboutContainer *authorPage = addScrolledContainerPage( authorPageTitle );

    if (!aboutData->customAuthorTextEnabled() || !aboutData->customAuthorRichText().isEmpty ())
    {
      TQString text;
      KActiveLabel* activeLabel = new KActiveLabel( authorPage );
      if (!aboutData->customAuthorTextEnabled())
      {
        if ( aboutData->bugAddress().isEmpty()
             || aboutData->bugAddress() == "submit@bugs.trinitydesktop.org"
             || aboutData->bugAddress() == "http://bugs.trinitydesktop.org")
          text = i18n( "Please use <a href=\"http://bugs.trinitydesktop.org\">http://bugs.trinitydesktop.org</a> to report bugs.\n" );
        else {
          if( aboutData->authors().count() == 1 && ( aboutData->authors().first().emailAddress() == aboutData->bugAddress() ) )
          {
            text = i18n( "Please report bugs to <a href=\"mailto:%1\">%2</a>.\n" ).arg( aboutData->authors().first().emailAddress() ).arg( aboutData->authors().first().emailAddress() );
          }
          else {
            text = i18n( "Please report bugs to <a href=\"mailto:%1\">%2</a>.\n" ).arg(aboutData->bugAddress()).arg(aboutData->bugAddress() );
          }
        }
      }
      else
      {
        text = aboutData->customAuthorRichText();
      }
      activeLabel->setText( text );
      authorPage->addWidget( activeLabel );
    }

    TQValueList<TDEAboutPerson>::ConstIterator it;
    for (it = aboutData->authors().begin();
   it != aboutData->authors().end(); ++it)
    {
      authorPage->addPerson( (*it).name(), (*it).emailAddress(),
           (*it).webAddress(), (*it).task() );
    }
  }

  int creditsCount = aboutData->credits().count();
  if (creditsCount)
  {
    TDEAboutContainer *creditsPage =
      addScrolledContainerPage( i18n("&Thanks To") );
    TQValueList<TDEAboutPerson>::ConstIterator it;
    for (it = aboutData->credits().begin();
   it != aboutData->credits().end(); ++it)
    {
      creditsPage->addPerson( (*it).name(), (*it).emailAddress(),
            (*it).webAddress(), (*it).task() );
    }
  }

  const TQValueList<TDEAboutTranslator> translatorList = aboutData->translators();

  if(translatorList.count() > 0)
  {
      TQString text = "<qt>";

      TQValueList<TDEAboutTranslator>::ConstIterator it;
      for(it = translatorList.begin(); it != translatorList.end(); ++it)
      {
   text += TQString("<p>%1<br>&nbsp;&nbsp;&nbsp;"
       "<a href=\"mailto:%2\">%2</a></p>")
     .arg((*it).name())
     .arg((*it).emailAddress())
     .arg((*it).emailAddress());
      }

      text += TDEAboutData::aboutTranslationTeam() + "</qt>";
      addTextPage( i18n("T&ranslation"), text, true);
  }

  if (!aboutData->license().isEmpty() )
  {
    addLicensePage( i18n("&License Agreement"), aboutData->license() );
  }

  //
  // Make sure the dialog has a reasonable width
  //
  setInitialSize( TQSize(400,1) );
}
