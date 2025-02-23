/* This file is part of the KDE project
 *
 * Copyright (C) 2002 David Faure <faure@kde.org>
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License version 2, as published by the Free Software Foundation.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */

#ifndef tdeparts_browserrun_h
#define tdeparts_browserrun_h

#include <krun.h>
#include <kservice.h>
#include <tdeparts/browserextension.h>

namespace KParts {

    /**
     * This class extends KRun to provide additional functionality for browsers:
     * <ul>
     * <li>"save or open" dialog boxes
     * <li>"save" functionality
     * <li>support for HTTP POST (including saving the result to a temp file if
     *   opening a separate application)
     * <li>warning before launching executables off the web
     * <li>custom error handling (i.e. treating errors as HTML pages)
     * <li>generation of SSL metadata depending on the previous URL shown by the part
     * </ul>
     *
     * @author David Faure <faure@kde.org>
     */
    class TDEPARTS_EXPORT BrowserRun : public KRun
    {
        TQ_OBJECT
    public:
        /**
         * @param url the URL we're probing
         * @param args URL args - includes data for a HTTP POST, etc.
         * @param part the part going to open this URL - can be 0L if not created yet
         * @param window the mainwindow - passed to TDEIO::Job::setWindow()
         * @param removeReferrer if true, the "referrer" metadata from @p args isn't passed on
         * @param trustedSource if false, a warning will be shown before launching an executable
         * Always pass false for @p trustedSource, except for local directory views.
         */
        BrowserRun( const KURL& url, const KParts::URLArgs& args,
                    KParts::ReadOnlyPart *part, TQWidget *window,
                    bool removeReferrer, bool trustedSource );

        // BIC: merge with above constructor
        /**
         * @param url the URL we're probing
         * @param args URL args - includes data for a HTTP POST, etc.
         * @param part the part going to open this URL - can be 0L if not created yet
         * @param window the mainwindow - passed to TDEIO::Job::setWindow()
         * @param removeReferrer if true, the "referrer" metadata from @p args isn't passed on
         * @param trustedSource if false, a warning will be shown before launching an executable.
         * Always pass false for @p  trustedSource, except for local directory views.
         * @param hideErrorDialog if true, no dialog will be shown in case of errors.
         * 
         */
        BrowserRun( const KURL& url, const KParts::URLArgs& args,
                    KParts::ReadOnlyPart *part, TQWidget *window,
                    bool removeReferrer, bool trustedSource, bool hideErrorDialog );

        virtual ~BrowserRun();

        //KParts::URLArgs urlArgs() const { return m_args; }
        //KParts::ReadOnlyPart* part() const { return m_part; }

	/**
	 * @return the URL we're probing
	 */
        KURL url() const { return m_strURL; }

	/**
	 * @return true if no dialog will be shown in case of errors
	 */
        bool hideErrorDialog() const;

        /**
	 * @return Suggested filename given by the server (e.g. HTTP content-disposition filename)
	 */
        TQString suggestedFilename() const { return m_suggestedFilename; }

        /**
	 * @return Suggested disposition by the server (e.g. HTTP content-disposition)
         * @since 3.5.2
	 */
        TQString contentDisposition() const;

        bool serverSuggestsSave() const { return contentDisposition() == TQString::fromLatin1("attachment"); }

        enum AskSaveResult { Save, Open, Cancel };
        /**
         * Ask the user whether to save or open a url in another application.
         * @param url the URL in question
         * @param offer the application that will be used to open the URL
         * @param mimeType the mimetype of the URL
         * @param suggestedFilename optional filename suggested by the server
         * @return Save, Open or Cancel.
         */
        static AskSaveResult askSave( const KURL & url, KService::Ptr offer, const TQString& mimeType, const TQString & suggestedFilename = TQString::null );

        enum AskEmbedOrSaveFlags { InlineDisposition = 0, AttachmentDisposition = 1 };
        /**
         * Similar to askSave() but for the case where the current application is
         * able to embed the url itself (instead of passing it to another app).
         * @param url the URL in question
         * @param mimeType the mimetype of the URL
         * @param suggestedFilename optional filename suggested by the server
         * @param flags set to AttachmentDisposition if suggested by the server
         * @return Save, Open or Cancel.
         */
        static AskSaveResult askEmbedOrSave( const KURL & url, const TQString& mimeType, const TQString & suggestedFilename = TQString::null, int flags = 0 );

        // virtual so that TDEHTML can implement differently (HTML cache)
        virtual void save( const KURL & url, const TQString & suggestedFilename );

        // static so that it can be called from other classes
        static void simpleSave( const KURL & url, const TQString & suggestedFilename,
                                TQWidget* window );

        /** BIC: Combine with the above function for KDE 4.0. */
        static void simpleSave( const KURL & url, const TQString & suggestedFilename );

        static bool allowExecution( const TQString &serviceType, const KURL &url );

        /** BIC: Obsoleted by KRun::isExecutable( const TQString &serviceType ); */
        static bool isExecutable( const TQString &serviceType );
        static bool isTextExecutable( const TQString &serviceType );

    protected:
        /**
         * Reimplemented from KRun
         */
        virtual void scanFile();
        /**
         * Reimplemented from KRun
         */
        virtual void init();
        /**
         * Called when an error happens.
         * NOTE: @p job could be 0L, if you passed hideErrorDialog=true.
         * The default implementation shows a message box, but only when job != 0 ....
         * It is strongly recommended to reimplement this method if
         * you passed hideErrorDialog=true.
         */
        virtual void handleError( TDEIO::Job * job );

        /**
         * NotHandled means that foundMimeType should call KRun::foundMimeType,
         * i.e. launch an external app.
         */
        enum NonEmbeddableResult { Handled, NotHandled, Delayed };

        /**
         * Helper for foundMimeType: call this if the mimetype couldn't be embedded
         */
        NonEmbeddableResult handleNonEmbeddable( const TQString& mimeType );

    protected slots:
        void slotBrowserScanFinished(TDEIO::Job *job);
        void slotBrowserMimetype(TDEIO::Job *job, const TQString &type);
        void slotCopyToTempFileResult(TDEIO::Job *job);
        virtual void slotStatResult( TDEIO::Job *job );

    protected:
        KParts::URLArgs m_args;
        KParts::ReadOnlyPart *m_part; // QGuardedPtr?
        TQGuardedPtr<TQWidget> m_window;
        // Suggested filename given by the server (e.g. HTTP content-disposition)
        // When set, we should really be saving instead of embedding
        TQString m_suggestedFilename;
        TQString m_sMimeType;
        bool m_bRemoveReferrer;
        bool m_bTrustedSource;
    private:
        void redirectToError( int error, const TQString& errorText );
        class BrowserRunPrivate;
        BrowserRunPrivate* d;

    };
}
#endif
