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

#include "droptionview.h"
#include "driver.h"
#include "driveritem.h"

#include <math.h>
#include <tqlineedit.h>
#include <tqslider.h>
#include <tqlabel.h>
#include <tdelistbox.h>
#include <tqvbuttongroup.h>
#include <tqradiobutton.h>
#include <tqwidgetstack.h>
#include <tqlayout.h>
#include <tqapplication.h>

#include <kcursor.h>
#include <kdialog.h>
#include <tdelocale.h>

OptionBaseView::OptionBaseView(TQWidget *parent, const char *name)
: TQWidget(parent,name)
{
	blockSS = false;
}

void OptionBaseView::setOption(DrBase*)
{
}

void OptionBaseView::setValue(const TQString&)
{
}

//******************************************************************************************************

OptionNumericView::OptionNumericView(TQWidget *parent, const char *name)
: OptionBaseView(parent,name)
{
	m_edit = new TQLineEdit(this);
	m_slider = new TQSlider(TQt::Horizontal,this);
	m_slider->setTickmarks(TQSlider::Below);
	TQLabel	*lab = new TQLabel(i18n("Value:"),this);
	m_minval = new TQLabel(this);
	m_maxval = new TQLabel(this);

	m_integer = true;

	TQVBoxLayout	*main_ = new TQVBoxLayout(this, 0, 10);
	TQHBoxLayout	*sub_ = new TQHBoxLayout(0, 0, 10);
	TQHBoxLayout	*sub2_ = new TQHBoxLayout(0, 0, 5);
	main_->addStretch(1);
	main_->addLayout(sub_,0);
	main_->addLayout(sub2_,0);
	main_->addStretch(1);
	sub_->addWidget(lab,0);
	sub_->addWidget(m_edit,0);
	sub_->addStretch(1);
	sub2_->addWidget(m_minval,0);
	sub2_->addWidget(m_slider,1);
	sub2_->addWidget(m_maxval,0);

	connect(m_slider,TQ_SIGNAL(valueChanged(int)),TQ_SLOT(slotSliderChanged(int)));
	connect(m_edit,TQ_SIGNAL(textChanged(const TQString&)),TQ_SLOT(slotEditChanged(const TQString&)));
}

void OptionNumericView::setOption(DrBase *opt)
{
	if (opt->type() != DrBase::Integer && opt->type() != DrBase::Float)
		return;

	blockSS = true;
	if (opt->type() == DrBase::Integer)
	{
		m_integer = true;
		int	min_ = opt->get("minval").toInt();
		int	max_ = opt->get("maxval").toInt();
		m_slider->setRange(min_,max_);
		m_slider->setSteps(1,TQMAX((max_-min_)/20,1));
		m_minval->setText(TQString::number(min_));
		m_maxval->setText(TQString::number(max_));
	}
	else
	{
		m_integer = false;
		int	min_ = (int)rint(opt->get("minval").toFloat()*1000);
		int	max_ = (int)rint(opt->get("maxval").toFloat()*1000);
		m_slider->setRange(min_,max_);
		m_slider->setSteps(1,TQMAX((max_-min_)/20,1));
		m_minval->setText(opt->get("minval"));
		m_maxval->setText(opt->get("maxval"));
	}
	m_slider->update();
	blockSS = false;

	setValue(opt->valueText());
}

void OptionNumericView::setValue(const TQString& val)
{
	m_edit->setText(val);
}

void OptionNumericView::slotSliderChanged(int value)
{
	if (blockSS) return;

	TQString	txt;
	if (m_integer)
		txt = TQString::number(value);
	else
		txt = TQString::number(float(value)/1000.0,'f',3);
	blockSS = true;
	m_edit->setText(txt);
	blockSS = false;
	emit valueChanged(txt);
}

void OptionNumericView::slotEditChanged(const TQString& txt)
{
	if (blockSS) return;

	bool	ok(false);
	int	val(0);
	if (m_integer)
		val = txt.toInt(&ok);
	else
		val = (int)rint(txt.toFloat(&ok)*1000);
	if (ok)
	{
		blockSS = true;
		m_slider->setValue(val);
		blockSS = false;
		emit valueChanged(txt);
	}
	else
	{
		m_edit->selectAll();
		TQApplication::beep();
	}
}

//******************************************************************************************************

OptionStringView::OptionStringView(TQWidget *parent, const char *name)
: OptionBaseView(parent,name)
{
	m_edit = new TQLineEdit(this);
	TQLabel	*lab = new TQLabel(i18n("String value:"),this);

	TQVBoxLayout	*main_ = new TQVBoxLayout(this, 0, 5);
	main_->addStretch(1);
	main_->addWidget(lab,0);
	main_->addWidget(m_edit,0);
	main_->addStretch(1);

	connect(m_edit,TQ_SIGNAL(textChanged(const TQString&)),TQ_SIGNAL(valueChanged(const TQString&)));
}

void OptionStringView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::String)
		m_edit->setText(opt->valueText());
}

void OptionStringView::setValue(const TQString& val)
{
	m_edit->setText(val);
}

//******************************************************************************************************

OptionListView::OptionListView(TQWidget *parent, const char *name)
: OptionBaseView(parent,name)
{
	m_list = new TDEListBox(this);

	TQVBoxLayout	*main_ = new TQVBoxLayout(this, 0, 10);
	main_->addWidget(m_list);

	connect(m_list,TQ_SIGNAL(selectionChanged()),TQ_SLOT(slotSelectionChanged()));
}

void OptionListView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::List)
	{
		blockSS = true;
		m_list->clear();
		m_choices.clear();
		TQPtrListIterator<DrBase>	it(*(((DrListOption*)opt)->choices()));
		for (;it.current();++it)
		{
			m_list->insertItem(it.current()->get("text"));
			m_choices.append(it.current()->name());
		}
		blockSS = false;
		setValue(opt->valueText());
	}
}

void OptionListView::setValue(const TQString& val)
{
	m_list->setCurrentItem(m_choices.findIndex(val));
}

void OptionListView::slotSelectionChanged()
{
	if (blockSS) return;

	TQString	s = m_choices[m_list->currentItem()];
	emit valueChanged(s);
}

//******************************************************************************************************

OptionBooleanView::OptionBooleanView(TQWidget *parent, const char *name)
: OptionBaseView(parent,name)
{
	m_group = new TQVButtonGroup(this);
	m_group->setFrameStyle(TQFrame::NoFrame);

	TQRadioButton	*btn = new TQRadioButton(m_group);
	btn->setCursor(KCursor::handCursor());
	btn = new TQRadioButton(m_group);
	btn->setCursor(KCursor::handCursor());

	TQVBoxLayout	*main_ = new TQVBoxLayout(this, 0, 10);
	main_->addWidget(m_group);

	connect(m_group,TQ_SIGNAL(clicked(int)),TQ_SLOT(slotSelected(int)));
}

void OptionBooleanView::setOption(DrBase *opt)
{
	if (opt->type() == DrBase::Boolean)
	{
		TQPtrListIterator<DrBase>	it(*(((DrBooleanOption*)opt)->choices()));
		m_choices.clear();
		static_cast<TQButton*>(m_group->find(0))->setText(it.toFirst()->get("text"));
		m_choices.append(it.toFirst()->name());
		static_cast<TQButton*>(m_group->find(1))->setText(it.toLast()->get("text"));
		m_choices.append(it.toLast()->name());
		setValue(opt->valueText());
	}
}

void OptionBooleanView::setValue(const TQString& val)
{
	int	ID = m_choices.findIndex(val);
	m_group->setButton(ID);
}

void OptionBooleanView::slotSelected(int ID)
{
	TQString	s = m_choices[ID];
	emit valueChanged(s);
}

//******************************************************************************************************

DrOptionView::DrOptionView(TQWidget *parent, const char *name)
: TQGroupBox(parent,name)
{
	m_stack = new TQWidgetStack(this);

	OptionBaseView	*w = new OptionListView(m_stack);
	connect(w,TQ_SIGNAL(valueChanged(const TQString&)),TQ_SLOT(slotValueChanged(const TQString&)));
	m_stack->addWidget(w,DrBase::List);

	w = new OptionStringView(m_stack);
	connect(w,TQ_SIGNAL(valueChanged(const TQString&)),TQ_SLOT(slotValueChanged(const TQString&)));
	m_stack->addWidget(w,DrBase::String);

	w = new OptionNumericView(m_stack);
	connect(w,TQ_SIGNAL(valueChanged(const TQString&)),TQ_SLOT(slotValueChanged(const TQString&)));
	m_stack->addWidget(w,DrBase::Integer);

	w = new OptionBooleanView(m_stack);
	connect(w,TQ_SIGNAL(valueChanged(const TQString&)),TQ_SLOT(slotValueChanged(const TQString&)));
	m_stack->addWidget(w,DrBase::Boolean);

	w = new OptionBaseView(m_stack);
	connect(w,TQ_SIGNAL(valueChanged(const TQString&)),TQ_SLOT(slotValueChanged(const TQString&)));
	m_stack->addWidget(w,0);	// empty widget

	m_stack->raiseWidget(w);
	setTitle(i18n("No Option Selected"));

	setColumnLayout(0, TQt::Vertical );
	layout()->setSpacing( KDialog::spacingHint() );
	layout()->setMargin( KDialog::marginHint() );
	TQVBoxLayout	*main_ = new TQVBoxLayout(layout(), KDialog::marginHint());
	main_->addWidget(m_stack);

	m_item = 0;
	m_block = false;
	m_allowfixed = true;
}

void DrOptionView::slotItemSelected(TQListViewItem *i)
{
	m_item = (DriverItem*)i;
	if (m_item && !m_item->drItem()->isOption())
		m_item = 0;
	int	ID(0);
	if (m_item)
		if (m_item->drItem()->type() == DrBase::Float) ID = DrBase::Integer;
		else ID = m_item->drItem()->type();

	OptionBaseView	*w = (OptionBaseView*)m_stack->widget(ID);
	if (w)
	{
		m_block = true;
		bool 	enabled(true);
		if (m_item)
		{
			w->setOption((m_item ? m_item->drItem() : 0));
			setTitle(m_item->drItem()->get("text"));
			enabled = ((m_item->drItem()->get("fixed") != "1") || m_allowfixed);
		}
		else
			setTitle(i18n("No Option Selected"));
		m_stack->raiseWidget(w);
		w->setEnabled(enabled);
		m_block = false;
	}
}

void DrOptionView::slotValueChanged(const TQString& val)
{
	if (m_item && m_item->drItem() && !m_block)
	{
		m_item->drItem()->setValueText(val);
		m_item->updateText();
		emit changed();
	}
}

#include "droptionview.moc"
