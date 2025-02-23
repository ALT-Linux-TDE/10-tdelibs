// kimgio module for SGI images
//
// Copyright (C) 2004  Melchior FRANZ  <mfranz@kde.org>
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the Lesser GNU General Public License as
// published by the Free Software Foundation; either version 2 of the
// License, or (at your option) any later version.


#ifndef KIMG_RGB_H
#define KIMG_RGB_H

#include <tqmap.h>
#include <tqptrvector.h>


class TQImage;
class TQImageIO;

extern "C" {
void kimgio_rgb_read(TQImageIO *);
void kimgio_rgb_write(TQImageIO *);
}


class RLEData : public TQMemArray<uchar> {
public:
	RLEData() {}
	RLEData(const uchar *d, uint l, uint o) : m_offset(o) { duplicate(d, l); }
	bool operator<(const RLEData&) const;
	void write(TQDataStream& s);
	void print(TQString) const;				// TODO remove
	uint offset() { return m_offset; }
private:
	uint			m_offset;
};


class RLEMap : public TQMap<RLEData, uint> {
public:
	RLEMap() : m_counter(0), m_offset(0) {}
	uint insert(const uchar *d, uint l);
	TQPtrVector<RLEData> vector();
	void setBaseOffset(uint o) { m_offset = o; }
private:
	uint			m_counter;
	uint			m_offset;
};


class SGIImage {
public:
	SGIImage(TQImageIO *);
	~SGIImage();

	bool readImage(TQImage&);
	bool writeImage(TQImage&);

private:
	enum { NORMAL, DITHERED, SCREEN, COLORMAP };		// colormap
	TQImageIO		*m_io;
	TQIODevice		*m_dev;
	TQDataStream		m_stream;

	TQ_UINT8			m_rle;
	TQ_UINT8			m_bpc;
	TQ_UINT16		m_dim;
	TQ_UINT16		m_xsize;
	TQ_UINT16		m_ysize;
	TQ_UINT16		m_zsize;
	TQ_UINT32		m_pixmin;
	TQ_UINT32		m_pixmax;
	char			m_imagename[80];
	TQ_UINT32		m_colormap;

	TQ_UINT32		*m_starttab;
	TQ_UINT32		*m_lengthtab;
	TQByteArray		m_data;
	TQByteArray::Iterator	m_pos;
	RLEMap			m_rlemap;
	TQPtrVector<RLEData>	m_rlevector;
	uint			m_numrows;

	bool readData(TQImage&);
	bool getRow(uchar *dest);

	void writeHeader();
	void writeRle();
	void writeVerbatim(const TQImage&);
	bool scanData(const TQImage&);
	uint compact(uchar *, uchar *);
	uchar intensity(uchar);
};

#endif

