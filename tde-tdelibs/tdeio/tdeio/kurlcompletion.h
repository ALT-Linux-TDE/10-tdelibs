/* This file is part of the KDE libraries
    Copyright (C) 2000 David Smith  <dsmith@algonet.se>

    This class was inspired by a previous KURLCompletion by
    Henner Zeller <zeller@think.de>

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

#ifndef KURLCOMPLETION_H
#define KURLCOMPLETION_H

#include <kcompletion.h>
#include <tdeio/jobclasses.h>
#include <tqstring.h>
#include <tqstringlist.h>

class KURL;
class KURLCompletionPrivate;

/**
 * This class does completion of URLs including user directories (~user)
 * and environment variables.  Remote URLs are passed to TDEIO.
 *
 * @short Completion of a single URL
 * @author David Smith <dsmith@algonet.se>
 */
class TDEIO_EXPORT KURLCompletion : public TDECompletion
{
	TQ_OBJECT

public:
	/**
	 * Determines how completion is done.
	 * @li ExeCompletion - executables in $PATH or with full path.
	 * @li FileCompletion - all files with full path or in dir(), URLs
	 * are listed using TDEIO.
	 * @li DirCompletion - Same as FileCompletion but only returns directories.
	 */
	enum Mode { ExeCompletion=1, FileCompletion, DirCompletion, SystemExeCompletion };

	/**
	 * Constructs a KURLCompletion object in FileCompletion mode.
	 */
	KURLCompletion();
	/**
	 * This overloaded constructor allows you to set the Mode to ExeCompletion
	 * or FileCompletion without using setMode. Default is FileCompletion.
	 */
	KURLCompletion(Mode);
	/**
	 * Destructs the KURLCompletion object.
	 */
	virtual ~KURLCompletion();

	/**
	 * Finds completions to the given text.
	 *
	 * Remote URLs are listed with TDEIO. For performance reasons, local files
	 * are listed with TDEIO only if KURLCOMPLETION_LOCAL_TDEIO is set.
	 * The completion is done asyncronously if TDEIO is used.
	 *
	 * Returns the first match for user, environment, and local dir completion
	 * and TQString::null for asynchronous completion (TDEIO or threaded).
	 *
	 * @param text the text to complete
	 * @return the first match, or TQString::null if not found
	 */
	virtual TQString makeCompletion(const TQString &text); // KDE4: remove return value, it's often null due to threading

	/**
	 * Sets the current directory (used as base for completion).
	 * Default = $HOME.
	 * @param dir the current directory, either as a path or URL
	 */
	virtual void setDir(const TQString &dir);

	/**
	 * Returns the current directory, as it was given in setDir
	 * @return the current directory (path or URL)
	 */
	virtual TQString dir() const;

	/**
	 * Check whether asynchronous completion is in progress.
	 * @return true if asynchronous completion is in progress
	 */
	virtual bool isRunning() const;

	/**
	 * Stops asynchronous completion.
	 */
	virtual void stop();

	/**
	 * Returns the completion mode: exe or file completion (default FileCompletion).
	 * @return the completion mode
	 */
	virtual Mode mode() const;

	/**
	 * Changes the completion mode: exe or file completion
	 * @param mode the new completion mode
	 */
	virtual void setMode( Mode mode );

	/**
	 * Checks whether environment variables are completed and
	 * whether they are replaced internally while finding completions.
	 * Default is enabled.
	 * @return true if environment vvariables will be replaced
	 */
	virtual bool replaceEnv() const;

	/**
	 * Enables/disables completion and replacement (internally) of
	 * environment variables in URLs. Default is enabled.
	 * @param replace true to replace environment variables
	 */
	virtual void setReplaceEnv( bool replace );

	/**
	 * Returns whether ~username is completed and whether ~username
	 * is replaced internally with the user's home directory while
	 * finding completions. Default is enabled.
	 * @return true to replace tilde with the home directory
	 */
	virtual bool replaceHome() const;

	/**
	 * Enables/disables completion of ~username and replacement
	 * (internally) of ~username with the user's home directory.
	 * Default is enabled.
	 * @param replace true to replace tilde with the home directory
	 */
	virtual void setReplaceHome( bool replace );

	/**
	 * Replaces username and/or environment variables, depending on the
	 * current settings and returns the filtered url. Only works with
	 * local files, i.e. returns back the original string for non-local
	 * urls.
	 * @param text the text to process
	 * @return the path or URL resulting from this operation. If you
         * want to convert it to a KURL, use KURL::fromPathOrURL.
	 */
	TQString replacedPath( const TQString& text );

	/**
	 * @internal I'll let ossi add a real one to KShell :)
	 * @since 3.2
	*/
	static TQString replacedPath( const TQString& text,
                                     bool replaceHome, bool replaceEnv = true );

	class MyURL;
protected:
	// Called by TDECompletion, adds '/' to directories
	void postProcessMatch( TQString *match ) const;
	void postProcessMatches( TQStringList *matches ) const;
	void postProcessMatches( TDECompletionMatches* matches ) const;

	virtual void customEvent( TQCustomEvent *e );

protected slots:
	void slotEntries( TDEIO::Job *, const TDEIO::UDSEntryList& );
	void slotIOFinished( TDEIO::Job * );

private:

	bool isAutoCompletion();

	bool userCompletion(const MyURL &url, TQString *match);
	bool envCompletion(const MyURL &url, TQString *match);
	bool exeCompletion(const MyURL &url, TQString *match);
	bool systemexeCompletion(const MyURL &url, TQString *match);
	bool fileCompletion(const MyURL &url, TQString *match);
	bool urlCompletion(const MyURL &url, TQString *match);

	// List a directory using readdir()
	void listDir( const TQString& dir,
	              TQStringList *matches,
	              const TQString& filter,
	              bool only_exe,
	              bool no_hidden );

	// List the next dir in m_dirs
	TQString listDirectories(const TQStringList &,
	                        const TQString &,
	                        bool only_exe = false,
	                        bool only_dir = false,
	                        bool no_hidden = false,
	                        bool stat_files = true);

	void listURLs( const TQValueList<KURL *> &urls,
	               const TQString &filter = TQString::null,
	               bool only_exe = false,
	               bool no_hidden = false );

	void addMatches( const TQStringList & );
	TQString finished();

	void init();

	void setListedURL(int compl_type /* enum ComplType */,
	                  const TQString& dir = TQString::null,
	                  const TQString& filter = TQString::null,
	                  bool no_hidden = false );

	bool isListedURL( int compl_type /* enum ComplType */,
	                  const TQString& dir = TQString::null,
	                  const TQString& filter = TQString::null,
	                  bool no_hidden = false );

	void adjustMatch( TQString& match ) const;

protected:
	virtual void virtual_hook( int id, void* data );
private:
	KURLCompletionPrivate *d;
};

#endif // KURLCOMPLETION_H
