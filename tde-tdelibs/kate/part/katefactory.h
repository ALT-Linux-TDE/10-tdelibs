/* This file is part of the KDE libraries
   Copyright (C) 2001-2004 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __KATE_FACTORY_H__
#define __KATE_FACTORY_H__

#include "katejscript.h"
#include <tdeparts/factory.h>

#include <ktrader.h>
#include <kinstance.h>
#include <tdeaboutdata.h>

// katepart version must be a string in double quotes, format: "x.x"
#define KATEPART_VERSION "2.5"

class KateCmd;
class KateFileTypeManager;
class KateSchemaManager;
class KateDocumentConfig;
class KateViewConfig;
class KateRendererConfig;
class KateDocument;
class KateRenderer;
class KateView;
class KateJScript;
class KateJScriptManager;
class KateIndentScriptManagerAbstract;
class KDirWatch;
class KVMAllocator;

namespace Kate {
  class Command;
}


class KateFactory
{
  private:
    /**
     * Default constructor, private, as singleton
     */
    KateFactory ();

  public:
    /**
     * Destructor
     */
    ~KateFactory ();

    /**
     * singleton accessor
     * @return instance of the factory
     */
    static KateFactory *self ();

    /**
     * reimplemented create object method
     * @param parentWidget parent widget
     * @param widgetName widget name
     * @param parent TQObject parent
     * @param name object name
     * @param classname class parent
     * @param args additional arguments
     * @return constructed part object
     */
    KParts::Part *createPartObject ( TQWidget *parentWidget, const char *widgetName,
                                     TQObject *parent, const char *name, const char *classname,
                                     const TQStringList &args );

    /**
     * public accessor to the instance
     * @return instance
     */
    inline TDEInstance *instance () { return &m_instance; };

    /**
     * register document at the factory
     * this allows us to loop over all docs for example on config changes
     * @param doc document to register
     */
    void registerDocument ( KateDocument *doc );

    /**
     * unregister document at the factory
     * @param doc document to register
     */
    void deregisterDocument ( KateDocument *doc );

    /**
     * register view at the factory
     * this allows us to loop over all views for example on config changes
     * @param view view to register
     */
    void registerView ( KateView *view );

    /**
     * unregister view at the factory
     * @param view view to unregister
     */
    void deregisterView ( KateView *view );

     /**
     * register renderer at the factory
     * this allows us to loop over all views for example on config changes
     * @param renderer renderer to register
     */
    void registerRenderer ( KateRenderer  *renderer );

    /**
     * unregister renderer at the factory
     * @param renderer renderer to unregister
     */
    void deregisterRenderer ( KateRenderer  *renderer );

    /**
     * return a list of all registered docs
     * @return all known documents
     */
    inline TQPtrList<KateDocument> *documents () { return &m_documents; };

    /**
     * return a list of all registered views
     * @return all known views
     */
    inline TQPtrList<KateView> *views () { return &m_views; };

    /**
     * return a list of all registered renderers
     * @return all known renderers
     */
    inline TQPtrList<KateRenderer> *renderers () { return &m_renderers; };

    /**
     * on start detected plugins
     * @return list of all at launch detected tdetexteditor::plugins
     */
    inline const TDETrader::OfferList &plugins () { return m_plugins; };

    /**
     * global dirwatch
     * @return dirwatch instance
     */
    inline KDirWatch *dirWatch () { return m_dirWatch; };

    /**
     * global filetype manager
     * used to manage the file types centrally
     * @return filetype manager
     */
    inline KateFileTypeManager *fileTypeManager () { return m_fileTypeManager; };

    /**
     * manager for the katepart schemas
     * @return schema manager
     */
    inline KateSchemaManager *schemaManager () { return m_schemaManager; };

    /**
     * fallback document config
     * @return default config for all documents
     */
    inline KateDocumentConfig *documentConfig () { return m_documentConfig; }

    /**
     * fallback view config
     * @return default config for all views
     */
    inline KateViewConfig *viewConfig () { return m_viewConfig; }

    /**
     * fallback renderer config
     * @return default config for all renderers
     */
    inline KateRendererConfig *rendererConfig () { return m_rendererConfig; }

    /**
     * Global allocator for swapping
     * @return allocator
     */
    inline KVMAllocator *vm () { return m_vm; }

    /**
     * global interpreter, for nice js stuff
     */
    KateJScript *jscript ();

    /**
     * Global javascript collection
     */
    KateJScriptManager *jscriptManager () { return m_jscriptManager; }


    /**
     * looks up a script given by name. If there are more than
     * one matching, the first found will be taken
     */
    KateIndentScript indentScript (const TQString &scriptname);

  private:
    /**
     * instance of this factory
     */
    static KateFactory *s_self;

    /**
     * about data (authors and more)
     */
    TDEAboutData m_aboutData;

    /**
     * our kinstance
     */
    TDEInstance m_instance;

    /**
     * registered docs
     */
    TQPtrList<KateDocument> m_documents;

    /**
     * registered views
     */
    TQPtrList<KateView> m_views;

    /**
     * registered renderers
     */
    TQPtrList<KateRenderer> m_renderers;

    /**
     * global dirwatch object
     */
    KDirWatch *m_dirWatch;

    /**
     * filetype manager
     */
    KateFileTypeManager *m_fileTypeManager;

    /**
     * schema manager
     */
    KateSchemaManager *m_schemaManager;

    /**
     * at start found plugins
     */
    TDETrader::OfferList m_plugins;

    /**
     * fallback document config
     */
    KateDocumentConfig *m_documentConfig;

    /**
     * fallback view config
     */
    KateViewConfig *m_viewConfig;

    /**
     * fallback renderer config
     */
    KateRendererConfig *m_rendererConfig;

    /**
     * vm allocator
     */
    KVMAllocator *m_vm;

    /**
     * internal commands
     */
    TQValueList<Kate::Command *> m_cmds;

    /**
     * js interpreter
     */
    KateJScript *m_jscript;


    /**
     * js script manager
     */
    KateJScriptManager *m_jscriptManager;


    /**
     * manager for js based indenters
     */
    TQPtrList<KateIndentScriptManagerAbstract> m_indentScriptManagers;

};

#endif
