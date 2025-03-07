/*
   This file is part of the KDE libraries
   Copyright (C) 2000 David Smith <dsmith@algonet.se>
   Copyright (C) 2004 Scott Wheeler <wheeler@kde.org>

   This class was inspired by a previous KURLCompletion by
   Henner Zeller <zeller@think.de>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation; either
   version 2 of the License, or (at your option) any later version.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.	See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.	If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <config.h>
#include <stdlib.h>
#include <assert.h>
#include <limits.h>

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqvaluelist.h>
#include <tqregexp.h>
#include <tqtimer.h>
#include <tqdir.h>
#include <tqfile.h>
#include <tqtextstream.h>
#include <tqdeepcopy.h>
#include <tqthread.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <kcompletion.h>
#include <kurl.h>
#include <tdeio/jobclasses.h>
#include <tdeio/job.h>
#include <kprotocolinfo.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kde_file.h>

#include <sys/types.h>
#include <dirent.h>
#include <unistd.h>
#include <sys/stat.h>
#include <pwd.h>
#include <time.h>
#include <sys/param.h>

#include "kurlcompletion.h"

static bool expandTilde(TQString &);
static bool expandEnv(TQString &);

static TQString unescape(const TQString &text);

// Permission mask for files that are executable by
// user, group or other
#define MODE_EXE (S_IXUSR | S_IXGRP | S_IXOTH)

// Constants for types of completion
enum ComplType {CTNone=0, CTEnv, CTUser, CTMan, CTExe, CTFile, CTUrl, CTInfo};

class CompletionThread;

/**
 * A custom event type that is used to return a list of completion
 * matches from an asyncrynous lookup.
 */

class CompletionMatchEvent : public TQCustomEvent
{
public:
	CompletionMatchEvent( CompletionThread *thread ) :
		TQCustomEvent( uniqueType() ),
		m_completionThread( thread )
	{}

	CompletionThread *completionThread() const { return m_completionThread; }
	static int uniqueType() { return User + 61080; }

private:
	CompletionThread *m_completionThread;
};

class CompletionThread : public TQThread
{
protected:
	CompletionThread( KURLCompletion *receiver ) :
		TQThread(),
		m_receiver( receiver ),
		m_terminationRequested( false )
	{}

public:
	void requestTermination() { m_terminationRequested = true; }
	TQDeepCopy<TQStringList> matches() const { return m_matches; }

protected:
	void addMatch( const TQString &match ) { m_matches.append( match ); }
	bool terminationRequested() const { return m_terminationRequested; }
	void done()
	{
		if ( !m_terminationRequested )
			kapp->postEvent( m_receiver, new CompletionMatchEvent( this ) );
		else
			delete this;
	}

private:
	KURLCompletion *m_receiver;
	TQStringList m_matches;
	bool m_terminationRequested;
};

/**
 * A simple thread that fetches a list of tilde-completions and returns this
 * to the caller via a CompletionMatchEvent.
 */

class UserListThread : public CompletionThread
{
public:
    UserListThread( KURLCompletion *receiver ) :
		CompletionThread( receiver )
	{}

protected:
	virtual void run()
	{
		static const TQChar tilde = '~';

		struct passwd *pw;
		while ( ( pw = ::getpwent() ) && !terminationRequested() )
			addMatch( tilde + TQString::fromLocal8Bit( pw->pw_name ) );

		::endpwent();

		addMatch( tilde );

		done();
	}
};

class DirectoryListThread : public CompletionThread
{
public:
	DirectoryListThread( KURLCompletion *receiver,
	                     const TQStringList &dirList,
	                     const TQString &filter,
	                     bool onlyExe,
	                     bool onlyDir,
	                     bool noHidden,
	                     bool appendSlashToDir ) :
		CompletionThread( receiver ),
		m_dirList( TQDeepCopy<TQStringList>( dirList ) ),
		m_filter( TQDeepCopy<TQString>( filter ) ),
		m_onlyExe( onlyExe ),
		m_onlyDir( onlyDir ),
		m_noHidden( noHidden ),
		m_appendSlashToDir( appendSlashToDir )
	{}

	virtual void run();

private:
	TQStringList m_dirList;
	TQString m_filter;
	bool m_onlyExe;
	bool m_onlyDir;
	bool m_noHidden;
	bool m_appendSlashToDir;
};

void DirectoryListThread::run()
{
	// Thread safety notes:
	//
	// There very possibly may be thread safety issues here, but I've done a check
	// of all of the things that would seem to be problematic.  Here are a few
	// things that I have checked to be safe here (some used indirectly):
	//
	// TQDir::currentDirPath(), TQDir::setCurrent(), TQFile::decodeName(), TQFile::encodeName()
	// TQString::fromLocal8Bit(), TQString::local8Bit(), TQTextCodec::codecForLocale()
	//
	// Also see (for POSIX functions):
	// http://www.opengroup.org/onlinepubs/009695399/functions/xsh_chap02_09.html

	DIR *dir = 0;

	for ( TQStringList::ConstIterator it = m_dirList.begin();
	      it != m_dirList.end() && !terminationRequested();
	      ++it )
	{
		// Open the next directory

		if ( !dir ) {
			dir = ::opendir( TQFile::encodeName( *it ) );
			if ( ! dir ) {
				kdDebug() << "Failed to open dir: " << *it << endl;
				done();
				return;
			}
		}

		// A trick from TDEIO that helps performance by a little bit:
		// chdir to the directroy so we won't have to deal with full paths
		// with stat()

		TQString path = TQDir::currentDirPath();
		TQDir::setCurrent( *it );

		// Loop through all directory entries
		// Solaris and IRIX dirent structures do not allocate space for d_name. On
		// systems that do (HP-UX, Linux, Tru64 UNIX), we overallocate space but
		// that's ok.
#ifndef HAVE_READDIR_R
		struct dirent *dirEntry = 0;
		while ( !terminationRequested() &&
		        (dirEntry = ::readdir( dir)))
#else
#if !defined(MAXPATHLEN) && defined(__GNU__)
#define MAXPATHLEN UCHAR_MAX
#endif
		struct dirent *dirPosition = (struct dirent *) malloc( sizeof( struct dirent ) + MAXPATHLEN + 1 );
		struct dirent *dirEntry = 0;
		while ( !terminationRequested() &&
		        ::readdir_r( dir, dirPosition, &dirEntry ) == 0 && dirEntry )
#endif 

		{
			// Skip hidden files if m_noHidden is true

			if ( dirEntry->d_name[0] == '.' && m_noHidden )
				continue;

			// Skip "."

			if ( dirEntry->d_name[0] == '.' && dirEntry->d_name[1] == '\0' )
				continue;

			// Skip ".."

			if ( dirEntry->d_name[0] == '.' && dirEntry->d_name[1] == '.' && dirEntry->d_name[2] == '\0' )
				continue;

			TQString file = TQFile::decodeName( dirEntry->d_name );

			if ( m_filter.isEmpty() || file.startsWith( m_filter ) ) {

				if ( m_onlyExe || m_onlyDir || m_appendSlashToDir ) {
					KDE_struct_stat sbuff;

					if ( KDE_stat( dirEntry->d_name, &sbuff ) == 0 ) {

						// Verify executable

						if ( m_onlyExe && ( sbuff.st_mode & MODE_EXE ) == 0 )
							continue;

						// Verify directory

						if ( m_onlyDir && !S_ISDIR( sbuff.st_mode ) )
							continue;

						// Add '/' to directories

						if ( m_appendSlashToDir && S_ISDIR( sbuff.st_mode ) )
							file.append( '/' );

					}
					else {
						kdDebug() << "Could not stat file " << file << endl;
						continue;
					}
				}

				addMatch( file );
			}
		}

		// chdir to the original directory

		TQDir::setCurrent( path );

		::closedir( dir );
		dir = 0;
#ifdef HAVE_READDIR_R
		free( dirPosition );
#endif
	}

	done();
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// MyURL - wrapper for KURL with some different functionality
//

class KURLCompletion::MyURL
{
public:
	MyURL(const TQString &url, const TQString &cwd);
	MyURL(const MyURL &url);
	~MyURL();

	KURL *kurl() const { return m_kurl; }

	TQString protocol() const { return m_kurl->protocol(); }
	// The directory with a trailing '/'
	TQString dir() const { return m_kurl->directory(false, false); }
	TQString file() const { return m_kurl->fileName(false); }

    // The initial, unparsed, url, as a string.
	TQString url() const { return m_url; }

	// Is the initial string a URL, or just a path (whether absolute or relative)
	bool isURL() const { return m_isURL; }

	void filter( bool replace_user_dir, bool replace_env );

private:
	void init(const TQString &url, const TQString &cwd);

	KURL *m_kurl;
	TQString m_url;
	bool m_isURL;
};

KURLCompletion::MyURL::MyURL(const TQString &url, const TQString &cwd)
{
	init(url, cwd);
}

KURLCompletion::MyURL::MyURL(const MyURL &url)
{
	m_kurl = new KURL( *(url.m_kurl) );
	m_url = url.m_url;
	m_isURL = url.m_isURL;
}

void KURLCompletion::MyURL::init(const TQString &url, const TQString &cwd)
{
	// Save the original text
	m_url = url;

	// Non-const copy
	TQString url_copy = url;

	// Special shortcuts for "man:" and "info:"
	if ( url_copy[0] == '#' ) {
		if ( url_copy[1] == '#' )
			url_copy.replace( 0, 2, TQString("info:") );
		else
			url_copy.replace( 0, 1, TQString("man:") );
	}

	// Look for a protocol in 'url'
	TQRegExp protocol_regex = TQRegExp( "^[^/\\s\\\\]*:" );

	// Assume "file:" or whatever is given by 'cwd' if there is
	// no protocol.  (KURL does this only for absoute paths)
	if ( protocol_regex.search( url_copy ) == 0 )
    {
		m_kurl = new KURL( url_copy );
		m_isURL = true;
	}
	else // relative path or ~ or $something
	{
		m_isURL = false;
		if ( cwd.isEmpty() )
		{
			m_kurl = new KURL();
			if ( !TQDir::isRelativePath(url_copy) || url_copy[0] == '$' || url_copy[0] == '~' )
				m_kurl->setPath( url_copy );
			else
				*m_kurl = url_copy;
		}
		else
		{
			KURL base = KURL::fromPathOrURL( cwd );
			base.adjustPath(+1);

			if ( !TQDir::isRelativePath(url_copy) || url_copy[0] == '~' || url_copy[0] == '$' )
			{
				m_kurl = new KURL();
				m_kurl->setPath( url_copy );
			}
			else // relative path
			{
				//m_kurl = new KURL( base, url_copy );
				m_kurl = new KURL( base );
				m_kurl->addPath( url_copy );
			}
		}
	}
}

KURLCompletion::MyURL::~MyURL()
{
	delete m_kurl;
}

void KURLCompletion::MyURL::filter( bool replace_user_dir, bool replace_env )
{
	TQString d = dir() + file();
	if ( replace_user_dir ) expandTilde( d );
	if ( replace_env ) expandEnv( d );
	m_kurl->setPath( d );
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// KURLCompletionPrivate
//
class KURLCompletionPrivate
{
public:
	KURLCompletionPrivate() : url_auto_completion(true),
	                          userListThread(0),
	                          dirListThread(0) {}
	~KURLCompletionPrivate();

	TQValueList<KURL*> list_urls;

	bool onlyLocalProto;

	// urlCompletion() in Auto/Popup mode?
	bool url_auto_completion;

	// Append '/' to directories in Popup mode?
	// Doing that stat's all files and is slower
	bool popup_append_slash;

	// Keep track of currently listed files to avoid reading them again
	TQString last_path_listed;
	TQString last_file_listed;
	TQString last_prepend;
	int last_compl_type;
	int last_no_hidden;

	TQString cwd; // "current directory" = base dir for completion

	KURLCompletion::Mode mode; // ExeCompletion, FileCompletion, DirCompletion
	bool replace_env;
	bool replace_home;
	bool complete_url; // if true completing a URL (i.e. 'prepend' is a URL), otherwise a path

	TDEIO::ListJob *list_job; // tdeio job to list directories

	TQString prepend; // text to prepend to listed items
	TQString compl_text; // text to pass on to TDECompletion

	// Filters for files read with  tdeio
	bool list_urls_only_exe; // true = only list executables
	bool list_urls_no_hidden;
	TQString list_urls_filter; // filter for listed files

	CompletionThread *userListThread;
	CompletionThread *dirListThread;
};

KURLCompletionPrivate::~KURLCompletionPrivate()
{
	if ( userListThread )
		userListThread->requestTermination();
	if ( dirListThread )
		dirListThread->requestTermination();
}

///////////////////////////////////////////////////////
///////////////////////////////////////////////////////
// KURLCompletion
//

KURLCompletion::KURLCompletion() : TDECompletion()
{
	init();
}


KURLCompletion::KURLCompletion( Mode mode ) : TDECompletion()
{
	init();
	setMode ( mode );
}

KURLCompletion::~KURLCompletion()
{
	stop();
	delete d;
}


void KURLCompletion::init()
{
	d = new KURLCompletionPrivate;

	d->cwd = TQDir::homeDirPath();

	d->replace_home = true;
	d->replace_env = true;
	d->last_no_hidden = false;
	d->last_compl_type = 0;
	d->list_job = 0L;
	d->mode = KURLCompletion::FileCompletion;

	// Read settings
	TDEConfig *c = TDEGlobal::config();
	TDEConfigGroupSaver cgs( c, "URLCompletion" );

	d->url_auto_completion = c->readBoolEntry("alwaysAutoComplete", true);
	d->popup_append_slash = c->readBoolEntry("popupAppendSlash", true);
	d->onlyLocalProto = c->readBoolEntry("LocalProtocolsOnly", false);
}

void KURLCompletion::setDir(const TQString &dir)
{
    d->cwd = dir;
}

TQString KURLCompletion::dir() const
{
	return d->cwd;
}

KURLCompletion::Mode KURLCompletion::mode() const
{
	return d->mode;
}

void KURLCompletion::setMode( Mode mode )
{
	d->mode = mode;
}

bool KURLCompletion::replaceEnv() const
{
	return d->replace_env;
}

void KURLCompletion::setReplaceEnv( bool replace )
{
	d->replace_env = replace;
}

bool KURLCompletion::replaceHome() const
{
	return d->replace_home;
}

void KURLCompletion::setReplaceHome( bool replace )
{
	d->replace_home = replace;
}

/*
 * makeCompletion()
 *
 * Entry point for file name completion
 */
TQString KURLCompletion::makeCompletion(const TQString &text)
{
	//kdDebug() << "KURLCompletion::makeCompletion: " << text << " d->cwd=" << d->cwd << endl;

	MyURL url(text, d->cwd);

	d->compl_text = text;

	// Set d->prepend to the original URL, with the filename [and ref/query] stripped.
	// This is what gets prepended to the directory-listing matches.
	int toRemove = url.file().length() - url.kurl()->query().length();
	if ( url.kurl()->hasRef() )
		toRemove += url.kurl()->ref().length() + 1;
	d->prepend = text.left( text.length() - toRemove );
	d->complete_url = url.isURL();

	TQString match;

	// Environment variables
	//
	if ( d->replace_env && envCompletion( url, &match ) )
		return match;

	// User directories
	//
	if ( d->replace_home && userCompletion( url, &match ) )
		return match;

	// Replace user directories and variables
	url.filter( d->replace_home, d->replace_env );

	//kdDebug() << "Filtered: proto=" << url.protocol()
	//          << ", dir=" << url.dir()
	//          << ", file=" << url.file()
	//          << ", kurl url=" << *url.kurl() << endl;

	if ( d->mode == ExeCompletion ) {
		// Executables
		//
		if ( exeCompletion( url, &match ) )
			return match;

		// KRun can run "man:" and "info:" etc. so why not treat them
		// as executables...

		if ( urlCompletion( url, &match ) )
			return match;
	}
	else if ( d->mode == SystemExeCompletion ) {
		// Executables
		//
		if ( systemexeCompletion( url, &match ) )
			return match;

		// KRun can run "man:" and "info:" etc. so why not treat them
		// as executables...

		if ( urlCompletion( url, &match ) )
			return match;
	}
	else {
		// Local files, directories
		//
		if ( fileCompletion( url, &match ) )
			return match;

		// All other...
		//
		if ( urlCompletion( url, &match ) )
			return match;
	}

	setListedURL( CTNone );
	stop();

	return TQString::null;
}

/*
 * finished
 *
 * Go on and call TDECompletion.
 * Called when all matches have been added
 */
TQString KURLCompletion::finished()
{
	if ( d->last_compl_type == CTInfo )
		return TDECompletion::makeCompletion( d->compl_text.lower() );
	else
		return TDECompletion::makeCompletion( d->compl_text );
}

/*
 * isRunning
 *
 * Return true if either a TDEIO job or the DirLister
 * is running
 */
bool KURLCompletion::isRunning() const
{
	return d->list_job || (d->dirListThread && !d->dirListThread->finished());
}

/*
 * stop
 *
 * Stop and delete a running TDEIO job or the DirLister
 */
void KURLCompletion::stop()
{
	if ( d->list_job ) {
		d->list_job->kill();
		d->list_job = 0L;
	}

	if ( !d->list_urls.isEmpty() ) {
		TQValueList<KURL*>::Iterator it = d->list_urls.begin();
		for ( ; it != d->list_urls.end(); it++ )
			delete (*it);
		d->list_urls.clear();
	}

	if ( d->dirListThread ) {
		d->dirListThread->requestTermination();
		d->dirListThread = 0;
	}
}

/*
 * Keep track of the last listed directory
 */
void KURLCompletion::setListedURL( int complType,
                                   const TQString& dir,
                                   const TQString& filter,
                                   bool no_hidden )
{
	d->last_compl_type = complType;
	d->last_path_listed = dir;
	d->last_file_listed = filter;
	d->last_no_hidden = (int)no_hidden;
	d->last_prepend = d->prepend;
}

bool KURLCompletion::isListedURL( int complType,
                                  const TQString& dir,
                                  const TQString& filter,
                                  bool no_hidden )
{
	return  d->last_compl_type == complType
			&& ( d->last_path_listed == dir
					|| (dir.isEmpty() && d->last_path_listed.isEmpty()) )
			&& ( filter.startsWith(d->last_file_listed)
					|| (filter.isEmpty() && d->last_file_listed.isEmpty()) )
			&& d->last_no_hidden == (int)no_hidden
			&& d->last_prepend == d->prepend; // e.g. relative path vs absolute
}

/*
 * isAutoCompletion
 *
 * Returns true if completion mode is Auto or Popup
 */
bool KURLCompletion::isAutoCompletion()
{
	return completionMode() == TDEGlobalSettings::CompletionAuto
	       || completionMode() == TDEGlobalSettings::CompletionPopup
	       || completionMode() == TDEGlobalSettings::CompletionMan
	       || completionMode() == TDEGlobalSettings::CompletionPopupAuto;
}
//////////////////////////////////////////////////
//////////////////////////////////////////////////
// User directories
//

bool KURLCompletion::userCompletion(const MyURL &url, TQString *match)
{
	if ( url.protocol() != "file"
	      || !url.dir().isEmpty()
	      || url.file().at(0) != '~' )
		return false;

	if ( !isListedURL( CTUser ) ) {
		stop();
		clear();

		if ( !d->userListThread ) {
			d->userListThread = new UserListThread( this );
			d->userListThread->start();

			// If the thread finishes quickly make sure that the results
			// are added to the first matching case.

			d->userListThread->wait( 200 );
			TQStringList l = d->userListThread->matches();
			addMatches( l );
		}
	}
	*match = finished();
	return true;
}

/////////////////////////////////////////////////////
/////////////////////////////////////////////////////
// Environment variables
//

#if !defined(__OpenBSD__) && !defined(__FreeBSD__)
extern char **environ; // Array of environment variables
#endif

bool KURLCompletion::envCompletion(const MyURL &url, TQString *match)
{
#if defined(__OpenBSD__) || defined(__FreeBSD__)
	return false;
#else
	if ( url.file().at(0) != '$' )
		return false;

	if ( !isListedURL( CTEnv ) ) {
		stop();
		clear();

		char **env = environ;

		TQString dollar = TQString("$");

		TQStringList l;

		while ( *env ) {
			TQString s = TQString::fromLocal8Bit( *env );

			int pos = s.find('=');

			if ( pos == -1 )
				pos = s.length();

			if ( pos > 0 )
				l.append( dollar + s.left(pos) );

			env++;
		}

		addMatches( l );
	}

	setListedURL( CTEnv );

	*match = finished();
	return true;
#endif
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Executables
//

bool KURLCompletion::exeCompletion(const MyURL &url, TQString *match)
{
	if ( url.protocol() != "file" )
		return false;

	TQString dir = url.dir();

	dir = unescape( dir ); // remove escapes

	// Find directories to search for completions, either
	//
	// 1. complete path given in url
	// 2. current directory (d->cwd)
	// 3. $PATH
	// 4. no directory at all

	TQStringList dirList;

	if ( !TQDir::isRelativePath(dir) ) {
		// complete path in url
		dirList.append( dir );
	}
	else if ( !dir.isEmpty() && !d->cwd.isEmpty() ) {
		// current directory
		dirList.append( d->cwd + '/' + dir );
	}
	else if ( !url.file().isEmpty() ) {
		// $PATH
		dirList = TQStringList::split(KPATH_SEPARATOR,
					TQString::fromLocal8Bit(::getenv("PATH")));

		TQStringList::Iterator it = dirList.begin();

		for ( ; it != dirList.end(); it++ )
			(*it).append('/');
	}

	// No hidden files unless the user types "."
	bool no_hidden_files = url.file().at(0) != '.';

	// List files if needed
	//
	if ( !isListedURL( CTExe, dir, url.file(), no_hidden_files ) )
	{
		stop();
		clear();

		setListedURL( CTExe, dir, url.file(), no_hidden_files );

		*match = listDirectories( dirList, url.file(), true, false, no_hidden_files );
	}
	else if ( !isRunning() ) {
		*match = finished();
	}
	else {
		if ( d->dirListThread )
			setListedURL( CTExe, dir, url.file(), no_hidden_files );
		*match = TQString::null;
	}

	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// System Executables
//

bool KURLCompletion::systemexeCompletion(const MyURL &url, TQString *match)
{
	if ( url.protocol() != "file" )
		return false;

	TQString dir = url.dir();

	dir = unescape( dir ); // remove escapes

	// Find directories to search for completions, either
	//
	// 1. complete path given in url
	// 2. current directory (d->cwd)
	// 3. $PATH
	// 4. no directory at all

	TQStringList dirList;

	if ( !url.file().isEmpty() ) {
		// $PATH
		dirList = TQStringList::split(KPATH_SEPARATOR,
					TQString::fromLocal8Bit(::getenv("PATH")));

		TQStringList::Iterator it = dirList.begin();

		for ( ; it != dirList.end(); it++ )
			(*it).append('/');
	}

	// No hidden files unless the user types "."
	bool no_hidden_files = url.file().at(0) != '.';

	// List files if needed
	//
	if ( !isListedURL( CTExe, dir, url.file(), no_hidden_files ) )
	{
		stop();
		clear();

		setListedURL( CTExe, dir, url.file(), no_hidden_files );

		*match = listDirectories( dirList, url.file(), true, false, no_hidden_files );
	}
	else if ( !isRunning() ) {
		*match = finished();
	}
	else {
		if ( d->dirListThread )
			setListedURL( CTExe, dir, url.file(), no_hidden_files );
		*match = TQString::null;
	}

	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Local files
//

bool KURLCompletion::fileCompletion(const MyURL &url, TQString *match)
{
	if ( url.protocol() != "file" )
		return false;

	TQString dir = url.dir();

	if (url.url()[0] == '.')
	{
		if (url.url().length() == 1)
		{
			*match =
				( completionMode() == TDEGlobalSettings::CompletionMan )? "." : "..";
			return true;
		}
		if (url.url().length() == 2 && url.url()[1]=='.')
		{
			*match="..";
			return true;
		}
	}

	//kdDebug() << "fileCompletion " << url.url() << " dir=" << dir << endl;

	dir = unescape( dir ); // remove escapes

	// Find directories to search for completions, either
	//
	// 1. complete path given in url
	// 2. current directory (d->cwd)
	// 3. no directory at all

	TQStringList dirList;

	if ( !TQDir::isRelativePath(dir) ) {
		// complete path in url
		dirList.append( dir );
	}
	else if ( !d->cwd.isEmpty() ) {
		// current directory
		dirList.append( d->cwd + '/' + dir );
	}

	// No hidden files unless the user types "."
	bool no_hidden_files = ( url.file().at(0) != '.' );

	// List files if needed
	//
	if ( !isListedURL( CTFile, dir, "", no_hidden_files ) )
	{
		stop();
		clear();

		setListedURL( CTFile, dir, "", no_hidden_files );

		// Append '/' to directories in Popup mode?
		bool append_slash = ( d->popup_append_slash
	    	&& (completionMode() == TDEGlobalSettings::CompletionPopup ||
		    completionMode() == TDEGlobalSettings::CompletionPopupAuto ) );

		bool only_dir = ( d->mode == DirCompletion );

		*match = listDirectories( dirList, "", false, only_dir, no_hidden_files,
		                          append_slash );
	}
	else if ( !isRunning() ) {
		*match = finished();
	}
	else {
		*match = TQString::null;
	}

	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// URLs not handled elsewhere...
//

bool KURLCompletion::urlCompletion(const MyURL &url, TQString *match)
{
	//kdDebug() << "urlCompletion: url = " << *url.kurl() << endl;
	if (d->onlyLocalProto && KProtocolInfo::protocolClass(url.protocol()) != ":local")
		return false;

	// Use d->cwd as base url in case url is not absolute
	KURL url_cwd = KURL::fromPathOrURL( d->cwd );

	// Create an URL with the directory to be listed
	KURL url_dir( url_cwd, url.kurl()->url() );

	// Don't try url completion if
	// 1. malformed url
	// 2. protocol that doesn't have listDir()
	// 3. there is no directory (e.g. "ftp://ftp.kd" shouldn't do anything)
	// 4. auto or popup completion mode depending on settings

	bool man_or_info = ( url_dir.protocol() == TQString("man")
	                     || url_dir.protocol() == TQString("info") );

	if ( !url_dir.isValid()
	     || !KProtocolInfo::supportsListing( url_dir )
	     || ( !man_or_info
	          && ( url_dir.directory(false,false).isEmpty()
	               || ( isAutoCompletion()
	                    && !d->url_auto_completion ) ) ) ) {
		return false;
        }

	url_dir.setFileName(""); // not really nesseccary, but clear the filename anyway...

	// Remove escapes
	TQString dir = url_dir.directory( false, false );

	dir = unescape( dir );

	url_dir.setPath( dir );

	// List files if needed
	//
	if ( !isListedURL( CTUrl, url_dir.prettyURL(), url.file() ) )
	{
		stop();
		clear();

		setListedURL( CTUrl, url_dir.prettyURL(), "" );

		TQValueList<KURL*> url_list;
		url_list.append( new KURL( url_dir ) );

		listURLs( url_list, "", false );

		*match = TQString::null;
	}
	else if ( !isRunning() ) {
		*match = finished();
	}
	else {
		*match = TQString::null;
	}

	return true;
}

//////////////////////////////////////////////////
//////////////////////////////////////////////////
// Directory and URL listing
//

/*
 * addMatches
 *
 * Called to add matches to TDECompletion
 */
void KURLCompletion::addMatches( const TQStringList &matches )
{
	TQStringList::ConstIterator it = matches.begin();
	TQStringList::ConstIterator end = matches.end();

	if ( d->complete_url )
		for ( ; it != end; it++ )
			addItem( d->prepend + KURL::encode_string(*it));
	else
		for ( ; it != end; it++ )
			addItem( d->prepend + (*it));
}

/*
 * listDirectories
 *
 * List files starting with 'filter' in the given directories,
 * either using DirLister or listURLs()
 *
 * In either case, addMatches() is called with the listed
 * files, and eventually finished() when the listing is done
 *
 * Returns the match if available, or TQString::null if
 * DirLister timed out or using tdeio
 */
TQString KURLCompletion::listDirectories(
		const TQStringList &dirList,
		const TQString &filter,
		bool only_exe,
		bool only_dir,
		bool no_hidden,
		bool append_slash_to_dir)
{
	assert( !isRunning() );

	if ( !::getenv("KURLCOMPLETION_LOCAL_TDEIO") ) {

		//kdDebug() << "Listing (listDirectories): " << dirList << " filter=" << filter << " without TDEIO" << endl;

		// Don't use TDEIO

		if ( d->dirListThread )
			d->dirListThread->requestTermination();

		TQStringList dirs;

		for ( TQStringList::ConstIterator it = dirList.begin();
		      it != dirList.end();
		      ++it )
		{
			KURL url;
			url.setPath(*it);
			if ( kapp->authorizeURLAction( "list", KURL(), url ) )
				dirs.append( *it );
		}

		d->dirListThread = new DirectoryListThread( this, dirs, filter, only_exe, only_dir,
		                                            no_hidden, append_slash_to_dir );
		d->dirListThread->start();
		d->dirListThread->wait( 200 );
		addMatches( d->dirListThread->matches() );

		return finished();
	}
	else {

		// Use TDEIO
		//kdDebug() << "Listing (listDirectories): " << dirList << " with TDEIO" << endl;

		TQValueList<KURL*> url_list;

		TQStringList::ConstIterator it = dirList.begin();

		for ( ; it != dirList.end(); it++ )
			url_list.append( new KURL(*it) );

		listURLs( url_list, filter, only_exe, no_hidden );
		// Will call addMatches() and finished()

		return TQString::null;
	}
}

/*
 * listURLs
 *
 * Use TDEIO to list the given urls
 *
 * addMatches() is called with the listed files
 * finished() is called when the listing is done
 */
void KURLCompletion::listURLs(
		const TQValueList<KURL *> &urls,
		const TQString &filter,
		bool only_exe,
		bool no_hidden )
{
	assert( d->list_urls.isEmpty() );
	assert( d->list_job == 0L );

	d->list_urls = urls;
	d->list_urls_filter = filter;
	d->list_urls_only_exe = only_exe;
	d->list_urls_no_hidden = no_hidden;

//	kdDebug() << "Listing URLs: " << urls[0]->prettyURL() << ",..." << endl;

	// Start it off by calling slotIOFinished
	//
	// This will start a new list job as long as there
	// are urls in d->list_urls
	//
	slotIOFinished(0L);
}

/*
 * slotEntries
 *
 * Receive files listed by TDEIO and call addMatches()
 */
void KURLCompletion::slotEntries(TDEIO::Job*, const TDEIO::UDSEntryList& entries)
{
	TQStringList matches;

	TDEIO::UDSEntryListConstIterator it = entries.begin();
	TDEIO::UDSEntryListConstIterator end = entries.end();

	TQString filter = d->list_urls_filter;

	int filter_len = filter.length();

	// Iterate over all files
	//
	for (; it != end; ++it) {
		TQString name;
		TQString url;
		bool is_exe = false;
		bool is_dir = false;

		TDEIO::UDSEntry e = *it;
		TDEIO::UDSEntry::ConstIterator it_2 = e.begin();

		for( ; it_2 != e.end(); it_2++ ) {
			switch ( (*it_2).m_uds ) {
				case TDEIO::UDS_NAME:
					name = (*it_2).m_str;
					break;
				case TDEIO::UDS_ACCESS:
					is_exe = ((*it_2).m_long & MODE_EXE) != 0;
					break;
				case TDEIO::UDS_FILE_TYPE:
					is_dir = ((*it_2).m_long & S_IFDIR) != 0;
					break;
				case TDEIO::UDS_URL:
					url = (*it_2).m_str;
					break;
			}
		}

		if (!url.isEmpty()) {
			// kdDebug() << "KURLCompletion::slotEntries url: " << url << endl;
			name = KURL(url).fileName();
		}

		// kdDebug() << "KURLCompletion::slotEntries name: " << name << endl;

		if ( name[0] == '.' &&
		     ( d->list_urls_no_hidden ||
		        name.length() == 1 ||
		          ( name.length() == 2 && name[1] == '.' ) ) )
			continue;

		if ( d->mode == DirCompletion && !is_dir )
			continue;

		if ( filter_len == 0 || name.left(filter_len) == filter ) {
			if ( is_dir )
				name.append( '/' );

			if ( is_exe || !d->list_urls_only_exe )
				matches.append( name );
		}
	}

	addMatches( matches );
}

/*
 * slotIOFinished
 *
 * Called when a TDEIO job is finished.
 *
 * Start a new list job if there are still urls in
 * d->list_urls, otherwise call finished()
 */
void KURLCompletion::slotIOFinished( TDEIO::Job * job )
{
//	kdDebug() << "slotIOFinished() " << endl;

	assert( job == d->list_job );

	if ( d->list_urls.isEmpty() ) {

		d->list_job = 0L;

		finished(); // will call TDECompletion::makeCompletion()

	}
	else {

		KURL *kurl = d->list_urls.first();

		d->list_urls.remove( kurl );

//		kdDebug() << "Start TDEIO: " << kurl->prettyURL() << endl;

		d->list_job = TDEIO::listDir( *kurl, false );
		d->list_job->addMetaData("no-auth-prompt", "true");

		assert( d->list_job );

		connect( d->list_job,
				TQ_SIGNAL(result(TDEIO::Job*)),
				TQ_SLOT(slotIOFinished(TDEIO::Job*)) );

		connect( d->list_job,
				TQ_SIGNAL( entries( TDEIO::Job*, const TDEIO::UDSEntryList&)),
				TQ_SLOT( slotEntries( TDEIO::Job*, const TDEIO::UDSEntryList&)) );

		delete kurl;
	}
}

///////////////////////////////////////////////////
///////////////////////////////////////////////////

/*
 * postProcessMatch, postProcessMatches
 *
 * Called by TDECompletion before emitting match() and matches()
 *
 * Append '/' to directories for file completion. This is
 * done here to avoid stat()'ing a lot of files
 */
void KURLCompletion::postProcessMatch( TQString *match ) const
{
//	kdDebug() << "KURLCompletion::postProcess: " << *match << endl;

	if ( !match->isEmpty() ) {

		// Add '/' to directories in file completion mode
		// unless it has already been done
		if ( d->last_compl_type == CTFile )
			adjustMatch( *match );
	}
}

void KURLCompletion::adjustMatch( TQString& match ) const
{
	if ( match.at( match.length()-1 ) != '/' )
	{
		TQString copy;

		if ( match.startsWith( TQString("file:") ) )
			copy = KURL(match).path();
		else
			copy = match;

		expandTilde( copy );
		expandEnv( copy );
		if ( TQDir::isRelativePath(copy) )
			copy.prepend( d->cwd + '/' );

//		kdDebug() << "postProcess: stating " << copy << endl;

		KDE_struct_stat sbuff;

		TQCString file = TQFile::encodeName( copy );

		if ( KDE_stat( (const char*)file, &sbuff ) == 0 ) {
			if ( S_ISDIR ( sbuff.st_mode ) )
				match.append( '/' );
		}
		else {
			kdDebug() << "Could not stat file " << copy << endl;
		}
	}
}

void KURLCompletion::postProcessMatches( TQStringList * matches ) const
{
	if ( !matches->isEmpty() && d->last_compl_type == CTFile ) {
		TQStringList::Iterator it = matches->begin();
		for (; it != matches->end(); ++it ) {
			adjustMatch( (*it) );
		}
	}
}

void KURLCompletion::postProcessMatches( TDECompletionMatches * matches ) const
{
	if ( !matches->isEmpty() && d->last_compl_type == CTFile ) {
		TDECompletionMatches::Iterator it = matches->begin();
		for (; it != matches->end(); ++it ) {
			adjustMatch( (*it).value() );
		}
	}
}

void KURLCompletion::customEvent(TQCustomEvent *e)
{
	if ( e->type() == CompletionMatchEvent::uniqueType() ) {

		CompletionMatchEvent *event = static_cast<CompletionMatchEvent *>( e );

		event->completionThread()->wait();

		if ( !isListedURL( CTUser ) ) {
			stop();
			clear();
			addMatches( event->completionThread()->matches() );
		}

		setListedURL( CTUser );

		if ( d->userListThread == event->completionThread() )
			d->userListThread = 0;

		if ( d->dirListThread == event->completionThread() )
			d->dirListThread = 0;

		delete event->completionThread();
	}
}

// static
TQString KURLCompletion::replacedPath( const TQString& text, bool replaceHome, bool replaceEnv )
{
	if ( text.isEmpty() )
		return text;

	MyURL url( text, TQString::null ); // no need to replace something of our current cwd
	if ( !url.kurl()->isLocalFile() )
		return text;

	url.filter( replaceHome, replaceEnv );
	return url.dir() + url.file();
}


TQString KURLCompletion::replacedPath( const TQString& text )
{
	return replacedPath( text, d->replace_home, d->replace_env );
}

/////////////////////////////////////////////////////////
/////////////////////////////////////////////////////////
// Static functions

/*
 * expandEnv
 *
 * Expand environment variables in text. Escaped '$' are ignored.
 * Return true if expansion was made.
 */
static bool expandEnv( TQString &text )
{
	// Find all environment variables beginning with '$'
	//
	int pos = 0;

	bool expanded = false;

	while ( (pos = text.find('$', pos)) != -1 ) {

		// Skip escaped '$'
		//
		if ( text[pos-1] == '\\' ) {
			pos++;
		}
		// Variable found => expand
		//
		else {
			// Find the end of the variable = next '/' or ' '
			//
			int pos2 = text.find( ' ', pos+1 );
			int pos_tmp = text.find( '/', pos+1 );

			if ( pos2 == -1 || (pos_tmp != -1 && pos_tmp < pos2) )
				pos2 = pos_tmp;

			if ( pos2 == -1 )
				pos2 = text.length();

			// Replace if the variable is terminated by '/' or ' '
			// and defined
			//
			if ( pos2 >= 0 ) {
				int len	= pos2 - pos;
				TQString key	= text.mid( pos+1, len-1);
				TQString value =
					TQString::fromLocal8Bit( ::getenv(key.local8Bit()) );

				if ( !value.isEmpty() ) {
					expanded = true;
					text.replace( pos, len, value );
					pos = pos + value.length();
				}
				else {
					pos = pos2;
				}
			}
		}
	}

	return expanded;
}

/*
 * expandTilde
 *
 * Replace "~user" with the users home directory
 * Return true if expansion was made.
 */
static bool expandTilde(TQString &text)
{
	if ( text[0] != '~' )
		return false;

	bool expanded = false;

	// Find the end of the user name = next '/' or ' '
	//
	int pos2 = text.find( ' ', 1 );
	int pos_tmp = text.find( '/', 1 );

	if ( pos2 == -1 || (pos_tmp != -1 && pos_tmp < pos2) )
		pos2 = pos_tmp;

	if ( pos2 == -1 )
		pos2 = text.length();

	// Replace ~user if the user name is terminated by '/' or ' '
	//
	if ( pos2 >= 0 ) {

		TQString user = text.mid( 1, pos2-1 );
		TQString dir;

		// A single ~ is replaced with $HOME
		//
		if ( user.isEmpty() ) {
			dir = TQDir::homeDirPath();
		}
		// ~user is replaced with the dir from passwd
		//
		else {
			struct passwd *pw = ::getpwnam( user.local8Bit() );

			if ( pw )
				dir = TQFile::decodeName( pw->pw_dir );

			::endpwent();
		}

		if ( !dir.isEmpty() ) {
			expanded = true;
			text.replace(0, pos2, dir);
		}
	}

	return expanded;
}

/*
 * unescape
 *
 * Remove escapes and return the result in a new string
 *
 */
static TQString unescape(const TQString &text)
{
	TQString result;

	for (uint pos = 0; pos < text.length(); pos++)
		if ( text[pos] != '\\' )
			result.insert( result.length(), text[pos] );

	return result;
}

void KURLCompletion::virtual_hook( int id, void* data )
{ TDECompletion::virtual_hook( id, data ); }

#include "kurlcompletion.moc"

