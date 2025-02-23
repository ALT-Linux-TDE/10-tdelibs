/* This file is part of the KDE libraries
   Copyright (c) 2003 Scott Wheeler <wheeler@kde.org>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include "tdelistviewsearchline.h"

#include <tdelistview.h>
#include <kiconloader.h>
#include <tdetoolbar.h>
#include <tdetoolbarbutton.h>
#include <kdebug.h>
#include <tdelocale.h>

#include <tqapplication.h>
#include <tqtimer.h>
#include <tqpopupmenu.h>
#include <tqlabel.h>
#include <tqheader.h>

#define KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID 2004

class TDEListViewSearchLine::TDEListViewSearchLinePrivate
{
public:
    TDEListViewSearchLinePrivate() :
        listView(0),
        caseSensitive(false),
        activeSearch(false),
        keepParentsVisible(true),
        queuedSearches(0) {}

    TDEListView *listView;
    bool caseSensitive;
    bool activeSearch;
    bool keepParentsVisible;
    TQString search;
    int queuedSearches;
    TQValueList<int> searchColumns;
};

////////////////////////////////////////////////////////////////////////////////
// public methods
////////////////////////////////////////////////////////////////////////////////

TDEListViewSearchLine::TDEListViewSearchLine(TQWidget *parent, TDEListView *listView, const char *name) :
    KLineEdit(parent, name)
{
    d = new TDEListViewSearchLinePrivate;

    d->listView = listView;

    connect(this, TQ_SIGNAL(textChanged(const TQString &)),
            this, TQ_SLOT(queueSearch(const TQString &)));

    if(listView) {
        connect(listView, TQ_SIGNAL(destroyed()),
                this, TQ_SLOT(listViewDeleted()));

        connect(listView, TQ_SIGNAL(itemAdded(TQListViewItem *)),
                this, TQ_SLOT(itemAdded(TQListViewItem *)));
    }
    else
        setEnabled(false);
}

TDEListViewSearchLine::TDEListViewSearchLine(TQWidget *parent, const char *name) :
    KLineEdit(parent, name)
{
    d = new TDEListViewSearchLinePrivate;

    d->listView = 0;

    connect(this, TQ_SIGNAL(textChanged(const TQString &)),
            this, TQ_SLOT(queueSearch(const TQString &)));

    setEnabled(false);
}

TDEListViewSearchLine::~TDEListViewSearchLine()
{
    delete d;
}

bool TDEListViewSearchLine::caseSensitive() const
{
    return d->caseSensitive;
}

TQValueList<int> TDEListViewSearchLine::searchColumns() const
{
    return d->searchColumns;
}

bool TDEListViewSearchLine::keepParentsVisible() const
{
    return d->keepParentsVisible;
}

TDEListView *TDEListViewSearchLine::listView() const
{
    return d->listView;
}

////////////////////////////////////////////////////////////////////////////////
// public slots
////////////////////////////////////////////////////////////////////////////////

void TDEListViewSearchLine::updateSearch(const TQString &s)
{
    if(!d->listView)
        return;

    d->search = s.isNull() ? text() : s;

    // If there's a selected item that is visible, make sure that it's visible
    // when the search changes too (assuming that it still matches).

    TQListViewItem *currentItem = 0;

    switch(d->listView->selectionMode())
    {
    case TDEListView::NoSelection:
        break;
    case TDEListView::Single:
        currentItem = d->listView->selectedItem();
        break;
    default:
    {
        int flags = TQListViewItemIterator::Selected | TQListViewItemIterator::Visible;
        for(TQListViewItemIterator it(d->listView, flags);
            it.current() && !currentItem;
            ++it)
        {
            if(d->listView->itemRect(it.current()).isValid())
                currentItem = it.current();
        }
    }
    }

    if(d->keepParentsVisible)
        checkItemParentsVisible(d->listView->firstChild());
    else
        checkItemParentsNotVisible();

    if(currentItem)
        d->listView->ensureItemVisible(currentItem);
}

void TDEListViewSearchLine::setCaseSensitive(bool cs)
{
    d->caseSensitive = cs;
}

void TDEListViewSearchLine::setKeepParentsVisible(bool v)
{
    d->keepParentsVisible = v;
}

void TDEListViewSearchLine::setSearchColumns(const TQValueList<int> &columns)
{
    d->searchColumns = columns;
}

void TDEListViewSearchLine::setListView(TDEListView *lv)
{
    if(d->listView) {
        disconnect(d->listView, TQ_SIGNAL(destroyed()),
                   this, TQ_SLOT(listViewDeleted()));

        disconnect(d->listView, TQ_SIGNAL(itemAdded(TQListViewItem *)),
                   this, TQ_SLOT(itemAdded(TQListViewItem *)));
    }

    d->listView = lv;

    if(lv) {
        connect(d->listView, TQ_SIGNAL(destroyed()),
                this, TQ_SLOT(listViewDeleted()));

        connect(d->listView, TQ_SIGNAL(itemAdded(TQListViewItem *)),
                this, TQ_SLOT(itemAdded(TQListViewItem *)));
    }

    setEnabled(bool(lv));
}

////////////////////////////////////////////////////////////////////////////////
// protected members
////////////////////////////////////////////////////////////////////////////////

bool TDEListViewSearchLine::itemMatches(const TQListViewItem *item, const TQString &s) const
{
    if(s.isEmpty())
        return true;

    // If the search column list is populated, search just the columns
    // specifified.  If it is empty default to searching all of the columns.

    if(!d->searchColumns.isEmpty()) {
        TQValueList<int>::ConstIterator it = d->searchColumns.begin();
        for(; it != d->searchColumns.end(); ++it) {
            if(*it < item->listView()->columns() &&
               item->text(*it).find(s, 0, d->caseSensitive) >= 0)
                return true;
        }
    }
    else {
        for(int i = 0; i < item->listView()->columns(); i++) {
            if(item->listView()->columnWidth(i) > 0 &&
               item->text(i).find(s, 0, d->caseSensitive) >= 0)
            {
                return true;
            }
        }
    }

    return false;
}

TQPopupMenu *TDEListViewSearchLine::createPopupMenu()
{
    TQPopupMenu *popup = KLineEdit::createPopupMenu();

    if (d->listView->columns()>1) {
        TQPopupMenu *subMenu = new TQPopupMenu(popup);
        connect(subMenu, TQ_SIGNAL(activated(int)), this, TQ_SLOT(searchColumnsMenuActivated(int)));

        popup->insertSeparator();
        popup->insertItem(i18n("Search Columns"), subMenu);
    
        subMenu->insertItem(i18n("All Visible Columns"), KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID);
        subMenu->insertSeparator();
    
        bool allColumnsAreSearchColumns = true;
	// TODO Make the entry order match the actual column order
	TQHeader* const header = d->listView->header();
	int visibleColumns = 0;
	for(int i = 0; i < d->listView->columns(); i++) {
	    if(d->listView->columnWidth(i)>0) {
	        TQString columnText = d->listView->columnText(i);
	        if(columnText.isEmpty()) {
		    int visiblePosition=1;
		    for(int j = 0; j < header->mapToIndex(i); j++)
		        if(d->listView->columnWidth(header->mapToSection(j))>0)
		            visiblePosition++;
		    columnText = i18n("Column number %1","Column No. %1").arg(visiblePosition);
	        }
                subMenu->insertItem(columnText, visibleColumns);
	        if(d->searchColumns.isEmpty() || d->searchColumns.find(i) != d->searchColumns.end())
		    subMenu->setItemChecked(visibleColumns, true);
                else
                    allColumnsAreSearchColumns = false;
	        visibleColumns++;
	    }
        }
        subMenu->setItemChecked(KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID, allColumnsAreSearchColumns);
    
        // searchColumnsMenuActivated() relies on one possible "all" representation
        if(allColumnsAreSearchColumns && !d->searchColumns.isEmpty())
            d->searchColumns.clear();
    }
    
    return popup;   
}    

////////////////////////////////////////////////////////////////////////////////
// protected slots
////////////////////////////////////////////////////////////////////////////////

void TDEListViewSearchLine::queueSearch(const TQString &search)
{
    d->queuedSearches++;
    d->search = search;
    TQTimer::singleShot(200, this, TQ_SLOT(activateSearch()));
}

void TDEListViewSearchLine::activateSearch()
{
    --(d->queuedSearches);

    if(d->queuedSearches == 0)
        updateSearch(d->search);
}

////////////////////////////////////////////////////////////////////////////////
// private slots
////////////////////////////////////////////////////////////////////////////////

void TDEListViewSearchLine::itemAdded(TQListViewItem *item) const
{
    item->setVisible(itemMatches(item, text()));
}

void TDEListViewSearchLine::listViewDeleted()
{
    d->listView = 0;
    setEnabled(false);
}

void TDEListViewSearchLine::searchColumnsMenuActivated(int id)
{
    if(id == KLISTVIEWSEARCHLINE_ALLVISIBLECOLUMNS_ID) {
        if(d->searchColumns.isEmpty())
            d->searchColumns.append(0);
        else
            d->searchColumns.clear();
    }
    else {
        if(d->searchColumns.find(id) != d->searchColumns.end())
            d->searchColumns.remove(id);
        else {
            if(d->searchColumns.isEmpty()) {
                for(int i = 0; i < d->listView->columns(); i++) {
                    if(i != id)
                        d->searchColumns.append(i);
                }
            }
            else
                d->searchColumns.append(id);
        }
    }
    updateSearch();
}

////////////////////////////////////////////////////////////////////////////////
// private methods
////////////////////////////////////////////////////////////////////////////////

void TDEListViewSearchLine::checkItemParentsNotVisible()
{
    TQListViewItemIterator it(d->listView);
    for(; it.current(); ++it)
    {
        TQListViewItem *item = it.current();
        if(itemMatches(item, d->search))
            item->setVisible(true);
        else
            item->setVisible(false);
    }
}

#include <kdebug.h>

/** Check whether \p item, its siblings and their descendents should be shown. Show or hide the items as necessary.
 *
 *  \p item  The list view item to start showing / hiding items at. Typically, this is the first child of another item, or the
 *              the first child of the list view.
 *  \p highestHiddenParent  The highest (closest to root) ancestor of \p item which is hidden. If 0, all parents of
 *                           \p item must be visible.
 *  \return \c true if an item which should be visible is found, \c false if all items found should be hidden. If this function
 *             returns true and \p highestHiddenParent was not 0, highestHiddenParent will have been shown.
 */
bool TDEListViewSearchLine::checkItemParentsVisible(TQListViewItem *item, TQListViewItem *highestHiddenParent)
{
    bool visible = false;
    TQListViewItem * first = item;
    for(; item; item = item->nextSibling())
    {
        //What we pass to our children as highestHiddenParent:
        TQListViewItem * hhp = highestHiddenParent ? highestHiddenParent : item->isVisible() ? 0L : item;
        bool childMatch = false;
        if(item->firstChild() && checkItemParentsVisible(item->firstChild(), hhp))
            childMatch = true;
        // Should this item be shown? It should if any children should be, or if it matches.
        if(childMatch || itemMatches(item, d->search))
        {
            visible = true;
            if (highestHiddenParent)
            {
                highestHiddenParent->setVisible(true);
                // Calling setVisible on our ancestor will unhide all its descendents. Hide the ones
                // before us that should not be shown.
                for(TQListViewItem *hide = first; hide != item; hide = hide->nextSibling())
                    hide->setVisible(false);
                highestHiddenParent = 0;
                // If we matched, than none of our children matched, yet the setVisible() call on our
                // ancestor unhid them, undo the damage:
                if(!childMatch)
                    for(TQListViewItem *hide = item->firstChild(); hide; hide = hide->nextSibling())
                        hide->setVisible(false);
            }
            else
                item->setVisible(true);
        }
        else
            item->setVisible(false);
    }
    return visible;
}

////////////////////////////////////////////////////////////////////////////////
// TDEListViewSearchLineWidget
////////////////////////////////////////////////////////////////////////////////

class TDEListViewSearchLineWidget::TDEListViewSearchLineWidgetPrivate
{
public:
    TDEListViewSearchLineWidgetPrivate() : listView(0), searchLine(0), clearButton(0) {}
    TDEListView *listView;
    TDEListViewSearchLine *searchLine;
    TQToolButton *clearButton;
};

TDEListViewSearchLineWidget::TDEListViewSearchLineWidget(TDEListView *listView,
                                                     TQWidget *parent,
                                                     const char *name) :
    TQHBox(parent, name)
{
    d = new TDEListViewSearchLineWidgetPrivate;
    d->listView = listView;

    setSpacing(5);

    TQTimer::singleShot(0, this, TQ_SLOT(createWidgets()));
}

TDEListViewSearchLineWidget::~TDEListViewSearchLineWidget()
{
    delete d;
}

TDEListViewSearchLine *TDEListViewSearchLineWidget::createSearchLine(TDEListView *listView)
{
    if(!d->searchLine)
        d->searchLine = new TDEListViewSearchLine(this, listView);
    return d->searchLine;
}

void TDEListViewSearchLineWidget::createWidgets()
{
    positionInToolBar();

    if(!d->clearButton) {
        d->clearButton = new TQToolButton(this);
        TQIconSet icon = SmallIconSet(TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase");
        d->clearButton->setIconSet(icon);
    }

    d->clearButton->show();

    TQLabel *label = new TQLabel(i18n("S&earch:"), this, "tde toolbar widget");

    d->searchLine = createSearchLine(d->listView);
    d->searchLine->show();

    label->setBuddy(d->searchLine);
    label->show();

    connect(d->clearButton, TQ_SIGNAL(clicked()), d->searchLine, TQ_SLOT(clear()));
}

TDEListViewSearchLine *TDEListViewSearchLineWidget::searchLine() const
{
    return d->searchLine;
}

void TDEListViewSearchLineWidget::positionInToolBar()
{
    TDEToolBar *toolBar = dynamic_cast<TDEToolBar *>(parent());

    if(toolBar) {

        // Here we have The Big Ugly.  Figure out how many widgets are in the
        // and do a hack-ish iteration over them to find this widget so that we
        // can insert the clear button before it.

        int widgetCount = toolBar->count();

        for(int index = 0; index < widgetCount; index++) {
            int id = toolBar->idAt(index);
            if(toolBar->getWidget(id) == this) {
                toolBar->setItemAutoSized(id);
                if(!d->clearButton) {
                    TQString icon = TQApplication::reverseLayout() ? "clear_left" : "locationbar_erase";
                    d->clearButton = new TDEToolBarButton(icon, 2005, toolBar);
                }
                toolBar->insertWidget(2005, d->clearButton->width(), d->clearButton, index);
                break;
            }
        }
    }

    if(d->searchLine)
        d->searchLine->show();
}

#include "tdelistviewsearchline.moc"
