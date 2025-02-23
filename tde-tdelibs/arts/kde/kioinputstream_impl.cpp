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

/*
 * How does it work?
 * -----------------
 *
 * First the buffer has to be filled. When it reaches a defined size the outdata
 * stream has to start pulling packets. If the buffer reaches a size of zero the
 * stream has to stop. If the buffer gets to big the job has to be suspended
 * until the buffer is small enough again.
 */

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdeio/job.h>
#include <tdeio/kmimetype.h>
#include <tdeio/jobclasses.h>
#include <tqtimer.h>
#include <tqdatastream.h>
#include "artsversion.h"
#include "kioinputstream_impl.moc"

using namespace Arts;

const unsigned int TDEIOInputStream_impl::PACKET_COUNT = 10;

TDEIOInputStream_impl::TDEIOInputStream_impl() : m_packetSize(2048)
{
	m_job = 0;
	m_finished = false;
	m_firstBuffer = false;
	m_packetBuffer = 16;
	m_streamStarted = false;
	m_streamSuspended = false;
	m_streamPulled = false;
	m_size = 0;
}

TDEIOInputStream_impl::~TDEIOInputStream_impl()
{
	if(m_job != 0)
	    m_job->kill();
}

void TDEIOInputStream_impl::streamStart()
{
	// prevent kill/reconnect
	if (m_streamStarted) {
		kdDebug( 400 ) << "not restarting stream!\n";
		if (m_job->isSuspended())
			m_job->resume();
		return;
	}

	kdDebug( 400 ) << "(re)starting stream\n";

	if(m_job != 0)
		m_job->kill();
	m_job = TDEIO::get(m_url, false, false);

	m_job->addMetaData("accept", "audio/x-mp3, video/mpeg, application/ogg");
	m_job->addMetaData("UserAgent", TQString::fromLatin1("aRts/") + TQString::fromLatin1(ARTS_VERSION));

	TQObject::connect(m_job, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)),
			 this, TQ_SLOT(slotData(TDEIO::Job *, const TQByteArray &)));		     
	TQObject::connect(m_job, TQ_SIGNAL(result(TDEIO::Job *)),
			 this, TQ_SLOT(slotResult(TDEIO::Job *)));		     
	TQObject::connect(m_job, TQ_SIGNAL(mimetype(TDEIO::Job *, const TQString &)),
			 this, TQ_SLOT(slotScanMimeType(TDEIO::Job *, const TQString &)));
	TQObject::connect(m_job, TQ_SIGNAL(totalSize( TDEIO::Job *, TDEIO::filesize_t)),
			 this, TQ_SLOT(slotTotalSize(TDEIO::Job *, TDEIO::filesize_t)));

	m_streamStarted = true;
}

void TDEIOInputStream_impl::streamEnd()
{
	kdDebug( 400 ) << "streamEnd()\n";

	if(m_job != 0)
	{
		TQObject::disconnect(m_job, TQ_SIGNAL(data(TDEIO::Job *, const TQByteArray &)),
	    				this, TQ_SLOT(slotData(TDEIO::Job *, const TQByteArray &)));
		TQObject::disconnect(m_job, TQ_SIGNAL(result(TDEIO::Job *)),
						this, TQ_SLOT(slotResult(TDEIO::Job *)));		     
		TQObject::disconnect(m_job, TQ_SIGNAL(mimetype(TDEIO::Job *, const TQString &)),
				 this, TQ_SLOT(slotScanMimeType(TDEIO::Job *, const TQString &)));
		TQObject::disconnect(m_job, TQ_SIGNAL(totalSize( TDEIO::Job *, TDEIO::filesize_t)),
				 this, TQ_SLOT(slotTotalSize(TDEIO::Job *, TDEIO::filesize_t)));

		if ( m_streamPulled )
			outdata.endPull();

		m_job->kill();
		m_job = 0;
	}	

	m_streamStarted = false;
}

bool TDEIOInputStream_impl::openURL(const std::string& url)
{
	m_url = KURL(url.c_str());
	m_size = 0;
	return true;
}

void TDEIOInputStream_impl::slotData(TDEIO::Job *, const TQByteArray &data)
{
	if(m_finished)
	    m_finished = false;

	TQDataStream dataStream(m_data, IO_WriteOnly | IO_Append);
	dataStream.writeRawBytes(data.data(), data.size());
	//kdDebug( 400 ) << "STREAMING: buffersize = " << m_data.size() << " bytes" << endl;
	
	processQueue();
}

void TDEIOInputStream_impl::slotResult(TDEIO::Job *job)
{
	// jobs delete themselves after emitting their result
	m_finished = true;
	m_streamStarted = false;
	m_job = 0;

	if(job->error()) {
		// break out of the event loop in case of
		// connection error
	    	emit mimeTypeFound("application/x-zerosize");
		job->showErrorDialog();
	}
}

void TDEIOInputStream_impl::slotScanMimeType(TDEIO::Job *, const TQString &mimetype)
{
	kdDebug( 400 ) << "got mimetype: " << mimetype << endl;
	emit mimeTypeFound(mimetype);
}

void TDEIOInputStream_impl::slotTotalSize(TDEIO::Job *, TDEIO::filesize_t size)
{
	m_size = size;
}

bool TDEIOInputStream_impl::eof()
{
	return (m_finished && m_data.size() == 0);
}

bool TDEIOInputStream_impl::seekOk()
{
	return false;
}

long TDEIOInputStream_impl::size()
{
	return m_size ? m_size : m_data.size();
}

long TDEIOInputStream_impl::seek(long)
{
	return -1;
}

void TDEIOInputStream_impl::processQueue()
{
	if(m_job != 0)
	{
		if(m_data.size() > (m_packetBuffer * m_packetSize * 2) && !m_job->isSuspended())
		{
			kdDebug( 400 ) << "STREAMING: suspend job" << endl;
			m_job->suspend();
		}
		else if(m_data.size() < (m_packetBuffer * m_packetSize) && m_job->isSuspended())
		{
			kdDebug( 400 ) << "STREAMING: resume job" << endl;
			m_job->resume();
		}
	}

	if (!m_firstBuffer) {
		if(m_data.size() < (m_packetBuffer * m_packetSize * 2) ) {
			kdDebug( 400 ) << "STREAMING: Buffering in progress... (Needed bytes before it starts to play: " << ((m_packetBuffer * m_packetSize * 2) - m_data.size()) << ")" << endl;
			return;
		} else {
			m_firstBuffer = true;
			m_streamPulled = true;
			outdata.setPull(PACKET_COUNT, m_packetSize);
		} 
	}
}

void TDEIOInputStream_impl::request_outdata(DataPacket<mcopbyte> *packet)
{
	processQueue();
	packet->size = std::min(m_packetSize, (unsigned int)m_data.size());
	kdDebug( 400 ) << "STREAMING: Filling one DataPacket with " << packet->size << " bytes of the stream!" << endl;

	if (!m_finished) {
		if( (unsigned)packet->size < m_packetSize || ! m_firstBuffer) {
			m_firstBuffer = false;
			packet->size = 0;
			outdata.endPull();
		}
	}
	
	if (packet->size > 0)
	{
		memcpy(packet->contents, m_data.data(), packet->size);
		memmove(m_data.data(), m_data.data() + packet->size, m_data.size() - packet->size);
		m_data.resize(m_data.size() - packet->size);
	}
	packet->send();
}

REGISTER_IMPLEMENTATION(TDEIOInputStream_impl);
