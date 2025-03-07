/*
 * Copyright (C)2000 Waldo Bastian <bastian@kde.org>
 *
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE AUTHOR OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 */
#ifndef __TDERECENTDIRS_H
#define __TDERECENTDIRS_H

#include <tqstringlist.h>

#include <tdelibs_export.h>

/**
 * The goal of this class is to make sure that, when the user needs to 
 * specify a file via the file selection dialog, this dialog will start 
 * in the directory most likely to contain the desired files.
 *
 * This works as follows: Each time the file selection dialog is
 * shown, the programmer can specify a "file-class". The file-dialog will
 * then start with the directory associated with this file-class. When
 * the dialog closes, the directory currently shown in the file-dialog
 * will be associated with the file-class.
 *
 * A file-class can either start with ':' or with '::'. If it starts with
 * a single ':' the file-class is specific to the current application.
 * If the file-class starts with '::' it is global to all applications.
 */
class TDEIO_EXPORT TDERecentDirs
{
public:
    /**
     * Returns a list of directories associated with this file-class.
     * The most recently used directory is at the front of the list.
     */
    static TQStringList list(const TQString &fileClass);
    
    /**
     * Returns the most recently used directory accociated with this file-class.
     */
    static TQString dir(const TQString &fileClass);
    
    /**
     * Associates @p directory with @p fileClass
     */
    static void add(const TQString &fileClass, const TQString &directory);
};

#endif
