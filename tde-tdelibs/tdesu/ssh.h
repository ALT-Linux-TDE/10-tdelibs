/*
 *
 * $Id$
 *
 * This file is part of the KDE project, module tdesu.
 * Copyright (C) 2000 Geert Jansen <jansen@kde.org>
 * 
 * This is free software; you can use this library under the GNU Library 
 * General Public License, version 2. See the file "COPYING.LIB" for the 
 * exact licensing terms.
 */

#ifndef __SSH_h_Included__
#define __SSH_h_Included__

#include <tqcstring.h>

#include "stub.h"

#include <tdelibs_export.h>

/**
 * Executes a remote command, using ssh.
 */

class TDESU_EXPORT SshProcess: public StubProcess
{
public:
    SshProcess(const TQCString &host=0, const TQCString &user=0, const TQCString &command=0);
    ~SshProcess();

    enum Errors { SshNotFound=1, SshNeedsPassword, SshIncorrectPassword };

    /**
     * Sets the target host.
     */
    void setHost(const TQCString &host) { m_Host = host; }

    /**
     * Sets the localtion of the remote stub.
     */
    void setStub(const TQCString &stub);

    /** 
     * Checks if the current user\@host needs a password. 
     * @return The prompt for the password if a password is required. A null
     * string otherwise.
     *
     * @todo The return doc is so obviously wrong that the C code needs to be checked.
     */
    int checkNeedPassword();

    /**
     * Checks if the stub is installed and if the password is correct.
     * @return Zero if everything is correct, nonzero otherwise.
     */
    int checkInstall(const char *password);

    /**
     * Executes the command.
     */
    int exec(const char *password, int check=0);

    TQCString prompt() { return m_Prompt; }
    TQCString error() { return m_Error; }

protected:
    virtual TQCString display();
    virtual TQCString displayAuth();
    virtual TQCString dcopServer();

private:
    TQCString dcopForward();
    int ConverseSsh(const char *password, int check);

    int m_dcopPort;
    int  m_dcopSrv;
    TQCString m_Prompt;
    TQCString m_Host;
    TQCString m_Error;
    TQCString m_Stub;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class SshProcessPrivate;
    SshProcessPrivate *d;
};

#endif
