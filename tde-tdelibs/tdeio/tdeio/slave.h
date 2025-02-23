/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>
 *                2000 Stephan Kulow <coolo@kde.org>
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

#ifndef TDEIO_SLAVE_H
#define TDEIO_SLAVE_H

#include <time.h>
#include <unistd.h>

#include <tqobject.h>

#include <kurl.h>

#include "tdeio/slaveinterface.h"
#include "tdeio/connection.h"

class TDEServerSocket;
class TDESocket;

namespace TDEIO {

    /** Attention developers: If you change the implementation of TDEIO::Slave,
    * do *not* use connection() or slaveconn but the respective TDEIO::Slave
    * accessor methods. Otherwise classes derived from Slave might break. (LS)
    */
    class TDEIO_EXPORT Slave : public TDEIO::SlaveInterface
    {
	TQ_OBJECT
	

    protected:
	/**
	 * Use this constructor if you derive your own class from Slave
	 * @p derived must be true in any case
	 * @internal
	 * @since 3.2
	 */
	Slave(bool derived, TDEServerSocket *unixdomain, const TQString &protocol,
		const TQString &socketname);	// TODO(BIC): Remove in KDE 4

    public:
	Slave(TDEServerSocket *unixdomain,
	      const TQString &protocol, const TQString &socketname);

        virtual ~Slave();

	void setPID(pid_t);

        int slave_pid() { return m_pid; }

	/**
	 * Force termination
	 */
	void kill();

        /**
         * @return true if the slave survived the last mission.
         */
        bool isAlive() { return !dead; }

        /**
         * Set host for url
         * @param host to connect to.
         * @param port to connect to.
         * @param user to login as
         * @param passwd to login with
         */
        void setHost( const TQString &host, int port,
                      const TQString &user, const TQString &passwd); // TODO(BIC): make virtual

        /**
         * Clear host info.
         */
        void resetHost();

        /**
         * Configure slave
         */
        void setConfig(const MetaData &config);	// TODO(BIC): make virtual

        /**
	 * The protocol this slave handles.
	 *
         * @return name of protocol handled by this slave, as seen by the user
         */
        TQString protocol() { return m_protocol; }

        void setProtocol(const TQString & protocol);
        /**
	 * The actual protocol used to handle the request.
	 *
	 * This method will return a different protocol than
	 * the one obtained by using protocol() if a
	 * proxy-server is used for the given protocol.  This
	 * usually means that this method will return "http"
	 * when the actuall request was to retrieve a resource
	 * from an "ftp" server by going through a proxy server.
	 *
         * @return the actual protocol (io-slave) that handled the request
         */
        TQString slaveProtocol() { return m_slaveProtocol; }

        /**
         * @return Host this slave is (was?) connected to
         */
        TQString host() { return m_host; }

        /**
         * @return port this slave is (was?) connected to
         */
        int port() { return m_port; }

        /**
         * @return User this slave is (was?) logged in as
         */
        TQString user() { return m_user; }

        /**
         * @return Passwd used to log in
         */
        TQString passwd() { return m_passwd; }

	/**
	 * Creates a new slave.
	 *
	 * @param protocol protocol the slave is for.
	 * @param url URL the slave should operate on.
	 * @param error is the error code on failure and undefined else.
	 * @param error_text is the error text on failure and undefined else.
	 *
	 * @return 0 on failure, or a pointer to a slave otherwise.
	 * @todo What are legal @p protocol values?
	 */
	static Slave* createSlave( const TQString &protocol, const KURL& url, int& error, TQString& error_text );

        static Slave* holdSlave( const TQString &protocol, const KURL& url );

	// == communication with connected tdeioslave ==
	// whenever possible prefer these methods over the respective
	// methods in connection()
	/**
	 * Suspends the operation of the attached tdeioslave.
	 */
        void suspend();		// TODO(BIC): make virtual
	/**
	 * Resumes the operation of the attached tdeioslave.
	 */
        void resume();		// TODO(BIC): make virtual
	/**
	 * Tells wether the tdeioslave is suspended.
	 * @return true if the tdeioslave is suspended.
	 * @since 3.2
	 */
        bool suspended();	// TODO(BIC): make virtual
	/**
	 * Sends the given command to the tdeioslave.
	 * @param cmd command id
	 * @param data byte array containing data
	 * @since 3.2
	 */
        void send(int cmd, const TQByteArray &data = TQByteArray());// TODO(BIC): make virtual
	// == end communication with connected tdeioslave ==

	/**
	 * Puts the tdeioslave associated with @p url at halt.
	 */
	void hold(const KURL &url);	// TODO(BIC): make virtual

	/**
	 * @return The time this slave has been idle.
	 */
	time_t idleTime();

	/**
	 * Marks this slave as idle.
	 */
	void setIdle();

        /*
         * @returns Whether the slave is connected
         * (Connection oriented slaves only)
         */
        bool isConnected() { return contacted; }
        void setConnected(bool c) { contacted = c; }

	/** @deprecated This method is obsolete, use the accessor methods
	  * within TDEIO::Slave instead. Old code directly accessing connection()
	  * will not be able to access special protocols.
	  */
        TDE_DEPRECATED Connection *connection() { return &slaveconn; }	// TODO(BIC): remove before KDE 4

        void ref() { m_refCount++; }
        void deref() { m_refCount--; if (!m_refCount) delete this; }

    public slots:
        void accept(TDESocket *socket);
	void gotInput();
	void timeout();
    signals:
        void slaveDied(TDEIO::Slave *slave);

    protected:
        void unlinkSocket();

    private:
        TQString m_protocol;
        TQString m_slaveProtocol;
        TQString m_host;
        int m_port;
        TQString m_user;
        TQString m_passwd;
	TDEServerSocket *serv;
	TQString m_socket;
	pid_t m_pid;
	bool contacted;
	bool dead;
	time_t contact_started;
	time_t idle_since;
	TDEIO::Connection slaveconn;
	int m_refCount;
    protected:
	virtual void virtual_hook( int id, void* data );
	// grant SlaveInterface all IDs < 0x200
	enum { VIRTUAL_SUSPEND = 0x200, VIRTUAL_RESUME, VIRTUAL_SEND,
		VIRTUAL_HOLD, VIRTUAL_SUSPENDED,
		VIRTUAL_SET_HOST, VIRTUAL_SET_CONFIG };
	struct SendParams {
	  int cmd;
	  const TQByteArray *arr;
	};
	struct HoldParams {
	  const KURL *url;
	};
	struct SuspendedParams {
	  bool retval;
	};
	struct SetHostParams {
	  const TQString *host;
	  int port;
	  const TQString *user;
	  const TQString *passwd;
	};
	struct SetConfigParams {
	  const MetaData *config;
	};
    private:
	class SlavePrivate* d;
    };

}

#endif
