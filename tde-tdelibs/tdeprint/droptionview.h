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

#ifndef DROPTIONVIEW_H
#define DROPTIONVIEW_H

#include <tqwidget.h>
#include <tqgroupbox.h>
#include <tqstringlist.h>

class TQLineEdit;
class TQSlider;
class TQLabel;
class TDEListBox;
class TQListBoxItem;
class TQVButtonGroup;
class TQWidgetStack;
class TQListViewItem;
class DrBase;
class DriverItem;

class OptionBaseView : public TQWidget
{
	TQ_OBJECT
public:
	OptionBaseView(TQWidget *parent = 0, const char *name = 0);
	virtual void setOption(DrBase*);
	virtual void setValue(const TQString&);

signals:
	void valueChanged(const TQString&);

protected:
	bool	blockSS;
};

class OptionNumericView : public OptionBaseView
{
	TQ_OBJECT
public:
	OptionNumericView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

protected slots:
	void slotSliderChanged(int);
	void slotEditChanged(const TQString&);

private:
	TQLineEdit	*m_edit;
	TQSlider	*m_slider;
	TQLabel		*m_minval, *m_maxval;
	bool		m_integer;
};

class OptionStringView : public OptionBaseView
{
public:
	OptionStringView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

private:
	TQLineEdit	*m_edit;
};

class OptionListView : public OptionBaseView
{
	TQ_OBJECT
public:
	OptionListView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

protected slots:
	void slotSelectionChanged();

private:
	TDEListBox	*m_list;
	TQStringList	m_choices;
};

class OptionBooleanView : public OptionBaseView
{
	TQ_OBJECT
public:
	OptionBooleanView(TQWidget *parent = 0, const char *name = 0);
	void setOption(DrBase *opt);
	void setValue(const TQString& val);

protected slots:
	void slotSelected(int);

private:
	TQVButtonGroup	*m_group;
	TQStringList	m_choices;
};

class DrOptionView : public TQGroupBox
{
	TQ_OBJECT
public:
	DrOptionView(TQWidget *parent = 0, const char *name = 0);
	void setAllowFixed(bool on) 	{ m_allowfixed = on; }

signals:
	void changed();

public slots:
	void slotValueChanged(const TQString&);
	void slotItemSelected(TQListViewItem*);

private:
	TQWidgetStack	*m_stack;
	DriverItem	*m_item;
	bool		m_block;
	bool		m_allowfixed;
};

#endif
