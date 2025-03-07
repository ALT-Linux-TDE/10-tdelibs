/***************************************************************************
                          tdemultitabbar.h -  description
                             -------------------
    begin                :  2001
    copyright            : (C) 2001,2002,2003 by Joseph Wenninger <jowenn@kde.org>
 ***************************************************************************/

/***************************************************************************
    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
 ***************************************************************************/

#ifndef _TDEMultitabbar_h_
#define _TDEMultitabbar_h_

#include <tqscrollview.h>
#include <tqvbox.h>
#include <tqhbox.h>
#include <tqlayout.h>
#include <tqstring.h>
#include <tqptrlist.h>
#include <tqpushbutton.h>

#include <tdelibs_export.h>

class TQPixmap;
class TQPainter;
class TQFrame;

class KMultiTabBarPrivate;
class KMultiTabBarTabPrivate;
class KMultiTabBarButtonPrivate;
class KMultiTabBarInternal;

/**
 * @ingroup main
 * @ingroup multitabbar
 * A Widget for horizontal and vertical tabs.
 * It is possible to add normal buttons to the top/left
 * The handling if only one tab at a time or multiple tabs
 * should be raisable is left to the "user".
 *@author Joseph Wenninger
 */
class TDEUTILS_EXPORT KMultiTabBar: public TQWidget
{
	TQ_OBJECT
public:
	/**
	 * The tab bar's orientation. Also constraints the bar's position.
	 */
	enum KMultiTabBarMode {
		Horizontal,  ///< Horizontal orientation (i.e. on top or bottom)
		Vertical     ///< Vertical orientation (i.e. on the left or right hand side)
	};
	
	/**
	 * The tab bar's position
	 */
	enum KMultiTabBarPosition {
		Left,   ///< Left hand side
		Right,  ///< Right hand side
		Top,    ///< On top
		Bottom  ///< On bottom
	};

	/**
	 * The list of available styles for KMultiTabBar
	 */
	enum KMultiTabBarStyle {
		VSNET=0,          ///< Visual Studio .Net like (only show the text of active tabs)
		KDEV3=1,          ///< KDevelop 3 like (always show the text)
		KONQSBC=2,        ///< Konqueror's classic sidebar style (unthemed) (currently disabled)
		KDEV3ICON=3,      ///< KDevelop 3 like with icons
		STYLELAST=0xffff  ///< Last style
	};

	/**
	 * Constructor.
	 * @param bm The tab bar's orientation
	 * @param parent The parent widget
	 * @param name The widget's name
	 */
	KMultiTabBar(KMultiTabBarMode bm,TQWidget *parent=0,const char *name=0);
	
	/**
	 * Destructor.
	 */
	virtual ~KMultiTabBar();

	/**
 	 * append  a new button to the button area. The button can later on be accessed with button(ID)
	 * eg for connecting signals to it
	 * @param pic a pixmap for the button
 	 * @param id an arbitraty ID value. It will be emitted in the clicked signal for identifying the button
	 *	if more than one button is connected to a signals.
	 * @param popup A popup menu which should be displayed if the button is clicked
	 * @param not_used_yet will be used for a popup text in the future
	 */
 	int appendButton(const TQPixmap &pic,int id=-1,TQPopupMenu* popup=0,const TQString& not_used_yet=TQString::null);
	/** 
         * remove a button with the given ID
	 */
	void removeButton(int id);
	/**
	 * append a new tab to the tab area. It can be accessed lateron with tabb(id);
	 * @param pic a bitmap for the tab
	 * @param id an arbitrary ID which can be used later on to identify the tab
	 * @param text if a mode with text is used it will be the tab text, otherwise a mouse over hint
	 * @return Always zero. Can be safely ignored.
	 */
	int appendTab(const TQPixmap &pic,int id=-1,const TQString& text=TQString::null);
	/**
	 * remove a tab with a given ID
	 * @param id The ID of the tab to remove
	 */
	void removeTab(int id);
	/**
	 * set a tab to "raised"
	 * @param id The ID of the tab to manipulate
	 * @param state true == activated/raised, false == not active
	 */
	void setTab(int id ,bool state);
	/**
	 * return the state of a tab, identified by it's ID
	 * @param id The ID of the tab to raise
	 */
	bool isTabRaised(int id) const;
	/**
	 * get a pointer to a button within the button area identified by its ID
	 * @param id The id of the tab
	 */
	class KMultiTabBarButton *button(int id) const;

	/**
	 * get a pointer to a tab within the tab area, identified by its ID
	 */
	class KMultiTabBarTab *tab(int id) const;
	/**
	 * set the real position of the widget.
	 * @param pos if the mode is horizontal, only use top, bottom, if it is vertical use left or right
	 */
	void setPosition(KMultiTabBarPosition pos);
	/**
	 * get the tabbar position.
	 * @return The tab bar's position
	 */
	KMultiTabBarPosition position() const;
	/**
	 * set the display style of the tabs
	 * @param style The new display style
	 */
	void setStyle(KMultiTabBarStyle style);
	/**
	 * get the display style of the tabs
	 * @return display style
	 */
	KMultiTabBarStyle tabStyle() const;
	/**
	 * Returns the list of pointers to the tabs of type KMultiTabBarTab.
	 * @return The list of tabs.
	 * @warning be careful, don't delete tabs yourself and don't delete the list itself
	 */
        TQPtrList<KMultiTabBarTab>* tabs();
	/**
	 * Returns the list of pointers to the tab buttons of type KMultiTabBarButton.
	 * @return The list of tab buttons.
	 * @warning be careful, don't delete buttons yourself and don't delete the list itself
	 */
	TQPtrList<KMultiTabBarButton>* buttons();

	/**
	 * might vanish, not sure yet
	 */
	void showActiveTabTexts(bool show=true);
protected:
	friend class KMultiTabBarButton;
	virtual void fontChange( const TQFont& );
	void updateSeparator();
private:
	class KMultiTabBarInternal *m_internal;
	TQBoxLayout *m_l;
	TQFrame *m_btnTabSep;
	TQPtrList<KMultiTabBarButton> m_buttons;
	KMultiTabBarPosition m_position;
	KMultiTabBarPrivate *d;
};

/**
 * @ingroup multitabbar
 * This class represents a tab bar button in a KMultiTabBarWidget.
 * This class should never be created except with the appendButton call of KMultiTabBar
 */
class TDEUTILS_EXPORT KMultiTabBarButton: public TQPushButton
{
	TQ_OBJECT
public:
	/** @internal */
	KMultiTabBarButton(const TQPixmap& pic,const TQString&, TQPopupMenu *popup,
		int id,TQWidget *parent, KMultiTabBar::KMultiTabBarPosition pos, KMultiTabBar::KMultiTabBarStyle style);
	/** @internal */
	KMultiTabBarButton(const TQString&, TQPopupMenu *popup,
		int id,TQWidget *parent, KMultiTabBar::KMultiTabBarPosition pos, KMultiTabBar::KMultiTabBarStyle style);
	/**
	 * Destructor
	 */
	virtual  ~KMultiTabBarButton();
	/**
	 * Returns the tab's ID
	 * @return The tab's ID
	 */
	int id() const;

public slots:
	/**
	 * this is used internaly, but can be used by the user, if (s)he wants to
	 * It the according call of KMultiTabBar is invoked though this modifications will be overwritten
	 */
	void setPosition(KMultiTabBar::KMultiTabBarPosition);
        /**
         * this is used internaly, but can be used by the user, if (s)he wants to
         * It the according call of KMultiTabBar is invoked though this modifications will be overwritten
         */
	void setStyle(KMultiTabBar::KMultiTabBarStyle);

        /**
	 * modify the text of the button
         */
	void setText(const TQString &);

	TQSize sizeHint() const;

protected:
	KMultiTabBar::KMultiTabBarPosition m_position;
	KMultiTabBar::KMultiTabBarStyle m_style;
	TQString m_text;
	virtual void hideEvent( class TQHideEvent*);
	virtual void showEvent( class TQShowEvent*);
private:
	int m_id;
	KMultiTabBarButtonPrivate *d;
signals:
	/**
	 * this is emitted if  the button is clicked
	 * @param id	the ID identifying the button
	 */
	void clicked(int id);
protected slots:
	virtual void slotClicked();
};

/**
 * @ingroup multitabbar
 * This class represents a tab bar's tab in a KMultiTabBarWidget.
 * This class should never be created except with the appendTab call of KMultiTabBar
 */
class TDEUTILS_EXPORT KMultiTabBarTab: public KMultiTabBarButton
{
	TQ_OBJECT
public:
  /** @internal */
	KMultiTabBarTab(const TQPixmap& pic,const TQString&,int id,TQWidget *parent,
		KMultiTabBar::KMultiTabBarPosition pos,KMultiTabBar::KMultiTabBarStyle style);
	/**
	 * Destructor.
	 */
	virtual ~KMultiTabBarTab();
	/**
	 * set the active state of the tab
	 * @param  state @c true if the tab should become active, @c false otherwise
	 */
	void setState(bool state);
	/**
	 * choose if the text should always be displayed
	 * this is only used in classic mode if at all
	 * @param show Whether or not to show the text
	 */
	void showActiveTabText(bool show);
	/**
	 * Resized the tab to the needed size.
	 */
	void resize(){ setSize( neededSize() ); }
private:
	bool m_showActiveTabText;
	int m_expandedSize;
	KMultiTabBarTabPrivate *d;
protected:
	friend class KMultiTabBarInternal;
	void setSize(int);
	int neededSize();
	void updateState();
	virtual void drawButton(TQPainter *);
	virtual void drawButtonLabel(TQPainter *);
	void drawButtonStyled(TQPainter *);
	void drawButtonClassic(TQPainter *);
protected slots:
	virtual void slotClicked();
	void setTabsPosition(KMultiTabBar::KMultiTabBarPosition);

public slots:
	virtual void setIcon(const TQString&);
	virtual void setIcon(const TQPixmap&);
};

#endif
