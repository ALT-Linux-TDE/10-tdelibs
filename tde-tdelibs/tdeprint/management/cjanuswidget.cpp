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

#include "cjanuswidget.h"

#include <tqwidgetstack.h>
#include <tqlabel.h>
#include <tqpainter.h>
#include <tdelistbox.h>
#include <tqlayout.h>
#include <kseparator.h>

class CJanusWidget::CPage
{
public:
	TQWidget	*m_widget;
	TQString	m_text;
	TQString	m_header;
	TQPixmap	m_pixmap;
	CListBoxItem	*m_item;
};

//***********************************************************************************

class CJanusWidget::CListBoxItem : public TQListBoxItem
{
public:
	CListBoxItem(TQListBox *lb, TQListBoxItem *after, const TQPixmap& pix, const TQString& text);
	int height(const TQListBox*) const;
	int width(const TQListBox*) const;

protected:
	void paint(TQPainter*);

private:
	TQPixmap	m_pix;
};

CJanusWidget::CListBoxItem::CListBoxItem(TQListBox *lb, TQListBoxItem *after, const TQPixmap& pix, const TQString& text)
: TQListBoxItem(lb, after), m_pix(pix)
{
	setText(text);
}

int CJanusWidget::CListBoxItem::height(const TQListBox *lb) const
{
	return (m_pix.height() + lb->fontMetrics().lineSpacing() + 12);
}

int CJanusWidget::CListBoxItem::width(const TQListBox *lb) const
{
	int	w = TQMAX(lb->fontMetrics().width(text()),m_pix.width());
	return (w + 10);
}

void CJanusWidget::CListBoxItem::paint(TQPainter *p)
{
	int	w1 = (listBox()->contentsWidth()-m_pix.width())/2;

	p->drawPixmap(w1,5,m_pix);
	p->drawText(0,7+m_pix.height(),listBox()->contentsWidth(),p->fontMetrics().lineSpacing(),TQt::AlignHCenter,text());
}

//***********************************************************************************

class CJanusWidget::CListBox : public TDEListBox
{
public:
	CListBox(TQWidget *parent = 0, const char *name = 0);
	~CListBox();

	void computeWidth();

protected:
	virtual bool eventFilter(TQObject*, TQEvent*);
};

CJanusWidget::CListBox::CListBox(TQWidget *parent, const char *name)
: TDEListBox(parent,name)
{
	verticalScrollBar()->installEventFilter(this);
}

CJanusWidget::CListBox::~CListBox()
{
}

bool CJanusWidget::CListBox::eventFilter(TQObject *o, TQEvent *e)
{
	if (e->type() == TQEvent::Show || e->type() == TQEvent::Hide)
		computeWidth();
	return TDEListBox::eventFilter(o,e);
}

void CJanusWidget::CListBox::computeWidth()
{
	TQListBoxItem	*item = firstItem();
	int	w(40);
	while (item)
	{
		w = TQMAX(w,item->width(this));
		item = item->next();
	}
	if (verticalScrollBar()->isVisible())
		w += verticalScrollBar()->sizeHint().width();
	w += (frameWidth()*2);
	setFixedWidth(w);
}

//***********************************************************************************

CJanusWidget::CJanusWidget(TQWidget *parent, const char *name)
: TQWidget(parent,name)
{
	m_pages.setAutoDelete(true);

	m_stack = new TQWidgetStack(this);
	m_header = new TQLabel(this);
	TQFont	f(m_header->font());
	f.setBold(true);
	m_header->setFont(f);

	KSeparator* sep = new KSeparator( KSeparator::HLine, this);
	sep->setFixedHeight(5);

	m_iconlist = new CListBox(this);
	f = m_iconlist->font();
	f.setBold(true);
	m_iconlist->setFont(f);
	connect(m_iconlist,TQ_SIGNAL(selectionChanged(TQListBoxItem*)),TQ_SLOT(slotSelected(TQListBoxItem*)));

	m_empty = new TQWidget(this, "Empty");
	m_stack->addWidget(m_empty,0);

	TQHBoxLayout	*main_ = new TQHBoxLayout(this, 0, 10);
	TQVBoxLayout	*sub_ = new TQVBoxLayout(0, 0, 5);
	main_->addWidget(m_iconlist,0);
	main_->addLayout(sub_,1);
	sub_->addWidget(m_header,0);
	sub_->addWidget(sep,0);
	sub_->addWidget(m_stack,1);
}

CJanusWidget::~CJanusWidget()
{
}

void CJanusWidget::addPage(TQWidget *w, const TQString& text, const TQString& header, const TQPixmap& pix)
{
	CPage	*page = new CPage();
	m_pages.append(page);
	page->m_widget = w;
	page->m_text = text;
	page->m_header = header;
	page->m_pixmap = pix;
	page->m_item = new CListBoxItem(m_iconlist,findPrevItem(page),pix,text);
	m_iconlist->computeWidth();
	m_stack->addWidget(w,m_pages.count());

	if (m_iconlist->count() == 1)
		m_iconlist->setSelected(page->m_item,true);
}

void CJanusWidget::enablePage(TQWidget *w)
{
	CPage	*page = findPage(w);
	if (page && !page->m_item)
	{
		page->m_item = new CListBoxItem(m_iconlist,findPrevItem(page),page->m_pixmap,page->m_text);
		m_iconlist->computeWidth();
		if (m_iconlist->count() == 1)
			m_iconlist->setSelected(page->m_item,true);
	}
}

void CJanusWidget::disablePage(TQWidget *w)
{
	CPage	*page = findPage(w);
	if (page && page->m_item)
	{
		bool	needReselect(m_iconlist->isSelected(page->m_item));
		delete page->m_item;
		page->m_item = 0;
		m_iconlist->computeWidth();
		if (needReselect)
			if (m_iconlist->count() > 0)
				m_iconlist->setSelected(m_iconlist->firstItem(),true);
			else
				slotSelected(0);
	}
}

void CJanusWidget::slotSelected(TQListBoxItem *item)
{
	CPage	*page = findPage(item);
	if (page)
	{
		m_stack->raiseWidget(page->m_widget);
		m_header->setText(page->m_header);
	}
	else
	{
		m_header->setText("");
		m_stack->raiseWidget(m_empty);
	}
}

CJanusWidget::CPage* CJanusWidget::findPage(TQWidget *w)
{
	TQPtrListIterator<CPage>	it(m_pages);
	for (;it.current();++it)
		if (it.current()->m_widget == w)
			return it.current();
	return 0;
}

CJanusWidget::CPage* CJanusWidget::findPage(TQListBoxItem *i)
{
	TQPtrListIterator<CPage>	it(m_pages);
	for (;it.current();++it)
		if (it.current()->m_item == i)
			return it.current();
	return 0;
}

TQListBoxItem* CJanusWidget::findPrevItem(CPage *p)
{
	if (m_pages.findRef(p) == -1)
		m_pages.last();
	else
		m_pages.prev();
	for (;m_pages.current();m_pages.prev())
		if (m_pages.current()->m_item)
			return m_pages.current()->m_item;
	return 0;
}

void CJanusWidget::clearPages()
{
	TQPtrListIterator<CPage>	it(m_pages);
	for (;it.current(); ++it)
	{
		delete it.current()->m_widget;
		delete it.current()->m_item;
	}
	m_pages.clear();
}

#include "cjanuswidget.moc"
