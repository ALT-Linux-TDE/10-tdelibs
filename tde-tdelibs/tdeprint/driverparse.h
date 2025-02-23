/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *  Copyright (c) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#ifndef DRIVERPARSE_H
#define DRIVERPARSE_H

#include <stdio.h>

#if !defined(UNUSED)
#  if defined(__GNUC__) && ( __GNUC__ > 2 || (__GNUC__ == 2 && __GNUC_MINOR__ > 4) )
#    define UNUSED(x) UNUSED_ ## x __attribute__((__unused__))
#  else
#    define UNUSED(x) UNUSED_ ## x
#  endif
#endif

void registerHandler(const char *name, void(*initf)(const char*), int(*parsef)(const char*, const char*, const char*, FILE*));
void initFoomatic(void);
int execute(int argc, char *argv[]);
void addFile(const char *filename, const char *origin, const char *metadata);

#endif
