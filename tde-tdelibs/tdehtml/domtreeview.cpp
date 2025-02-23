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

#include "tdehtml_part.h"
#include "domtreeview.moc"
#include "xml/dom_nodeimpl.h"

DOMTreeView::DOMTreeView(TQWidget *parent, TDEHTMLPart *currentpart, const char * name) : TDEListView(parent, name)
{
    setCaption(name);
    setRootIsDecorated(true);
    addColumn("Name");
    addColumn("Value");
    addColumn("Renderer");
    setSorting(-1);
    part = currentpart;
    connect(part, TQ_SIGNAL(nodeActivated(const DOM::Node &)), this, TQ_SLOT(showTree(const DOM::Node &)));
    connect(this, TQ_SIGNAL(clicked(TQListViewItem *)), this, TQ_SLOT(slotItemClicked(TQListViewItem *)));
    m_nodedict.setAutoDelete(true);
}

DOMTreeView::~DOMTreeView()
{
    disconnect(part);
}

void DOMTreeView::showTree(const DOM::Node &pNode)
{
    if(pNode.isNull() || document != pNode.ownerDocument())
    {
	clear();
	m_itemdict.clear();
	m_nodedict.clear();
	if(pNode.isNull())
	    return;
	else if(pNode.ownerDocument().isNull())
	    recursive(0, pNode);
	else
	    recursive(0, pNode.ownerDocument());
    }
    setCurrentItem(m_itemdict[pNode.handle()]);
    ensureItemVisible(m_itemdict[pNode.handle()]);
}

void DOMTreeView::recursive(const DOM::Node &pNode, const DOM::Node &node)
{
    TQListViewItem *cur_item;
    if(pNode.ownerDocument() != document)
    {
	TQString val = node.nodeValue().string();
	if ( val.length() > 20 )
	    val.truncate( 20 );
	cur_item = new TQListViewItem(static_cast<TQListView *>(this), node.nodeName().string(), val );
	document = pNode.ownerDocument();
    }
    else {
	TQString val = node.nodeValue().string();
	if ( val.length() > 20 )
	    val.truncate( 20 );
	cur_item = new TQListViewItem(m_itemdict[pNode.handle()], node.nodeName().string(), val);
    }

    if(node.handle())
    {
	m_itemdict.insert(node.handle(), cur_item);
	m_nodedict.insert(cur_item, new DOM::Node(node));
    }

    DOM::Node cur_child = node.lastChild();
    while(!cur_child.isNull())
    {
	recursive(node, cur_child);
	cur_child = cur_child.previousSibling();
    }
}

void DOMTreeView::slotItemClicked(TQListViewItem *cur_item)
{
    DOM::Node *handle = m_nodedict[cur_item];
    if(handle) {
	emit part->setActiveNode(*handle);
    }
}
