/*
 $Id$

 This file is part of the KDE libraries
 (c) 2002 Maksim Orlovich <mo002j@mail.rochester.edu>,

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

#ifndef TDESTYLE_DIRS_H
#define TDESTYLE_DIRS_H


#include <tqsettings.h>
#include <tqstringlist.h>
#include <kstandarddirs.h>

/**
* @short Access to the standard KDE directories for the pixmap style
* @author Maksim Orlovich<mo002j@mail.rochester.edu> is responsible for this file,
    but all the interesting work is done by TDEStandardDirs
* @version $Id$
*
* This class provides a this wrapper for styles around TDEStandardDirs,
* permitting integration with TQSettings and easy loading of pixmaps
*
* It add share/apps/tdestyle/themes as "themerc",
*    share/apps/tdestyle/pixmaps "themepixmap"
*/
class TDEStyleDirs: public TDEStandardDirs
{
public:
    static TDEStyleDirs* dirs()
    {
        if ( !instance)
            instance = new TDEStyleDirs;
        return instance;
    }

    static void release()
    {
        delete instance;
        instance = 0;
    }

    /**
    Adds all of KDE directories of type type to the seach path of q.

    For example, when one does the following:
    TQSettings settings;
    TDEStyleDirs dirs;
    dirs.addToSearch("config",settings);

    The one can do settings.readEntry("tdestyle/TDE/WidgetStyle") to access a settings in tdestylerc.
    */
    void addToSearch( const char* type, TQSettings& q) const; //Better name?

protected:
    static TDEStyleDirs* instance;
    /**
    Creates an instance of the class, and calculates the path information.
    */
    TDEStyleDirs();
    TDEStyleDirs(const TDEStyleDirs&);
    TDEStyleDirs& operator= (const TDEStyleDirs&);

    virtual ~TDEStyleDirs();
};

#endif
