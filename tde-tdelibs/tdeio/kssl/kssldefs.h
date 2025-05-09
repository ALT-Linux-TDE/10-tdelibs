/* This file is part of the KDE project
 *
 * Copyright (C) 2000-2003 George Staikos <staikos@kde.org>
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
 */ 
#ifndef __kssldefs_h
#define __kssldefs_h

#ifdef TQ_WS_WIN
#include "ksslconfig_win.h"
#else
#include "ksslconfig.h"
#endif

#ifdef HAVE_OLD_SSL_API
#define OPENSSL_malloc Malloc
#define OPENSSL_malloc_locked Malloc_locked
#define OPENSSL_realloc Realloc
#define OPENSSL_free Free
#endif

#endif

