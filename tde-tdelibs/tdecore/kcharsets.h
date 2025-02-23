/* This file is part of the KDE libraries
    Copyright (C) 1999 Lars Knoll (knoll@kde.org)

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
#ifndef KCHARSETS_H
#define KCHARSETS_H

#include <tqstring.h>
#include <tqfont.h>
#include <tqstringlist.h>
#include <tqptrlist.h>
#include "tdelibs_export.h"

class TDEGlobal;
class KCharsetsPrivate;

class TQTextCodec;

/**
 * Charset font and encoder/decoder handling.
 *
 * This is needed, because Qt's font matching algorithm gives the font
 * family a higher priority than the charset.  For many applications
 * this is not acceptable, since it can totally obscure the output,
 * in languages which use non iso-8859-1 charsets.
 *
 * @author Lars Knoll <knoll@kde.org>
 */
class TDECORE_EXPORT KCharsets
{
    friend class TDEGlobal;

protected:
    /** Protected constructor. If you need the kcharsets object, use
 TDEGlobal::charsets() instead.
    */
    KCharsets();

public:

    /**
     * Destructor.
     */
    virtual ~KCharsets();

    /**
     * Provided for compatibility.
     * @param name the name of the codec
     * @return the TQTextCodec. If the desired codec could not be found,
     *         it returns a default (Latin-1) codec
     */
    TQTextCodec *codecForName(const TQString &name) const;

    /**
     * Tries to find a TQTextCodec to convert the given encoding from and to
     * Unicode. If no codec could be found the latin1 codec will be returned an
     * @p ok will be set to false.
     * @return the TQTextCodec. If the desired codec could not be found,
     *         it returns a default (Latin-1) codec
     */
    TQTextCodec *codecForName(const TQString &n, bool &ok) const;

    /**
     * Converts an entity to a character. The string must contain only the
     * entity without the trailing ';'.
     * @param str the entity
     * @return TQChar::null if the entity could not be decoded.
     */
    static TQChar fromEntity(const TQString &str);
    /**
     * Overloaded member function. Tries to find an entity in the
     * TQString str.
     * @param str the string containing entified
     * @param len is a return value, that gives the length of the decoded
     * entity.
     * @return a decoded entity if one could be found, TQChar::null
     * otherwise
     */
    static TQChar fromEntity(const TQString &str, int &len);

    /**
     * Converts a TQChar to an entity. The returned string does already
     * contain the leading '&' and the trailing ';'.
     * @param ch the char to convert
     * @return the entity
     */
    static TQString toEntity(const TQChar &ch);

    /**
     * Scans the given string for entities (like &amp;amp;) and resolves them
     * using fromEntity.
     * @param text the string containing the entities
     * @return the clean string
     * @since 3.1
     */
    static TQString resolveEntities( const TQString &text );

    /**
     * Lists all available encodings as names.
     * @return the list of all encodings
     */
    TQStringList availableEncodingNames();

    /**
     * Lists the available encoding names together with a more descriptive language.
     * @return the list of descriptive encoding names
     */
    TQStringList descriptiveEncodingNames();

    /**
     * Returns the language the encoding is used for.
     * @param encoding the encoding for the language
     * @return the language of the encoding
     */
    TQString languageForEncoding( const TQString &encoding );

    /**
     * Returns the encoding for a string obtained with descriptiveEncodingNames().
     * @param descriptiveName the descriptive name for the encoding
     * @return the name of the encoding
     */
    TQString encodingForName( const TQString &descriptiveName );

    /**
     * Returns the descriptive encoding name for an encoding name.
     * @param encodingName the name of the encoding
     * @return the descriptive name for the encoding
     */
    TQString descriptiveNameForEncoding( const TQString &encodingName );

private:
    KCharsetsPrivate *d;
};

#endif
