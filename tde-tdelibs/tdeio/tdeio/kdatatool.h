/* This file is part of the KDE project
   Copyright (C) 1998, 1999, 2000 Torben Weis <weis@kde.org>
   Copyright (C) 2001 David Faure <david@mandrakesoft.com>

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
*/

#ifndef KDATATOOL_H
#define KDATATOOL_H

#include <tqobject.h>
#include <tqvaluelist.h>

#include <tdeaction.h>
#include <kservice.h>

class KDataTool;
class TQPixmap;
class TQStringList;
class TDEInstance;

// If you're only looking at implementing a data-tool, skip directly to the last
// class definition, KDataTool.

/**
 * This is a convenience class for KService. You can use it if you have
 * a KService describing a KDataTool. In this case the KDataToolInfo class
 * is more convenient to work with.
 *
 * Especially useful is the method createTool which creates the datatool
 * described by the service.
 * @see KDataTool
 */
class TDEIO_EXPORT KDataToolInfo
{
public:
    /**
     * Create an invalid KDataToolInfo.
     */
    KDataToolInfo();
    /**
     * Create a valid KDataToolInfo.
     * @param service the corresponding service
     * @param instance the instance to use
     */
    KDataToolInfo( const KService::Ptr& service, TDEInstance* instance );
    /**
     * Copy constructor.
     */
    KDataToolInfo( const KDataToolInfo& info );
    /**
     * Assignment operator.
     */
    KDataToolInfo& operator= ( const KDataToolInfo& info );

    /**
     * Returns the data type that the DataTool can accept.
     * @return the C++ data type that this DataTool accepts.
     *         For example "TQString" or "TQImage" or something more
     *         complicated.
     */
    TQString dataType() const;
    /**
     * Returns a list of mime type that will be accepted by the DataTool.
     * The mimetypes are only used if the dataType can be used to store
     * different mimetypes. For example in a "TQString" you could save "text/plain"
     * or "text/html" or "text/xml".
     *
     * @return the mime types accepted by this DataTool. For example
     *         "image/gif" or "text/plain". In some cases the dataType
     *         determines the accepted type of data perfectly. In this cases
     *         this list may be empty.
     */
    TQStringList mimeTypes() const;

    /**
     * Checks whether the DataTool is read-only.
     * @return true if the DataTool does not modify the data passed to it by KDataTool::run.
     */
    bool isReadOnly() const;

    /**
     * Returns the icon of this data tool.
     * @return a large pixmap for the DataTool.
     * @deprecated, use iconName()
     */
    TQPixmap icon() const TDE_DEPRECATED;
    /**
     * Returns the mini icon of this data tool.
     * @return a mini pixmap for the DataTool.
     * @deprecated, use iconName()
     */
    TQPixmap miniIcon() const TDE_DEPRECATED;
    /**
     * Returns the icon name for this DataTool.
     * @return the name of the icon for the DataTool
     */
    TQString iconName() const;
    /**
     * Returns a list of strings that you can put in a TQPopupMenu item, for example to
     * offer the DataTools services to the user. The returned value
     * is usually something like "Spell checking", "Shrink Image", "Rotate Image"
     * or something like that.     
     * This list comes from the Comment field of the tool's desktop file
     * (so that it can be translated).
     *
     * Each of the strings returned corresponds to a string in the list returned by
     * commands.
     *
     * @return a list of strings that you can put in a TQPopupMenu item
     */
    TQStringList userCommands() const;
    /**
     * Returns the list of commands the DataTool can execute. The application
     * passes the command to the KDataTool::run method.
     *
     * This list comes from the Commands field of the tool's desktop file.
     *
     * Each of the strings returned corresponds to a string in the list returned by
     * userCommands.
     * @return the list of commands the DataTool can execute, suitable for 
     *         the KDataTool::run method.
     */
    TQStringList commands() const;

    /**
     * Creates the data tool described by this KDataToolInfo.
     * @param parent the parent of the TQObject (or 0 for parent-less KDataTools)
     * @param name the name of the TQObject, can be 0
     * @return a pointer to the created data tool or 0 on error.
     */
    KDataTool* createTool( TQObject* parent = 0, const char* name = 0 ) const;

    /**
     * The KDataToolInfo's service that is represented by this class.
     * @return the service
     */
    KService::Ptr service() const;

    /**
     * The instance of the service.
     * @return the instance
     */
    TDEInstance* instance() const { return m_instance; }

    /**
     * A DataToolInfo may be invalid if the KService passed to its constructor does
     * not feature the service type "KDataTool".
     * @return true if valid, false otherwise
     */
    bool isValid() const;

    /**
     * Queries the TDETrader about installed KDataTool implementations.
     * @param datatype a type that the application can 'export' to the tools (e.g. TQString)
     * @param mimetype the mimetype of the data (e.g. text/plain)
     * @param instance the application (or the part)'s instance (to check if a tool is excluded from this part,
     * and also used if the tool wants to read its configuration in the app's config file).
     * @return the list of results
     */
    static TQValueList<KDataToolInfo> query( const TQString& datatype, const TQString& mimetype, TDEInstance * instance );

private:
    KService::Ptr m_service;
    TDEInstance* m_instance;
private:
    class KDataToolInfoPrivate* d;
};


/**
 * This class helps applications implement support for KDataTool.
 * The steps to follow are simple:
 * @li query for the available tools using KDataToolInfo::query
 * @li pass the result to KDataToolAction::dataToolActionList (with a slot)
 * @li plug the resulting actions, either using KXMLGUIClient::plugActionList, or by hand.
 *
 * The slot defined for step 2 is called when the action is activated, and
 * that's where the tool should be created and run.
 */
class TDEIO_EXPORT KDataToolAction : public TDEAction
{
    TQ_OBJECT
    
public:
    /**
     * Constructs a new KDataToolAction.
     *
     * @param text The text that will be displayed.
     * @param info the corresponding KDataToolInfo
     * @param command the command of the action
     * @param parent This action's parent.
     * @param name An internal name for this action.
     */
    KDataToolAction( const TQString & text, const KDataToolInfo & info, const TQString & command, TQObject * parent = 0, const char * name = 0);

    /**
     * Creates a list of actions from a list of information about data-tools.
     * The slot must have a signature corresponding to the toolActivated signal.
     *
     * Note that it's the caller's responsibility to delete the actions when they're not needed anymore.
     * @param tools the list of data tool descriptions
     * @param receiver the receiver for toolActivated() signals
     * @param slot the slot that will receive the toolActivated() signals
     * @return the TDEActions
     */
    static TQPtrList<TDEAction> dataToolActionList( const TQValueList<KDataToolInfo> & tools, const TQObject *receiver, const char* slot );

signals:
    /**
     * Emitted when a tool has been activated.
     * @param info a description of the activated tools
     * @param command the command for the tool
     */
    void toolActivated( const KDataToolInfo & info, const TQString & command );

protected:
    virtual void slotActivated();

private:
    TQString m_command;
    KDataToolInfo m_info;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KDataToolActionPrivate* d;

};

/**
 * A generic tool that processes data.
 *
 * A data-tool is a "plugin" for an application, that acts (reads/modifies)
 * on a portion of the data present in the document (e.g. a text document,
 * a single word or paragraph, a KSpread cell, an image, etc.)
 *
 * The application has some generic code for presenting the tools in a popupmenu
 * @see KDataToolAction, and for activating a tool, passing it the data
 * (and possibly getting modified data from it).
 */
class TDEIO_EXPORT KDataTool : public TQObject
{
    TQ_OBJECT
    
public:
    /**
     * Constructor
     * The data-tool is only created when a menu-item, that relates to it, is activated.
     * @param parent the parent of the TQObject (or 0 for parent-less KDataTools)
     * @param name the name of the TQObject, can be 0
     */
    KDataTool( TQObject* parent = 0, const char* name = 0 );

    /**
     * @internal. Do not use under any circumstance (including bad weather).
     */
    void setInstance( TDEInstance* instance ) { m_instance = instance; }

    /**
     * Returns the instance of the part that created this tool.
     * Usually used if the tool wants to read its configuration in the app's config file.
     * @return the instance of the part that created this tool.
     */
    TDEInstance* instance() const;

    /**
     * Interface for 'running' this tool.
     * This is the method that the data-tool must implement.
     *
     * @param command is the command that was selected (see KDataToolInfo::commands())
     * @param data the data provided by the application, on which to run the tool.
     *             The application is responsible for setting that data before running the tool,
     *             and for getting it back and updating itself with it, after the tool ran.
     * @param datatype defines the type of @p data.
     * @param mimetype defines the mimetype of the data (for instance datatype may be
     *                 TQString, but the mimetype can be text/plain, text/html etc.)
     * @return true if successful, false otherwise
     */
    virtual bool run( const TQString& command, void* data, const TQString& datatype, const TQString& mimetype) = 0;

private:
    TDEInstance * m_instance;
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KDataToolPrivate;
    KDataToolPrivate * d;
};

#endif
