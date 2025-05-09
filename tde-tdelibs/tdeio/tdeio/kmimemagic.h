/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

/*
 * KMimeMagic is inspired by the code of the
 * Apache Web Server.
 *
 * Rewritten for KDE by Fritz Elfert
 * fritz@kde.org
 * Adaptations by Torben Weis <weis@kde.org>
 * Fixes and documentation by David Faure <faure@kde.org>
 */

#ifndef KMIMEMAGIC_H
#define KMIMEMAGIC_H

#include <tqstring.h>
#include <tdelibs_export.h>

class KMimeMagic; // see below (read this one first)

/**
 * @deprecated Use KMimeType::findByContent() instead
 * May be removed in KDE 4.0.
 * Returned by KMimeMagic @p find...Type methods.
 *
 * It contains the mimetype and the encoding of
 * the file or buffer read.
 */
class TDEIO_EXPORT KMimeMagicResult
{
public:
  KMimeMagicResult() { m_iAccuracy = 100; }
  ~KMimeMagicResult() { }

  /**
   * Retrieve the mimetype (e.g. "text/html") of the file or buffer parsed.
   */
  TQString mimeType() const { return m_strMimeType; }
  /**
   * Retrieve the accuracy of the matching.
   */
  int accuracy() const { return m_iAccuracy; }
  /**
   * Returns whether the result is valid (i.e. mimetype not empty).
   */
  bool isValid() const { return !m_strMimeType.isEmpty(); }

  /////////////////
  // Internal functions only
  /////////////////
  void setMimeType( const TQString& _mime ) { m_strMimeType = _mime; }
  void setAccuracy( int _accuracy ) { m_iAccuracy = _accuracy; }
  void setInvalid() { m_strMimeType = TQString::null; }

protected:
  TQString m_strMimeType;
  int m_iAccuracy;
};

/**
 * @deprecated Use KMimeType::findByContent() instead
 * May be removed in KDE 4.0.
 * Determine auto-magically the type of file,
 * not only by using its extension, but also by reading its contents.
 *
 *
 * Unless specified otherwise, KMimeMagic uses
 * $TDEDIR/share/mimelnk/magic for this purpose.
 *
 * To make KMimeMagic restore the 'atime' of a file after it opened it,
 * add its directory in kmimemagicrc like:
 * [Settings]
 * atimeDirs=/tmp,/var/tmp,/home/dfaure/tmp
 * This isn't done by default because it changes the 'ctime'.
 * See kmimemagic.cpp for a full discussion on this issue.
 *
 * The basic usage of KMimeMagic is :
 * @li Get a pointer to it, using KMimeMagic::self().
 * @li Use it for any file or buffer you want, using one of the three
 * @p find...Type() methods.
 *
 * The result is contained in the class KMimeMagicResult.
 */
class TDEIO_EXPORT KMimeMagic
{
public:
  /**
   * Create a parser and initialize it with the KDE-global data:
   * the "magic" config file as well as the snippets from share/config/magic.
   * @since 3.1
   */
  KMimeMagic();

  /**
   * Create a parser and initialize it with the given config file.
   */
  KMimeMagic( const TQString & configFile );

  /**
   * Destroy the parser.
   */
  ~KMimeMagic();

  /**
   * Merge an existing parse table with the data from the
   * given file.
   *
   * @return @p true on success.
   */
  bool mergeConfig( const TQString & configFile );

  /**
   * Enable/Disable follow-links.
   *
   * (Default is disabled.)
   */
  void setFollowLinks( bool _enable );

  /**
   * Try to find a MimeType for the given file.
   *
   * If no special
   * MimeType is found, the default MimeType is returned.
   * This function looks at the content of the file.
   *
   * @return A pointer to the result object. Do @em not delete the
   *         result object. After another call to KMimeMagic
   *         the returned result object changes its value
   *         since it is reused by KMimeMagic.
   */
  KMimeMagicResult* findFileType( const TQString & _filename );

  /**
   * Same functionality as above, except data is not
   * read from a file.
   *
   * Instead a buffer can be supplied which
   * is examined.
   *
   * @return A pointer to the result object. Do @em not delete the
   *         result object. After another call to KMimeMagic
   *         the returned result object changes its value
   *         since it is reused by KMimeMagic.
   */
  KMimeMagicResult* findBufferType( const TQByteArray &p );

  /**
   * Same functionality as findBufferType() but with
   * additional capability of distinguishing between
   * C-headers and C-Source.
   *
   * For this purpose this function looks
   * at the extension of the filename. This means that 'filename'
   * can be a filename on some FTP server, too.
   *
   * @return A pointer to the result object. Do @em not delete the
   *         result object. After another call to KMimeMagic
   *         the returned result object changes its value
   *         since it is reused by KMimeMagic.
   */
  KMimeMagicResult * findBufferFileType( const TQByteArray &, const TQString & filename );

  /**
   * Returns a pointer to the unique KMimeMagic instance in this process.
   */
  static KMimeMagic* self();

protected:
  /**
   * The result type.
   */
  KMimeMagicResult * magicResult;

  static void initStatic();
  static KMimeMagic* s_pSelf;

private:
  void init( const TQString& configFile );

  bool bunused;
  TQString sunused;

  int parse_line(char *line, int *rule, int lineno);
  int parse(char *, int);
  int buff_apprentice(char*buff);
  int apprentice(const TQString &configFile);

  struct config_rec *conf; // this is also our "d pointer"
  int iunused;
};

#endif

