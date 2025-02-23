/*
 * This file shows an example for the KDE about dialog.
 *
 * copyright:  (C) Mirko Boehm, 1999
 * license:    GNU Public License, Version 2
 * mail to:    Mirko Boehm <mirko@kde.org>
 * requires:   recent C++-compiler, at least Qt 1.4
 * $Revision$
 */

#include "tdeaboutdialog.h"
// #include <kimgio.h>
#include <tqimage.h>
#include <tdeapplication.h>
#include <tdelocale.h>

int main(int argc, char** argv)
{
  (void)new TDEApplication(argc, argv, "TDEAboutDialogTest");
  TQImage logo;
  TQImage bg;
  TQPixmap pix;
  TDEAboutDialog about;
  // -----
  // kimgioRegister();
  if(logo.load("RayTracedGear.png"))
    {
      pix=logo;
      about.setLogo(pix);
    } else {
      tqDebug("main: No logo loaded.");
    }
  if(bg.load("background_1.png"))
    {
      pix=bg;
      about.setBackgroundTile(&pix);
      about.showTile( true );
      about.enableLinkedHelp( true );

      //about.showBaseFrameTile(true);
    } else {
      tqDebug("main: No tile loaded.");
    }
//    about.setButtonOKText
//      ("Cl&ose", "Close this dialog.",
//       "<b>Close</b> the dialog<br>"
//  	  "<i>(there are no settings to save).</i>");
  about.setCaption("TDEAboutDialog example");
  about.setVersion("TDEAboutDialog, Draft Study");
  about.setHelp("kdehelp/intro.html", "intro");
  // ----- set the application author:
  about.setAuthor
    ("Mirko Boehm", "mirko@kde.org", "", "Initial developer.");
  // ----- set the application maintainer:
  about.setMaintainer("Any One", // name
		      "anyone@kde.org", // email address
		      "http://www.anyhere.com", // URL
		      "Current maintainer."); // description
  // ----- add some contributors:
  about.addContributor("Some One", "someone@kde.org", "http://www.somehere.com",
		       "Making coffee");
  about.addContributor("Another One",
		       "anotherone@kde.org",
		       "http://www.blablax.com",
		       "Pizza donator");
  // ----- contents of the dialog have changed, adapt sizes:
  about.adjust();
  about.exec();
  // -----
  return 0;
}

