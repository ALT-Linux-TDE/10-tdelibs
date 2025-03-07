/* This file is part of the KDE libraries
   Copyright (C) 2000 David Faure <faure@kde.org>

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

#include <config.h>

#include <sys/types.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <assert.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdio.h>

#include "tdeio/global.h"
#include "tdeio/job.h"

#include <kdebug.h>
#include <tdelocale.h>
#include <tdeglobal.h>
#include <tdeprotocolmanager.h>
#include <kde_file.h>

#ifdef HAVE_VOLMGT
#include <volmgt.h>
#endif

TDEIO_EXPORT TQString TDEIO::convertSizeWithBytes( TDEIO::filesize_t size )
{
    if ( size >= 1024 )
        return convertSize( size ) + " (" + i18n( "%1 B" ).arg( TDEGlobal::locale()->formatNumber(size, 0) ) + ")";
    else
	return convertSize( size );
}

TDEIO_EXPORT TQString TDEIO::convertSize( TDEIO::filesize_t size )
{
    double fsize = size;
    TQString s;
    // Giga-byte
    if ( size >= 1073741824 )
    {
        fsize /= 1073741824.0;
        if ( fsize > 1024 ) // Tera-byte
            s = i18n( "%1 TB" ).arg( TDEGlobal::locale()->formatNumber(fsize / 1024.0, 1));
        else
            s = i18n( "%1 GB" ).arg( TDEGlobal::locale()->formatNumber(fsize, 1));
    }
    // Mega-byte
    else if ( size >= 1048576 )
    {
        fsize /= 1048576.0;
        s = i18n( "%1 MB" ).arg( TDEGlobal::locale()->formatNumber(fsize, 1));
    }
    // Kilo-byte
    else if ( size >= 1024 )
    {
        fsize /= 1024.0;
        s = i18n( "%1 KB" ).arg( TDEGlobal::locale()->formatNumber(fsize, 1));
    }
    // Just byte
    else if ( size > 0 )
    {
        s = i18n( "%1 B" ).arg( TDEGlobal::locale()->formatNumber(fsize, 0));
    }
    // Nothing
    else
    {
        s = i18n( "0 B" );
    }
    return s;
}

TDEIO_EXPORT TQString TDEIO::convertSizeFromKB( TDEIO::filesize_t kbSize )
{
    return convertSize(kbSize * 1024);
}

TDEIO_EXPORT TQString TDEIO::number( TDEIO::filesize_t size )
{
    char charbuf[256];
    sprintf(charbuf, "%lld", size);
    return TQString::fromLatin1(charbuf);
}

TDEIO_EXPORT unsigned int TDEIO::calculateRemainingSeconds( TDEIO::filesize_t totalSize,
                                                        TDEIO::filesize_t processedSize, TDEIO::filesize_t speed )
{
  if ( (speed != 0) && (totalSize != 0) )
    return ( totalSize - processedSize ) / speed;
  else
    return 0;
}

TDEIO_EXPORT TQString TDEIO::convertSeconds( unsigned int seconds )
{
  unsigned int days  = seconds / 86400;
  unsigned int hours = (seconds - (days * 86400)) / 3600;
  unsigned int mins  = (seconds - (days * 86400) - (hours * 3600)) / 60;
  seconds            = (seconds - (days * 86400) - (hours * 3600) - (mins * 60));

  const TQTime time(hours, mins, seconds);
  const TQString timeStr( TDEGlobal::locale()->formatTime(time, true /*with seconds*/, true /*duration*/) );
  if ( days > 0 )
    return i18n("1 day %1", "%n days %1", days).arg(timeStr);
  else
    return timeStr;
}

TDEIO_EXPORT TQTime TDEIO::calculateRemaining( TDEIO::filesize_t totalSize, TDEIO::filesize_t processedSize, TDEIO::filesize_t speed )
{
  TQTime remainingTime;

  if ( speed != 0 ) {
    TDEIO::filesize_t secs;
    if ( totalSize == 0 ) {
      secs = 0;
    } else {
      secs = ( totalSize - processedSize ) / speed;
    }
    if (secs >= (24*60*60)) // Limit to 23:59:59
       secs = (24*60*60)-1;
    int hr = secs / ( 60 * 60 );
    int mn = ( secs - hr * 60 * 60 ) / 60;
    int sc = ( secs - hr * 60 * 60 - mn * 60 );

    remainingTime.setHMS( hr, mn, sc );
  }

  return remainingTime;
}

TDEIO_EXPORT TQString TDEIO::itemsSummaryString(uint items, uint files, uint dirs, TDEIO::filesize_t size, bool showSize)
{
    TQString text = items == 0 ? i18n( "No Items" ) : i18n( "One Item", "%n Items", items );
    text += " - ";
    text += files == 0 ? i18n( "No Files" ) : i18n( "One File", "%n Files", files );
    if ( showSize && files > 0 )
    {
        text += " ";
        text += i18n("(%1 Total)").arg(TDEIO::convertSize( size ) );
    }
    text += " - ";
    text += dirs == 0 ? i18n( "No Folders" ) : i18n("One Folder", "%n Folders", dirs);
    return text;
}

TDEIO_EXPORT TQString TDEIO::encodeFileName( const TQString & _str )
{
  TQString str( _str );
  bool unicode_supported = (TQString::fromLocal8Bit(TQString(TQChar((uint)0x2215)).local8Bit())[0].unicode() != 0x3f);

  int i = 0;
  while ( ( i = str.find( "%", i ) ) != -1 ) {
    str.replace( i, 1, "%%");
    i += 2;
  }
  while ( ( i = str.find( "/" ) ) != -1 ) {
      if (unicode_supported) {
          // Use U+2215 (DIVISION SLASH) to represent the forward slash
          // While U+2044 (FRACTION SLASH) is a tempting replacement, it can indicate to
          // rendering engines that a combined fraction character should be displayed
          str.replace( i, 1, TQChar((uint)0x2215));
      }
      else {
          // Unicode does not appear to be supported on this system!
          // Fall back to older encoding method...
          str.replace( i, 1, "%2f");
      }
  }
  return str;
}

TDEIO_EXPORT TQString TDEIO::decodeFileName( const TQString & _str )
{
  TQString str;
  bool unicode_supported = (TQString::fromLocal8Bit(TQString(TQChar((uint)0x2215)).local8Bit())[0].unicode() != 0x3f);

  unsigned int i = 0;
  for ( ; i < _str.length() ; ++i ) {
    if ( _str[i]=='%' ) {
      if ( _str[i+1]=='%' ) // %% -> %
      {
        str.append('%');
        ++i;
      }
      else if ((!unicode_supported) && ( _str[i+1]=='2' && (i+2<_str.length()) && _str[i+2].lower()=='f' )) // %2f -> /
      {
        str.append('/');
        i += 2;
      }
      else
      {
        str.append('%');
      }
    }
    else if ( _str[i] == TQChar((uint)0x2215) ) {
        str.append('/');
    }
    else {
      str.append(_str[i]);
    }
  }

  return str;
}

TDEIO_EXPORT TQString TDEIO::Job::errorString() const
{
  return TDEIO::buildErrorString(m_error, m_errorText);
}

TDEIO_EXPORT TQString TDEIO::buildErrorString(int errorCode, const TQString &errorText)
{
  TQString result;

  switch( errorCode )
    {
    case  TDEIO::ERR_CANNOT_OPEN_FOR_READING:
      result = i18n( "Could not read %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_CANNOT_OPEN_FOR_WRITING:
      result = i18n( "Could not write to %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_CANNOT_LAUNCH_PROCESS:
      result = i18n( "Could not start process %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_INTERNAL:
      result = i18n( "Internal Error\nPlease send a full bug report at http://bugs.trinitydesktop.org\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_MALFORMED_URL:
      result = i18n( "Malformed URL %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_UNSUPPORTED_PROTOCOL:
      result = i18n( "The protocol %1 is not supported." ).arg( errorText );
      break;
    case  TDEIO::ERR_NO_SOURCE_PROTOCOL:
      result = i18n( "The protocol %1 is only a filter protocol.").arg( errorText );
      break;
    case  TDEIO::ERR_UNSUPPORTED_ACTION:
      result = errorText;
//       result = i18n( "Unsupported action %1" ).arg( errorText );
      break;
    case  TDEIO::ERR_IS_DIRECTORY:
      result = i18n( "%1 is a folder, but a file was expected." ).arg( errorText );
      break;
    case  TDEIO::ERR_IS_FILE:
      result = i18n( "%1 is a file, but a folder was expected." ).arg( errorText );
      break;
    case  TDEIO::ERR_DOES_NOT_EXIST:
      result = i18n( "The file or folder %1 does not exist." ).arg( errorText );
      break;
    case  TDEIO::ERR_FILE_ALREADY_EXIST:
      result = i18n( "A file named %1 already exists." ).arg( errorText );
      break;
    case  TDEIO::ERR_DIR_ALREADY_EXIST:
      result = i18n( "A folder named %1 already exists." ).arg( errorText );
      break;
    case  TDEIO::ERR_UNKNOWN_HOST:
      result = errorText.isEmpty() ? i18n( "No hostname specified." ) : i18n( "Unknown host %1" ).arg( errorText );
      break;
    case  TDEIO::ERR_ACCESS_DENIED:
      result = i18n( "Access denied to %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_WRITE_ACCESS_DENIED:
      result = i18n( "Access denied.\nCould not write to %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_CANNOT_ENTER_DIRECTORY:
      result = i18n( "Could not enter folder %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      result = i18n( "The protocol %1 does not implement a folder service." ).arg( errorText );
      break;
    case  TDEIO::ERR_CYCLIC_LINK:
      result = i18n( "Found a cyclic link in %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_USER_CANCELED:
      // Do nothing in this case. The user doesn't need to be told what he just did.
      break;
    case  TDEIO::ERR_CYCLIC_COPY:
      result = i18n( "Found a cyclic link while copying %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_CREATE_SOCKET:
      result = i18n( "Could not create socket for accessing %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_CONNECT:
      result = i18n( "Could not connect to host %1." ).arg( errorText.isEmpty() ? TQString::fromLatin1("localhost") : errorText );
      break;
    case  TDEIO::ERR_CONNECTION_BROKEN:
      result = i18n( "Connection to host %1 is broken." ).arg( errorText );
      break;
    case  TDEIO::ERR_NOT_FILTER_PROTOCOL:
      result = i18n( "The protocol %1 is not a filter protocol." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_MOUNT:
      result = i18n( "Could not mount device.\nThe reported error was:\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_UNMOUNT:
      result = i18n( "Could not unmount device.\nThe reported error was:\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_READ:
      result = i18n( "Could not read file %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_WRITE:
      result = i18n( "Could not write to file %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_BIND:
      result = i18n( "Could not bind %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_LISTEN:
      result = i18n( "Could not listen %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_ACCEPT:
      result = i18n( "Could not accept %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_LOGIN:
      result = errorText;
      break;
    case  TDEIO::ERR_COULD_NOT_STAT:
      result = i18n( "Could not access %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_CLOSEDIR:
      result = i18n( "Could not terminate listing %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_MKDIR:
      result = i18n( "Could not make folder %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_RMDIR:
      result = i18n( "Could not remove folder %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_CANNOT_RESUME:
      result = i18n( "Could not resume file %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_CANNOT_RENAME:
      result = i18n( "Could not rename file %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_CANNOT_CHMOD:
      result = i18n( "Could not change permissions for %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_CANNOT_DELETE:
      result = i18n( "Could not delete file %1." ).arg( errorText );
      break;
    case  TDEIO::ERR_SLAVE_DIED:
      result = i18n( "The process for the %1 protocol died unexpectedly." ).arg( errorText );
      break;
    case  TDEIO::ERR_OUT_OF_MEMORY:
      result = i18n( "Error. Out of memory.\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_UNKNOWN_PROXY_HOST:
      result = i18n( "Unknown proxy host\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_COULD_NOT_AUTHENTICATE:
      result = i18n( "Authorization failed, %1 authentication not supported" ).arg( errorText );
      break;
    case  TDEIO::ERR_ABORTED:
      result = i18n( "User canceled action\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_INTERNAL_SERVER:
      result = i18n( "Internal error in server\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_SERVER_TIMEOUT:
      result = i18n( "Timeout on server\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_UNKNOWN:
      result = i18n( "Unknown error\n%1" ).arg( errorText );
      break;
    case  TDEIO::ERR_UNKNOWN_INTERRUPT:
      result = i18n( "Unknown interrupt\n%1" ).arg( errorText );
      break;
/*
    case  TDEIO::ERR_CHECKSUM_MISMATCH:
      if (errorText)
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg(errorText);
      else
        result = i18n( "Warning: MD5 Checksum for %1 does not match checksum returned from server" ).arg("document");
      break;
*/
    case TDEIO::ERR_CANNOT_DELETE_ORIGINAL:
      result = i18n( "Could not delete original file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case TDEIO::ERR_CANNOT_DELETE_PARTIAL:
      result = i18n( "Could not delete partial file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case TDEIO::ERR_CANNOT_RENAME_ORIGINAL:
      result = i18n( "Could not rename original file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case TDEIO::ERR_CANNOT_RENAME_PARTIAL:
      result = i18n( "Could not rename partial file %1.\nPlease check permissions." ).arg( errorText );
      break;
    case TDEIO::ERR_CANNOT_SYMLINK:
      result = i18n( "Could not create symlink %1.\nPlease check permissions." ).arg( errorText );
      break;
    case TDEIO::ERR_NO_CONTENT:
      result = errorText;
      break;
    case TDEIO::ERR_DISK_FULL:
      result = i18n( "Could not write file %1.\nDisk full." ).arg( errorText );
      break;
    case TDEIO::ERR_IDENTICAL_FILES:
      result = i18n( "The source and destination are the same file.\n%1" ).arg( errorText );
      break;
    case TDEIO::ERR_SLAVE_DEFINED:
      result = errorText;
      break;
    case TDEIO::ERR_UPGRADE_REQUIRED:
      result = i18n( "%1 is required by the server, but is not available." ).arg(errorText);
      break;
    case TDEIO::ERR_POST_DENIED:
      result = i18n( "Access to restricted port in POST denied.");
      break;
    case TDEIO::ERR_OFFLINE_MODE:
      result = i18n( "Could not access %1.\nOffline mode active.").arg( errorText ); 
      break;
    default:
      result = i18n( "Unknown error code %1\n%2\nPlease send a full bug report at http://bugs.trinitydesktop.org." ).arg( errorCode ).arg( errorText );
      break;
    }

  return result;
}

TDEIO_EXPORT TQString TDEIO::unsupportedActionErrorString(const TQString &protocol, int cmd) {
  switch (cmd) {
    case CMD_CONNECT:
      return i18n("Opening connections is not supported with the protocol %1." ).arg(protocol);
    case CMD_DISCONNECT:
      return i18n("Closing connections is not supported with the protocol %1." ).arg(protocol);
    case CMD_STAT:
      return i18n("Accessing files is not supported with the protocol %1.").arg(protocol);
    case CMD_PUT:
      return i18n("Writing to %1 is not supported.").arg(protocol);
    case CMD_SPECIAL:
      return i18n("There are no special actions available for protocol %1.").arg(protocol);
    case CMD_LISTDIR:
      return i18n("Listing folders is not supported for protocol %1.").arg(protocol);
    case CMD_GET:
      return i18n("Retrieving data from %1 is not supported.").arg(protocol);
    case CMD_MIMETYPE:
      return i18n("Retrieving mime type information from %1 is not supported.").arg(protocol);
    case CMD_RENAME:
      return i18n("Renaming or moving files within %1 is not supported.").arg(protocol);
    case CMD_SYMLINK:
      return i18n("Creating symlinks is not supported with protocol %1.").arg(protocol);
    case CMD_COPY:
      return i18n("Copying files within %1 is not supported.").arg(protocol);
    case CMD_DEL:
      return i18n("Deleting files from %1 is not supported.").arg(protocol);
    case CMD_MKDIR:
      return i18n("Creating folders is not supported with protocol %1.").arg(protocol);
    case CMD_CHMOD:
      return i18n("Changing the attributes of files is not supported with protocol %1.").arg(protocol);
    case CMD_SUBURL:
      return i18n("Using sub-URLs with %1 is not supported.").arg(protocol);
    case CMD_MULTI_GET:
      return i18n("Multiple get is not supported with protocol %1.").arg(protocol);
    default:
      return i18n("Protocol %1 does not support action %2.").arg(protocol).arg(cmd);
  }/*end switch*/
}

TDEIO_EXPORT TQStringList TDEIO::Job::detailedErrorStrings( const KURL *reqUrl /*= 0L*/,
                                            int method /*= -1*/ ) const
{
  TQString errorName, techName, description, ret2;
  TQStringList causes, solutions, ret;

  TQByteArray raw = rawErrorDetail( m_error, m_errorText, reqUrl, method );
  TQDataStream stream(raw, IO_ReadOnly);

  stream >> errorName >> techName >> description >> causes >> solutions;

  TQString url, protocol, datetime;
  if ( reqUrl ) {
    url = reqUrl->htmlURL();
    protocol = reqUrl->protocol();
  } else {
    url = i18n( "(unknown)" );
  }

  datetime = TDEGlobal::locale()->formatDateTime( TQDateTime::currentDateTime(),
                                                false );

  ret << errorName;
  ret << TQString::fromLatin1( "<qt><p><b>" ) + errorName +
         TQString::fromLatin1( "</b></p><p>" ) + description +
         TQString::fromLatin1( "</p>" );
  ret2 = TQString::fromLatin1( "<qt><p>" );
  if ( !techName.isEmpty() )
    ret2 += i18n( "<b>Technical reason</b>: " ) + techName + TQString::fromLatin1( "</p>" );
  ret2 += i18n( "</p><p><b>Details of the request</b>:" );
  ret2 += i18n( "</p><ul><li>URL: %1</li>" ).arg( url );
  if ( !protocol.isEmpty() ) {
    ret2 += i18n( "<li>Protocol: %1</li>" ).arg( protocol );
  }
  ret2 += i18n( "<li>Date and time: %1</li>" ).arg( datetime );
  ret2 += i18n( "<li>Additional information: %1</li></ul>" ).arg( m_errorText );
  if ( !causes.isEmpty() ) {
    ret2 += i18n( "<p><b>Possible causes</b>:</p><ul><li>" );
    ret2 += causes.join( "</li><li>" );
    ret2 += TQString::fromLatin1( "</li></ul>" );
  }
  if ( !solutions.isEmpty() ) {
    ret2 += i18n( "<p><b>Possible solutions</b>:</p><ul><li>" );
    ret2 += solutions.join( "</li><li>" );
    ret2 += TQString::fromLatin1( "</li></ul>" );
  }
  ret << ret2;
  return ret;
}

TDEIO_EXPORT TQByteArray TDEIO::rawErrorDetail(int errorCode, const TQString &errorText,
                               const KURL *reqUrl /*= 0L*/, int /*method = -1*/ )
{
  TQString url, host, protocol, datetime, domain, path, dir, filename;
  bool isSlaveNetwork = false;
  if ( reqUrl ) {
    url = reqUrl->prettyURL();
    host = reqUrl->host();
    protocol = reqUrl->protocol();

    if ( host.left(4) == "www." )
      domain = host.mid(4);
    else
      domain = host;

    path = reqUrl->path(1);
    filename = reqUrl->fileName();
    dir =  path + filename;

    // detect if protocol is a network protocol...
    // add your hacks here...
    if ( protocol == "http" ||
         protocol == "https" ||
         protocol == "ftp" ||
         protocol == "sftp" ||
         protocol == "webdav" ||
         protocol == "webdavs" ||
         protocol == "finger" ||
         protocol == "fish" ||
         protocol == "gopher" ||
         protocol == "imap" ||
         protocol == "imaps" ||
         protocol == "lan" ||
         protocol == "ldap" ||
         protocol == "mailto" ||
         protocol == "news" ||
         protocol == "nntp" ||
         protocol == "pop3" ||
         protocol == "pop3s" ||
         protocol == "smtp" ||
         protocol == "smtps" ||
         protocol == "telnet"
        ) {
      isSlaveNetwork = false;
    }
  } else {
    // assume that the errorText has the location we are interested in
    url = host = domain = path = filename = dir = errorText;
    protocol = i18n( "(unknown)" );
  }

  datetime = TDEGlobal::locale()->formatDateTime( TQDateTime::currentDateTime(),
                                                false );

  TQString errorName, techName, description;
  TQStringList causes, solutions;

  // c == cause, s == solution
  TQString sSysadmin = i18n( "Contact your appropriate computer support system, "
    "whether the system administrator, or technical support group for further "
    "assistance." );
  TQString sServeradmin = i18n( "Contact the administrator of the server "
    "for further assistance." );
  // FIXME active link to permissions dialog
  TQString sAccess = i18n( "Check your access permissions on this resource." );
  TQString cAccess = i18n( "Your access permissions may be inadequate to "
    "perform the requested operation on this resource." );
  TQString cLocked = i18n( "The file may be in use (and thus locked) by "
    "another user or application." );
  TQString sQuerylock = i18n( "Check to make sure that no other "
    "application or user is using the file or has locked the file." );
  TQString cHardware = i18n( "Although unlikely, a hardware error may have "
    "occurred." );
  TQString cBug = i18n( "You may have encountered a bug in the program." );
  TQString cBuglikely = i18n( "This is most likely to be caused by a bug in the "
    "program. Please consider submitting a full bug report as detailed below." );
  TQString sUpdate = i18n( "Update your software to the latest version. "
    "Your distribution should provide tools to update your software." );
  TQString sBugreport = i18n( "When all else fails, please consider helping the "
    "TDE team or the third party maintainer of this software by submitting a "
    "high quality bug report. If the software is provided by a third party, "
    "please contact them directly. Otherwise, first look to see if "
    "the same bug has been submitted by someone else by searching at the "
    "<a href=\"http://bugs.trinitydesktop.org/\">TDE bug reporting website</a>. If not, take "
    "note of the details given above, and include them in your bug report, along "
    "with as many other details as you think might help." );
  TQString cNetwork = i18n( "There may have been a problem with your network "
    "connection." );
  // FIXME netconf kcontrol link
  TQString cNetconf = i18n( "There may have been a problem with your network "
    "configuration. If you have been accessing the Internet with no problems "
    "recently, this is unlikely." );
  TQString cNetpath = i18n( "There may have been a problem at some point along "
    "the network path between the server and this computer." );
  TQString sTryagain = i18n( "Try again, either now or at a later time." );
  TQString cProtocol = i18n( "A protocol error or incompatibility may have occurred." );
  TQString sExists = i18n( "Ensure that the resource exists, and try again." );
  TQString cExists = i18n( "The specified resource may not exist." );
  TQString cTypo = i18n( "You may have incorrectly typed the location." );
  TQString sTypo = i18n( "Double-check that you have entered the correct location "
    "and try again." );
  TQString sNetwork = i18n( "Check your network connection status." );

  switch( errorCode ) {
    case  TDEIO::ERR_CANNOT_OPEN_FOR_READING:
      errorName = i18n( "Cannot Open Resource For Reading" );
      description = i18n( "This means that the contents of the requested file "
        "or folder <strong>%1</strong> could not be retrieved, as read "
        "access could not be obtained." ).arg( dir );
      causes << i18n( "You may not have permissions to read the file or open "
        "the folder.") << cLocked << cHardware;
      solutions << sAccess << sQuerylock << sSysadmin;
      break;

    case  TDEIO::ERR_CANNOT_OPEN_FOR_WRITING:
      errorName = i18n( "Cannot Open Resource For Writing" );
      description = i18n( "This means that the file, <strong>%1</strong>, could "
        "not be written to as requested, because access with permission to "
        "write could not be obtained." ).arg( filename );
      causes << cAccess << cLocked << cHardware;
      solutions << sAccess << sQuerylock << sSysadmin;
      break;

    case  TDEIO::ERR_CANNOT_LAUNCH_PROCESS:
      errorName = i18n( "Cannot Initiate the %1 Protocol" ).arg( protocol );
      techName = i18n( "Unable to Launch Process" );
      description = i18n( "The program on your computer which provides access "
        "to the <strong>%1</strong> protocol could not be started. This is "
        "usually due to technical reasons." ).arg( protocol );
      causes << i18n( "The program which provides compatibility with this "
        "protocol may not have been updated with your last update of TDE. "
        "This can cause the program to be incompatible with the current version "
        "and thus not start." ) << cBug;
      solutions << sUpdate << sSysadmin;
      break;

    case  TDEIO::ERR_INTERNAL:
      errorName = i18n( "Internal Error" );
      description = i18n( "The program on your computer which provides access "
        "to the <strong>%1</strong> protocol has reported an internal error." )
        .arg( protocol );
      causes << cBuglikely;
      solutions << sUpdate << sBugreport;
      break;

    case  TDEIO::ERR_MALFORMED_URL:
      errorName = i18n( "Improperly Formatted URL" );
      description = i18n( "The <strong>U</strong>niform <strong>R</strong>esource "
        "<strong>L</strong>ocator (URL) that you entered was not properly "
        "formatted. The format of a URL is generally as follows:"
        "<blockquote><strong>protocol://user:password@www.example.org:port/folder/"
        "filename.extension?query=value</strong></blockquote>" );
      solutions << sTypo;
      break;

    case  TDEIO::ERR_UNSUPPORTED_PROTOCOL:
      errorName = i18n( "Unsupported Protocol %1" ).arg( protocol );
      description = i18n( "The protocol <strong>%1</strong> is not supported "
        "by the TDE programs currently installed on this computer." )
        .arg( protocol );
      causes << i18n( "The requested protocol may not be supported." )
        << i18n( "The versions of the %1 protocol supported by this computer and "
        "the server may be incompatible." ).arg( protocol );
      solutions << i18n( "You may perform a search on the Trinity website for a TDE "
        "program (called a tdeioslave, ioslave or tdeio) which supports this protocol. "
        "Places to search include <a href=\"https://mirror.git.trinitydesktop.org/cgit/\">"
        "https://mirror.git.trinitydesktop.org/cgit/</a> or the repos for your distribution." )
        << sUpdate << sSysadmin;
      break;

    case  TDEIO::ERR_NO_SOURCE_PROTOCOL:
      errorName = i18n( "URL Does Not Refer to a Resource." );
      techName = i18n( "Protocol is a Filter Protocol" );
      description = i18n( "The <strong>U</strong>niform <strong>R</strong>esource "
        "<strong>L</strong>ocator (URL) that you entered did not refer to a "
        "specific resource." );
      causes << i18n( "TDE is able to communicate through a protocol within a "
        "protocol; the protocol specified is only for use in such situations, "
        "however this is not one of these situations. This is a rare event, and "
        "is likely to indicate a programming error." );
      solutions << sTypo;
      break;

    case  TDEIO::ERR_UNSUPPORTED_ACTION:
      errorName = i18n( "Unsupported Action: %1" ).arg( errorText );
      description = i18n( "The requested action is not supported by the TDE "
        "program which is implementing the <strong>%1</strong> protocol." )
        .arg( protocol );
      causes << i18n( "This error is very much dependent on the TDE program. The "
        "additional information should give you more information than is available "
        "to the TDE input/output architecture." );
      solutions << i18n( "Attempt to find another way to accomplish the same "
        "outcome." );
      break;

    case  TDEIO::ERR_IS_DIRECTORY:
      errorName = i18n( "File Expected" );
      description = i18n( "The request expected a file, however the "
        "folder <strong>%1</strong> was found instead." ).arg( dir );
      causes << i18n( "This may be an error on the server side." ) << cBug;
      solutions << sUpdate << sSysadmin;
      break;

    case  TDEIO::ERR_IS_FILE:
      errorName = i18n( "Folder Expected" );
      description = i18n( "The request expected a folder, however "
        "the file <strong>%1</strong> was found instead." ).arg( filename );
      causes << cBug;
      solutions << sUpdate << sSysadmin;
      break;

    case  TDEIO::ERR_DOES_NOT_EXIST:
      errorName = i18n( "File or Folder Does Not Exist" );
      description = i18n( "The specified file or folder <strong>%1</strong> "
        "does not exist." ).arg( dir );
      causes << cBug;
      solutions << sUpdate << sSysadmin;
      break;

    case  TDEIO::ERR_FILE_ALREADY_EXIST:
      errorName = i18n( "File Already Exists" );
      description = i18n( "The requested file could not be created because a "
        "file with the same name already exists." );
      solutions << i18n ( "Try moving the current file out of the way first, "
        "and then try again." )
        << i18n ( "Delete the current file and try again." )
        << i18n( "Choose an alternate filename for the new file." );
      break;

    case  TDEIO::ERR_DIR_ALREADY_EXIST:
      errorName = i18n( "Folder Already Exists" );
      description = i18n( "The requested folder could not be created because "
        "a folder with the same name already exists." );
      solutions << i18n( "Try moving the current folder out of the way first, "
        "and then try again." )
        << i18n( "Delete the current folder and try again." )
        << i18n( "Choose an alternate name for the new folder." );
      break;

    case  TDEIO::ERR_UNKNOWN_HOST:
      errorName = i18n( "Unknown Host" );
      description = i18n( "An unknown host error indicates that the server with "
        "the requested name, <strong>%1</strong>, could not be "
        "located on the Internet." ).arg( host );
      causes << i18n( "The name that you typed, %1, may not exist: it may be "
        "incorrectly typed." ).arg( host )
        << cNetwork << cNetconf;
      solutions << sNetwork << sSysadmin;
      break;

    case  TDEIO::ERR_ACCESS_DENIED:
      errorName = i18n( "Access Denied" );
      description = i18n( "Access was denied to the specified resource, "
        "<strong>%1</strong>." ).arg( url );
      causes << i18n( "You may have supplied incorrect authentication details or "
        "none at all." )
        << i18n( "Your account may not have permission to access the "
        "specified resource." );
      solutions << i18n( "Retry the request and ensure your authentication details "
        "are entered correctly." ) << sSysadmin;
      if ( !isSlaveNetwork ) solutions << sServeradmin;
      break;

    case  TDEIO::ERR_WRITE_ACCESS_DENIED:
      errorName = i18n( "Write Access Denied" );
      description = i18n( "This means that an attempt to write to the file "
        "<strong>%1</strong> was rejected." ).arg( filename );
      causes << cAccess << cLocked << cHardware;
      solutions << sAccess << sQuerylock << sSysadmin;
      break;

    case  TDEIO::ERR_CANNOT_ENTER_DIRECTORY:
      errorName = i18n( "Unable to Enter Folder" );
      description = i18n( "This means that an attempt to enter (in other words, "
        "to open) the requested folder <strong>%1</strong> was rejected." )
        .arg( dir );
      causes << cAccess << cLocked;
      solutions << sAccess << sQuerylock << sSysadmin;
      break;

    case  TDEIO::ERR_PROTOCOL_IS_NOT_A_FILESYSTEM:
      errorName = i18n( "Folder Listing Unavailable" );
      techName = i18n( "Protocol %1 is not a Filesystem" ).arg( protocol );
      description = i18n( "This means that a request was made which requires "
        "determining the contents of the folder, and the TDE program supporting "
        "this protocol is unable to do so." );
      causes << cBug;
      solutions << sUpdate << sBugreport;
      break;

    case  TDEIO::ERR_CYCLIC_LINK:
      errorName = i18n( "Cyclic Link Detected" );
      description = i18n( "UNIX environments are commonly able to link a file or "
        "folder to a separate name and/or location. TDE detected a link or "
        "series of links that results in an infinite loop - i.e. the file was "
        "(perhaps in a roundabout way) linked to itself." );
      solutions << i18n( "Delete one part of the loop in order that it does not "
        "cause an infinite loop, and try again." ) << sSysadmin;
      break;

    case  TDEIO::ERR_USER_CANCELED:
      // Do nothing in this case. The user doesn't need to be told what he just did.
      // rodda: However, if we have been called, an application is about to display
      // this information anyway. If we don't return sensible information, the
      // user sees a blank dialog (I have seen this myself)
      errorName = i18n( "Request Aborted By User" );
      description = i18n( "The request was not completed because it was "
        "aborted." );
      solutions << i18n( "Retry the request." );
      break;

    case  TDEIO::ERR_CYCLIC_COPY:
      errorName = i18n( "Cyclic Link Detected During Copy" );
      description = i18n( "UNIX environments are commonly able to link a file or "
        "folder to a separate name and/or location. During the requested copy "
        "operation, TDE detected a link or series of links that results in an "
        "infinite loop - i.e. the file was (perhaps in a roundabout way) linked "
        "to itself." );
      solutions << i18n( "Delete one part of the loop in order that it does not "
        "cause an infinite loop, and try again." ) << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_CREATE_SOCKET:
      errorName = i18n( "Could Not Create Network Connection" );
      techName = i18n( "Could Not Create Socket" );
      description = i18n( "This is a fairly technical error in which a required "
        "device for network communications (a socket) could not be created." );
      causes << i18n( "The network connection may be incorrectly configured, or "
        "the network interface may not be enabled." );
      solutions << sNetwork << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_CONNECT:
      errorName = i18n( "Connection to Server Refused" );
      description = i18n( "The server <strong>%1</strong> refused to allow this "
        "computer to make a connection." ).arg( host );
      causes << i18n( "The server, while currently connected to the Internet, "
        "may not be configured to allow requests." )
        << i18n( "The server, while currently connected to the Internet, "
        "may not be running the requested service (%1)." ).arg( protocol )
        << i18n( "A network firewall (a device which restricts Internet "
        "requests), either protecting your network or the network of the server, "
        "may have intervened, preventing this request." );
      solutions << sTryagain << sServeradmin << sSysadmin;
      break;

    case  TDEIO::ERR_CONNECTION_BROKEN:
      errorName = i18n( "Connection to Server Closed Unexpectedly" );
      description = i18n( "Although a connection was established to "
        "<strong>%1</strong>, the connection was closed at an unexpected point "
        "in the communication." ).arg( host );
      causes << cNetwork << cNetpath << i18n( "A protocol error may have occurred, "
        "causing the server to close the connection as a response to the error." );
      solutions << sTryagain << sServeradmin << sSysadmin;
      break;

    case  TDEIO::ERR_NOT_FILTER_PROTOCOL:
      errorName = i18n( "URL Resource Invalid" );
      techName = i18n( "Protocol %1 is not a Filter Protocol" ).arg( protocol );
      description = i18n( "The <strong>U</strong>niform <strong>R</strong>esource "
        "<strong>L</strong>ocator (URL) that you entered did not refer to "
        "a valid mechanism of accessing the specific resource, "
        "<strong>%1%2</strong>." )
        .arg( !host.isNull() ? host + '/' : TQString::null ).arg( dir );
      causes << i18n( "TDE is able to communicate through a protocol within a "
        "protocol. This request specified a protocol be used as such, however "
        "this protocol is not capable of such an action. This is a rare event, "
        "and is likely to indicate a programming error." );
      solutions << sTypo << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_MOUNT:
      errorName = i18n( "Unable to Initialize Input/Output Device" );
      techName = i18n( "Could Not Mount Device" );
      description = i18n( "The requested device could not be initialized "
        "(\"mounted\"). The reported error was: <strong>%1</strong>" )
        .arg( errorText );
      causes << i18n( "The device may not be ready, for example there may be "
        "no media in a removable media device (i.e. no CD-ROM in a CD drive), "
        "or in the case of a peripheral/portable device, the device may not "
        "be correctly connected." )
        << i18n( "You may not have permissions to initialize (\"mount\") the "
        "device. On UNIX systems, often system administrator privileges are "
        "required to initialize a device." )
        << cHardware;
      solutions << i18n( "Check that the device is ready; removable drives "
        "must contain media, and portable devices must be connected and powered "
        "on.; and try again." ) << sAccess << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_UNMOUNT:
      errorName = i18n( "Unable to Uninitialize Input/Output Device" );
      techName = i18n( "Could Not Unmount Device" );
      description = i18n( "The requested device could not be uninitialized "
        "(\"unmounted\"). The reported error was: <strong>%1</strong>" )
        .arg( errorText );
      causes << i18n( "The device may be busy, that is, still in use by "
        "another application or user. Even such things as having an open "
        "browser window on a location on this device may cause the device to "
        "remain in use." )
        << i18n( "You may not have permissions to uninitialize (\"unmount\") "
        "the device. On UNIX systems, system administrator privileges are "
        "often required to uninitialize a device." )
        << cHardware;
      solutions << i18n( "Check that no applications are accessing the device, "
        "and try again." ) << sAccess << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_READ:
      errorName = i18n( "Cannot Read From Resource" );
      description = i18n( "This means that although the resource, "
        "<strong>%1</strong>, was able to be opened, an error occurred while "
        "reading the contents of the resource." ).arg( url );
      causes << i18n( "You may not have permissions to read from the resource." );
      if ( !isSlaveNetwork ) causes << cNetwork;
      causes << cHardware;
      solutions << sAccess;
      if ( !isSlaveNetwork ) solutions << sNetwork;
      solutions << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_WRITE:
      errorName = i18n( "Cannot Write to Resource" );
      description = i18n( "This means that although the resource, <strong>%1</strong>"
        ", was able to be opened, an error occurred while writing to the resource." )
        .arg( url );
      causes << i18n( "You may not have permissions to write to the resource." );
      if ( !isSlaveNetwork ) causes << cNetwork;
      causes << cHardware;
      solutions << sAccess;
      if ( !isSlaveNetwork ) solutions << sNetwork;
      solutions << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_BIND:
      errorName = i18n( "Could Not Listen for Network Connections" );
      techName = i18n( "Could Not Bind" );
      description = i18n( "This is a fairly technical error in which a required "
        "device for network communications (a socket) could not be established "
        "to listen for incoming network connections." );
      causes << i18n( "The network connection may be incorrectly configured, or "
        "the network interface may not be enabled." );
      solutions << sNetwork << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_LISTEN:
      errorName = i18n( "Could Not Listen for Network Connections" );
      techName = i18n( "Could Not Listen" );
      description = i18n( "This is a fairly technical error in which a required "
        "device for network communications (a socket) could not be established "
        "to listen for incoming network connections." );
      causes << i18n( "The network connection may be incorrectly configured, or "
        "the network interface may not be enabled." );
      solutions << sNetwork << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_ACCEPT:
      errorName = i18n( "Could Not Accept Network Connection" );
      description = i18n( "This is a fairly technical error in which an error "
        "occurred while attempting to accept an incoming network connection." );
      causes << i18n( "The network connection may be incorrectly configured, or "
        "the network interface may not be enabled." )
        << i18n( "You may not have permissions to accept the connection." );
      solutions << sNetwork << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_LOGIN:
      errorName = i18n( "Could Not Login: %1" ).arg( errorText );
      description = i18n( "An attempt to login to perform the requested "
        "operation was unsuccessful." );
      causes << i18n( "You may have supplied incorrect authentication details or "
        "none at all." )
        << i18n( "Your account may not have permission to access the "
        "specified resource." ) << cProtocol;
      solutions << i18n( "Retry the request and ensure your authentication details "
        "are entered correctly." ) << sServeradmin << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_STAT:
      errorName = i18n( "Could Not Determine Resource Status" );
      techName = i18n( "Could Not Stat Resource" );
      description = i18n( "An attempt to determine information about the status "
        "of the resource <strong>%1</strong>, such as the resource name, type, "
        "size, etc., was unsuccessful." ).arg( url );
      causes << i18n( "The specified resource may not have existed or may "
        "not be accessible." ) << cProtocol << cHardware;
      solutions << i18n( "Retry the request and ensure your authentication details "
        "are entered correctly." ) << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_CLOSEDIR:
      //result = i18n( "Could not terminate listing %1" ).arg( errorText );
      errorName = i18n( "Could Not Cancel Listing" );
      techName = i18n( "FIXME: Document this" );
      break;

    case  TDEIO::ERR_COULD_NOT_MKDIR:
      errorName = i18n( "Could Not Create Folder" );
      description = i18n( "An attempt to create the requested folder failed." );
      causes << cAccess << i18n( "The location where the folder was to be created "
        "may not exist." );
      if ( !isSlaveNetwork ) causes << cProtocol;
      solutions << i18n( "Retry the request." ) << sAccess;
      break;

    case  TDEIO::ERR_COULD_NOT_RMDIR:
      errorName = i18n( "Could Not Remove Folder" );
      description = i18n( "An attempt to remove the specified folder, "
        "<strong>%1</strong>, failed." ).arg( dir );
      causes << i18n( "The specified folder may not exist." )
        << i18n( "The specified folder may not be empty." )
        << cAccess;
      if ( !isSlaveNetwork ) causes << cProtocol;
      solutions << i18n( "Ensure that the folder exists and is empty, and try "
        "again." ) << sAccess;
      break;

    case  TDEIO::ERR_CANNOT_RESUME:
      errorName = i18n( "Could Not Resume File Transfer" );
      description = i18n( "The specified request asked that the transfer of "
        "file <strong>%1</strong> be resumed at a certain point of the "
        "transfer. This was not possible." ).arg( filename );
      causes << i18n( "The protocol, or the server, may not support file "
        "resuming." );
      solutions << i18n( "Retry the request without attempting to resume "
        "transfer." );
      break;

    case  TDEIO::ERR_CANNOT_RENAME:
      errorName = i18n( "Could Not Rename Resource" );
      description = i18n( "An attempt to rename the specified resource "
        "<strong>%1</strong> failed." ).arg( url );
      causes << cAccess << cExists;
      if ( !isSlaveNetwork ) causes << cProtocol;
      solutions << sAccess << sExists;
      break;

    case  TDEIO::ERR_CANNOT_CHMOD:
      errorName = i18n( "Could Not Alter Permissions of Resource" );
      description = i18n( "An attempt to alter the permissions on the specified "
        "resource <strong>%1</strong> failed." ).arg( url );
      causes << cAccess << cExists;
      solutions << sAccess << sExists;
      break;

    case  TDEIO::ERR_CANNOT_DELETE:
      errorName = i18n( "Could Not Delete Resource" );
      description = i18n( "An attempt to delete the specified resource "
        "<strong>%1</strong> failed." ).arg( url );
      causes << cAccess << cExists;
      solutions << sAccess << sExists;
      break;

    case  TDEIO::ERR_SLAVE_DIED:
      errorName = i18n( "Unexpected Program Termination" );
      description = i18n( "The program on your computer which provides access "
        "to the <strong>%1</strong> protocol has unexpectedly terminated." )
        .arg( url );
      causes << cBuglikely;
      solutions << sUpdate << sBugreport;
      break;

    case  TDEIO::ERR_OUT_OF_MEMORY:
      errorName = i18n( "Out of Memory" );
      description = i18n( "The program on your computer which provides access "
        "to the <strong>%1</strong> protocol could not obtain the memory "
        "required to continue." ).arg( protocol );
      causes << cBuglikely;
      solutions << sUpdate << sBugreport;
      break;

    case  TDEIO::ERR_UNKNOWN_PROXY_HOST:
      errorName = i18n( "Unknown Proxy Host" );
      description = i18n( "While retrieving information about the specified "
        "proxy host, <strong>%1</strong>, an Unknown Host error was encountered. "
        "An unknown host error indicates that the requested name could not be "
        "located on the Internet." ).arg( errorText );
      causes << i18n( "There may have been a problem with your network "
        "configuration, specifically your proxy's hostname. If you have been "
        "accessing the Internet with no problems recently, this is unlikely." )
        << cNetwork;
      solutions << i18n( "Double-check your proxy settings and try again." )
        << sSysadmin;
      break;

    case  TDEIO::ERR_COULD_NOT_AUTHENTICATE:
      errorName = i18n( "Authentication Failed: Method %1 Not Supported" )
         .arg( errorText );
      description = i18n( "Although you may have supplied the correct "
        "authentication details, the authentication failed because the "
        "method that the server is using is not supported by the TDE "
        "program implementing the protocol %1." ).arg( protocol );
      solutions << i18n( "Please file a bug at <a href=\"http://bugs.trinitydesktop.org/\">"
        "http://bugs.trinitydesktop.org/</a> to inform the TDE team of the unsupported "
        "authentication method." ) << sSysadmin;
      break;

    case  TDEIO::ERR_ABORTED:
      errorName = i18n( "Request Aborted" );
      description = i18n( "The request was not completed because it was "
        "aborted." );
      solutions << i18n( "Retry the request." );
      break;

    case  TDEIO::ERR_INTERNAL_SERVER:
      errorName = i18n( "Internal Error in Server" );
      description = i18n( "The program on the server which provides access "
        "to the <strong>%1</strong> protocol has reported an internal error: "
        "%0." ).arg( protocol );
      causes << i18n( "This is most likely to be caused by a bug in the "
        "server program. Please consider submitting a full bug report as "
        "detailed below." );
      solutions << i18n( "Contact the administrator of the server "
        "to advise them of the problem." )
        << i18n( "If you know who the authors of the server software are, "
        "submit the bug report directly to them." );
      break;

    case  TDEIO::ERR_SERVER_TIMEOUT:
    {
      int connTimeout   = KProtocolManager::connectTimeout();
      int respTimeout   = KProtocolManager::responseTimeout();
      int prConnTimeout = KProtocolManager::proxyConnectTimeout();
      errorName = i18n( "Timeout Error" );
      description = i18n("Although contact was made with the server, a "
          "response was not received within the amount of time allocated for "
          "the request as follows:")
        .append(TQString::fromLatin1("<ul><li>"))
        .append(i18n("Timeout for establishing a connection: %n second",
                     "Timeout for establishing a connection: %n seconds",
                     connTimeout))
        .append(TQString::fromLatin1("</li><li>"))
        .append(i18n("Timeout for receiving a response: %n second",
                     "Timeout for receiving a response: %n seconds",
                     respTimeout))
        .append(TQString::fromLatin1("</li><li>"))
        .append(i18n("Timeout for accessing proxy servers: %n second",
                     "Timeout for accessing proxy servers: %n seconds",
                     prConnTimeout))
        .append(TQString::fromLatin1("</li></ul>"))
        .append(i18n("Please note that you can alter these timeout settings in the TDE "
                     "Control Center, by selecting Network -> Preferences." ));
      causes << cNetpath << i18n( "The server was too busy responding to other "
        "requests to respond." );
      solutions << sTryagain << sServeradmin;
      break;
    }

    case  TDEIO::ERR_UNKNOWN:
      errorName = i18n( "Unknown Error" );
      description = i18n( "The program on your computer which provides access "
        "to the <strong>%1</strong> protocol has reported an unknown error: "
        "%2." ).arg( protocol ).arg( errorText );
      causes << cBug;
      solutions << sUpdate << sBugreport;
      break;

    case  TDEIO::ERR_UNKNOWN_INTERRUPT:
      errorName = i18n( "Unknown Interruption" );
      description = i18n( "The program on your computer which provides access "
        "to the <strong>%1</strong> protocol has reported an interruption of "
        "an unknown type: %2." ).arg( protocol ).arg( errorText );
      causes << cBug;
      solutions << sUpdate << sBugreport;
      break;

    case TDEIO::ERR_CANNOT_DELETE_ORIGINAL:
      errorName = i18n( "Could Not Delete Original File" );
      description = i18n( "The requested operation required the deleting of "
        "the original file, most likely at the end of a file move operation. "
        "The original file <strong>%1</strong> could not be deleted." )
        .arg( errorText );
      causes << cAccess;
      solutions << sAccess;
      break;

    case TDEIO::ERR_CANNOT_DELETE_PARTIAL:
      errorName = i18n( "Could Not Delete Temporary File" );
      description = i18n( "The requested operation required the creation of "
        "a temporary file in which to save the new file while being "
        "downloaded. This temporary file <strong>%1</strong> could not be "
        "deleted." ).arg( errorText );
      causes << cAccess;
      solutions << sAccess;
      break;

    case TDEIO::ERR_CANNOT_RENAME_ORIGINAL:
      errorName = i18n( "Could Not Rename Original File" );
      description = i18n( "The requested operation required the renaming of "
        "the original file <strong>%1</strong>, however it could not be "
        "renamed." ).arg( errorText );
      causes << cAccess;
      solutions << sAccess;
      break;

    case TDEIO::ERR_CANNOT_RENAME_PARTIAL:
      errorName = i18n( "Could Not Rename Temporary File" );
      description = i18n( "The requested operation required the creation of "
        "a temporary file <strong>%1</strong>, however it could not be "
        "created." ).arg( errorText );
      causes << cAccess;
      solutions << sAccess;
      break;

    case TDEIO::ERR_CANNOT_SYMLINK:
      errorName = i18n( "Could Not Create Link" );
      techName = i18n( "Could Not Create Symbolic Link" );
      description = i18n( "The requested symbolic link %1 could not be created." )
        .arg( errorText );
      causes << cAccess;
      solutions << sAccess;
      break;

    case TDEIO::ERR_NO_CONTENT:
      errorName = i18n( "No Content" );
      description = errorText;
      break;

    case TDEIO::ERR_DISK_FULL:
      errorName = i18n( "Disk Full" );
      description = i18n( "The requested file <strong>%1</strong> could not be "
        "written to as there is inadequate disk space." ).arg( errorText );
      solutions << i18n( "Free up enough disk space by 1) deleting unwanted and "
        "temporary files; 2) archiving files to removable media storage such as "
        "CD-Recordable discs; or 3) obtain more storage capacity." )
        << sSysadmin;
      break;

    case TDEIO::ERR_IDENTICAL_FILES:
      errorName = i18n( "Source and Destination Files Identical" );
      description = i18n( "The operation could not be completed because the "
        "source and destination files are the same file." );
      solutions << i18n( "Choose a different filename for the destination file." );
      break;

    // We assume that the slave has all the details
    case TDEIO::ERR_SLAVE_DEFINED:
      errorName = TQString::null;
      description = errorText;
      break;

    default:
      // fall back to the plain error...
      errorName = i18n( "Undocumented Error" );
      description = buildErrorString( errorCode, errorText );
  }

  TQByteArray ret;
  TQDataStream stream(ret, IO_WriteOnly);
  stream << errorName << techName << description << causes << solutions;
  return ret;
}

#ifdef Q_OS_UNIX

#include <limits.h>
#include <stdlib.h>
#include <stdio.h>
#include <tqfile.h>

#include <config.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif
#ifdef HAVE_SYS_STAT_H
#include <sys/stat.h>
#endif
#include <sys/param.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif
#ifdef HAVE_SYS_MNTTAB_H
#include <sys/mnttab.h>
#endif
#ifdef HAVE_MNTENT_H
#include <mntent.h>
#elif defined(HAVE_SYS_MNTENT_H)
#include <sys/mntent.h>
#endif
#ifdef HAVE_SYS_UCRED_H
#include <sys/ucred.h>
#elif defined(HAVE_UCRED_H)
#include <ucred.h>
#endif
#ifdef HAVE_SYS_MOUNT_H
#include <sys/mount.h>
#endif
#ifdef HAVE_FSTAB_H
#include <fstab.h>
#endif
#if defined(_AIX)
#include <sys/mntctl.h>
#include <sys/vmount.h>
#include <sys/vfs.h>

/* AIX does not prototype mntctl anywhere that I can find */
#ifndef mntctl
extern "C" {
int mntctl(int command, int size, void* buffer);
}
#endif
extern "C" struct vfs_ent *getvfsbytype(int vfsType);
extern "C" void endvfsent( );
#endif

/***************************************************************
 *
 * Utility functions
 *
 ***************************************************************/

#ifndef HAVE_GETMNTINFO

#ifdef _PATH_MOUNTED
// On some Linux, MNTTAB points to /etc/fstab !
# undef MNTTAB
# define MNTTAB _PATH_MOUNTED
#else
# ifndef MNTTAB
#  ifdef MTAB_FILE
#   define MNTTAB MTAB_FILE
#  else
#   define MNTTAB "/etc/mnttab"
#  endif
# endif
#endif

#ifndef FSTAB
# ifdef _PATH_FSTAB
#  define FSTAB _PATH_FSTAB
# else
#  define FSTAB "/etc/fstab"
# endif
#endif

#ifdef __CYGWIN__                
#define hasmntopt(var,opt) (0)   
#endif                           
                                 
// There are (at least) four kind of APIs:
// setmntent + getmntent + struct mntent (linux...)
//             getmntent + struct mnttab
// mntctl                + struct vmount (AIX)
// getmntinfo + struct statfs&flags (BSD 4.4 and friends)
// getfsent + char* (BSD 4.3 and friends)

#ifdef HAVE_SETMNTENT
#define SETMNTENT setmntent
#define ENDMNTENT endmntent
#define STRUCT_MNTENT struct mntent *
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) ((var = getmntent(file)) != 0)
#define MOUNTPOINT(var) var->mnt_dir
#define MOUNTTYPE(var) var->mnt_type
#define HASMNTOPT(var, opt) hasmntopt(var, opt)
#define FSNAME(var) var->mnt_fsname
#elif defined(_AIX)
/* we don't need this stuff */
#else
#define SETMNTENT fopen
#define ENDMNTENT fclose
#define STRUCT_MNTENT struct mnttab
#define STRUCT_SETMNTENT FILE *
#define GETMNTENT(file, var) (getmntent(file, &var) == 0)
#define MOUNTPOINT(var) var.mnt_mountp
#define MOUNTTYPE(var) var.mnt_fstype
#define HASMNTOPT(var, opt) hasmntopt(&var, opt)
#define FSNAME(var) var.mnt_special
#endif

#endif /* HAVE_GETMNTINFO */

TQString TDEIO::findDeviceMountPoint( const TQString& filename )
{
    TQString result;

#ifdef HAVE_VOLMGT
	/*
	 *  support for Solaris volume management
	 */
	const char *volpath;
	FILE *mnttab;
	struct mnttab mnt;
	int len;
	TQCString devname;

	if( (volpath = volmgt_root()) == NULL ) {
		kdDebug( 7007 ) << "findDeviceMountPoint: "
			<< "VOLMGT: can't find volmgt root dir" << endl;
		return TQString::null;
	}

	if( (mnttab = fopen( MNTTAB, "r" )) == NULL ) {
		kdDebug( 7007 ) << "findDeviceMountPoint: "
			<< "VOLMGT: can't open mnttab" << endl;
		return TQString::null;
	}

	devname = volpath;
	devname += TQFile::encodeName( filename );
	devname += '/';
	len = devname.length();
//	kdDebug( 7007 ) << "findDeviceMountPoint: "
//		<< "VOLMGT: searching mountpoint for \"" << devname << "\""
//		<< endl;

	/*
	 *  find the mountpoint
	 *  floppies:
	 *	/dev/disketteN    => <volpath>/dev/disketteN
	 *  CDROM, ZIP, and other media:
	 *	/dev/dsk/cXtYdZs2 => <volpath>/dev/dsk/cXtYdZ  (without slice#)
	 */
	rewind( mnttab );
	result = TQString::null;
	while( getmntent( mnttab, &mnt ) == 0 ) {
		/*
		 *  either match the exact device name (floppies),
		 *  or the device name without the slice#
		 */
		if( strncmp( devname.data(), mnt.mnt_special, len ) == 0
			|| (strncmp( devname.data(), mnt.mnt_special, len - 3 ) == 0
				&& mnt.mnt_special[len - 3] == '/' )
			|| (strcmp(TQFile::encodeName(filename).data()
					, mnt.mnt_special)==0)) {
			result = mnt.mnt_mountp;
			break;
		}
	}
	fclose( mnttab );
#else

    char    realpath_buffer[MAXPATHLEN];
    TQCString realname;

    realname = TQFile::encodeName(filename);
    /* If the path contains symlinks, get the real name */
    if (realpath(realname, realpath_buffer) != 0)
      // succes, use result from realpath
      realname = realpath_buffer;

    //kdDebug(7007) << "findDeviceMountPoint realname=" << realname << endl;

#ifdef HAVE_GETMNTINFO

#ifdef GETMNTINFO_USES_STATVFS
    struct statvfs *mounted;
#else
    struct statfs *mounted;
#endif

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i=0;i<num_fs;i++) {

        TQCString device_name = mounted[i].f_mntfromname;

        // If the path contains symlinks, get
        // the real name
        if (realpath(device_name, realpath_buffer) != 0)
            // succes, use result from realpath
            device_name = realpath_buffer;

        if (realname == device_name) {
            result = mounted[i].f_mntonname;
            break;
        }
    }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    int buf_sz = 4096;

    /* mntctl can be used to query mounted file systems.
     * mntctl takes only the command MCTL_QUERY so far.
     * The buffer is filled with an array of vmount structures, but these
     * vmount structures have variable size.
     * mntctl return values:
     * -1 error
     *  0 look in first word of buffer for required bytes, 4096 may be
     *    a good starting size, but if tables grow too large, look here.
     * >0 number of vmount structures
     */
    mntctl_buffer = (struct vmount*)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0)
    {
	buf_sz = *(int*)mntctl_buffer;
	free(mntctl_buffer);
	mntctl_buffer = (struct vmount*)malloc(buf_sz);
	num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0)
    {
        /* iterate through items in the vmount structure: */
        vm = mntctl_buffer;
        for ( ; num > 0; num-- )
        {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char*)malloc(fsname_len + 1);
	    mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            /* get the mount-from information: */
            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char*)malloc(fsname_len + 1);
	    mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

            TQCString device_name = mountedfrom;

            if (realpath(device_name, realpath_buffer) != 0)
                // success, use result from realpath
                device_name = realpath_buffer;

            free(mountedfrom);

            if (realname == device_name) {
                result = mountedto;
                free(mountedto);
                break;
            }

            free(mountedto);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }
    }

    free( mntctl_buffer );

#else

    STRUCT_SETMNTENT mtab;

    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == 0) {
        perror("setmntent");
        return TQString::null;
    }

    /* Loop over all file systems and see if we can find our
     * mount point.
     * Note that this is the mount point with the longest match.
     * XXX: Fails if me->mnt_dir is not a realpath but goes
     * through a symlink, e.g. /foo/bar where /foo is a symlink
     * pointing to /local/foo.
     *
     * How kinky can you get with a filesystem?
     */

    STRUCT_MNTENT me;

    while (GETMNTENT(mtab, me))
    {
      // There may be symbolic links into the /etc/mnttab
      // So we have to find the real device name here as well!
      TQCString device_name = FSNAME(me);
      if (device_name.isEmpty() || (device_name == "none"))
         continue;

      //kdDebug( 7007 ) << "device_name=" << device_name << endl;

      // If the path contains symlinks, get
      // the real name
      if (realpath(device_name, realpath_buffer) != 0)
          // succes, use result from realpath
         device_name = realpath_buffer;

      //kdDebug( 7007 ) << "device_name after realpath =" << device_name << endl;

      if (realname == device_name)
      {
          result = MOUNTPOINT(me);
          break;
      }
    }

    ENDMNTENT(mtab);

#endif /* GET_MNTINFO */
#endif /* HAVE_VOLMGT */

    //kdDebug( 7007 ) << "Returning result " << result << endl;
    return result;
}

// Don't just trust the return value, keep iterating to check for a better match (bigger max)
static bool is_my_mountpoint( const char *mountpoint, const char *realname, int &max )
{
    int length = strlen(mountpoint);

    if (!strncmp(mountpoint, realname, length)
        && length > max) {
        max = length;
        if (length == 1 || realname[length] == '/' || realname[length] == '\0')
            return true;
    }
    return false;
}

typedef enum { Unseen, Right, Wrong } MountState;

/**
 * Idea and code base by Olaf Kirch <okir@caldera.de>
 **/
static void check_mount_point(const char *mounttype,
                              const char *fsname,
                              MountState &isslow, MountState &isautofs)
{
    bool nfs = !strcmp(mounttype, "nfs");
    bool autofs = !strcmp(mounttype, "autofs") || !strcmp(mounttype,"subfs");
    bool pid = (strstr(fsname, ":(pid") != 0);

    if (nfs && !pid)
        isslow = Right;
    else if (isslow == Right)
        isslow = Wrong;

    /* Does this look like automounted? */
    if (autofs || (nfs && pid)) {
        isautofs = Right;
        isslow = Right;
    }
}

// returns the mount point, checks the mount state.
// if ismanual == Wrong this function does not check the manual mount state
static TQString get_mount_info(const TQString& filename,
    MountState& isautofs, MountState& isslow, MountState& ismanual,
    TQString& fstype)
{
    static bool gotRoot = false;
    static dev_t rootDevice;

    struct cachedDevice_t
    {
       dev_t device;
       TQString mountPoint;
       MountState isautofs;
       MountState isslow;
       MountState ismanual;
       TQString fstype;
    };
    static struct cachedDevice_t *cachedDevice = 0;

    if (!gotRoot)
    {
       KDE_struct_stat stat_buf;
       KDE_stat("/", &stat_buf);
       gotRoot = true;
       rootDevice = stat_buf.st_dev;
    }

    bool gotDevice = false;
    KDE_struct_stat stat_buf;
    if (KDE_stat(TQFile::encodeName(filename), &stat_buf) == 0)
    {
       gotDevice = true;
       if (stat_buf.st_dev == rootDevice)
       {
          static const TQString &root = TDEGlobal::staticQString("/");
          isautofs = Wrong;
          isslow = Wrong;
          ismanual = Wrong;
          fstype = TQString::null; // ### do we need it?
          return root;
       }
       if (cachedDevice && (stat_buf.st_dev == cachedDevice->device))
       {
          bool interestedInIsManual = ismanual != Wrong;
          isautofs = cachedDevice->isautofs;
          isslow = cachedDevice->isslow;
          ismanual = cachedDevice->ismanual;
          fstype = cachedDevice->fstype;
          // Don't use the cache if it doesn't have the information we're looking for
          if ( !interestedInIsManual || ismanual != Unseen )
              return cachedDevice->mountPoint;
       }
    }

    char realname[MAXPATHLEN];

    memset(realname, 0, MAXPATHLEN);

    /* If the path contains symlinks, get the real name */
    if (realpath(TQFile::encodeName(filename), realname) == 0) {
        if( strlcpy(realname, TQFile::encodeName(filename), MAXPATHLEN)>=MAXPATHLEN)
            return TQString::null;
    }

    int max = 0;
    TQString mountPoint;

    /* Loop over all file systems and see if we can find our
     * mount point.
     * Note that this is the mount point with the longest match.
     * XXX: Fails if me->mnt_dir is not a realpath but goes
     * through a symlink, e.g. /foo/bar where /foo is a symlink
     * pointing to /local/foo.
     *
     * How kinky can you get with a filesystem?
     */

#ifdef HAVE_GETMNTINFO

#ifdef GETMNTINFO_USES_STATVFS
    struct statvfs *mounted;
#else
    struct statfs *mounted;
#endif

    char    realpath_buffer[MAXPATHLEN];

    int num_fs = getmntinfo(&mounted, MNT_NOWAIT);

    for (int i=0;i<num_fs;i++) {

        TQCString device_name = mounted[i].f_mntfromname;

        // If the path contains symlinks, get
        // the real name
        if (realpath(device_name, realpath_buffer) != 0)
            // succes, use result from realpath
            device_name = realpath_buffer;
#ifdef __osf__
        char * mounttype = mnt_names[mounted[i].f_type];
#else
        char * mounttype = mounted[i].f_fstypename;
#endif
        if ( is_my_mountpoint( mounted[i].f_mntonname, realname, max ) )
        {
            mountPoint = TQFile::decodeName(mounted[i].f_mntonname);
            fstype = TQString::fromLatin1(mounttype);
            check_mount_point( mounttype, mounted[i].f_mntfromname,
                               isautofs, isslow );
            // keep going, looking for a potentially better one

            if (ismanual == Unseen)
            {
                struct fstab *ft = getfsfile(mounted[i].f_mntonname);
                if (!ft || strstr(ft->fs_mntops, "noauto"))
                  ismanual = Right;
            }
        }
    }

#elif defined(_AIX)

    struct vmount *mntctl_buffer;
    struct vmount *vm;
    char *mountedfrom;
    char *mountedto;
    int fsname_len, num;
    char realpath_buffer[MAXPATHLEN];
    int buf_sz = 4096;

    mntctl_buffer = (struct vmount*)malloc(buf_sz);
    num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    if (num == 0)
    {
	buf_sz = *(int*)mntctl_buffer;
	free(mntctl_buffer);
	mntctl_buffer = (struct vmount*)malloc(buf_sz);
	num = mntctl(MCTL_QUERY, buf_sz, mntctl_buffer);
    }

    if (num > 0)
    {
        /* iterate through items in the vmount structure: */
        vm = (struct vmount *)mntctl_buffer;
        for ( ; num > 0; num-- )
        {
            /* get the name of the mounted file systems: */
            fsname_len = vmt2datasize(vm, VMT_STUB);
            mountedto     = (char*)malloc(fsname_len + 1);
	    mountedto[fsname_len] = '\0';
            strncpy(mountedto, (char *)vmt2dataptr(vm, VMT_STUB), fsname_len);

            fsname_len = vmt2datasize(vm, VMT_OBJECT);
            mountedfrom     = (char*)malloc(fsname_len + 1);
	    mountedfrom[fsname_len] = '\0';
            strncpy(mountedfrom, (char *)vmt2dataptr(vm, VMT_OBJECT), fsname_len);

            /* get the mount-from information: */
            TQCString device_name = mountedfrom;

            if (realpath(device_name, realpath_buffer) != 0)
                // success, use result from realpath
                device_name = realpath_buffer;

	    /* Look up the string for the file system type,
             * as listed in /etc/vfs.
             * ex.: nfs,jfs,afs,cdrfs,sfs,cachefs,nfs3,autofs
             */
            struct vfs_ent* ent = getvfsbytype(vm->vmt_gfstype);

            if ( is_my_mountpoint( mountedto, realname, max ) )
            {
                mountPoint = TQFile::decodeName(mountedto);
                fstype = TQString::fromLatin1(ent->vfsent_name);
                check_mount_point(ent->vfsent_name, device_name, isautofs, isslow);

                if (ismanual == Unseen)
                {
                    // TODO: add check for ismanual, I couldn't find any way
                    // how to get the mount attribute from /etc/filesystems
                    ismanual == Wrong;
                }
            }

            free(mountedfrom);
            free(mountedto);

            /* goto the next vmount structure: */
            vm = (struct vmount *)((char *)vm + vm->vmt_length);
        }

	endvfsent( );
    }

    free( mntctl_buffer );

#else

    STRUCT_SETMNTENT mtab;
    /* Get the list of mounted file systems */

    if ((mtab = SETMNTENT(MNTTAB, "r")) == 0) {
        perror("setmntent");
        return TQString::null;
    }

    STRUCT_MNTENT me;

    while (true) {
        if (!GETMNTENT(mtab, me))
            break;

        if ( is_my_mountpoint( MOUNTPOINT(me), realname, max ) )
        {
            mountPoint = TQFile::decodeName( MOUNTPOINT(me) );
            fstype = MOUNTTYPE(me);
            check_mount_point(MOUNTTYPE(me), FSNAME(me), isautofs, isslow);
            // we don't check if ismanual is Right, if /a/b is manually
            // mounted /a/b/c can't be automounted. At least IMO.
            if (ismanual == Unseen)
            {
                // The next GETMNTENT call may destroy 'me'
                // Copy out the info that we need
                TQCString fsname_me = FSNAME(me);
                TQCString mounttype_me = MOUNTTYPE(me);

                STRUCT_SETMNTENT fstab;
                if ((fstab = SETMNTENT(FSTAB, "r")) == 0) {
                    continue;
                }

                bool found = false;
                STRUCT_MNTENT fe;
                while (GETMNTENT(fstab, fe))
                {
                    if (fsname_me == FSNAME(fe))
                    {
                        found = true;
                        if (HASMNTOPT(fe, "noauto") ||
                            !strcmp(MOUNTTYPE(fe), "supermount"))
                            ismanual = Right;
                        break;
                    }
                }
                if (!found || (mounttype_me == "supermount"))
                  ismanual = Right;

                ENDMNTENT(fstab);
            }
        }
    }

    ENDMNTENT(mtab);

#endif

    if (isautofs == Right && isslow == Unseen)
        isslow = Right;

    if (gotDevice)
    {
       if (!cachedDevice)
          cachedDevice = new cachedDevice_t;

       cachedDevice->device = stat_buf.st_dev;
       cachedDevice->mountPoint = mountPoint;
       cachedDevice->isautofs = isautofs;
       cachedDevice->isslow = isslow;
       cachedDevice->ismanual = ismanual;
       cachedDevice->fstype = fstype;
    }

    return mountPoint;
}

#else //!Q_OS_UNIX
//dummy
TQString TDEIO::findDeviceMountPoint( const TQString& filename )
{
	return TQString::null;
}
#endif

TQString TDEIO::findPathMountPoint(const TQString& filename)
{
#ifdef Q_OS_UNIX
  MountState isautofs = Unseen, isslow = Unseen, ismanual = Wrong;
  TQString fstype;
  return get_mount_info(filename, isautofs, isslow, ismanual, fstype);
#else //!Q_OS_UNIX
  return TQString::null;
#endif 
}

bool TDEIO::manually_mounted(const TQString& filename)
{
#ifdef Q_OS_UNIX
  MountState isautofs = Unseen, isslow = Unseen, ismanual = Unseen;
  TQString fstype;
  TQString mountPoint = get_mount_info(filename, isautofs, isslow, ismanual, fstype);
  return !mountPoint.isNull() && (ismanual == Right);
#else //!Q_OS_UNIX
  return false;
#endif 
}

bool TDEIO::probably_slow_mounted(const TQString& filename)
{
#ifdef Q_OS_UNIX
  MountState isautofs = Unseen, isslow = Unseen, ismanual = Wrong;
  TQString fstype;
  TQString mountPoint = get_mount_info(filename, isautofs, isslow, ismanual, fstype);
  return !mountPoint.isNull() && (isslow == Right);
#else //!Q_OS_UNIX
  return false;
#endif 
}

bool TDEIO::testFileSystemFlag(const TQString& filename, FileSystemFlag flag)
{
#ifdef Q_OS_UNIX
  MountState isautofs = Unseen, isslow = Unseen, ismanual = Wrong;
  TQString fstype;
  TQString mountPoint = get_mount_info(filename, isautofs, isslow, ismanual, fstype);
    kdDebug() << "testFileSystemFlag: fstype=" << fstype << endl;
  if (mountPoint.isNull())
      return false;
  bool isMsDos = ( fstype == "msdos" || fstype == "fat" || fstype == "vfat" );
  switch (flag)  {
  case SupportsChmod:
  case SupportsChown:
  case SupportsUTime:
  case SupportsSymlinks:
      return !isMsDos; // it's amazing the number of things FAT doesn't support :)
  case CaseInsensitive:
      return isMsDos;
  }
#endif 
  return false;
}

TDEIO::CacheControl TDEIO::parseCacheControl(const TQString &cacheControl)
{
  TQString tmp = cacheControl.lower();

  if (tmp == "cacheonly")
     return TDEIO::CC_CacheOnly;
  if (tmp == "cache")
     return TDEIO::CC_Cache;
  if (tmp == "verify")
     return TDEIO::CC_Verify;
  if (tmp == "refresh")
     return TDEIO::CC_Refresh;
  if (tmp == "reload")
     return TDEIO::CC_Reload;

  kdDebug() << "unrecognized Cache control option:"<<cacheControl<<endl;
  return TDEIO::CC_Verify;
}

TQString TDEIO::getCacheControlString(TDEIO::CacheControl cacheControl)
{
    if (cacheControl == TDEIO::CC_CacheOnly)
	return "CacheOnly";
    if (cacheControl == TDEIO::CC_Cache)
	return "Cache";
    if (cacheControl == TDEIO::CC_Verify)
	return "Verify";
    if (cacheControl == TDEIO::CC_Refresh)
	return "Refresh";
    if (cacheControl == TDEIO::CC_Reload)
	return "Reload";
    kdDebug() << "unrecognized Cache control enum value:"<<cacheControl<<endl;
    return TQString::null;
}
