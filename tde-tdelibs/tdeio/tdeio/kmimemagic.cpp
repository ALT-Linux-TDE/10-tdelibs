/* This file is part of the TDE libraries
   Copyright (C) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>

   Small portions (the original KDE interface and utime code) are:
   Copyright (C) 2000 Fritz Elfert <fritz@kde.org>
   Copyright (C) 2004 Allan Sandfeld Jensen <kde@carewolf.com>

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

#include "config.h"
#include "kmimemagic.h"
#include <kdebug.h>
#include <tdeapplication.h>
#include <tqfile.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>
#include <kstaticdeleter.h>
#include <klargefile.h>
#include <assert.h>

#include <magic.h>

#ifndef MAGIC_MIME_TYPE
#define MAGIC_MIME_TYPE MAGIC_MIME
#endif

// Taken from file/file.h
// Keep in sync with that header!
#define	FILE_LOAD	0

static void process(struct config_rec* conf,  const TQString &);

KMimeMagic* KMimeMagic::s_pSelf;
static KStaticDeleter<KMimeMagic> kmimemagicsd;

KMimeMagic* KMimeMagic::self() {
	if( !s_pSelf ) {
		initStatic();
	}
	return s_pSelf;
}

void KMimeMagic::initStatic() {
	s_pSelf = kmimemagicsd.setObject( s_pSelf, new KMimeMagic() );
	s_pSelf->setFollowLinks( true );
}

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <ctype.h>
#include <time.h>
#include <utime.h>
#include <stdarg.h>
#include <tqregexp.h>
#include <tqstring.h>

#define MIME_INODE_DIR         "inode/directory"
#define MIME_INODE_CDEV        "inode/chardevice"
#define MIME_INODE_BDEV        "inode/blockdevice"
#define MIME_INODE_FIFO        "inode/fifo"
#define MIME_INODE_LINK        "inode/link"
#define MIME_INODE_SOCK        "inode/socket"
#define MIME_BINARY_UNREADABLE "application/x-unreadable"
#define MIME_BINARY_ZEROSIZE   "application/x-zerosize"

/**
 * Configuration for the utime() problem.
 * Here's the problem:
 * By looking into a file to determine its mimetype, we change its "last access"
 * time (atime) and this can have side effects, like files in /tmp never being
 * cleaned up because of that. So in temp directories, we restore the atime.
 * Since this changes the ctime (last change of attributes), we don't do that
 * anywhere else, because that breaks archiving programs, that check the ctime.
 * Hence this class, to configure the directories where the atime should be restored.
 */
class KMimeMagicUtimeConf {
	public:
		KMimeMagicUtimeConf() {
			tmpDirs << TQString::fromLatin1("/tmp"); // default value

			// The trick is that we also don't want the user to override globally set
			// directories. So we have to misuse TDEStandardDirs :}
			TQStringList confDirs = TDEGlobal::dirs()->resourceDirs( "config" );
			if ( !confDirs.isEmpty() ) {
				TQString globalConf = confDirs.last() + "kmimemagicrc";
				if ( TQFile::exists( globalConf ) ) {
					KSimpleConfig cfg( globalConf );
					cfg.setGroup( "Settings" );
					tmpDirs = cfg.readListEntry( "atimeDirs" );
				}
				if ( confDirs.count() > 1 ) {
					TQString localConf = confDirs.first() + "kmimemagicrc";
					if ( TQFile::exists( localConf ) ) {
						KSimpleConfig cfg( localConf );
						cfg.setGroup( "Settings" );
						tmpDirs += cfg.readListEntry( "atimeDirs" );
					}
				}
				for ( TQStringList::Iterator it = tmpDirs.begin() ; it != tmpDirs.end() ; ++it ) {
					TQString dir = *it;
					if ( !dir.isEmpty() && dir[ dir.length()-1 ] != '/' ) {
						(*it) += '/';
					}
				}
			}
		#if 0
			// debug code
			for ( TQStringList::Iterator it = tmpDirs.begin() ; it != tmpDirs.end() ; ++it ) {
				kdDebug(7018) << " atimeDir: " << *it << endl;
			}
		#endif
		}

		bool restoreAccessTime( const TQString & file ) const {
			TQString dir = file.left( file.findRev( '/' ) );
			bool res = tmpDirs.contains( dir );
			//kdDebug(7018) << "restoreAccessTime " << file << " dir=" << dir << " result=" << res << endl;
			return res;
		}
		TQStringList tmpDirs;
};

TQString fixupMagicOutput(TQString &mime) {
	if (mime == "inode/x-empty") {
		return MIME_BINARY_ZEROSIZE;
	}
	else if (mime.contains("no read permission")) {
		return MIME_BINARY_UNREADABLE;
	}
	else {
		return mime;
	}
}

/* current config */
struct config_rec {
	bool followLinks;
	TQString resultBuf;
	int accuracy;

	magic_t magic;
	TQStringList databases;

	KMimeMagicUtimeConf * utimeConf;
};

/*
 * apprentice - load configuration from the magic file.
 */
int KMimeMagic::apprentice( const TQString& magicfile ) {
	TQString maindatabase = magicfile;
	if (maindatabase == "") {
#ifdef HAVE_LIBMAGIC_GETPATH
		maindatabase = magic_getpath(0, FILE_LOAD);
#else
		maindatabase = TQString(LIBMAGIC_PATH);
#endif
		if (maindatabase == "") {
			kdWarning() << k_funcinfo << "Unable to locate system mime magic database; mime type detection will not function correctly!" << endl;
		}
	}
	conf->databases.clear();
	conf->databases.append(maindatabase);
	return magic_load(conf->magic, conf->databases[0].latin1());
}

/*
 * magic_process - process input file fn. Opens the file and reads a
 * fixed-size buffer to begin processing the contents.
 */

void process(struct config_rec* conf, const TQString & fn) {
	KDE_struct_stat sb;
        TQCString fileName = TQFile::encodeName( fn );

	int magic_flags = MAGIC_ERROR|MAGIC_MIME_TYPE/*|MAGIC_DEBUG*/;
	if (conf->followLinks) {
		magic_flags |= MAGIC_SYMLINK;
	}
	magic_setflags(conf->magic, magic_flags);
	conf->resultBuf = TQString(magic_file(conf->magic, fileName));
	conf->resultBuf = fixupMagicOutput(conf->resultBuf);

	if ( conf->utimeConf && conf->utimeConf->restoreAccessTime( fn ) ) {
		/*
		* Try to restore access, modification times if read it.
		* This changes the "change" time (ctime), but we can't do anything
		* about that.
		*/
		struct utimbuf utbuf;
		utbuf.actime = sb.st_atime;
		utbuf.modtime = sb.st_mtime;
		(void) utime(fileName, &utbuf);
	}
}

KMimeMagic::KMimeMagic() {
	// Magic file detection init
	TQString mimefile = locate( "mime", "magic" );
	init( mimefile );
	// Add snippets from share/config/magic/*
	TQStringList snippets = TDEGlobal::dirs()->findAllResources( "config", "magic/*.magic", true );
	for ( TQStringList::Iterator it = snippets.begin() ; it != snippets.end() ; ++it ) {
		if ( !mergeConfig( *it ) ) {
			kdWarning() << k_funcinfo << "Failed to parse " << *it << endl;
		}
	}
}

KMimeMagic::KMimeMagic(const TQString & _configfile) {
	init( _configfile );
}

void KMimeMagic::init( const TQString& _configfile ) {
	int result;
	conf = new config_rec;

	/* initialize libmagic */
	conf->magic = magic_open(MAGIC_MIME_TYPE);
	magicResult = NULL;
	conf->followLinks = false;

        conf->utimeConf = 0L; // created on demand
	/* on the first time through we read the magic file */
	result = apprentice(_configfile);
	if (result == -1) {
		return;
	}
}

/*
 * The destructor.
 * Free the magic-table and other resources.
 */
KMimeMagic::~KMimeMagic() {
	if (conf) {
		magic_close(conf->magic);
                delete conf->utimeConf;
		delete conf;
	}
        delete magicResult;
}

bool KMimeMagic::mergeConfig(const TQString & _configfile) {
	conf->databases.append(_configfile);
	TQString merged_databases = conf->databases.join(":");
#ifdef MAGIC_VERSION
	int magicvers = magic_version();
#else // MAGIC_VERSION
	int magicvers = 0;
#endif // MAGIC_VERSION
	if ((magicvers < 512) || (magicvers >= 518)) {
		if (magic_load(conf->magic, merged_databases.latin1()) == 0) {
			return true;
		}
		else {
			return false;
		}
	}
	else {
		kdWarning(7018) << k_funcinfo << "KMimeMagic::mergeConfig disabled due to buggy libmagic version " << magicvers << endl;
		return false;
	}
}

void KMimeMagic::setFollowLinks( bool _enable ) {
	conf->followLinks = _enable;
}

KMimeMagicResult *KMimeMagic::findBufferType(const TQByteArray &array) {
	conf->resultBuf = TQString::null;
	if ( !magicResult ) {
		magicResult = new KMimeMagicResult();
	}
	magicResult->setInvalid();
	conf->accuracy = 100;

	int nbytes = array.size();
	if (nbytes == 0) {
		conf->resultBuf = MIME_BINARY_ZEROSIZE;
	}
	else {
		int magic_flags = MAGIC_ERROR|MAGIC_MIME_TYPE/*|MAGIC_DEBUG*/;
		if (conf->followLinks) {
			magic_flags |= MAGIC_SYMLINK;
		}
		magic_setflags(conf->magic, magic_flags);
		conf->resultBuf = TQString(magic_buffer(conf->magic, array.data(), nbytes));
		conf->resultBuf = fixupMagicOutput(conf->resultBuf);
	}
	/* if we have any results, put them in the request structure */
	magicResult->setMimeType(conf->resultBuf.stripWhiteSpace());
	magicResult->setAccuracy(conf->accuracy);
	return magicResult;
}

static void refineResult(KMimeMagicResult *r, const TQString & _filename) {
	TQString tmp = r->mimeType();
	if (tmp.isEmpty())
		return;
	if ( tmp == "text/x-c" || tmp == "text/x-objc" )
	{
		if ( _filename.right(2) == ".h" )
			tmp += "hdr";
		else
			tmp += "src";
		r->setMimeType(tmp);
	}
	else
	if ( tmp == "text/x-c++" )
	{
		if ( _filename.endsWith(".h")
		  || _filename.endsWith(".hh")
		  || _filename.endsWith(".H")
		  || !_filename.right(4).contains('.'))
			tmp += "hdr";
		else
			tmp += "src";
		r->setMimeType(tmp);
	}
	else
	if ( tmp == "application/x-sharedlib" )
	{
		if ( _filename.find( ".so" ) == -1 ) 
		{
			tmp = "application/x-executable";
			r->setMimeType( tmp );
		}
	}
}

KMimeMagicResult *KMimeMagic::findBufferFileType( const TQByteArray &data, const TQString &fn) {
        KMimeMagicResult * r = findBufferType( data );
	refineResult(r, fn);
        return r;
}

/*
 * Find the content-type of the given file.
 */
KMimeMagicResult* KMimeMagic::findFileType(const TQString & fn) {
#ifdef DEBUG_MIMEMAGIC
	kdDebug(7018) << "KMimeMagic::findFileType " << fn << endl;
#endif
	conf->resultBuf = TQString::null;

	if ( !magicResult ) {
		magicResult = new KMimeMagicResult();
	}
	magicResult->setInvalid();
	conf->accuracy = 100;

	if ( !conf->utimeConf ) {
		conf->utimeConf = new KMimeMagicUtimeConf();
	}

	/* process it based on the file contents */
	process(conf, fn );

	/* if we have any results, put them in the request structure */
	magicResult->setMimeType(conf->resultBuf.stripWhiteSpace());
	magicResult->setAccuracy(conf->accuracy);
	refineResult(magicResult, fn);
	return magicResult;
}
