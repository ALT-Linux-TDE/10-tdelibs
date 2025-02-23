/* This file is part of the KDE project
   Copyright (C) 2001 Christoph Cullmann (cullmann@kde.org)

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

#include <config.h>

#include "document.h"
#include "view.h"
#include "plugin.h"
#include "editor.h"

#include <tdeaction.h>
#include <tdeparts/factory.h>
#include <tdeparts/componentfactory.h>

#include "document.moc"
#include "view.moc"
#include "plugin.moc"
#include "editor.moc"

using namespace KTextEditor;

namespace KTextEditor
{
  class PrivateDocument
  {
  public:
    PrivateDocument ()
    {
    }

    ~PrivateDocument()
    {
    }
  };
  
  class PrivateView
  {
  public:
    PrivateView ()
    {
    }

    ~PrivateView()
    {
    }
  };
  
  class PrivatePlugin
  {
  public:
    PrivatePlugin ()
    {
    }

    ~PrivatePlugin ()
    {
    }       
    
    class Document *m_doc;
  };
  
  class PrivatePluginViewInterface
  {
  public:
    PrivatePluginViewInterface ()
    {
    }

    ~PrivatePluginViewInterface ()
    {
    }
  };

  class PrivateEditor
  {
  public:
    PrivateEditor ()
    {
    }

    ~PrivateEditor()
    {
    }
  };
}

unsigned int Document::globalDocumentNumber = 0;
unsigned int View::globalViewNumber = 0;
unsigned int Plugin::globalPluginNumber = 0;
unsigned int PluginViewInterface::globalPluginViewInterfaceNumber = 0;
unsigned int Editor::globalEditorNumber = 0;

Document::Document( TQObject *parent, const char *name ) : KTextEditor::Editor (parent, name )
{
  globalDocumentNumber++;
  myDocumentNumber = globalDocumentNumber;
  myDocumentListPosition = -1;	// Don't care
}

Document::~Document()
{
}

unsigned int Document::documentNumber () const
{
  return myDocumentNumber;
}

long Document::documentListPosition () const
{
  return myDocumentListPosition;
}

void Document::setDocumentListPosition (long pos)
{
  myDocumentListPosition = pos;
}

TQCString Document::documentDCOPSuffix () const
{
  TQCString num;
  num.setNum (documentNumber());
  
  return num;
}

View::View( Document *, TQWidget *parent, const char *name ) : TQWidget( parent, name )
{
  globalViewNumber++;
  myViewNumber = globalViewNumber;
}

View::~View()
{
}

unsigned int View::viewNumber () const
{
  return myViewNumber;
}

TQCString View::viewDCOPSuffix () const
{
  TQCString num1, num2;
  num1.setNum (viewNumber());
  num2.setNum (document()->documentNumber());
  
  return num2 + "-" + num1;
}

Plugin::Plugin( Document *document, const char *name ) : TQObject (document, name )
{
  globalPluginNumber++;
  myPluginNumber = globalPluginNumber; 
  d = new PrivatePlugin ();
  d->m_doc = document;
}

Plugin::~Plugin()
{
  delete d;
}

unsigned int Plugin::pluginNumber () const
{
  return myPluginNumber;
}     

Document *Plugin::document () const
{
  return d->m_doc;
}

PluginViewInterface::PluginViewInterface()
{
  globalPluginViewInterfaceNumber++;
  myPluginViewInterfaceNumber = globalPluginViewInterfaceNumber;
}

PluginViewInterface::~PluginViewInterface()
{
}

unsigned int PluginViewInterface::pluginViewInterfaceNumber () const
{
  return myPluginViewInterfaceNumber;
}

Editor::Editor( TQObject *parent, const char *name ) : KParts::ReadWritePart( parent, name )
{
  globalEditorNumber++;
  myEditorNumber = globalEditorNumber;
}

Editor::~Editor()
{
}

unsigned int Editor::editorNumber () const
{
  return myEditorNumber;
}                         

Editor *KTextEditor::createEditor ( const char* libname, TQWidget *parentWidget, const char *widgetName, TQObject *parent, const char *name )
{
  return KParts::ComponentFactory::createPartInstanceFromLibrary<Editor>( libname, parentWidget, widgetName, parent, name );
}

Document *KTextEditor::createDocument ( const char* libname, TQObject *parent, const char *name )
{
  return KParts::ComponentFactory::createPartInstanceFromLibrary<Document>( libname, 0, 0, parent, name );
}     

Plugin *KTextEditor::createPlugin ( const char* libname, Document *document, const char *name )
{
  return KParts::ComponentFactory::createInstanceFromLibrary<Plugin>( libname, document, name );
}

PluginViewInterface *KTextEditor::pluginViewInterface (Plugin *plugin)
{       
  if (!plugin)
    return 0;

  return dynamic_cast<KTextEditor::PluginViewInterface*>(plugin);
}

