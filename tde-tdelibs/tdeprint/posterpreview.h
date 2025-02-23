/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <tdeprint@swing.be>
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

#ifndef POSTERPREVIEW_H
#define POSTERPREVIEW_H

#include <tqframe.h>
#include <tqvaluelist.h>

class TDEProcess;

class PosterPreview : public TQFrame
{
	TQ_OBJECT
public:
	PosterPreview( TQWidget *parent = 0, const char *name = 0 );
	PosterPreview( const TQString& postersize, const TQString& mediasize, TQWidget *parent = 0, const char *name = 0 );
	~PosterPreview();

public slots:
	void setPosterSize( int );
	void setPosterSize( const TQString& );
	void setMediaSize( int );
	void setMediaSize( const TQString& );
	void setCutMargin( int );
	void updatePoster();
	void setSelectedPages( const TQString& );

signals:
	void selectionChanged( const TQString& );

protected:
	void parseBuffer();
	void drawContents( TQPainter* );
	void init();
	void setDirty();
	void mouseMoveEvent( TQMouseEvent* );
	void mousePressEvent( TQMouseEvent* );
	void emitSelectedPages();

protected slots:
	void slotProcessStderr( TDEProcess*, char*, int );
	void slotProcessExited( TDEProcess* );

private:
	int m_rows, m_cols;
	int m_pw, m_ph; // page size
	int m_mw, m_mh; // cur margins
	TQRect m_posterbb; // poster bounding box (without any margin)
	TDEProcess *m_process;
	TQString m_buffer;
	TQString m_postersize, m_mediasize;
	int m_cutmargin;
	bool m_dirty;
	TQRect m_boundingrect;
	TQValueList<int> m_selectedpages;
};

#endif /* POSTERPREVIEW_H */
