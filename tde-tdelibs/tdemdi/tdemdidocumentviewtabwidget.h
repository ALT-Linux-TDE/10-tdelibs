//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#ifndef _TDEMDI_DOCUMENT_VIEW_TAB_WIDGET_H_
#define _TDEMDI_DOCUMENT_VIEW_TAB_WIDGET_H_

#include <ktabwidget.h>
#include <tdemdidefines.h>

class TDEPopupMenu;

//KDE4: Add a d pointer
/**
 * A reimplementation of KTabWidget for KMDI
 */
class KMDI_EXPORT KMdiDocumentViewTabWidget:
			public KTabWidget
{
	TQ_OBJECT
public:
	KMdiDocumentViewTabWidget( TQWidget* parent, const char* name = 0 );
	~KMdiDocumentViewTabWidget();

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( TQWidget * child, const TQString & label );

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label );

	/**
	 * Add a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void addTab ( TQWidget * child, TQTab * tab );

	/**
	 * Insert a tab into the tabwidget with a label
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( TQWidget * child, const TQString & label, int index = -1 );

	/**
	 * Inserts a tab into the tabwidget with an icon and label
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label, int index = -1 );

	/**
	 * Inserts a tab into the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void insertTab ( TQWidget * child, TQTab * tab, int index = -1 );

	/**
	 * Removes the tab from the tabwidget
	 * \sa QTabWidget
	 * \sa KTabWidget
	 */
	virtual void removePage ( TQWidget * w );

	/**
	 * Set the tab widget's visibility and then make the change
	 * to match the new setting
	 */
	KMdi::TabWidgetVisibility tabWidgetVisibility();

	/** Get the tab widget's visibility */
	void setTabWidgetVisibility( KMdi::TabWidgetVisibility );

private slots:

	/** Close the tab specified by w */
	void closeTab( TQWidget* w );
	
	/** Moves a tab. Reimplemented for internal reasons. */
	void moveTab( int from, int to );

public slots:

	/** Change the icon for the tab */
	void updateIconInView( TQWidget*, TQPixmap );

	/** Change the caption for the tab */
	void updateCaptionInView( TQWidget*, const TQString& );

private slots:

	/**
	 * Determine whether or not we should show the tab bar
	 * The tab bar is hidden if it's determined that it should be hidden
	 * and shown if it's determined that it should be shown
	 */
	void maybeShow();

private:

	KMdi::TabWidgetVisibility m_visibility;
	
signals:
	void initiateTabMove(int, int);	
};



#endif 
