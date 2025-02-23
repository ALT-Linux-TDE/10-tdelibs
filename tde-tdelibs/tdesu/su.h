/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __SU_h_Included__
#define __SU_h_Included__

#include <tqcstring.h>

#include <tdelibs_export.h>

#include "stub.h"

/**
 * Executes a command under elevated privileges, using su.
 */

class TDESU_EXPORT SuProcess: public StubProcess
{
public:
    SuProcess(const TQCString &user=0, const TQCString &command=0);
    ~SuProcess();

    enum Errors { SuNotFound=1, SuNotAllowed, SuIncorrectPassword };

    /**
     * Executes the command. This will wait for the command to finish.
     */
    enum checkMode { NoCheck=0, Install=1, NeedPassword=2 } ;
    int exec(const char *password, int check=NoCheck);

    /** 
     * Checks if the stub is installed and the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /**
     * Checks if a password is needed.
     */
    int checkNeedPassword();

private:
    enum SuErrors { error=-1, ok=0, killme=1, notauthorized=2 } ;
    int ConverseSU(const char *password);

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SuProcessPrivate;
    SuProcessPrivate *d;
    TQString superUserCommand;
};

#endif
