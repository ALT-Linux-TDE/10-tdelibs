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
 *
 * client.h: client to access tdesud.
 */

#ifndef __KDE_su_Client_h_Included__
#define __KDE_su_Client_h_Included__

#include <tqglobal.h>
#include <tdelibs_export.h>

#ifdef Q_OS_UNIX

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>

#include <tqcstring.h>
#include <tqvaluelist.h>

typedef TQValueList<TQCString> QCStringList;

/**
 * A client class to access tdesud, the KDE su daemon. Kdesud can assist in 
 * password caching in two ways:
 *
 * @li For high security passwords, like for su and ssh, it executes the
 * password requesting command for you. It feeds the password to the
 * command, without ever returning it to you, the user. The daemon should 
 * be installed setgid nogroup, in order to be able to act as an inaccessible, 
 * trusted 3rd party. 
 * See exec, setPass, delCommand.
 *
 * @li For lower security passwords, like web and ftp passwords, it can act
 * as a persistent storage for string variables. These variables are
 * returned to the user, and the daemon doesn't need to be setgid nogroup
 * for this.
 * See setVar, delVar, delGroup.
 */

class TDESU_EXPORT TDEsuClient {
public:
    TDEsuClient();
    ~TDEsuClient();

    /**
     * Lets tdesud execute a command. If the daemon does not have a password
     * for this command, this will fail and you need to call setPass().
     *
     * @param command The command to execute.
     * @param user The user to run the command as.
     * @param options Extra options.
     * @param env Extra environment variables.
     * @return Zero on success, -1 on failure.
     */
    int exec(const TQCString &command, const TQCString &user, const TQCString &options=0, const QCStringList &env=QCStringList());

    /**
     * Wait for the last command to exit and return the exit code.
     * @return Exit code of last command, -1 on failure.
     */
    int exitCode();

    /**
     * Set root's password, lasts one session.
     *
     * @param pass Root's password.
     * @param timeout The time that a password will live.
     * @return Zero on success, -1 on failure.
     */
    int setPass(const char *pass, int timeout);

    /**
     * Set the target host (optional).
     */
    int setHost(const TQCString &host);

    /**
     * Set the desired priority (optional), see StubProcess.
     */
    int setPriority(int priority);

    /**
     * Set the desired scheduler (optional), see StubProcess.
     */
    int setScheduler(int scheduler);

    /**
     * Remove a password for a user/command.
     * @param command The command.
     * @param user The user.
     * @return zero on success, -1 on an error
     */
    int delCommand(const TQCString &command, const TQCString &user);

    /**
     * Set a persistent variable.
     * @param key The name of the variable.
     * @param value Its value.
     * @param timeout The timeout in seconds for this key. Zero means
     * no timeout.
     * @param group Make the key part of a group. See delGroup.
     * @return zero on success, -1 on failure.
     */
    int setVar(const TQCString &key, const TQCString &value, int timeout=0, const TQCString &group=0);

    /**
     * Get a persistent variable.
     * @param key The name of the variable.
     * @return Its value.
     */
    TQCString getVar(const TQCString &key);

    /**
     * Gets all the keys that are membes of the given group.
     * @param group the group name of the variables.
     * @return a list of the keys in the group.
     */
    TQValueList<TQCString> getKeys(const TQCString &group);

    /**
     * Returns true if the specified group exists is
     * cached.
     *
     * @param group the group key
     * @return true if the group is found
     */
    bool findGroup(const TQCString &group);

    /**
     * Delete a persistent variable.
     * @param key The name of the variable.
     * @return zero on success, -1 on failure.
     */
    int delVar(const TQCString &key);

    /**
     * Delete all persistent variables with the given key.
     *
     * A specicalized variant of delVar(TQCString) that removes all
     * subsets of the cached varaibles given by @p key. In order for all
     * cached variables related to this key to be deleted properly, the
     * value given to the @p group argument when the setVar function
     * was called, must be a subset of the argument given here and the key
     *
     * @note Simply supplying the group key here WILL not necessarily
     * work. If you only have a group key, then use delGroup instead.
     *
     * @param special_key the name of the variable.
     * @return zero on success, -1 on failure.
     */
    int delVars(const TQCString &special_key);

    /**
     * Delete all persistent variables in a group.
     *
     * @param group the group name. See setVar.
     * @return
     */
    int delGroup(const TQCString &group);

    /**
     * Ping tdesud. This can be used for diagnostics.
     * @return Zero on success, -1 on failure
     */
    int ping();

    /**
     * Stop the daemon.
     */
    int stopServer();

    /**
     * Try to start up tdesud
     */
    int startServer();

    /**
     * Returns true if the server is safe (installed setgid), false otherwise.
     */
    bool isServerSGID();

private:
    int connect();

    int sockfd;
    TQCString sock;

    int command(const TQCString &cmd, TQCString *result=0L);
    TQCString escape(const TQCString &str);

    class TDEsuClientPrivate;
    TDEsuClientPrivate *d;
};

#endif //Q_OS_UNIX

#endif //__KDE_su_Client_h_Included__
