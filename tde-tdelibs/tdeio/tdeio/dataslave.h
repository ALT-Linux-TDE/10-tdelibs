/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2003 Leo Savernik <l.savernik@aon.at>
 *  Derived from slave.h
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

#ifndef __TDEIO_DATASLAVE_H__
#define __TDEIO_DATASLAVE_H__

#include <tdeio/global.h>
#include <tdeio/slave.h>

class TQTimer;

// don't forget to sync DISPATCH_IMPL in dataslave.h
#define DISPATCH_DECL(type) \
	void dispatch_##type();

// don't forget to sync DISPATCH_IMPL1 in dataslave.h
#define DISPATCH_DECL1(type, paramtype, param) \
	void dispatch_##type(paramtype param);

namespace TDEIO {

    /**
     * This class provides a high performance implementation for the data
     * url scheme (rfc2397).
     *
     * @internal
     * Do not use this class in external applications. It is an implementation
     * detail of TDEIO and subject to change without notice.
     * @author Leo Savernik
     */
    class DataSlave : public TDEIO::Slave {
	TQ_OBJECT
    public:
	DataSlave();

        virtual ~DataSlave();

        virtual void setHost(const TQString &host, int port,
			const TQString &user, const TQString &passwd);
	virtual void setConfig(const MetaData &config);

        virtual void suspend();
        virtual void resume();
        virtual bool suspended();
        virtual void send(int cmd, const TQByteArray &data = TQByteArray());

	virtual void hold(const KURL &url);

	// pure virtual methods that are defined by the actual protocol
	virtual void get(const KURL &url) = 0;
	virtual void mimetype(const KURL &url) = 0;

    protected:
        /**
	 * Sets metadata
	 * @internal
	 */
	void setAllMetaData(const MetaData &);
	/**
	 * Sends metadata set with setAllMetaData
	 * @internal
	 */
	void sendMetaData();

	// queueing methods
	/** identifiers of functions to be queued */
	enum QueueType { Queue_mimeType = 1, Queue_totalSize,
		Queue_sendMetaData, Queue_data, Queue_finished };
	/** structure for queueing. It is very primitive, it doesn't
	 * even try to conserve memory.
	 */
	struct QueueStruct {
	  QueueType type;
	  TQString s;
	  TDEIO::filesize_t size;
	  TQByteArray ba;

	  QueueStruct() {}
	  QueueStruct(QueueType type) : type(type) {}
	};
        typedef TQValueList<QueueStruct> DispatchQueue;
	DispatchQueue dispatchQueue;

	DISPATCH_DECL1(mimeType, const TQString &, s)
	DISPATCH_DECL1(totalSize, TDEIO::filesize_t, size)
	DISPATCH_DECL(sendMetaData)
	DISPATCH_DECL1(data, const TQByteArray &, ba)
	DISPATCH_DECL(finished)

    protected slots:
	/** dispatches next queued method. Does nothing if there are no
	 * queued methods.
	 */
	void dispatchNext();
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
	MetaData meta_data;
	bool _suspended;
	TQTimer *timer;
    };

}

#undef DISPATCH_DECL
#undef DISPATCH_DECL1

#endif /*__TDEIO_DATASLAVE_H__*/
