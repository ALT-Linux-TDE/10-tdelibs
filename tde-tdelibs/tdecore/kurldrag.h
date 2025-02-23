/* This file is part of the KDE project
   Copyright (C) 2000 David Faure <faure@kde.org>

   This program is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License.

   This program is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this program; see the file COPYING.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef __KURLDRAG_H
#define __KURLDRAG_H

#include <tqstringlist.h>
#include <tqdragobject.h>
#include <kurl.h>
#include "tdelibs_export.h"
class TQMimeSource;

class KURLDragPrivate;
/**
 * This class is to be used instead of TQUriDrag when using KURL.
 * The reason is: TQUriDrag (and the XDND/W3C standards) expect URLs to
 * be encoded in UTF-8 (unicode), but KURL uses the current locale
 * by default.
 * The other reasons for using this class are:
 * @li it exports text/plain (for dropping/pasting into lineedits, mails etc.)
 * @li it has support for metadata, shipped as part of the dragobject
 * This is important, for instance to set a correct HTTP referrer (some websites
 * require it for downloading e.g. an image).
 *
 * To create a drag object, use the KURLDrag constructor.
 * To handle drops, use TQUriDrag::canDecode() and KURLDrag::decode()
 */
class TDECORE_EXPORT KURLDrag : public TQUriDrag
{
public:
  /**
   * Constructs an object to drag the list of URLs in @p urls.
   * The @p dragSource and @p name arguments are passed on to TQUriDrag,
   * and the list of urls is converted to UTF-8 before being passed
   * to TQUriDrag.
   * @param urls the list of URLs
   * @param dragSource the parent of the TQObject. Should be set when doing drag-n-drop,
   * but should be 0 when copying to the clipboard
   * @param name the name of the TQObject
   */
  KURLDrag( const KURL::List &urls, TQWidget* dragSource = 0, const char * name = 0 );
  /**
   * Constructs an object to drag the list of URLs in @p urls.
   * This version also includes metadata.
   * @param urls the list of URLs
   * @param metaData a map containing meta data
   * @param dragSource the parent of the TQObject. Should be set when doing drag-n-drop,
   * but should be 0 when copying to the clipboard
   * @param name the name of the TQObject
   * @see metaData()
   */
  KURLDrag( const KURL::List &urls, const TQMap<TQString, TQString>& metaData,
            TQWidget* dragSource = 0, const char * name = 0 );

  virtual ~KURLDrag();

  /**
   * By default, KURLDrag also exports the URLs as plain text, for e.g. dropping onto a text editor.
   * But in some cases this might not be wanted, e.g. if using the KURLDrag in a KMultipleDrag
   * and another component of the multiple-drag provides better plain text data.
   * In such a case, setExportAsText( false ) should be called.
   * @since 3.4
   */
  void setExportAsText( bool exp );

  /**
   * @deprecated Is equivalent with "new KURLDrag(urls, dragSource, name)".
   */
  static KURLDrag * newDrag( const KURL::List &urls, TQWidget* dragSource = 0, const char * name = 0 ) TDE_DEPRECATED;

  /**
   * @deprecated Is equivalent with "new KURLDrag(urls, metaData, dragSource, name)".
   */
  static KURLDrag * newDrag( const KURL::List &urls, const TQMap<TQString, TQString>& metaData,
                             TQWidget* dragSource = 0, const char * name = 0 ) TDE_DEPRECATED;

  /**
   * Meta-data to associate with those URLs.
   * This is an alternative way of setting the metadata:
   * either use the constructor to pass it all at once, or use
   * drag->metaData()["key"] = data;
   * @see TDEIO::TransferJob
   */
  TQMap<TQString, TQString> &metaData() { return m_metaData; }

  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs. Decoding will fail if at least one decoded value
   * is not a valid KURL.
   * @param e the mime source
   * @param urls the list of urls will be written here
   * @return true if successful, false otherwise
   */
  static bool decode( const TQMimeSource *e, KURL::List &urls );

  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs and a set of metadata. Decoding will fail if
   * at least one decoded value is not a valid KURL.
   * You should be using this one, if possible.
   * @param e the mime source
   * @param urls the list of urls will be written here
   * @param metaData the metadata map will be written here
   * @return true if successful, false otherwise
   */
  static bool decode( const TQMimeSource *e, KURL::List &urls, TQMap<TQString,TQString>& metaData );

  /**
   * Converts a URL to a string representation suitable for dragging.
   * @since 3.2
   */
  static TQString urlToString(const KURL &url);

  /**
   * Converts a string used for dragging to a URL.
   * @since 3.2
   */
  static KURL stringToUrl(const TQCString &s);

#ifdef TQ_WS_QWS
  /**
   * Convenience method that decodes the contents of @p e
   * into a list of KURLs for Qt versions without a MIME clipboard.
   * Decoding will fail if at least one value in the list is not a valid KURL.
   */
  static bool decode( TQStringList const &e, KURL::List &uris );
#endif

  /// @reimp
  virtual const char * format( int i ) const;
  /// @reimp
  virtual TQByteArray encodedData( const char* mime ) const;

protected:
  /**
   * @deprecated Use a KURLDrag constructor with a KURL::List
   */
  KURLDrag( const TQStrList & urls, const TQMap<TQString,TQString>& metaData,
            TQWidget * dragSource, const char* name ) TDE_DEPRECATED;

private:
  void init(const KURL::List &urls);

  TQStrList m_urls;
  TQMap<TQString,TQString> m_metaData;
  KURLDragPrivate* d;
};

#endif
