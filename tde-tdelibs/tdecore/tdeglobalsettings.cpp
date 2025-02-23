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
#include "config.h"
#include "tdeglobalsettings.h"

#include <tqdir.h>
#include <tqpixmap.h>
#include <tqfontdatabase.h>
#include <tqcursor.h>

#include <tdeconfig.h>
#include <ksimpleconfig.h>
#include <tdeapplication.h>

#include <kipc.h>

#ifdef TQ_WS_WIN
#include <windows.h>
#include "qt_windows.h"
#include <win32_utils.h>
static TQRgb qt_colorref2qrgb(COLORREF col)
{
    return tqRgb(GetRValue(col),GetGValue(col),GetBValue(col));
}
#endif

#include <kdebug.h>
#include <tdeglobal.h>
#include <tdeshortcut.h>
#include <kstandarddirs.h>
#include <kcharsets.h>
#include <tdeaccel.h>
#include <tdelocale.h>
#include <tqfontinfo.h>
#include <stdlib.h>
#include <kprotocolinfo.h>

#include <tqtextcodec.h>
#include <tqtextstream.h>
#include <tqfile.h>

#ifdef TQ_WS_X11
#include <X11/Xlib.h>
#endif

TQString* TDEGlobalSettings::s_desktopPath = 0;
TQString* TDEGlobalSettings::s_autostartPath = 0;
TQString* TDEGlobalSettings::s_trashPath = 0;
TQString* TDEGlobalSettings::s_documentPath = 0;
TQString* TDEGlobalSettings::s_videosPath = 0;
TQString* TDEGlobalSettings::s_musicPath = 0;
TQString* TDEGlobalSettings::s_downloadPath = 0;
TQString* TDEGlobalSettings::s_picturesPath = 0;
TQString* TDEGlobalSettings::s_templatesPath = 0;
TQString* TDEGlobalSettings::s_publicSharePath = 0;
TQFont *TDEGlobalSettings::_generalFont = 0;
TQFont *TDEGlobalSettings::_fixedFont = 0;
TQFont *TDEGlobalSettings::_toolBarFont = 0;
TQFont *TDEGlobalSettings::_menuFont = 0;
TQFont *TDEGlobalSettings::_windowTitleFont = 0;
TQFont *TDEGlobalSettings::_taskbarFont = 0;
TQFont *TDEGlobalSettings::_largeFont = 0;
TQColor *TDEGlobalSettings::_trinity4Blue = 0;
TQColor *TDEGlobalSettings::_inactiveBackground = 0;
TQColor *TDEGlobalSettings::_inactiveForeground = 0;
TQColor *TDEGlobalSettings::_activeBackground = 0;
TQColor *TDEGlobalSettings::_buttonBackground = 0;
TQColor *TDEGlobalSettings::_selectBackground = 0;
TQColor *TDEGlobalSettings::_linkColor = 0;
TQColor *TDEGlobalSettings::_visitedLinkColor = 0;
TQColor *TDEGlobalSettings::alternateColor = 0;

TDEGlobalSettings::KMouseSettings *TDEGlobalSettings::s_mouseSettings = 0;

// Helper function for reading xdg user dirs.
// Returns sane values in case the user dir file can't be read
static void readXdgUserDirs(TQString *desktop, TQString *documents, TQString *download, TQString *music,
                     TQString *pictures, TQString *publicShare, TQString *templates, TQString *videos)
{
	TQFile dirsFile(TQDir::homeDirPath() + "/.config/user-dirs.dirs");
	if (dirsFile.open(IO_ReadOnly))
	{
		// set the codec for the current locale
		TQTextStream stream(&dirsFile);
		stream.setCodec(TQTextCodec::codecForLocale());

		while (!stream.atEnd())
		{
			TQString line = stream.readLine();
			if (line.startsWith("XDG_DESKTOP_DIR="))
			{
				*desktop = line.remove("XDG_DESKTOP_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
			else if (line.startsWith("XDG_DOCUMENTS_DIR="))
			{
				*documents = line.remove("XDG_DOCUMENTS_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
			else if (line.startsWith("XDG_DOWNLOAD_DIR="))
			{
				*download = line.remove("XDG_DOWNLOAD_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
			else if (line.startsWith("XDG_MUSIC_DIR="))
			{
				*music = line.remove("XDG_MUSIC_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
			else if (line.startsWith("XDG_PICTURES_DIR="))
			{
				*pictures = line.remove("XDG_PICTURES_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
			else if (line.startsWith("XDG_PUBLICSHARE_DIR="))
			{
				*publicShare = line.remove("XDG_PUBLICSHARE_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
			else if (line.startsWith("XDG_TEMPLATES_DIR="))
			{
				*templates = line.remove("XDG_TEMPLATES_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
			else if (line.startsWith("XDG_VIDEOS_DIR="))
			{
				*videos = line.remove("XDG_VIDEOS_DIR=").remove("\"").replace("$HOME", TQDir::homeDirPath());
			}
		}
		dirsFile.close();
	}

	// Use sane values in case some paths are missing
	if (desktop->isEmpty())
	{
		*desktop = TQDir::homeDirPath() + "/" + "Desktop" + "/";
		if (!TQDir(*desktop).exists())
		{
			*desktop = TQDir::homeDirPath() + "/" + i18n("Desktop") + "/";
		}
	}
	if (documents->isEmpty())
	{
		*documents = TQDir::homeDirPath() + "/" + "Documents" + "/";
		if (!TQDir(*documents).exists())
		{
			*documents = TQDir::homeDirPath() + "/" + i18n("Documents") + "/";
		}
	}
	if (download->isEmpty())
	{
		*download = TQDir::homeDirPath() + "/" + "Downloads" + "/";
		if (!TQDir(*download).exists())
		{
			*download = TQDir::homeDirPath() + "/" + i18n("Downloads") + "/";
		}
	}
	if (music->isEmpty())
	{
		*music = TQDir::homeDirPath() + "/" + "Music" + "/";
		if (!TQDir(*music).exists())
		{
			*music = TQDir::homeDirPath() + "/" + i18n("Music") + "/";
		}
	}
	if (pictures->isEmpty())
	{
		*pictures = TQDir::homeDirPath() + "/" + "Pictures" + "/";
		if (!TQDir(*pictures).exists())
		{
			*pictures = TQDir::homeDirPath() + "/" + i18n("Pictures") + "/";
		}
	}
	if (publicShare->isEmpty())
	{
		*publicShare = TQDir::homeDirPath() + "/" + "Public" + "/";
		if (!TQDir(*publicShare).exists())
		{
			*publicShare = TQDir::homeDirPath() + "/" + i18n("Public") + "/";
		}
	}
	if (templates->isEmpty())
	{
		*templates = TQDir::homeDirPath() + "/" + "Templates" + "/";
		if (!TQDir(*templates).exists())
		{
			*templates = TQDir::homeDirPath() + "/" + i18n("Templates") + "/";
		}
	}
	if (videos->isEmpty())
	{
		*videos = TQDir::homeDirPath() + "/" + "Videos" + "/";
		if (!TQDir(*videos).exists())
		{
			*videos = TQDir::homeDirPath() + "/" + i18n("Videos") + "/";
		}
	}
}

static void checkAndCreateXdgFolder(const TQString &folder, const TQString &path, TDEConfig *config)
{
	bool pathOk = true;
	if (!TQDir(path).exists())
	{
		if (!TDEStandardDirs::makeDir(path))
		{
			pathOk = false;
		}
	}

	if (pathOk)
	{
		config->writePathEntry(folder, '"' + path + '"', true, false, false, false );
	}
}

int TDEGlobalSettings::dndEventDelay()
{
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readNumEntry("StartDragDist", TQApplication::startDragDistance());
}

bool TDEGlobalSettings::singleClick()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry("SingleClick", KDE_DEFAULT_SINGLECLICK);
}

bool TDEGlobalSettings::iconUseRoundedRect()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry("IconUseRoundedRect", KDE_DEFAULT_ICONTEXTROUNDED);
}

TDEGlobalSettings::TearOffHandle TDEGlobalSettings::insertTearOffHandle()
{
    int tearoff;
    bool effectsenabled;
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    effectsenabled = g.readBoolEntry( "EffectsEnabled", false);
    tearoff = g.readNumEntry("InsertTearOffHandle", KDE_DEFAULT_INSERTTEAROFFHANDLES);
    return effectsenabled ? (TearOffHandle) tearoff : Disable;
}

bool TDEGlobalSettings::changeCursorOverIcon()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry("ChangeCursor", KDE_DEFAULT_CHANGECURSOR);
}

bool TDEGlobalSettings::visualActivate()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry("VisualActivate", KDE_DEFAULT_VISUAL_ACTIVATE);
}

unsigned int TDEGlobalSettings::visualActivateSpeed()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return
        g.readNumEntry(
            "VisualActivateSpeed",
            KDE_DEFAULT_VISUAL_ACTIVATE_SPEED
        );
}



int TDEGlobalSettings::autoSelectDelay()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readNumEntry("AutoSelectDelay", KDE_DEFAULT_AUTOSELECTDELAY);
}

TDEGlobalSettings::Completion TDEGlobalSettings::completionMode()
{
    int completion;
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    completion = g.readNumEntry("completionMode", -1);
    if ((completion < (int) CompletionNone) ||
        (completion > (int) CompletionPopupAuto))
      {
        completion = (int) CompletionPopup; // Default
      }
  return (Completion) completion;
}

bool TDEGlobalSettings::showContextMenusOnPress ()
{
    TDEConfigGroup g(TDEGlobal::config(), "ContextMenus");
    return g.readBoolEntry("ShowOnPress", true);
}

int TDEGlobalSettings::contextMenuKey ()
{
    TDEConfigGroup g(TDEGlobal::config(), "Shortcuts");
    TDEShortcut cut (g.readEntry ("PopupMenuContext", "Menu"));
    return cut.keyCodeQt();
}

TQColor TDEGlobalSettings::toolBarHighlightColor()
{
    initColors();
    TDEConfigGroup g( TDEGlobal::config(), "Toolbar style" );
    return g.readColorEntry("HighlightColor", _trinity4Blue);
}

TQColor TDEGlobalSettings::inactiveTitleColor()
{
#ifdef TQ_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTION));
#else
    if (!_inactiveBackground)
        _inactiveBackground = new TQColor(157, 170, 186);
    TDEConfigGroup g( TDEGlobal::config(), "WM" );
    return g.readColorEntry( "inactiveBackground", _inactiveBackground );
#endif
}

TQColor TDEGlobalSettings::inactiveTextColor()
{
#ifdef TQ_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_INACTIVECAPTIONTEXT));
#else
    if (!_inactiveForeground)
       _inactiveForeground = new TQColor(221,221,221);
    TDEConfigGroup g( TDEGlobal::config(), "WM" );
    return g.readColorEntry( "inactiveForeground", _inactiveForeground );
#endif
}

TQColor TDEGlobalSettings::activeTitleColor()
{
#ifdef TQ_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_ACTIVECAPTION));
#else
    initColors();
    if (!_activeBackground)
      _activeBackground = new TQColor(65,142,220);
    TDEConfigGroup g( TDEGlobal::config(), "WM" );
    return g.readColorEntry( "activeBackground", _activeBackground);
#endif
}

TQColor TDEGlobalSettings::activeTextColor()
{
#ifdef TQ_WS_WIN
    return qt_colorref2qrgb(GetSysColor(COLOR_CAPTIONTEXT));
#else
    TDEConfigGroup g( TDEGlobal::config(), "WM" );
    return g.readColorEntry( "activeForeground", &TQt::white );
#endif
}

int TDEGlobalSettings::contrast()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readNumEntry( "contrast", 7 );
}

TQColor TDEGlobalSettings::buttonBackground()
{
    if (!_buttonBackground)
      _buttonBackground = new TQColor(221,223,228);
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "buttonBackground", _buttonBackground );
}

TQColor TDEGlobalSettings::buttonTextColor()
{
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "buttonForeground", &TQt::black );
}

// IMPORTANT:
//  This function should be kept in sync with
//   TDEApplication::tdedisplaySetPalette()
TQColor TDEGlobalSettings::baseColor()
{
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "windowBackground", &TQt::white );
}

// IMPORTANT:
//  This function should be kept in sync with
//   TDEApplication::tdedisplaySetPalette()
TQColor TDEGlobalSettings::textColor()
{
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "windowForeground", &TQt::black );
}

// IMPORTANT:
//  This function should be kept in sync with
//   TDEApplication::tdedisplaySetPalette()
TQColor TDEGlobalSettings::highlightedTextColor()
{
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "selectForeground", &TQt::white );
}

// IMPORTANT:
//  This function should be kept in sync with
//   TDEApplication::tdedisplaySetPalette()
TQColor TDEGlobalSettings::highlightColor()
{
    initColors();
    if (!_selectBackground)
        _selectBackground = new TQColor(103,141,178);
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "selectBackground", _selectBackground );
}

TQColor TDEGlobalSettings::alternateBackgroundColor()
{
    initColors();
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    *alternateColor = calculateAlternateBackgroundColor( baseColor() );
    return g.readColorEntry( "alternateBackground", alternateColor );
}

TQColor TDEGlobalSettings::calculateAlternateBackgroundColor(const TQColor& base)
{
    if (base == TQt::white)
        return TQColor(238,246,255);
    else
    {
        int h, s, v;
        base.hsv( &h, &s, &v );
        if (v > 128)
            return base.dark(106);
        else if (base != TQt::black)
            return base.light(110);

        return TQColor(32,32,32);
    }
}

bool TDEGlobalSettings::shadeSortColumn()
{
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readBoolEntry( "shadeSortColumn", KDE_DEFAULT_SHADE_SORT_COLUMN );
}

TQColor TDEGlobalSettings::linkColor()
{
    initColors();
    if (!_linkColor)
        _linkColor = new TQColor(0,0,238);
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "linkColor", _linkColor );
}

TQColor TDEGlobalSettings::visitedLinkColor()
{
    if (!_visitedLinkColor)
        _visitedLinkColor = new TQColor(82,24,139);
    TDEConfigGroup g( TDEGlobal::config(), "General" );
    return g.readColorEntry( "visitedLinkColor", _visitedLinkColor );
}

TQFont TDEGlobalSettings::generalFont()
{
    if (_generalFont)
        return *_generalFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _generalFont = new TQFont("Sans Serif", 10);
    _generalFont->setPointSize(10);
    _generalFont->setStyleHint(TQFont::SansSerif);

    TDEConfigGroup g( TDEGlobal::config(), "General" );
    *_generalFont = g.readFontEntry("font", _generalFont);

    return *_generalFont;
}

TQFont TDEGlobalSettings::fixedFont()
{
    if (_fixedFont)
        return *_fixedFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _fixedFont = new TQFont("Monospace", 10);
    _fixedFont->setPointSize(10);
    _fixedFont->setStyleHint(TQFont::TypeWriter);

    TDEConfigGroup g( TDEGlobal::config(), "General" );
    *_fixedFont = g.readFontEntry("fixed", _fixedFont);

    return *_fixedFont;
}

TQFont TDEGlobalSettings::toolBarFont()
{
    if(_toolBarFont)
        return *_toolBarFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _toolBarFont = new TQFont("Sans Serif", 10);
    _toolBarFont->setPointSize(10);
    _toolBarFont->setStyleHint(TQFont::SansSerif);

    TDEConfigGroup g( TDEGlobal::config(), "General" );
    *_toolBarFont = g.readFontEntry("toolBarFont", _toolBarFont);

    return *_toolBarFont;
}

TQFont TDEGlobalSettings::menuFont()
{
    if(_menuFont)
        return *_menuFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _menuFont = new TQFont("Sans Serif", 10);
    _menuFont->setPointSize(10);
    _menuFont->setStyleHint(TQFont::SansSerif);

    TDEConfigGroup g( TDEGlobal::config(), "General" );
    *_menuFont = g.readFontEntry("menuFont", _menuFont);

    return *_menuFont;
}

TQFont TDEGlobalSettings::windowTitleFont()
{
    if(_windowTitleFont)
        return *_windowTitleFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _windowTitleFont = new TQFont("Sans Serif", 9, TQFont::Bold);
    _windowTitleFont->setPointSize(10);
    _windowTitleFont->setStyleHint(TQFont::SansSerif);

    TDEConfigGroup g( TDEGlobal::config(), "WM" );
    *_windowTitleFont = g.readFontEntry("activeFont", _windowTitleFont); // inconsistency

    return *_windowTitleFont;
}

TQFont TDEGlobalSettings::taskbarFont()
{
    if(_taskbarFont)
        return *_taskbarFont;

    // Sync default with tdebase/kcontrol/fonts/fonts.cpp
    _taskbarFont = new TQFont("Sans Serif", 10);
    _taskbarFont->setPointSize(10);
    _taskbarFont->setStyleHint(TQFont::SansSerif);

    TDEConfigGroup g( TDEGlobal::config(), "General" );
    *_taskbarFont = g.readFontEntry("taskbarFont", _taskbarFont);

    return *_taskbarFont;
}


TQFont TDEGlobalSettings::largeFont(const TQString &text)
{
    TQFontDatabase db;
    TQStringList fam = db.families();

    // Move a bunch of preferred fonts to the front.
    if (fam.remove("Arial"))
       fam.prepend("Arial");
    if (fam.remove("Verdana"))
       fam.prepend("Verdana");
    if (fam.remove("Tahoma"))
       fam.prepend("Tahoma");
    if (fam.remove("Lucida Sans"))
       fam.prepend("Lucida Sans");
    if (fam.remove("Lucidux Sans"))
       fam.prepend("Lucidux Sans");
    if (fam.remove("Nimbus Sans"))
       fam.prepend("Nimbus Sans");
    if (fam.remove("Gothic I"))
       fam.prepend("Gothic I");

    if (_largeFont)
        fam.prepend(_largeFont->family());

    for(TQStringList::ConstIterator it = fam.begin();
        it != fam.end(); ++it)
    {
        if (db.isSmoothlyScalable(*it) && !db.isFixedPitch(*it))
        {
            TQFont font(*it);
            font.setPixelSize(75);
            TQFontMetrics metrics(font);
            int h = metrics.height();
            if ((h < 60) || ( h > 90))
                continue;

            bool ok = true;
            for(unsigned int i = 0; i < text.length(); i++)
            {
                if (!metrics.inFont(text[i]))
                {
                    ok = false;
                    break;
                }
            }
            if (!ok)
                continue;

            font.setPointSize(48);
            _largeFont = new TQFont(font);
            return *_largeFont;
        }
    }
    _largeFont = new TQFont(TDEGlobalSettings::generalFont());
    _largeFont->setPointSize(48);
    return *_largeFont;
}

void TDEGlobalSettings::initStatic()
{
    // The method is primarily to ensure backward compatibility of the API.
    // Don't put anything else here.
    initPaths();
}

void TDEGlobalSettings::initPaths()
{
    if (s_desktopPath)
    {
      return;
    }

    s_autostartPath = new TQString();
    s_trashPath = new TQString();
    s_desktopPath = new TQString();
    s_documentPath = new TQString();
    s_downloadPath = new TQString();
    s_musicPath = new TQString();
    s_picturesPath = new TQString();
    s_publicSharePath = new TQString();
    s_templatesPath = new TQString();
    s_videosPath = new TQString();

    TDEConfigGroup g( TDEGlobal::config(), "Paths" );

    // Read xdg folder paths
    readXdgUserDirs(s_desktopPath, s_documentPath, s_downloadPath, s_musicPath,
                    s_picturesPath, s_publicSharePath, s_templatesPath, s_videosPath);

    *s_desktopPath = TQDir::cleanDirPath(*s_desktopPath);
    if (!s_desktopPath->endsWith("/"))
      s_desktopPath->append('/');

    *s_documentPath = TQDir::cleanDirPath(*s_documentPath);
    if (!s_documentPath->endsWith("/"))
      s_documentPath->append('/');

    *s_downloadPath = TQDir::cleanDirPath(*s_downloadPath);
    if (!s_downloadPath->endsWith("/"))
      s_downloadPath->append('/');

    *s_musicPath = TQDir::cleanDirPath(*s_musicPath);
    if (!s_musicPath->endsWith("/"))
      s_musicPath->append('/');

    *s_picturesPath = TQDir::cleanDirPath(*s_picturesPath);
    if (!s_picturesPath->endsWith("/"))
      s_picturesPath->append('/');

    *s_publicSharePath = TQDir::cleanDirPath(*s_publicSharePath);
    if (!s_publicSharePath->endsWith("/"))
      s_publicSharePath->append('/');

    *s_templatesPath = TQDir::cleanDirPath(*s_templatesPath);
    if (!s_templatesPath->endsWith("/"))
      s_templatesPath->append('/');

    *s_videosPath = TQDir::cleanDirPath(*s_videosPath);
    if (!s_videosPath->endsWith("/"))
      s_videosPath->append('/');

    // Trash Path - TODO remove in KDE4 (tdeio_trash can't use it for interoperability reasons)
    *s_trashPath = *s_desktopPath + i18n("Trash") + "/";
    *s_trashPath = g.readPathEntry( "Trash" , *s_trashPath);
    *s_trashPath = TQDir::cleanDirPath( *s_trashPath );
    if ( !s_trashPath->endsWith("/") )
      s_trashPath->append('/');
    // We need to save it in any case, in case the language changes later on,
    if ( !g.hasKey( "Trash" ) )
    {
      g.writePathEntry( "Trash", *s_trashPath, true, true );
      g.sync();
    }

    // Create folders if they do not exists.
    TDEConfig *xdgconfig = new TDEConfig(TQDir::homeDirPath()+"/.config/user-dirs.dirs");
    checkAndCreateXdgFolder("XDG_DESKTOP_DIR", *s_desktopPath, xdgconfig);
    checkAndCreateXdgFolder("XDG_DOCUMENTS_DIR", *s_documentPath, xdgconfig);
    checkAndCreateXdgFolder("XDG_DOWNLOAD_DIR", *s_downloadPath, xdgconfig);
    checkAndCreateXdgFolder("XDG_MUSIC_DIR", *s_musicPath, xdgconfig);
    checkAndCreateXdgFolder("XDG_PICTURES_DIR", *s_picturesPath, xdgconfig);
    checkAndCreateXdgFolder("XDG_PUBLICSHARE_DIR", *s_publicSharePath, xdgconfig);
    checkAndCreateXdgFolder("XDG_TEMPLATES_DIR", *s_templatesPath, xdgconfig);
    checkAndCreateXdgFolder("XDG_VIDEOS_DIR", *s_videosPath, xdgconfig);
    xdgconfig->sync();

    // Autostart Path
    *s_autostartPath = TDEGlobal::dirs()->localtdedir() + "Autostart/";
    *s_autostartPath = g.readPathEntry( "Autostart" , *s_autostartPath);
    *s_autostartPath = TQDir::cleanDirPath( *s_autostartPath );
    if (!s_autostartPath->endsWith("/"))
    {
      s_autostartPath->append('/');
    }
    if (!TQDir(*s_autostartPath).exists())
    {
      TDEStandardDirs::makeDir(*s_autostartPath);
    }

    // Make sure this app gets the notifications about those paths
    if (kapp)
        kapp->addKipcEventMask(KIPC::SettingsChanged);
}

void TDEGlobalSettings::initColors()
{
    if (!_trinity4Blue) {
      if (TQPixmap::defaultDepth() > 8)
        _trinity4Blue = new TQColor(103,141,178);
      else
        _trinity4Blue = new TQColor(0, 0, 192);
    }
    if (!alternateColor)
      alternateColor = new TQColor(237, 244, 249);
}

void TDEGlobalSettings::rereadFontSettings()
{
    delete _generalFont;
    _generalFont = 0L;
    delete _fixedFont;
    _fixedFont = 0L;
    delete _menuFont;
    _menuFont = 0L;
    delete _toolBarFont;
    _toolBarFont = 0L;
    delete _windowTitleFont;
    _windowTitleFont = 0L;
    delete _taskbarFont;
    _taskbarFont = 0L;
}

void TDEGlobalSettings::rereadPathSettings()
{
    kdDebug() << "TDEGlobalSettings::rereadPathSettings" << endl;
    delete s_autostartPath;
    s_autostartPath = 0L;
    delete s_trashPath;
    s_trashPath = 0L;
    delete s_desktopPath;
    s_desktopPath = 0L;
    delete s_documentPath;
    s_documentPath = 0L;
    delete s_downloadPath;
    s_downloadPath = 0L;
    delete s_musicPath;
    s_musicPath = 0L;
    delete s_picturesPath;
    s_picturesPath = 0L;
    delete s_publicSharePath;
    s_publicSharePath = 0L;
    delete s_templatesPath;
    s_templatesPath = 0L;
    delete s_videosPath;
    s_videosPath = 0L;
}

TDEGlobalSettings::KMouseSettings & TDEGlobalSettings::mouseSettings()
{
    if ( ! s_mouseSettings )
    {
        s_mouseSettings = new KMouseSettings;
        KMouseSettings & s = *s_mouseSettings; // for convenience

#ifndef TQ_WS_WIN
        TDEConfigGroup g( TDEGlobal::config(), "Mouse" );
        TQString setting = g.readEntry("MouseButtonMapping");
        if (setting == "RightHanded")
            s.handed = KMouseSettings::RightHanded;
        else if (setting == "LeftHanded")
            s.handed = KMouseSettings::LeftHanded;
        else
        {
#ifdef TQ_WS_X11
            // get settings from X server
            // This is a simplified version of the code in input/mouse.cpp
            // Keep in sync !
            s.handed = KMouseSettings::RightHanded;
            unsigned char map[20];
            int num_buttons = XGetPointerMapping(kapp->getDisplay(), map, 20);
            if( num_buttons == 2 )
            {
                if ( (int)map[0] == 1 && (int)map[1] == 2 )
                    s.handed = KMouseSettings::RightHanded;
                else if ( (int)map[0] == 2 && (int)map[1] == 1 )
                    s.handed = KMouseSettings::LeftHanded;
            }
            else if( num_buttons >= 3 )
            {
                if ( (int)map[0] == 1 && (int)map[2] == 3 )
                    s.handed = KMouseSettings::RightHanded;
                else if ( (int)map[0] == 3 && (int)map[2] == 1 )
                    s.handed = KMouseSettings::LeftHanded;
            }
#else
        // FIXME(E): Implement in Qt Embedded
#endif
        }
#endif //TQ_WS_WIN
    }
#ifdef TQ_WS_WIN
    //not cached
    s_mouseSettings->handed = (GetSystemMetrics(SM_SWAPBUTTON) ? KMouseSettings::LeftHanded : KMouseSettings::RightHanded);
#endif
    return *s_mouseSettings;
}

void TDEGlobalSettings::rereadMouseSettings()
{
#ifndef TQ_WS_WIN
    delete s_mouseSettings;
    s_mouseSettings = 0L;
#endif
}

bool TDEGlobalSettings::isMultiHead()
{
#ifdef TQ_WS_WIN
    return GetSystemMetrics(SM_CMONITORS) > 1;
#else
    TQCString multiHead = getenv("TDE_MULTIHEAD");
    if (!multiHead.isEmpty()) {
        return (multiHead.lower() == "true");
    }
    return false;
#endif
}

bool TDEGlobalSettings::wheelMouseZooms()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry( "WheelMouseZooms", KDE_DEFAULT_WHEEL_ZOOM );
}

TQRect TDEGlobalSettings::splashScreenDesktopGeometry()
{
    TQDesktopWidget *dw = TQApplication::desktop();

    if (dw->isVirtualDesktop()) {
        TDEConfigGroup group(TDEGlobal::config(), "Windows");
        int scr = group.readNumEntry("Unmanaged", -3);
        if (group.readBoolEntry("XineramaEnabled", true) && scr != -2) {
            if (scr == -3)
                scr = dw->screenNumber(TQCursor::pos());
            return dw->screenGeometry(scr);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

TQRect TDEGlobalSettings::desktopGeometry(const TQPoint& point)
{
    TQDesktopWidget *dw = TQApplication::desktop();

    if (dw->isVirtualDesktop()) {
        TDEConfigGroup group(TDEGlobal::config(), "Windows");
        if (group.readBoolEntry("XineramaEnabled", true) &&
            group.readBoolEntry("XineramaPlacementEnabled", true)) {
            return dw->screenGeometry(dw->screenNumber(point));
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

TQRect TDEGlobalSettings::desktopGeometry(TQWidget* w)
{
    TQDesktopWidget *dw = TQApplication::desktop();

    if (dw->isVirtualDesktop()) {
        TDEConfigGroup group(TDEGlobal::config(), "Windows");
        if (group.readBoolEntry("XineramaEnabled", true) &&
            group.readBoolEntry("XineramaPlacementEnabled", true)) {
            if (w)
                return dw->screenGeometry(dw->screenNumber(w));
            else return dw->screenGeometry(-1);
        } else {
            return dw->geometry();
        }
    } else {
        return dw->geometry();
    }
}

bool TDEGlobalSettings::showIconsOnPushButtons()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry("ShowIconsOnPushButtons",
        KDE_DEFAULT_ICON_ON_PUSHBUTTON);
}

bool TDEGlobalSettings::showFilePreview(const KURL &url)
{
    TDEConfigGroup g(TDEGlobal::config(), "PreviewSettings");
    TQString protocol = url.protocol();
    bool defaultSetting = KProtocolInfo::showFilePreview( protocol );
    return g.readBoolEntry(protocol, defaultSetting );
}

bool TDEGlobalSettings::showKonqIconActivationEffect()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry("ShowKonqIconActivationEffect",
        KDE_DEFAULT_KONQ_ACTIVATION_EFFECT);
}

bool TDEGlobalSettings::opaqueResize()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readBoolEntry("OpaqueResize",
        KDE_DEFAULT_OPAQUE_RESIZE);
}

int TDEGlobalSettings::buttonLayout()
{
    TDEConfigGroup g( TDEGlobal::config(), "KDE" );
    return g.readNumEntry("ButtonLayout",
        KDE_DEFAULT_BUTTON_LAYOUT);
}
