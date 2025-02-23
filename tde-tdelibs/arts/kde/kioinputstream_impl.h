	/*

	Copyright (C) 2001 Nikolas Zimmermann <wildfox@kde.org>

	This library is free software; you can redistribute it and/or
	modify it under the terms of the GNU Library General Public
	License as published by the Free Software Foundation; either
	version 2 of the License, or (at your option) any later version.
  
	This library is distributed in the hope that it will be useful,
	but WITHOUT ANY WARRANTY; without even the implied warranty of
	MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
	Library General Public License for more details.

	You should have received a copy of the GNU Library General Public License
	along with this library; see the file COPYING.LIB.  If not, write to
	the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
	Boston, MA 02110-1301, USA.

	*/

#ifndef KIOINPUTSTREAM_IMPL
#define KIOINPUTSTREAM_IMPL

#include <tqobject.h>
#include <tqcstring.h>
#include <tdeio/jobclasses.h>
#include <kurl.h>
#include "artskde.h"
#include "stdsynthmodule.h" 

namespace Arts {

class TDEIOInputStream_impl : public TQObject, virtual public TDEIOInputStream_skel, 
					    virtual public InputStream_skel,
					    virtual public StdSynthModule
{
TQ_OBJECT
public:
	TDEIOInputStream_impl();
	~TDEIOInputStream_impl();
	
	void streamStart();	
	void streamEnd();

	bool eof();
	bool seekOk();
	long size();
	long seek(long);

	bool openURL(const std::string& url);

	void processQueue();
	void request_outdata(DataPacket<mcopbyte> *packet);

	long bufferPackets() { return m_packetBuffer; }
	void bufferPackets(long i) { m_packetBuffer = i; }

	long packetSize() { return m_packetSize; }

signals:
	void mimeTypeFound(const TQString & mimetype);
	
private slots:
	void slotData(TDEIO::Job *, const TQByteArray &);
	void slotResult(TDEIO::Job *);
	void slotScanMimeType(TDEIO::Job *, const TQString &mimetype);
	void slotTotalSize(TDEIO::Job *, TDEIO::filesize_t size);

private:
	KURL m_url;
	TDEIO::TransferJob *m_job;
	TQByteArray m_data;
	bool m_finished;
	bool m_firstBuffer;
	bool m_streamStarted;
	bool m_streamSuspended;
	bool m_streamPulled;

	unsigned int m_packetBuffer;
	const unsigned int m_packetSize;
	TDEIO::filesize_t m_size;
	
	static const unsigned int PACKET_COUNT;
};

}

#endif
