/* This file is part of the KDE project
   Copyright (C) 1999 Simon Hausmann <hausmann@kde.org>
             (C) 1999 David Faure <faure@kde.org>

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
#ifndef __tdeparts_factory_h__
#define __tdeparts_factory_h__

#include <klibloader.h>

class TQWidget;

namespace KParts
{

class Part;

/**
 * A generic factory object to create a Part.
 *
 * Factory is an abstract class. Reimplement the 
 * createPartObject() method to give it functionality.
 *
 * @see KLibFactory.
 */
class TDEPARTS_EXPORT Factory : public KLibFactory
{
  TQ_OBJECT
public:
  Factory( TQObject *parent = 0, const char *name = 0 );
  virtual ~Factory();

    /**
     * Creates a part.
     *
     * The TQStringList can be used to pass additional arguments to the part.
     * If the part needs additional arguments, it should take them as
     * name="value" pairs. This is the way additional arguments will get passed
     * to the part from eg. tdehtml. You can for example embed the part into HTML
     * by using the following code:
     * \code
     *    <object type="my_mimetype" data="url_to_my_data">
     *        <param name="name1" value="value1">
     *        <param name="name2" value="value2">
     *    </object>
     * \endcode
     * This could result in a call to
     * \code
     *     createPart( parentWidget, name, parentObject, parentName, "KParts::Part",
     *                 TQStringList("name1="value1"", "name2="value2") );
     * \endcode
     *
     * @returns the newly created part.
     *
     * createPart() automatically emits a signal KLibFactory::objectCreated to tell
     * the library about its newly created object.  This is very
     * important for reference counting, and allows unloading the
     * library automatically once all its objects have been destroyed.
     */
     Part *createPart( TQWidget *parentWidget = 0, const char *widgetName = 0, TQObject *parent = 0, const char *name = 0, const char *classname = "KParts::Part", const TQStringList &args = TQStringList() );

     /**
      * If you have a part contained in a shared library you might want to query
      * for meta-information like the about-data, or the TDEInstance in general.
      * If the part is exported using KParts::GenericFactory then this method will
      * return the instance that belongs to the part without the need to instantiate
      * the part component.
      */
     const TDEInstance *partInstance();

     /**
      * A convenience method for partInstance() that takes care of retrieving
      * the factory for a given library name and calling partInstance() on it.
      *
      * @param libraryName name of the library to query the instance from
      */
     static const TDEInstance *partInstanceFromLibrary( const TQCString &libraryName );

protected:

    /**
     * Reimplement this method in your implementation to create the Part.
     *
     * The TQStringList can be used to pass additional arguments to the part.
     * If the part needs additional arguments, it should take them as
     * name="value" pairs. This is the way additional arguments will get passed
     * to the part from eg. tdehtml. You can for example emebed the part into HTML
     * by using the following code:
     * \code
     *    <object type="my_mimetype" data="url_to_my_data">
     *        <param name="name1" value="value1">
     *        <param name="name2" value="value2">
     *    </object>
     * \endcode
     * This could result in a call to
     * \code
     *     createPart( parentWidget, name, parentObject, parentName, "Kparts::Part",
     *                 TQStringList("name1="value1"", "name2="value2") );
     * \endcode
     *
     * @returns the newly created part.
     */
    virtual Part *createPartObject( TQWidget *parentWidget = 0, const char *widgetName = 0, TQObject *parent = 0, const char *name = 0, const char *classname = "KParts::Part", const TQStringList &args = TQStringList() ) = 0;
    
    /**
     * Reimplemented from KLibFactory. Calls createPart()
     */
    virtual TQObject *createObject( TQObject *parent = 0, const char *name = 0, const char *classname = "TQObject", const TQStringList &args = TQStringList() );

    /** This 'enum' along with the structure below is NOT part of the public API.
      * It's going to disappear in KDE 4.0 and is likely to change inbetween.
      *
      * @internal
      */
    enum { VIRTUAL_QUERY_INSTANCE_PARAMS = 0x10 };
    struct QueryInstanceParams
    {
        const TDEInstance *instance;
    };
};

}
#endif
