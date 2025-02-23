/* This file is part of the KDE project
   Copyright (C) 2000 Simon Hausmann <hausmann@kde.org>
                      David Faure <faure@kde.org>

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
#ifndef __kxmlguibuilder_h__
#define __kxmlguibuilder_h__

#include <tqdom.h>
#include <tqstringlist.h>

#include <tdelibs_export.h>

class KXMLGUIBuilderPrivate;
class TQWidget;
class TDEInstance;
class KXMLGUIClient;

/**
 * Abstract interface for a "GUI builder", used by the GUIFactory
 * This interface is implemented by TDEMainWindow for the case where
 * the toplevel widget is a TDEMainWindow. Other implementations may appear
 * in the future (dialogs for instance)
 */
class TDEUI_EXPORT KXMLGUIBuilder
{
 public:

  KXMLGUIBuilder( TQWidget *widget );
  virtual ~KXMLGUIBuilder();

  /* @internal */
  KXMLGUIClient *builderClient() const;
  /* @internal */
  void setBuilderClient( KXMLGUIClient *client );
  /* @internal */
  TDEInstance *builderInstance() const;
  /* @internal */
  void setBuilderInstance( TDEInstance *instance );
  /* @internal */
  TQWidget *widget();

  virtual TQStringList containerTags() const;

  /**
   * Creates a container (menubar/menu/toolbar/statusbar/separator/...)
   * from an element in the XML file
   *
   * @param parent The parent for the container
   * @param index The index where the container should be inserted
   *              into the parent container/widget
   * @param element The element from the DOM tree describing the
   *                container (use it to access container specified
   *                attributes or child elements)
   * @param id The id to be used for this container
   */
  virtual TQWidget *createContainer( TQWidget *parent, int index,
          const TQDomElement &element, int &id );

  /**
   * Removes the given (and previously via createContainer )
   * created container.
   *
   */
  virtual void removeContainer( TQWidget *container, TQWidget *parent,
				TQDomElement &element, int id );

  virtual TQStringList customTags() const;

  virtual int createCustomElement( TQWidget *parent, int index, const TQDomElement &element );

  virtual void removeCustomElement( TQWidget *parent, int id );

  virtual void finalizeGUI( KXMLGUIClient *client );

protected:
  virtual void virtual_hook( int id, void* data );
private:
  KXMLGUIBuilderPrivate *d;
};

#endif
