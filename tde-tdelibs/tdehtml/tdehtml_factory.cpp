/* This file is part of the KDE project
 *
 * Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#include "tdehtml_factory.h"
#include "tdehtml_part.h"
#include "tdehtml_settings.h"

#include "css/cssstyleselector.h"
#include "html/html_imageimpl.h"
#include "rendering/render_style.h"
#include "rendering/break_lines.h"
#include "misc/loader.h"
#include "misc/arena.h"

#include <kinstance.h>
#include <tdeaboutdata.h>
#include <tdelocale.h>

#include <assert.h>

#include <kdebug.h>

template class TQPtrList<TDEHTMLPart>;

extern "C" TDE_EXPORT void *init_libtdehtml()
{
    // We can't use a plain self() here, because that would
    // return the global factory, which might already exist
    // at the time init_libtdehtml is called! As soon as someone
    // does new TDEHTMLPart() in his application and loads up
    // an html document into that part which either embeds
    // embeds another TDEHTMLPart instance via <object> or
    // as html frame, then we cannot return self(), as
    // what we return here is what the KLibLoader deletes
    // in the end, and we don't want the libloader to
    // delete our global instance. Anyway, the new
    // TDEHTMLFactory we create here is very cheap :)
    // (Simon)
    return new TDEHTMLFactory( true );
}

TDEHTMLFactory *TDEHTMLFactory::s_self = 0;
unsigned long int TDEHTMLFactory::s_refcnt = 0;
TDEInstance *TDEHTMLFactory::s_instance = 0;
TDEAboutData *TDEHTMLFactory::s_about = 0;
TDEHTMLSettings *TDEHTMLFactory::s_settings = 0;
TQPtrList<TDEHTMLPart> *TDEHTMLFactory::s_parts = 0;
TQString *TDEHTMLSettings::avFamilies = 0;

TDEHTMLFactory::TDEHTMLFactory( bool clone )
{
    if ( clone )
        ref();
}

TDEHTMLFactory::~TDEHTMLFactory()
{
    if ( s_self == this )
    {
        assert( !s_refcnt );

        delete s_instance;
        delete s_about;
        delete s_settings;
	delete TDEHTMLSettings::avFamilies;
        if ( s_parts )
        {
            assert( s_parts->isEmpty() );
            delete s_parts;
        }

        s_instance = 0;
        s_about = 0;
        s_settings = 0;
        s_parts = 0;
	TDEHTMLSettings::avFamilies = 0;

        // clean up static data
        tdehtml::CSSStyleSelector::clear();
        tdehtml::RenderStyle::cleanup();
        tdehtml::Cache::clear();
        tdehtml::cleanup_thaibreaks();
        tdehtml::ArenaFinish();
    }
    else
        deref();
}

KParts::Part *TDEHTMLFactory::createPartObject( TQWidget *parentWidget, const char *widgetName, TQObject *parent, const char *name, const char *className, const TQStringList & )
{
  TDEHTMLPart::GUIProfile prof = TDEHTMLPart::DefaultGUI;
  if ( strcmp( className, "Browser/View" ) == 0 )
    prof = TDEHTMLPart::BrowserViewGUI;

  return new TDEHTMLPart( parentWidget, widgetName, parent, name, prof );
}

void TDEHTMLFactory::ref()
{
    if ( !s_refcnt && !s_self )
    {
        // we can't use a staticdeleter here, because that would mean
        // that the factory gets deleted from within a qPostRoutine, called
        // from the TQApplication destructor. That however is too late, because
        // we want to destruct a TDEInstance object, which involves destructing
        // a TDEConfig object, which might call TDEGlobal::dirs() (in sync()) which
        // probably is not going to work ;-)
        // well, perhaps I'm wrong here, but as I'm unsure I try to stay on the
        // safe side ;-) -> let's use a simple reference counting scheme
        // (Simon)
        s_self = new TDEHTMLFactory;
        tdehtml::Cache::init();
    }

    s_refcnt++;
}

void TDEHTMLFactory::deref()
{
    if ( !--s_refcnt && s_self )
    {
        delete s_self;
        s_self = 0;
    }
}

void TDEHTMLFactory::registerPart( TDEHTMLPart *part )
{
    if ( !s_parts )
        s_parts = new TQPtrList<TDEHTMLPart>;

    if ( !s_parts->containsRef( part ) )
    {
        s_parts->append( part );
        ref();
    }
}

void TDEHTMLFactory::deregisterPart( TDEHTMLPart *part )
{
    assert( s_parts );

    if ( s_parts->removeRef( part ) )
    {
        if ( s_parts->isEmpty() )
        {
            delete s_parts;
            s_parts = 0;
        }
        deref();
    }
}

TDEInstance *TDEHTMLFactory::instance()
{
  assert( s_self );

  if ( !s_instance )
  {
    s_about = new TDEAboutData( "tdehtml", I18N_NOOP( "TDEHTML" ), "4.0",
                              I18N_NOOP( "Embeddable HTML component" ),
                              TDEAboutData::License_LGPL );
    s_about->addAuthor( "Lars Knoll", 0, "knoll@kde.org" );
    s_about->addAuthor( "Antti Koivisto", 0, "koivisto@kde.org" );
    s_about->addAuthor( "Waldo Bastian", 0, "bastian@kde.org" );
    s_about->addAuthor( "Dirk Mueller", 0, "mueller@kde.org" );
    s_about->addAuthor( "Peter Kelly", 0, "pmk@kde.org" );
    s_about->addAuthor( "Torben Weis", 0, "weis@kde.org" );
    s_about->addAuthor( "Martin Jones", 0, "mjones@kde.org" );
    s_about->addAuthor( "Simon Hausmann", 0, "hausmann@kde.org" );
    s_about->addAuthor( "Tobias Anton", 0, "anton@stud.fbi.fh-darmstadt.de" );

    s_instance = new TDEInstance( s_about );
  }

  return s_instance;
}

TDEHTMLSettings *TDEHTMLFactory::defaultHTMLSettings()
{
  assert( s_self );
  if ( !s_settings )
    s_settings = new TDEHTMLSettings();

  return s_settings;
}

using namespace KParts;
#include "tdehtml_factory.moc"

