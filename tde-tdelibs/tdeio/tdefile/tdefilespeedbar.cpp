/* This file is part of the KDE libraries
    Copyright (C) 2002 Carsten Pfeiffer <pfeiffer@kde.org>

    library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation, version 2.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include "tdefilespeedbar.h"
#include "config-tdefile.h"

#include <tqdir.h>
#include <tqfile.h>
#include <tqtextcodec.h>
#include <tqtextstream.h>

#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tdelocale.h>
#include <kprotocolinfo.h>
#include <kstandarddirs.h>
#include <kurl.h>

KFileSpeedBar::KFileSpeedBar( TQWidget *parent, const char *name )
    : KURLBar( true, parent, name )
{
    TDEConfig *config = TDEGlobal::config();
    TDEConfigGroupSaver cs( config, ConfigGroup );
    m_initializeSpeedbar = config->readBoolEntry( "Set speedbar defaults",
                                                   true );
    setIconSize(TDEIcon::SizeSmallMedium);
    readConfig( TDEGlobal::config(), "KFileDialog Speedbar" );

    if ( m_initializeSpeedbar )
    {
        insertItem(TQDir::homeDirPath(), i18n("Home Folder"), false, "folder_home");
		insertItem(TDEGlobalSettings::desktopPath(), i18n("Desktop"), false, "desktop");
		insertItem(TDEGlobalSettings::documentPath(), i18n("Documents"), false, "folder_wordprocessing");
		insertItem(TDEGlobalSettings::downloadPath(), i18n( "Downloads" ), false, "folder_html");
		insertItem(TDEGlobalSettings::musicPath(), i18n( "Music" ), false, "folder_sound");
		insertItem(TDEGlobalSettings::picturesPath(), i18n( "Pictures" ), false, "folder_image");
		insertItem(TDEGlobalSettings::publicSharePath(), i18n( "Public" ), false, "folder_open");
		insertItem(TDEGlobalSettings::templatesPath(), i18n( "Templates" ), false, "folder_grey");
		insertItem(TDEGlobalSettings::videosPath(), i18n( "Videos" ), false, "folder_video");

        KURL u = "media:/";
        if (KProtocolInfo::isKnownProtocol(u))
        {
            insertItem(u, i18n("Storage Media"), false, KProtocolInfo::icon("media"));
        }

        u = "remote:/";
        if (KProtocolInfo::isKnownProtocol(u))
        {
            insertItem(u, i18n("Network Folders"), false, KProtocolInfo::icon("remote"));
        }
    }
}

KFileSpeedBar::~KFileSpeedBar()
{
}

void KFileSpeedBar::save( TDEConfig *config )
{
    if ( m_initializeSpeedbar && isModified() )
    {
        TDEConfigGroup conf( config, ConfigGroup );
        // write to kdeglobals
        conf.writeEntry( "Set speedbar defaults", false, true, true );
    }

    writeConfig( config, "KFileDialog Speedbar" );
}

TQSize KFileSpeedBar::sizeHint() const
{
    TQSize sizeHint = KURLBar::sizeHint();
    int ems = fontMetrics().width("mmmmmmmmmmmm");
    if (sizeHint.width() < ems)
    {
        sizeHint.setWidth(ems);
    }
    return sizeHint;
}

#include "tdefilespeedbar.moc"
