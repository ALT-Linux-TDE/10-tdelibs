//----------------------------------------------------------------------------
//
//    This program is free software; you can redistribute it and/or modify
//    it under the terms of the GNU Library General Public License as
//    published by the Free Software Foundation; either version 2 of the
//    License, or (at your option) any later version.
//
//----------------------------------------------------------------------------

#include <tqtimer.h>

#include <ktabbar.h>
#include <tdepopupmenu.h>
#include "tdemdidocumentviewtabwidget.h"

KMdiDocumentViewTabWidget::KMdiDocumentViewTabWidget( TQWidget* parent, const char* name ) : KTabWidget( parent, name )
{
	m_visibility = KMdi::ShowWhenMoreThanOneTab;
	tabBar() ->hide();
	setHoverCloseButton( true );
	connect( this, TQ_SIGNAL( closeRequest( TQWidget* ) ), this, TQ_SLOT( closeTab( TQWidget* ) ) );
}

KMdiDocumentViewTabWidget::~KMdiDocumentViewTabWidget()
{}

void KMdiDocumentViewTabWidget::closeTab( TQWidget* w )
{
	w->close();
}
void KMdiDocumentViewTabWidget::addTab ( TQWidget * child, const TQString & label )
{
	KTabWidget::addTab( child, label );
	showPage( child );
	TQTimer::singleShot(0, this, TQ_SLOT(maybeShow()));
}

void KMdiDocumentViewTabWidget::addTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label )
{
	KTabWidget::addTab( child, iconset, label );
	showPage( child );
	TQTimer::singleShot(0, this, TQ_SLOT(maybeShow()));
}

void KMdiDocumentViewTabWidget::addTab ( TQWidget * child, TQTab * tab )
{
	KTabWidget::addTab( child, tab );
	showPage( child );
	TQTimer::singleShot(0, this, TQ_SLOT(maybeShow()));
}

void KMdiDocumentViewTabWidget::insertTab ( TQWidget * child, const TQString & label, int index )
{
	KTabWidget::insertTab( child, label, index );
	showPage( child );
	TQTimer::singleShot(0, this, TQ_SLOT(maybeShow()));
	tabBar() ->repaint();
}

void KMdiDocumentViewTabWidget::insertTab ( TQWidget * child, const TQIconSet & iconset, const TQString & label, int index )
{
	KTabWidget::insertTab( child, iconset, label, index );
	showPage( child );
	TQTimer::singleShot(0, this, TQ_SLOT(maybeShow()));
	tabBar() ->repaint();
}

void KMdiDocumentViewTabWidget::insertTab ( TQWidget * child, TQTab * tab, int index )
{
	KTabWidget::insertTab( child, tab, index );
	showPage( child );
	TQTimer::singleShot(0, this, TQ_SLOT(maybeShow()));
	tabBar() ->repaint();
}

void KMdiDocumentViewTabWidget::removePage ( TQWidget * w )
{
	KTabWidget::removePage( w );
	TQTimer::singleShot(0, this, TQ_SLOT(maybeShow()));
}

void KMdiDocumentViewTabWidget::updateIconInView( TQWidget *w, TQPixmap icon )
{
	changeTab( w, icon, tabLabel( w ) );
}

void KMdiDocumentViewTabWidget::updateCaptionInView( TQWidget *w, const TQString &caption )
{
	changeTab( w, caption );
}

void KMdiDocumentViewTabWidget::maybeShow()
{
	switch (m_visibility)
	{
		case KMdi::AlwaysShowTabs:
			tabBar() ->show();
			if ( cornerWidget() )
			{
				if ( count() == 0 )
					cornerWidget() ->hide();
				else
					cornerWidget() ->show();
			}
			break;

		case KMdi::ShowWhenMoreThanOneTab:
			if ( count() < 2 )
				tabBar() ->hide();
			else tabBar() ->show();
			if ( cornerWidget() )
			{
				if ( count() < 2 )
					cornerWidget() ->hide();
				else
					cornerWidget() ->show();
			}
			break;
		case KMdi::NeverShowTabs:
			tabBar() ->hide();
			break;
	}
}

void KMdiDocumentViewTabWidget::setTabWidgetVisibility( KMdi::TabWidgetVisibility visibility )
{
	m_visibility = visibility;
	maybeShow();
}

void KMdiDocumentViewTabWidget::moveTab( int from, int to )
{
  emit initiateTabMove( from, to );
  KTabWidget::moveTab( from, to );
}

KMdi::TabWidgetVisibility KMdiDocumentViewTabWidget::tabWidgetVisibility( )
{
	return m_visibility;
}


#ifndef NO_INCLUDE_MOCFILES
#include "tdemdidocumentviewtabwidget.moc"
#endif
