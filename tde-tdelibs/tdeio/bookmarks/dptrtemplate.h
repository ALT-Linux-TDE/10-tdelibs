/* This file is part of the KDE project
   Copyright (C) 2003 Alexander Kellett <lypanov@kde.org>

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

#ifndef __dptrtemplate_h__
#define __dptrtemplate_h__

#include <tqptrdict.h>

template<class Instance, class PrivateData>
class dPtrTemplate {
public:
    static PrivateData* d( const Instance* instance )
    {
        if ( !d_ptr ) {
            cleanup_d_ptr();
            d_ptr = new TQPtrDict<PrivateData>;
            tqAddPostRoutine( cleanup_d_ptr );
        }
        PrivateData* ret = d_ptr->find( (void*) instance );
        if ( ! ret ) {
            ret = new PrivateData;
            d_ptr->replace( (void*) instance, ret );
        }
        return ret;
    }
    static void delete_d( const Instance* instance )
    {
        if ( d_ptr )
            d_ptr->remove( (void*) instance );
    }
private:
    static void cleanup_d_ptr()
    {
        delete d_ptr;
    }
    static TQPtrDict<PrivateData>* d_ptr;
};

#endif
