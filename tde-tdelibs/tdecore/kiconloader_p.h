/*
 *
 * This file is part of the KDE project, module tdecore.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 *                    Antonio Larrosa <larrosa@kde.org>
 *
 * This is free software; it comes under the GNU Library General
 * Public License, version 2. See the file "COPYING.LIB" for the
 * exact licensing terms.
 */

#ifndef __TDEIconLoader_p_h_Included__
#define __TDEIconLoader_p_h_Included__

#include <tqobject.h>
#include <tqstringlist.h>
#include <kicontheme.h>
#include <kiconloader.h>
#include <kiconeffect.h>
#include <tqdict.h>

class TDEIconThemeNode
{
public:
    TDEIconThemeNode(TDEIconTheme *_theme);
    ~TDEIconThemeNode();

    void queryIcons(TQStringList *lst, int size, TDEIcon::Context context) const;
    void queryIconsByContext(TQStringList *lst, int size, TDEIcon::Context context) const;
    TDEIcon findIcon(const TQString& name, int size, TDEIcon::MatchType match) const;
    void printTree(TQString& dbgString) const;

    TDEIconTheme *theme;
};

class TDEIconLoaderPrivate : public TQObject
{
    TQ_OBJECT
public:
    TQStringList mThemesInTree;
    TDEIconGroup *mpGroups;
    TDEIconThemeNode *mpThemeRoot;
    TDEStandardDirs *mpDirs;
    TDEIconLoader *q;
    TDEIconEffect mpEffect;
    TQDict<TQImage> imgDict;
    TQImage lastImage; // last loaded image without effect applied
    TQString lastImageKey; // key for icon without effect
    TQString appname;
    int lastIconType; // see TDEIcon::type
    int lastIconThreshold; // see TDEIcon::threshold
    TQPtrList<TDEIconThemeNode> links;
    bool extraDesktopIconsLoaded;
    bool delayedLoading;

public slots:
    void reconfigure();
};

#endif // __TDEIconLoader_p_h_Included__
