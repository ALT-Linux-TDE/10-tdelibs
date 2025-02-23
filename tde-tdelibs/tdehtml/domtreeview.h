/***************************************************************************
                               domtreeview.cpp
                             -------------------

    copyright            : (C) 2001 - The Kafka Team
    email                : kde-kafka@master.kde.org
 ***************************************************************************/

/***************************************************************************
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 ***************************************************************************/

#ifndef DOMTREEVIEW_H
#define DOMTREEVIEW_H

#include <tdelistview.h>
#include <kdebug.h>
#include <tqlistview.h>
#include <tqptrdict.h>
#include "dom/dom_core.h"

class DOMTreeView : public TDEListView
{
    TQ_OBJECT
    public: 
	DOMTreeView(TQWidget *parent, TDEHTMLPart *part, const char * name = 0);
	~DOMTreeView();

	void recursive(const DOM::Node &pNode, const DOM::Node &node);

    signals:
	void sigNodeClicked(const DOM::Node &);
	
    public slots:
	void showTree(const DOM::Node &pNode);

    protected slots:
	void slotItemClicked(TQListViewItem *);

    private:
	TQPtrDict<TQListViewItem> m_itemdict;
	TQPtrDict<DOM::Node> m_nodedict;
	DOM::Node document;

	TDEHTMLPart *part;

};

#endif
