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

#ifndef __tdetexteditor_view_h__
#define __tdetexteditor_view_h__

#include <tqwidget.h>
#include <kxmlguiclient.h>

namespace KTextEditor
{

/**
 * The View class represents a single view of a Document .
 */

class KTEXTEDITOR_EXPORT View : public TQWidget, public KXMLGUIClient
{
  friend class PrivateView;

  TQ_OBJECT

  public:
    /**
    * Create a new view to the given document. The document must be non-null.
    */
    View ( class Document *, TQWidget *parent, const char *name = 0 );
    virtual ~View ();

    /**
     * Returns the number of this view
     */
    unsigned int viewNumber () const;

    /**
     * Returns the DCOP suffix to allow identification of this view's DCOP interface.
     */
    TQCString viewDCOPSuffix () const;

    /**
    * Acess the parent Document.
    */
    virtual class Document *document () const = 0;
    
  private:
    class PrivateView *d;
    static unsigned int globalViewNumber;
    unsigned int myViewNumber;
};

}

#endif
