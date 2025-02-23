/* This file is part of the KDE project
   Copyright (C) 1999 David Faure <faure@kde.org>

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

#ifndef __konq_htmlsettings_h__
#define __konq_htmlsettings_h__

class TDEConfig;
#include <tqcolor.h>
#include <tqstring.h>
#include <tqstringlist.h>
#include <tqfont.h>
#include <tqmap.h>

#include <tdelibs_export.h>

struct KPerDomainSettings;
class TDEHTMLSettingsPrivate;

/**
 * Settings for the HTML view.
 */
class TDEHTML_EXPORT TDEHTMLSettings
{
public:

    /**
     * This enum specifies whether Java/JavaScript execution is allowed.
     */
    enum KJavaScriptAdvice {
	KJavaScriptDunno=0,
	KJavaScriptAccept,
	KJavaScriptReject
    };

    enum KAnimationAdvice {
        KAnimationDisabled=0,
        KAnimationLoopOnce,
        KAnimationEnabled
    };

    /**
     * This enum specifies the policy for window.open
     */
    enum KJSWindowOpenPolicy {
    	KJSWindowOpenAllow=0,
    	KJSWindowOpenAsk,
    	KJSWindowOpenDeny,
    	KJSWindowOpenSmart
    };

    /**
     * This enum specifies the policy for window.status and .defaultStatus
     */
    enum KJSWindowStatusPolicy {
    	KJSWindowStatusAllow=0,
    	KJSWindowStatusIgnore
    };

    /**
     * This enum specifies the policy for window.moveBy and .moveTo
     */
    enum KJSWindowMovePolicy {
    	KJSWindowMoveAllow=0,
    	KJSWindowMoveIgnore
    };

    /**
     * This enum specifies the policy for window.resizeBy and .resizeTo
     */
    enum KJSWindowResizePolicy {
    	KJSWindowResizeAllow=0,
    	KJSWindowResizeIgnore
    };

    /**
     * This enum specifies the policy for window.focus
     */
    enum KJSWindowFocusPolicy {
    	KJSWindowFocusAllow=0,
    	KJSWindowFocusIgnore
    };

    /**
     * @internal Constructor
     */
    TDEHTMLSettings();
    TDEHTMLSettings(const TDEHTMLSettings &other);

    /**
     * Called by constructor and reparseConfiguration
     */
    void init();

    /** Read settings from @p config.
     * @param config is a pointer to TDEConfig object.
     * @param reset if true, settings are always set; if false,
     *  settings are only set if the config file has a corresponding key.
     */
    void init( TDEConfig * config, bool reset = true );

    /**
     * Destructor. Don't delete any instance by yourself.
     */
    virtual ~TDEHTMLSettings();

    // Behavior settings
    bool changeCursor() const;
    bool underlineLink() const;
    bool hoverLink() const;
    bool allowTabulation() const;
    bool autoSpellCheck() const;
    KAnimationAdvice showAnimations() const;

    // Font settings
    TQString stdFontName() const;
    TQString fixedFontName() const;
    TQString serifFontName() const;
    TQString sansSerifFontName() const;
    TQString cursiveFontName() const;
    TQString fantasyFontName() const;

    // these two can be set. Mainly for historical reasons (the method in TDEHTMLPart exists...)
    void setStdFontName(const TQString &n);
    void setFixedFontName(const TQString &n);

    int minFontSize() const;
    int mediumFontSize() const;

    bool jsErrorsEnabled() const;
    void setJSErrorsEnabled(bool enabled);

    const TQString &encoding() const;

    bool followSystemColors() const;

    // Color settings
    const TQColor& textColor() const;
    const TQColor& baseColor() const;
    const TQColor& linkColor() const;
    const TQColor& vLinkColor() const;

    // Autoload images
    bool autoLoadImages() const;
    bool unfinishedImageFrame() const;

    bool isOpenMiddleClickEnabled();
    bool isBackRightClickEnabled();

    // Java and JavaScript
    // ### BIC make these const
    bool isJavaEnabled( const TQString& hostname = TQString::null );
    bool isJavaScriptEnabled( const TQString& hostname = TQString::null );
    bool isJavaScriptDebugEnabled( const TQString& hostname = TQString::null );
    bool isJavaScriptErrorReportingEnabled( const TQString& hostname = TQString::null ) const;
    bool isPluginsEnabled( const TQString& hostname = TQString::null );

    // AdBlocK Filtering
    bool isAdFiltered( const TQString &url ) const;
    bool isAdFilterEnabled() const;
    bool isHideAdsEnabled() const;
    void addAdFilter( const TQString &url );

    // Access Keys
    bool accessKeysEnabled() const;

    KJSWindowOpenPolicy windowOpenPolicy( const TQString& hostname = TQString::null ) const;
    KJSWindowMovePolicy windowMovePolicy( const TQString& hostname = TQString::null ) const;
    KJSWindowResizePolicy windowResizePolicy( const TQString& hostname = TQString::null ) const;
    KJSWindowStatusPolicy windowStatusPolicy( const TQString& hostname = TQString::null ) const;
    KJSWindowFocusPolicy windowFocusPolicy( const TQString& hostname = TQString::null ) const;

    // helpers for parsing domain-specific configuration, used in KControl module as well
    static KJavaScriptAdvice strToAdvice(const TQString& _str);
    static void splitDomainAdvice(const TQString& configStr, TQString &domain,
				  KJavaScriptAdvice &javaAdvice, KJavaScriptAdvice& javaScriptAdvice);
    static const char* adviceToStr(KJavaScriptAdvice _advice);

    /** reads from @p config's current group, forcing initialization
      * if @p reset is true.
      * @param config is a pointer to TDEConfig object.
      * @param reset true if initialization is to be forced.
      * @param global true if the global domain is to be read.
      * @param pd_settings will be initialised with the computed (inherited)
      *		settings.
      */
    void readDomainSettings(TDEConfig *config, bool reset,
			bool global, KPerDomainSettings &pd_settings);

    TQString settingsToCSS() const;
    static const TQString &availableFamilies();

    TQString userStyleSheet() const;

    // Form completion
    bool isFormCompletionEnabled() const;
    int maxFormCompletionItems() const;

    // Meta refresh/redirect (http-equiv)
    bool isAutoDelayedActionsEnabled () const;

    TQValueList< TQPair< TQString, TQChar > > fallbackAccessKeysAssignments() const;

    // Whether to show passive popup when windows are blocked
    // @since 3.5
    void setJSPopupBlockerPassivePopup(bool enabled);
    bool jsPopupBlockerPassivePopup() const;

private:
    friend class TDEHTMLFactory;
    TQString lookupFont(int i) const;

    TDEHTMLSettingsPrivate *d;
    static TQString *avFamilies;
};

#endif
