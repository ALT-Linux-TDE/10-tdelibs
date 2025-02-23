/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
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

#ifndef DRIVER_H
#define DRIVER_H

#if !defined( _TDEPRINT_COMPILE ) && defined( __GNUC__ )
#warning internal header, do not use except if you are a TDEPrint developer
#endif

#include <tqstring.h>
#include <tqptrlist.h>
#include <tqdict.h>
#include <tqmap.h>
#include <tqrect.h>
#include <tqsize.h>

#include <tdelibs_export.h>

class DriverItem;
class TQListView;

/***********************
 * Forward definitions *
 ***********************/

class DrBase;
class DrMain;
class DrGroup;
class DrConstraint;
class DrPageSize;

/*************************************
 * Base class for all driver objects *
 *************************************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrBase
{
public:
	enum Type { Base = 0, Main, ChoiceGroup, Group, String, Integer, Float, List, Boolean };

	DrBase();
	virtual ~DrBase();

	Type type() const 					{ return m_type; }
	bool isOption() const 					{ return (m_type >= DrBase::String); }

	const TQString& get(const TQString& key) const 		{ return m_map[key]; }
	void set(const TQString& key, const TQString& val)	{ m_map[key] = val; }
	bool has(const TQString& key) const 			{ return m_map.contains(key); }
	const TQString& name() const				{ return m_name; }
	void setName(const TQString& s)				{ m_name = s; }
	bool conflict() const 					{ return m_conflict; }
	void setConflict(bool on)				{ m_conflict = on; }

	virtual TQString valueText();
	virtual TQString prettyText();
	virtual void setValueText(const TQString&);
	virtual DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
	virtual void setOptions(const TQMap<TQString,TQString>& opts);
	virtual void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
	virtual DrBase* clone();

protected:
	TQMap<TQString,TQString>	m_map;
	TQString			m_name;		// used as a search key, better to have defined directly
	Type			m_type;
	bool			m_conflict;
};

/**********************
 * Option group class *
 **********************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrGroup : public DrBase
{
public:
	DrGroup();
	~DrGroup();

	void addOption(DrBase *opt);
	void addGroup(DrGroup *grp);
	void addObject(DrBase *optgrp);
	void clearConflict();
	void removeOption(const TQString& name);
	void removeGroup(DrGroup *grp);
	bool isEmpty();

	virtual DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
	DrBase* findOption(const TQString& name, DrGroup **parentGroup = 0);
	DrGroup* findGroup(DrGroup *grp, DrGroup **parentGroup = 0);
	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
	DrBase* clone();

	const TQPtrList<DrGroup>& groups()	{ return m_subgroups; }
	const TQPtrList<DrBase>& options()	{ return m_listoptions; }

	static TQString groupForOption( const TQString& optname );

protected:
	void createTree(DriverItem *parent);
	void flattenGroup(TQMap<TQString, DrBase*>&, int&);

protected:
	TQPtrList<DrGroup>	m_subgroups;
	TQDict<DrBase>	m_options;
	TQPtrList<DrBase>	m_listoptions;	// keep track of order of appearance
};

/*********************
 * Main driver class *
 *********************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrMain : public DrGroup
{
public:
	DrMain();
	~DrMain();

	DriverItem* createTreeView(TQListView *parent);
	void addConstraint(DrConstraint *c)		{ m_constraints.append(c); }
	int checkConstraints();
	DrPageSize* findPageSize(const TQString& name)	{ return m_pagesizes.find(name); }
	void addPageSize(DrPageSize *sz);
	void removeOptionGlobally(const TQString& name);
	void removeGroupGlobally(DrGroup *grp);
	TQMap<TQString, DrBase*> flatten();
	DrMain* cloneDriver();

protected:
	TQPtrList<DrConstraint>	m_constraints;
	TQDict<DrPageSize>	m_pagesizes;
};

/**********************************************************
 * Choice group class: a choice that involve a sub-option *
 **********************************************************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class DrChoiceGroup : public DrGroup
{
public:
	DrChoiceGroup();
	~DrChoiceGroup();

	DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
};

/***********************
 * String option class *
 ***********************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrStringOption : public DrBase
{
public:
	DrStringOption();
	~DrStringOption();

	virtual TQString valueText();
	virtual void setValueText(const TQString& s);

protected:
	TQString	m_value;
};

/**********************************
 * Integer numerical option class *
 **********************************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrIntegerOption : public DrBase
{
public:
	DrIntegerOption();
	~DrIntegerOption();

	virtual TQString valueText();
	virtual void setValueText(const TQString& s);
	TQString fixedVal();

protected:
	int	m_value;
};

/********************************
 * Float numerical option class *
 ********************************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrFloatOption : public DrBase
{
public:
	DrFloatOption();
	~DrFloatOption();

	virtual TQString valueText();
	virtual void setValueText(const TQString& s);
	TQString fixedVal();

protected:
	float	m_value;
};

/***********************
 * Single choice class *
 ***********************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrListOption : public DrBase
{
public:
	DrListOption();
	~DrListOption();

	void addChoice(DrBase *ch)	{ m_choices.append(ch); }
	TQPtrList<DrBase>* choices()	{ return &m_choices; }
	DrBase* currentChoice() const 	{ return m_current; }
	DrBase* findChoice(const TQString& txt);
	void setChoice(int choicenum);

	virtual TQString valueText();
	virtual TQString prettyText();
	virtual void setValueText(const TQString& s);
	void setOptions(const TQMap<TQString,TQString>& opts);
	void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
	DriverItem* createItem(DriverItem *parent, DriverItem *after = 0);
	DrBase* clone();

protected:
	TQPtrList<DrBase>	m_choices;
	DrBase		*m_current;
};

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class TDEPRINT_EXPORT DrBooleanOption : public DrListOption
{
	/* just an overloaded class, with different type */
public:
	DrBooleanOption() : DrListOption() { m_type = DrBase::Boolean; }
	~DrBooleanOption() {}
};

/********************
 * Constraint class *
 ********************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class DrConstraint
{
public:
	DrConstraint(const TQString& o1, const TQString& o2, const TQString& c1 = TQString::null, const TQString& c2 = TQString::null);
	DrConstraint(const DrConstraint&);

	bool check(DrMain*);

protected:
	TQString		m_opt1, m_opt2;
	TQString		m_choice1, m_choice2;
	DrListOption	*m_option1, *m_option2;
};

/*******************
 * Page Size class *
 *******************/

/**
 * @internal
 * This class is internal to TDEPrint and is not intended to be
 * used outside it. Please do not make use of this header, except
 * if you're a TDEPrint developer. The API might change in the
 * future and binary compatibility might be broken.
 */
class DrPageSize
{
public:
	DrPageSize(const TQString& s, float width, float height, float left, float bottom, float right, float top);
	DrPageSize(const DrPageSize&);

	/**
	 * All dimensions are int dot: 1/72th of an inch ( PostScript ).
	 * When rounded, the rounding is made safely: upward for a margin,
	 * downward for a page size.
	 */
	float pageWidth() const    { return m_width; }
	float pageHeight() const   { return m_height; }
	float leftMargin() const   { return m_left; }
	float rightMargin() const  { return m_right; }
	float topMargin() const    { return m_top; }
	float bottomMargin() const { return m_bottom; }
	TQString pageName() const   { return m_name; }

	TQSize pageSize() const;
	TQRect pageRect() const;
	TQSize margins() const;

protected:
	TQString	m_name;
	float m_width, m_height, m_left, m_bottom, m_right, m_top;
};

#endif
