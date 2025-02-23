/* This file is part of the KDE libraries
   Copyright (C) 2001 Christoph Cullmann <cullmann@kde.org>

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

#ifndef __tdetexteditor_document_h__
#define __tdetexteditor_document_h__

#include "editor.h"

namespace KTextEditor
{

/**
 * The main class representing a text document.
 * This class provides access to the document's views.
 */
class KTEXTEDITOR_EXPORT Document : public KTextEditor::Editor
{
  friend class PrivateDocument;

  TQ_OBJECT
  

  public:
    Document ( TQObject *parent = 0, const char *name = 0 );
    virtual ~Document ();

    /**
     * Returns the global number of this document in your app.
     */
    unsigned int documentNumber () const;

    /**
     * Returns this document's DCOP suffix for identifiying its DCOP interface.
     */
    TQCString documentDCOPSuffix () const;

    /**
    * Create a view that will display the document data. You can create as many
    * views as you like. When the user modifies data in one view then all other
    * views will be updated as well.
    */
    virtual class View *createView ( TQWidget *parent, const char *name = 0 ) = 0;

    /*
    * Returns a list of all views of this document.
    */
    virtual TQPtrList<class View> views () const = 0;

    /**
     * Returns the list position of this document in your app, if applicable.
     */
    long documentListPosition () const;

    /**
     * Sets the list position of this document in your app, if applicable.
     */
    void setDocumentListPosition (long pos);

  private:
    class PrivateDocument *d;
    static unsigned int globalDocumentNumber;
    unsigned int myDocumentNumber;
    long myDocumentListPosition;
};

KTEXTEDITOR_EXPORT Document *createDocument ( const char* libname, TQObject *parent = 0, const char *name = 0 );

}

#endif
