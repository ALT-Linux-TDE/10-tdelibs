/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
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

#include "kmlistview.h"
#include "kmprinter.h"
#include "kmobject.h"

#include <tqheader.h>
#include <tqpainter.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <kcursor.h>

class KMListViewItem : public TQListViewItem, public KMObject
{
public:
	KMListViewItem(TQListView *parent, const TQString& txt);
	KMListViewItem(TQListViewItem *parent, const TQString& txt);
	KMListViewItem(TQListViewItem *parent, KMPrinter *p);

	virtual void paintCell(TQPainter*, const TQColorGroup&, int, int, int);
	void updatePrinter(KMPrinter *p);
	bool isClass() const	{ return m_isclass; }

protected:
	void init(KMPrinter *p = 0);

private:
	int	m_state;
	bool	m_isclass;
};

KMListViewItem::KMListViewItem(TQListView *parent, const TQString& txt)
: TQListViewItem(parent,txt)
{
	init();
}

KMListViewItem::KMListViewItem(TQListViewItem *parent, const TQString& txt)
: TQListViewItem(parent,txt)
{
	init();
}

KMListViewItem::KMListViewItem(TQListViewItem *parent, KMPrinter *p)
: TQListViewItem(parent)
{
	init(p);
}

void KMListViewItem::init(KMPrinter *p)
{
	m_state = 0;
	if (p)
		updatePrinter(p);
	setSelectable(depth() == 2);
}

void KMListViewItem::updatePrinter(KMPrinter *p)
{
	bool	update(false);
	if (p)
	{
		int	oldstate = m_state;
		int	st(p->isValid() ? (int)TDEIcon::DefaultState : (int)TDEIcon::LockOverlay);
		m_state = ((p->isHardDefault() ? 0x1 : 0x0) | (p->ownSoftDefault() ? 0x2 : 0x0) | (p->isValid() ? 0x4 : 0x0));
		update = (oldstate != m_state);
		TQString	name = (p->isVirtual() ? p->instanceName() : p->name());
		if (name != text(0))
			setText(0, name);
		setPixmap(0, SmallIcon(p->pixmap(), 0, st));
		m_isclass = p->isClass();
	}
	setDiscarded(false);
	if (update)
		repaint();
}

void KMListViewItem::paintCell(TQPainter *p, const TQColorGroup& cg, int c, int w, int a)
{
	if (m_state != 0)
	{
		TQFont	f(p->font());
		if (m_state & 0x1) f.setBold(true);
		if (m_state & 0x2) f.setItalic(true);
		p->setFont(f);
	}
	TQListViewItem::paintCell(p,cg,c,w,a);
}

//************************************************************************************************

KMListView::KMListView(TQWidget *parent, const char *name)
: TQListView(parent,name)
{
	m_items.setAutoDelete(false);

	addColumn("");
	header()->hide();
	setFrameStyle(TQFrame::WinPanel|TQFrame::Sunken);
	setLineWidth(1);
	setSorting(0);

	connect(this,TQ_SIGNAL(contextMenuRequested(TQListViewItem*,const TQPoint&,int)),TQ_SLOT(slotRightButtonClicked(TQListViewItem*,const TQPoint&,int)));
	connect(this,TQ_SIGNAL(selectionChanged()),TQ_SLOT(slotSelectionChanged()));
	connect(this,TQ_SIGNAL(onItem(TQListViewItem*)),TQ_SLOT(slotOnItem(TQListViewItem*)));
	connect(this,TQ_SIGNAL(onViewport()),TQ_SLOT(slotOnViewport()));

	m_root = new KMListViewItem(this,i18n("Print System"));
	m_root->setPixmap(0,SmallIcon("tdeprint_printer"));
	m_root->setOpen(true);
	m_classes = new KMListViewItem(m_root,i18n("Classes"));
	m_classes->setPixmap(0,SmallIcon("package"));
	m_classes->setOpen(true);
	m_printers = new KMListViewItem(m_root,i18n("Printers"));
	m_printers->setPixmap(0,SmallIcon("package"));
	m_printers->setOpen(true);
	m_specials = new KMListViewItem(m_root,i18n("Specials"));
	m_specials->setPixmap(0,SmallIcon("package"));
	m_specials->setOpen(true);

	sort();
}

KMListView::~KMListView()
{
}

void KMListView::slotRightButtonClicked(TQListViewItem *item, const TQPoint& p, int)
{
	emit rightButtonClicked(item && item->depth() == 2 ? item->text(0) : TQString::null, p);
}

KMListViewItem* KMListView::findItem(KMPrinter *p)
{
	if (p)
	{
		TQPtrListIterator<KMListViewItem>	it(m_items);
		bool	isVirtual(p->isVirtual()), isClass(p->isClass());
		for (;it.current();++it)
			if (isVirtual)
			{
				if (it.current()->depth() == 3 && it.current()->text(0) == p->instanceName()
						&& it.current()->parent()->text(0) == p->printerName())
					return it.current();
			}
			else
			{
				if (it.current()->isClass() == isClass && it.current()->text(0) == p->name())
					return it.current();
			}
	}
	return 0;
}

KMListViewItem* KMListView::findItem(const TQString& prname)
{
	TQPtrListIterator<KMListViewItem>	it(m_items);
	for (; it.current(); ++it)
		if (it.current()->depth() == 2 && it.current()->text(0) == prname)
			return it.current();
	return 0;
}

void KMListView::setPrinterList(TQPtrList<KMPrinter> *list)
{
	bool 	changed(false);

	TQPtrListIterator<KMListViewItem>	it(m_items);
	for (;it.current();++it)
		it.current()->setDiscarded(true);

	if (list)
	{
		TQPtrListIterator<KMPrinter>	it(*list);
		KMListViewItem			*item (0);
		for (;it.current();++it)
		{
			item = findItem(it.current());
			if (!item)
			{
				if (it.current()->isVirtual())
				{
					KMListViewItem	*pItem = findItem(it.current()->printerName());
					if (!pItem)
						continue;
					item = new KMListViewItem(pItem, it.current());
					pItem->setOpen(true);
				}
				else
					item = new KMListViewItem((it.current()->isSpecial() ? m_specials : (it.current()->isClass(false) ? m_classes : m_printers)),it.current());
				m_items.append(item);
				changed = true;
			}
			else
				item->updatePrinter(it.current());
		}
	}

	TQPtrList<KMListViewItem>	deleteList;
	deleteList.setAutoDelete(true);
	for (uint i=0; i<m_items.count(); i++)
		if (m_items.at(i)->isDiscarded())
		{
			// instance items are put in front of the list
			// so that they are destroyed first
			KMListViewItem	*item = m_items.take(i);
			if (item->depth() == 2)
				deleteList.append(item);
			else
				deleteList.prepend(item);
			i--;
			changed = true;
		}
	deleteList.clear();

	if (changed) sort();
	emit selectionChanged();
}

void KMListView::slotSelectionChanged()
{
	KMListViewItem	*item = static_cast<KMListViewItem*>(currentItem());
	emit printerSelected((item && !item->isDiscarded() && item->depth() == 2 ? item->text(0) : TQString::null));
}

void KMListView::setPrinter(const TQString& prname)
{
	TQPtrListIterator<KMListViewItem>	it(m_items);
	for (;it.current();++it)
		if (it.current()->text(0) == prname)
		{
			setSelected(it.current(),true);
			break;
		}
}

void KMListView::setPrinter(KMPrinter *p)
{
	setPrinter(p ? p->name() : TQString::null);
}

void KMListView::slotOnItem(TQListViewItem *)
{
	setCursor(KCursor::handCursor());
}

void KMListView::slotOnViewport()
{
	setCursor(KCursor::arrowCursor());
}
#include "kmlistview.moc"
