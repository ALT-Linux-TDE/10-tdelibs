/*
    This file is part of the KDE libraries

    Copyright (C) 1999 Lars Knoll (knoll@mpi-hd.mpg.de)

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
#ifndef TDEHTMLDECODER_H
#define TDEHTMLDECODER_H

#include <tqstring.h>
class TQTextCodec;
class TQTextDecoder;

namespace tdehtml {

class JapaneseCode;

/**
 * @internal
 */
class Decoder
{
public:
    enum EncodingType {
        DefaultEncoding,
        AutoDetectedEncoding,
        EncodingFromXMLHeader,
        EncodingFromMetaTag,
        EncodingFromHTTPHeader,
        UserChosenEncoding
    };

    Decoder();
    ~Decoder();

    void setEncoding(const char *encoding, EncodingType type);
    const char *encoding() const;

    TQString decode(const char *data, int len);

    bool visuallyOrdered() const { return visualRTL; }

    const TQTextCodec *codec() const { return m_codec; }

    TQString flush() const;


    enum AutoDetectLanguage {
        SemiautomaticDetection,
        Arabic,
        Baltic,
        CentralEuropean,
        Chinese,
        Greek,
        Hebrew,
        Japanese,
        Korean,
        Russian,
        Thai,
        Turkish,
        Ukrainian,
        Unicode,
        WesternEuropean
    };

    void setAutoDetectLanguage( AutoDetectLanguage _language ) { m_autoDetectLanguage = _language; }
    AutoDetectLanguage autoDetectLanguage() { return m_autoDetectLanguage; }



private:
    TQCString automaticDetectionForArabic( const unsigned char* str, int size );
    TQCString automaticDetectionForBaltic( const unsigned char* str, int size );
    TQCString automaticDetectionForCentralEuropean( const unsigned char* str, int size );
    TQCString automaticDetectionForCyrillic( const unsigned char* str, int size, AutoDetectLanguage _language );
    TQCString automaticDetectionForGreek( const unsigned char* str, int size );
    TQCString automaticDetectionForHebrew( const unsigned char* str, int size );
    TQCString automaticDetectionForJapanese( const unsigned char* str, int size );
    TQCString automaticDetectionForTurkish( const unsigned char* str, int size );
    TQCString automaticDetectionForWesternEuropean( const unsigned char* str, int size );

    // codec used for decoding. default is Latin1.
    TQTextCodec *m_codec;
    TQTextDecoder *m_decoder; // only used for utf16
    TQCString enc;
    EncodingType m_type;

    TQCString buffer;

    bool body;
    bool beginning;
    bool visualRTL;

    AutoDetectLanguage m_autoDetectLanguage;

    JapaneseCode *kc;
};

}
#endif
