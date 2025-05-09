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

#ifndef __tdetexteditor_editinterface_h__
#define __tdetexteditor_editinterface_h__

#include <tqstring.h>

#include <tdelibs_export.h>

namespace KTextEditor
{

/**
* This is the main interface for accessing and modifying
* text of the Document class.
*/
class KTEXTEDITOR_EXPORT EditInterface
{
  friend class PrivateEditInterface;

  public:
    EditInterface();
    virtual ~EditInterface();

    uint editInterfaceNumber () const;

  protected:  
    void setEditInterfaceDCOPSuffix (const TQCString &suffix);  
    
  public:
  /**
  * slots !!!
  */
    /**
    * @return the complete document as a single TQString
    */
    virtual TQString text () const = 0;

    /**
    * @return a TQString
    */
    virtual TQString text ( uint startLine, uint startCol, uint endLine, uint endCol ) const = 0;

    /**
    * @return All the text from the requested line.
    */
    virtual TQString textLine ( uint line ) const = 0;

    /**
    * @return The current number of lines in the document
    */
    virtual uint numLines () const = 0;

    /**
    * @return the number of characters in the document
    */
    virtual uint length () const = 0;

    /**
    * @return the number of characters in the line (-1 if no line "line")
    */
    virtual int lineLength ( uint line ) const = 0;

    /**
    * Set the given text into the view.
    * Warning: This will overwrite any data currently held in this view.
    */
    virtual bool setText ( const TQString &text ) = 0;

    /**
    *  clears the document
    * Warning: This will overwrite any data currently held in this view.
    */
    virtual bool clear () = 0;

    /**
    *  Inserts text at line "line", column "col"
    *  returns true if success
    *  Use insertText(numLines(), ...) to append text at end of document
    */
    virtual bool insertText ( uint line, uint col, const TQString &text ) = 0;

    /**
    *  remove text at line "line", column "col"
    *  returns true if success
    */
    virtual bool removeText ( uint startLine, uint startCol, uint endLine, uint endCol ) = 0;

    /**
    * Insert line(s) at the given line number. 
    * Use insertLine(numLines(), text) to append line at end of document
    */
    virtual bool insertLine ( uint line, const TQString &text ) = 0;

    /**
    * Remove line(s) at the given line number.
    */
    virtual bool removeLine ( uint line ) = 0;

  /**
  * signals !!!
  */
  public:
    virtual void textChanged () = 0;

    virtual void charactersInteractivelyInserted(int ,int ,const TQString&)=0; //line, col, characters if you don't support this, don't create a signal, just overload it.

  /**
  * only for the interface itself - REAL PRIVATE
  */
  private:
    class PrivateEditInterface *d;
    static uint globalEditInterfaceNumber;
    uint myEditInterfaceNumber;
};

KTEXTEDITOR_EXPORT EditInterface *editInterface (class Document *doc);

}

#endif
