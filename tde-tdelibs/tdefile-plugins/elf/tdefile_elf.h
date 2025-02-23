/* This file is part of the KDE project
 * Copyright (C) 2002 Shane Wright <me@shanewright.co.uk>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#ifndef __TDEFILE_BMP_H__
#define __TDEFILE_BMP_H__

#include <tdefilemetainfo.h>

// Elven things
extern "C" {
	#include <libr.h>
	#include <libr-icons.h>
}

#include <string.h>

#include <sys/stat.h>
#include <stdlib.h>
#include <stdio.h>

class TQStringList;

class KElfPlugin: public KFilePlugin
{
    TQ_OBJECT
  
    
public:
    KElfPlugin( TQObject *parent, const char *name, const TQStringList& args );
    
    virtual bool readInfo( KFileMetaInfo& info, uint what);
};

#endif
