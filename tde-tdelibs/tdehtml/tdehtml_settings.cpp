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

#include <tqfontdatabase.h>

#include "tdehtml_settings.h"
#include "tdehtmldefaults.h"
#include <tdeglobalsettings.h>
#include <tdeconfig.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <tqregexp.h>
#include <tqvaluevector.h>
#include <tdemessagebox.h>

/**
 * @internal
 * Contains all settings which are both available globally and per-domain
 */
struct KPerDomainSettings {
    bool m_bEnableJava : 1;
    bool m_bEnableJavaScript : 1;
    bool m_bEnablePlugins : 1;
    // don't forget to maintain the bitfields as the enums grow
    TDEHTMLSettings::KJSWindowOpenPolicy m_windowOpenPolicy : 2;
    TDEHTMLSettings::KJSWindowStatusPolicy m_windowStatusPolicy : 1;
    TDEHTMLSettings::KJSWindowFocusPolicy m_windowFocusPolicy : 1;
    TDEHTMLSettings::KJSWindowMovePolicy m_windowMovePolicy : 1;
    TDEHTMLSettings::KJSWindowResizePolicy m_windowResizePolicy : 1;

#ifdef DEBUG_SETTINGS
    void dump(const TQString &infix = TQString::null) const {
      kdDebug() << "KPerDomainSettings " << infix << " @" << this << ":" << endl;
      kdDebug() << "  m_bEnableJava: " << m_bEnableJava << endl;
      kdDebug() << "  m_bEnableJavaScript: " << m_bEnableJavaScript << endl;
      kdDebug() << "  m_bEnablePlugins: " << m_bEnablePlugins << endl;
      kdDebug() << "  m_windowOpenPolicy: " << m_windowOpenPolicy << endl;
      kdDebug() << "  m_windowStatusPolicy: " << m_windowStatusPolicy << endl;
      kdDebug() << "  m_windowFocusPolicy: " << m_windowFocusPolicy << endl;
      kdDebug() << "  m_windowMovePolicy: " << m_windowMovePolicy << endl;
      kdDebug() << "  m_windowResizePolicy: " << m_windowResizePolicy << endl;
    }
#endif
};

typedef TQMap<TQString,KPerDomainSettings> PolicyMap;

class TDEHTMLSettingsPrivate
{
public:
    bool m_bChangeCursor : 1;
    bool m_bOpenMiddleClick : 1;
    bool m_bBackRightClick : 1;
    bool m_underlineLink : 1;
    bool m_hoverLink : 1;
    bool m_bEnableJavaScriptDebug : 1;
    bool m_bEnableJavaScriptErrorReporting : 1;
    bool enforceCharset : 1;
    bool m_bAutoLoadImages : 1;
    bool m_bUnfinishedImageFrame : 1;
    bool m_formCompletionEnabled : 1;
    bool m_autoDelayedActionsEnabled : 1;
    bool m_jsErrorsEnabled : 1;
    bool m_follow_system_colors : 1;
    bool m_allowTabulation : 1;
    bool m_autoSpellCheck : 1;
    bool m_adFilterEnabled : 1;
    bool m_hideAdsEnabled : 1;
    bool m_jsPopupBlockerPassivePopup : 1;
    bool m_accessKeysEnabled : 1;

    // the virtual global "domain"
    KPerDomainSettings global;

    int m_fontSize;
    int m_minFontSize;
    int m_maxFormCompletionItems;
    TDEHTMLSettings::KAnimationAdvice m_showAnimations;

    TQString m_encoding;
    TQString m_userSheet;

    TQColor m_textColor;
    TQColor m_baseColor;
    TQColor m_linkColor;
    TQColor m_vLinkColor;

    PolicyMap domainPolicy;
    TQStringList fonts;
    TQStringList defaultFonts;

    TQValueVector<TQRegExp> adFilters;
    TQValueList< TQPair< TQString, TQChar > > m_fallbackAccessKeysAssignments;
};


/** Returns a writeable per-domains settings instance for the given domain
  * or a deep copy of the global settings if not existent.
  */
static KPerDomainSettings &setup_per_domain_policy(
				TDEHTMLSettingsPrivate *d,
				const TQString &domain) {
  if (domain.isEmpty()) {
    kdWarning() << "setup_per_domain_policy: domain is empty" << endl;
  }
  const TQString ldomain = domain.lower();
  PolicyMap::iterator it = d->domainPolicy.find(ldomain);
  if (it == d->domainPolicy.end()) {
    // simply copy global domain settings (they should have been initialized
    // by this time)
    it = d->domainPolicy.insert(ldomain,d->global);
  }
  return *it;
}


TDEHTMLSettings::KJavaScriptAdvice TDEHTMLSettings::strToAdvice(const TQString& _str)
{
  KJavaScriptAdvice ret = KJavaScriptDunno;

  if (!_str)
        ret = KJavaScriptDunno;

  if (_str.lower() == TQString::fromLatin1("accept"))
        ret = KJavaScriptAccept;
  else if (_str.lower() == TQString::fromLatin1("reject"))
        ret = KJavaScriptReject;

  return ret;
}

const char* TDEHTMLSettings::adviceToStr(KJavaScriptAdvice _advice)
{
    switch( _advice ) {
    case KJavaScriptAccept: return I18N_NOOP("Accept");
    case KJavaScriptReject: return I18N_NOOP("Reject");
    default: return 0;
    }
        return 0;
}


void TDEHTMLSettings::splitDomainAdvice(const TQString& configStr, TQString &domain,
                                      KJavaScriptAdvice &javaAdvice, KJavaScriptAdvice& javaScriptAdvice)
{
    TQString tmp(configStr);
    int splitIndex = tmp.find(':');
    if ( splitIndex == -1)
    {
        domain = configStr.lower();
        javaAdvice = KJavaScriptDunno;
        javaScriptAdvice = KJavaScriptDunno;
    }
    else
    {
        domain = tmp.left(splitIndex).lower();
        TQString adviceString = tmp.mid( splitIndex+1, tmp.length() );
        int splitIndex2 = adviceString.find( ':' );
        if( splitIndex2 == -1 ) {
            // Java advice only
            javaAdvice = strToAdvice( adviceString );
            javaScriptAdvice = KJavaScriptDunno;
        } else {
            // Java and JavaScript advice
            javaAdvice = strToAdvice( adviceString.left( splitIndex2 ) );
            javaScriptAdvice = strToAdvice( adviceString.mid( splitIndex2+1,
                                                              adviceString.length() ) );
        }
    }
}

void TDEHTMLSettings::readDomainSettings(TDEConfig *config, bool reset,
	bool global, KPerDomainSettings &pd_settings) {
  TQString jsPrefix = global ? TQString::null
  				: TQString::fromLatin1("javascript.");
  TQString javaPrefix = global ? TQString::null
  				: TQString::fromLatin1("java.");
  TQString pluginsPrefix = global ? TQString::null
  				: TQString::fromLatin1("plugins.");

  // The setting for Java
  TQString key = javaPrefix + TQString::fromLatin1("EnableJava");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnableJava = config->readBoolEntry( key, false );
  else if ( !global )
    pd_settings.m_bEnableJava = d->global.m_bEnableJava;

  // The setting for Plugins
  key = pluginsPrefix + TQString::fromLatin1("EnablePlugins");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnablePlugins = config->readBoolEntry( key, true );
  else if ( !global )
    pd_settings.m_bEnablePlugins = d->global.m_bEnablePlugins;

  // The setting for JavaScript
  key = jsPrefix + TQString::fromLatin1("EnableJavaScript");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_bEnableJavaScript = config->readBoolEntry( key, true );
  else if ( !global )
    pd_settings.m_bEnableJavaScript = d->global.m_bEnableJavaScript;

  // window property policies
  key = jsPrefix + TQString::fromLatin1("WindowOpenPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowOpenPolicy = (KJSWindowOpenPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowOpenSmart );
  else if ( !global )
    pd_settings.m_windowOpenPolicy = d->global.m_windowOpenPolicy;

  key = jsPrefix + TQString::fromLatin1("WindowMovePolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowMovePolicy = (KJSWindowMovePolicy)
    		config->readUnsignedNumEntry( key, KJSWindowMoveAllow );
  else if ( !global )
    pd_settings.m_windowMovePolicy = d->global.m_windowMovePolicy;

  key = jsPrefix + TQString::fromLatin1("WindowResizePolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowResizePolicy = (KJSWindowResizePolicy)
    		config->readUnsignedNumEntry( key, KJSWindowResizeAllow );
  else if ( !global )
    pd_settings.m_windowResizePolicy = d->global.m_windowResizePolicy;

  key = jsPrefix + TQString::fromLatin1("WindowStatusPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowStatusPolicy = (KJSWindowStatusPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowStatusAllow );
  else if ( !global )
    pd_settings.m_windowStatusPolicy = d->global.m_windowStatusPolicy;

  key = jsPrefix + TQString::fromLatin1("WindowFocusPolicy");
  if ( (global && reset) || config->hasKey( key ) )
    pd_settings.m_windowFocusPolicy = (KJSWindowFocusPolicy)
    		config->readUnsignedNumEntry( key, KJSWindowFocusAllow );
  else if ( !global )
    pd_settings.m_windowFocusPolicy = d->global.m_windowFocusPolicy;

}


TDEHTMLSettings::TDEHTMLSettings()
{
  d = new TDEHTMLSettingsPrivate();
  init();
}

TDEHTMLSettings::TDEHTMLSettings(const TDEHTMLSettings &other)
{
  d = new TDEHTMLSettingsPrivate();
  *d = *other.d;
}

TDEHTMLSettings::~TDEHTMLSettings()
{
  delete d;
}

bool TDEHTMLSettings::changeCursor() const
{
  return d->m_bChangeCursor;
}

bool TDEHTMLSettings::underlineLink() const
{
  return d->m_underlineLink;
}

bool TDEHTMLSettings::hoverLink() const
{
  return d->m_hoverLink;
}

void TDEHTMLSettings::init()
{
  TDEConfig global( "tdehtmlrc", true, false );
  init( &global, true );

  TDEConfig *local = TDEGlobal::config();
  if ( !local )
    return;

  init( local, false );
}

void TDEHTMLSettings::init( TDEConfig * config, bool reset )
{
  TQString group_save = config->group();
  if (reset || config->hasGroup("MainView Settings"))
  {
    config->setGroup( "MainView Settings" );

    if ( reset || config->hasKey( "OpenMiddleClick" ) )
        d->m_bOpenMiddleClick = config->readBoolEntry( "OpenMiddleClick", true );

    if ( reset || config->hasKey( "BackRightClick" ) )
        d->m_bBackRightClick = config->readBoolEntry( "BackRightClick", false );
  }

  if (reset || config->hasGroup("Access Keys")) {
      config->setGroup( "Access Keys" );
      d->m_accessKeysEnabled = config->readBoolEntry( "Enabled", true );
  }

  if (reset || config->hasGroup("Filter Settings"))
  {
      config->setGroup( "Filter Settings" );
      d->m_adFilterEnabled = config->readBoolEntry("Enabled", false);
      d->m_hideAdsEnabled = config->readBoolEntry("Shrink", false);

      d->adFilters.clear();

      TQMap<TQString,TQString> entryMap = config->entryMap("Filter Settings");
      TQMap<TQString,TQString>::ConstIterator it;
      d->adFilters.reserve(entryMap.count());
      for( it = entryMap.constBegin(); it != entryMap.constEnd(); ++it )
      {
          TQString name = it.key();
          TQString url = it.data();

          if (url.startsWith("!"))
              continue;

          if (name.startsWith("Filter"))
          {
              if (url.length()>2 && url[0]=='/' && url[url.length()-1] == '/')
              {
                  TQString inside = url.mid(1, url.length()-2);
                  TQRegExp rx(inside);
                  d->adFilters.append(rx);
              }
              else
              {
                  TQRegExp rx;
                  int left,right;

                  for (right=url.length(); right>0 && url[right-1]=='*' ; --right);
                  for (left=0; left<right && url[left]=='*' ; ++left);

                  rx.setWildcard(true);
                  rx.setPattern(url.mid(left,right-left));

                  d->adFilters.append(rx);
              }
          }
      }
  }


  if (reset || config->hasGroup("HTML Settings"))
  {
    config->setGroup( "HTML Settings" );
    // Fonts and colors
    if( reset ) {
        d->defaultFonts = TQStringList();
        d->defaultFonts.append( config->readEntry( "StandardFont", TDEGlobalSettings::generalFont().family() ) );
        d->defaultFonts.append( config->readEntry( "FixedFont", TDEGlobalSettings::fixedFont().family() ) );
        d->defaultFonts.append( config->readEntry( "SerifFont", HTML_DEFAULT_VIEW_SERIF_FONT ) );
        d->defaultFonts.append( config->readEntry( "SansSerifFont", HTML_DEFAULT_VIEW_SANSSERIF_FONT ) );
        d->defaultFonts.append( config->readEntry( "CursiveFont", HTML_DEFAULT_VIEW_CURSIVE_FONT ) );
        d->defaultFonts.append( config->readEntry( "FantasyFont", HTML_DEFAULT_VIEW_FANTASY_FONT ) );
        d->defaultFonts.append( TQString( "0" ) ); // font size adjustment
    }

    if ( reset || config->hasKey( "MinimumFontSize" ) )
        d->m_minFontSize = config->readNumEntry( "MinimumFontSize", HTML_DEFAULT_MIN_FONT_SIZE );

    if ( reset || config->hasKey( "MediumFontSize" ) )
        d->m_fontSize = config->readNumEntry( "MediumFontSize", 12 );

    d->fonts = config->readListEntry( "Fonts" );

    if ( reset || config->hasKey( "DefaultEncoding" ) )
        d->m_encoding = config->readEntry( "DefaultEncoding", "" );

    if ( reset || config->hasKey( "EnforceDefaultCharset" ) )
        d->enforceCharset = config->readBoolEntry( "EnforceDefaultCharset", false );

    // Behavior
    if ( reset || config->hasKey( "ChangeCursor" ) )
        d->m_bChangeCursor = config->readBoolEntry( "ChangeCursor", KDE_DEFAULT_CHANGECURSOR );

    if ( reset || config->hasKey("UnderlineLinks") )
        d->m_underlineLink = config->readBoolEntry( "UnderlineLinks", true );

    if ( reset || config->hasKey( "HoverLinks" ) )
    {
        if ( ( d->m_hoverLink = config->readBoolEntry( "HoverLinks", false ) ) )
            d->m_underlineLink = false;
    }

    if ( reset || config->hasKey( "AllowTabulation" ) )
        d->m_allowTabulation = config->readBoolEntry( "AllowTabulation", false );

    if ( reset || config->hasKey( "AutoSpellCheck" ) )
        d->m_autoSpellCheck = config->readBoolEntry( "AutoSpellCheck", true );

    // Other
    if ( reset || config->hasKey( "AutoLoadImages" ) )
      d->m_bAutoLoadImages = config->readBoolEntry( "AutoLoadImages", true );

    if ( reset || config->hasKey( "UnfinishedImageFrame" ) )
      d->m_bUnfinishedImageFrame = config->readBoolEntry( "UnfinishedImageFrame", true );

    if ( reset || config->hasKey( "ShowAnimations" ) )
    {
      TQString value = config->readEntry( "ShowAnimations").lower();
      if (value == "disabled")
         d->m_showAnimations = KAnimationDisabled;
      else if (value == "looponce")
         d->m_showAnimations = KAnimationLoopOnce;
      else
         d->m_showAnimations = KAnimationEnabled;
    }

    if ( config->readBoolEntry( "UserStyleSheetEnabled", false ) == true ) {
        if ( reset || config->hasKey( "UserStyleSheet" ) )
            d->m_userSheet = config->readEntry( "UserStyleSheet", "" );
    }

    d->m_formCompletionEnabled = config->readBoolEntry("FormCompletion", true);
    d->m_maxFormCompletionItems = config->readNumEntry("MaxFormCompletionItems", 10);
    d->m_autoDelayedActionsEnabled = config->readBoolEntry ("AutoDelayedActions", true);
    d->m_jsErrorsEnabled = config->readBoolEntry("ReportJSErrors", true);
    TQStringList accesskeys = config->readListEntry("FallbackAccessKeysAssignments");
    d->m_fallbackAccessKeysAssignments.clear();
    for( TQStringList::ConstIterator it = accesskeys.begin(); it != accesskeys.end(); ++it )
        if( (*it).length() > 2 && (*it)[ 1 ] == ':' )
            d->m_fallbackAccessKeysAssignments.append( qMakePair( (*it).mid( 2 ), (*it)[ 0 ] ));
  }

  // Colors

  if ( reset || config->hasKey( "FollowSystemColors" ) )
      d->m_follow_system_colors = config->readBoolEntry( "FollowSystemColors", false );

  if ( reset || config->hasGroup( "General" ) )
  {
    config->setGroup( "General" ); // group will be restored by cgs anyway
    if ( reset || config->hasKey( "foreground" ) )
      d->m_textColor = config->readColorEntry( "foreground", &HTML_DEFAULT_TXT_COLOR );

    if ( reset || config->hasKey( "linkColor" ) )
      d->m_linkColor = config->readColorEntry( "linkColor", &HTML_DEFAULT_LNK_COLOR );

    if ( reset || config->hasKey( "visitedLinkColor" ) )
      d->m_vLinkColor = config->readColorEntry( "visitedLinkColor", &HTML_DEFAULT_VLNK_COLOR);

    if ( reset || config->hasKey( "background" ) )
      d->m_baseColor = config->readColorEntry( "background", &HTML_DEFAULT_BASE_COLOR);
  }

  if( reset || config->hasGroup( "Java/JavaScript Settings" ) )
  {
    config->setGroup( "Java/JavaScript Settings" );

    // The global setting for JavaScript debugging
    // This is currently always enabled by default
    if ( reset || config->hasKey( "EnableJavaScriptDebug" ) )
      d->m_bEnableJavaScriptDebug = config->readBoolEntry( "EnableJavaScriptDebug", false );

    // The global setting for JavaScript error reporting
    if ( reset || config->hasKey( "ReportJavaScriptErrors" ) )
      d->m_bEnableJavaScriptErrorReporting = config->readBoolEntry( "ReportJavaScriptErrors", false );

    // The global setting for popup block passive popup
    if ( reset || config->hasKey( "PopupBlockerPassivePopup" ) )
      d->m_jsPopupBlockerPassivePopup = config->readBoolEntry("PopupBlockerPassivePopup", true);

    // Read options from the global "domain"
    readDomainSettings(config,reset,true,d->global);
#ifdef DEBUG_SETTINGS
    d->global.dump("init global");
#endif

    // The domain-specific settings.

    static const char *const domain_keys[] = {	// always keep order of keys
    	"ECMADomains", "JavaDomains", "PluginDomains"
    };
    bool check_old_ecma_settings = true;
    bool check_old_java_settings = true;
    // merge all domains into one list
    TQMap<TQString,int> domainList;	// why can't Qt have a QSet?
    for (unsigned i = 0; i < sizeof domain_keys/sizeof domain_keys[0]; ++i) {
      if ( reset || config->hasKey(domain_keys[i]) ) {
        if (i == 0) check_old_ecma_settings = false;
	else if (i == 1) check_old_java_settings = false;
        const TQStringList dl = config->readListEntry( domain_keys[i] );
	const TQMap<TQString,int>::Iterator notfound = domainList.end();
	TQStringList::ConstIterator it = dl.begin();
	const TQStringList::ConstIterator itEnd = dl.end();
	for (; it != itEnd; ++it) {
	  const TQString domain = (*it).lower();
	  TQMap<TQString,int>::Iterator pos = domainList.find(domain);
	  if (pos == notfound) domainList.insert(domain,0);
	}/*next it*/
      }
    }/*next i*/

    if (reset)
      d->domainPolicy.clear();

    TQString js_group_save = config->group();
    {
      TQMap<TQString,int>::ConstIterator it = domainList.begin();
      const TQMap<TQString,int>::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
      {
        const TQString domain = it.key();
        config->setGroup(domain);
        readDomainSettings(config,reset,false,d->domainPolicy[domain]);
#ifdef DEBUG_SETTINGS
        d->domainPolicy[domain].dump("init "+domain);
#endif
      }
    }
    config->setGroup(js_group_save);

    bool check_old_java = true;
    if( ( reset || config->hasKey( "JavaDomainSettings" ) )
    	&& check_old_java_settings )
    {
      check_old_java = false;
      const TQStringList domainList = config->readListEntry( "JavaDomainSettings" );
      TQStringList::ConstIterator it = domainList.begin();
      const TQStringList::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
      {
        TQString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        setup_per_domain_policy(d,domain).m_bEnableJava =
		javaAdvice == KJavaScriptAccept;
#ifdef DEBUG_SETTINGS
	setup_per_domain_policy(d,domain).dump("JavaDomainSettings 4 "+domain);
#endif
      }
    }

    bool check_old_ecma = true;
    if( ( reset || config->hasKey( "ECMADomainSettings" ) )
	&& check_old_ecma_settings )
    {
      check_old_ecma = false;
      const TQStringList domainList = config->readListEntry( "ECMADomainSettings" );
      TQStringList::ConstIterator it = domainList.begin();
      const TQStringList::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
      {
        TQString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        setup_per_domain_policy(d,domain).m_bEnableJavaScript =
			javaScriptAdvice == KJavaScriptAccept;
#ifdef DEBUG_SETTINGS
	setup_per_domain_policy(d,domain).dump("ECMADomainSettings 4 "+domain);
#endif
      }
    }

    if( ( reset || config->hasKey( "JavaScriptDomainAdvice" ) )
             && ( check_old_java || check_old_ecma )
	     && ( check_old_ecma_settings || check_old_java_settings ) )
    {
      const TQStringList domainList = config->readListEntry( "JavaScriptDomainAdvice" );
      TQStringList::ConstIterator it = domainList.begin();
      const TQStringList::ConstIterator itEnd = domainList.end();
      for ( ; it != itEnd; ++it)
      {
        TQString domain;
        KJavaScriptAdvice javaAdvice;
        KJavaScriptAdvice javaScriptAdvice;
        splitDomainAdvice(*it, domain, javaAdvice, javaScriptAdvice);
        if( check_old_java )
          setup_per_domain_policy(d,domain).m_bEnableJava =
	  		javaAdvice == KJavaScriptAccept;
        if( check_old_ecma )
          setup_per_domain_policy(d,domain).m_bEnableJavaScript =
	  		javaScriptAdvice == KJavaScriptAccept;
#ifdef DEBUG_SETTINGS
	setup_per_domain_policy(d,domain).dump("JavaScriptDomainAdvice 4 "+domain);
#endif
      }

      //save all the settings into the new keywords if they don't exist
#if 0
      if( check_old_java )
      {
        TQStringList domainConfig;
        PolicyMap::Iterator it;
        for( it = d->javaDomainPolicy.begin(); it != d->javaDomainPolicy.end(); ++it )
        {
          TQCString javaPolicy = adviceToStr( it.data() );
          TQCString javaScriptPolicy = adviceToStr( KJavaScriptDunno );
          domainConfig.append(TQString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
        }
        config->writeEntry( "JavaDomainSettings", domainConfig );
      }

      if( check_old_ecma )
      {
        TQStringList domainConfig;
        PolicyMap::Iterator it;
        for( it = d->javaScriptDomainPolicy.begin(); it != d->javaScriptDomainPolicy.end(); ++it )
        {
          TQCString javaPolicy = adviceToStr( KJavaScriptDunno );
          TQCString javaScriptPolicy = adviceToStr( it.data() );
          domainConfig.append(TQString::fromLatin1("%1:%2:%3").arg(it.key()).arg(javaPolicy).arg(javaScriptPolicy));
        }
        config->writeEntry( "ECMADomainSettings", domainConfig );
      }
#endif
    }
  }
  config->setGroup(group_save);
}


/** Local helper for retrieving per-domain settings.
  *
  * In case of doubt, the global domain is returned.
  */
static const KPerDomainSettings &lookup_hostname_policy(
			const TDEHTMLSettingsPrivate *d,
			const TQString& hostname)
{
#ifdef DEBUG_SETTINGS
  kdDebug() << "lookup_hostname_policy(" << hostname << ")" << endl;
#endif
  if (hostname.isEmpty()) {
#ifdef DEBUG_SETTINGS
    d->global.dump("global");
#endif
    return d->global;
  }

  const PolicyMap::const_iterator notfound = d->domainPolicy.end();

  // First check whether there is a perfect match.
  PolicyMap::const_iterator it = d->domainPolicy.find(hostname);
  if( it != notfound ) {
#ifdef DEBUG_SETTINGS
    kdDebug() << "perfect match" << endl;
    (*it).dump(hostname);
#endif
    // yes, use it (unless dunno)
    return *it;
  }

  // Now, check for partial match.  Chop host from the left until
  // there's no dots left.
  TQString host_part = hostname;
  int dot_idx = -1;
  while( (dot_idx = host_part.find(TQChar('.'))) >= 0 ) {
    host_part.remove(0,dot_idx);
    it = d->domainPolicy.find(host_part);
    Q_ASSERT(notfound == d->domainPolicy.end());
    if( it != notfound ) {
#ifdef DEBUG_SETTINGS
      kdDebug() << "partial match" << endl;
      (*it).dump(host_part);
#endif
      return *it;
    }
    // assert(host_part[0] == TQChar('.'));
    host_part.remove(0,1); // Chop off the dot.
  }

  // No domain-specific entry: use global domain
#ifdef DEBUG_SETTINGS
  kdDebug() << "no match" << endl;
  d->global.dump("global");
#endif
  return d->global;
}

bool TDEHTMLSettings::isOpenMiddleClickEnabled()
{
  return d->m_bOpenMiddleClick;
}

bool TDEHTMLSettings::isBackRightClickEnabled()
{
  return d->m_bBackRightClick;
}

bool TDEHTMLSettings::accessKeysEnabled() const
{
    return d->m_accessKeysEnabled;
}

bool TDEHTMLSettings::isAdFilterEnabled() const
{
    return d->m_adFilterEnabled;
}

bool TDEHTMLSettings::isHideAdsEnabled() const
{
    return d->m_hideAdsEnabled;
}

bool TDEHTMLSettings::isAdFiltered( const TQString &url ) const
{
    if (d->m_adFilterEnabled)
    {
        if (!url.startsWith("data:"))
        {
            TQValueVector<TQRegExp>::const_iterator it(d->adFilters.constBegin());
            TQValueVector<TQRegExp>::const_iterator end(d->adFilters.constEnd());
            for (; it != end; ++it)
            {
                if ((*it).search(url) != -1)
                {
                    kdDebug( 6080 ) << "Filtered: " << url << endl;
                    return true;
                }
            }
        }
    }
    return false;
}

void TDEHTMLSettings::addAdFilter( const TQString &url )
{
    TDEConfig config( "tdehtmlrc", false, false );
    config.setGroup( "Filter Settings" );

    TQRegExp rx;
    if (url.length()>2 && url[0]=='/' && url[url.length()-1] == '/')
    {
        TQString inside = url.mid(1, url.length()-2);
        rx.setWildcard(false);
        rx.setPattern(inside);
    }
    else
    {
        int left,right;

        rx.setWildcard(true);
        for (right=url.length(); right>0 && url[right-1]=='*' ; --right);
        for (left=0; left<right && url[left]=='*' ; ++left);

        rx.setPattern(url.mid(left,right-left));
    }

    if (rx.isValid())
    {
        int last=config.readNumEntry("Count",0);
        TQString key = "Filter-" + TQString::number(last);
        config.writeEntry(key, url);
        config.writeEntry("Count",last+1);
        config.sync();

        d->adFilters.append(rx);
    }
    else
    {
        KMessageBox::error(0,
                           rx.errorString(),
                           i18n("Filter error"));
    }
}

bool TDEHTMLSettings::isJavaEnabled( const TQString& hostname )
{
  return lookup_hostname_policy(d,hostname.lower()).m_bEnableJava;
}

bool TDEHTMLSettings::isJavaScriptEnabled( const TQString& hostname )
{
  return lookup_hostname_policy(d,hostname.lower()).m_bEnableJavaScript;
}

bool TDEHTMLSettings::isJavaScriptDebugEnabled( const TQString& /*hostname*/ )
{
  // debug setting is global for now, but could change in the future
  return d->m_bEnableJavaScriptDebug;
}

bool TDEHTMLSettings::isJavaScriptErrorReportingEnabled( const TQString& /*hostname*/ ) const
{
  // error reporting setting is global for now, but could change in the future
  return d->m_bEnableJavaScriptErrorReporting;
}

bool TDEHTMLSettings::isPluginsEnabled( const TQString& hostname )
{
  return lookup_hostname_policy(d,hostname.lower()).m_bEnablePlugins;
}

TDEHTMLSettings::KJSWindowOpenPolicy TDEHTMLSettings::windowOpenPolicy(
				const TQString& hostname) const {
  return lookup_hostname_policy(d,hostname.lower()).m_windowOpenPolicy;
}

TDEHTMLSettings::KJSWindowMovePolicy TDEHTMLSettings::windowMovePolicy(
				const TQString& hostname) const {
  return lookup_hostname_policy(d,hostname.lower()).m_windowMovePolicy;
}

TDEHTMLSettings::KJSWindowResizePolicy TDEHTMLSettings::windowResizePolicy(
				const TQString& hostname) const {
  return lookup_hostname_policy(d,hostname.lower()).m_windowResizePolicy;
}

TDEHTMLSettings::KJSWindowStatusPolicy TDEHTMLSettings::windowStatusPolicy(
				const TQString& hostname) const {
  return lookup_hostname_policy(d,hostname.lower()).m_windowStatusPolicy;
}

TDEHTMLSettings::KJSWindowFocusPolicy TDEHTMLSettings::windowFocusPolicy(
				const TQString& hostname) const {
  return lookup_hostname_policy(d,hostname.lower()).m_windowFocusPolicy;
}

int TDEHTMLSettings::mediumFontSize() const
{
    return d->m_fontSize;
}

int TDEHTMLSettings::minFontSize() const
{
  return d->m_minFontSize;
}

TQString TDEHTMLSettings::settingsToCSS() const
{
    // lets start with the link properties
    TQString str = "a:link {\ncolor: ";
    str += d->m_linkColor.name();
    str += ";";
    if(d->m_underlineLink)
        str += "\ntext-decoration: underline;";

    if( d->m_bChangeCursor )
    {
        str += "\ncursor: pointer;";
        str += "\n}\ninput[type=image] { cursor: pointer;";
    }
    str += "\n}\n";
    str += "a:visited {\ncolor: ";
    str += d->m_vLinkColor.name();
    str += ";";
    if(d->m_underlineLink)
        str += "\ntext-decoration: underline;";

    if( d->m_bChangeCursor )
        str += "\ncursor: pointer;";
    str += "\n}\n";

    if(d->m_hoverLink)
        str += "a:link:hover, a:visited:hover { text-decoration: underline; }\n";

    return str;
}

const TQString &TDEHTMLSettings::availableFamilies()
{
    if ( !avFamilies ) {
        avFamilies = new TQString;
        TQFontDatabase db;
        TQStringList families = db.families();
        TQStringList s;
        TQRegExp foundryExp(" \\[.+\\]");

        //remove foundry info
        TQStringList::Iterator f = families.begin();
        const TQStringList::Iterator fEnd = families.end();

        for ( ; f != fEnd; ++f ) {
                (*f).replace( foundryExp, "");
                if (!s.contains(*f))
                        s << *f;
        }
        s.sort();

        *avFamilies = ',' + s.join(",") + ',';
    }

  return *avFamilies;
}

TQString TDEHTMLSettings::lookupFont(int i) const
{
    TQString font;
    if (d->fonts.count() > (uint) i)
       font = d->fonts[i];
    if (font.isEmpty())
        font = d->defaultFonts[i];
    return font;
}

TQString TDEHTMLSettings::stdFontName() const
{
    return lookupFont(0);
}

TQString TDEHTMLSettings::fixedFontName() const
{
    return lookupFont(1);
}

TQString TDEHTMLSettings::serifFontName() const
{
    return lookupFont(2);
}

TQString TDEHTMLSettings::sansSerifFontName() const
{
    return lookupFont(3);
}

TQString TDEHTMLSettings::cursiveFontName() const
{
    return lookupFont(4);
}

TQString TDEHTMLSettings::fantasyFontName() const
{
    return lookupFont(5);
}

void TDEHTMLSettings::setStdFontName(const TQString &n)
{
    while(d->fonts.count() <= 0)
        d->fonts.append(TQString::null);
    d->fonts[0] = n;
}

void TDEHTMLSettings::setFixedFontName(const TQString &n)
{
    while(d->fonts.count() <= 1)
        d->fonts.append(TQString::null);
    d->fonts[1] = n;
}

TQString TDEHTMLSettings::userStyleSheet() const
{
    return d->m_userSheet;
}

bool TDEHTMLSettings::isFormCompletionEnabled() const
{
  return d->m_formCompletionEnabled;
}

int TDEHTMLSettings::maxFormCompletionItems() const
{
  return d->m_maxFormCompletionItems;
}

const TQString &TDEHTMLSettings::encoding() const
{
  return d->m_encoding;
}

bool TDEHTMLSettings::followSystemColors() const
{
    return d->m_follow_system_colors;
}

const TQColor& TDEHTMLSettings::textColor() const
{
  return d->m_textColor;
}

const TQColor& TDEHTMLSettings::baseColor() const
{
  return d->m_baseColor;
}

const TQColor& TDEHTMLSettings::linkColor() const
{
  return d->m_linkColor;
}

const TQColor& TDEHTMLSettings::vLinkColor() const
{
  return d->m_vLinkColor;
}

bool TDEHTMLSettings::autoLoadImages() const
{
  return d->m_bAutoLoadImages;
}

bool TDEHTMLSettings::unfinishedImageFrame() const
{
  return d->m_bUnfinishedImageFrame;
}

TDEHTMLSettings::KAnimationAdvice TDEHTMLSettings::showAnimations() const
{
  return d->m_showAnimations;
}

bool TDEHTMLSettings::isAutoDelayedActionsEnabled() const
{
  return d->m_autoDelayedActionsEnabled;
}

bool TDEHTMLSettings::jsErrorsEnabled() const
{
  return d->m_jsErrorsEnabled;
}

void TDEHTMLSettings::setJSErrorsEnabled(bool enabled)
{
  d->m_jsErrorsEnabled = enabled;
  // save it
  TDEConfig *config = TDEGlobal::config();
  config->setGroup("HTML Settings");
  config->writeEntry("ReportJSErrors", enabled);
  config->sync();
}

bool TDEHTMLSettings::allowTabulation() const
{
    return d->m_allowTabulation;
}

bool TDEHTMLSettings::autoSpellCheck() const
{
    return d->m_autoSpellCheck;
}

TQValueList< TQPair< TQString, TQChar > > TDEHTMLSettings::fallbackAccessKeysAssignments() const
{
    return d->m_fallbackAccessKeysAssignments;
}

void TDEHTMLSettings::setJSPopupBlockerPassivePopup(bool enabled)
{
    d->m_jsPopupBlockerPassivePopup = enabled;
    // save it
    TDEConfig *config = TDEGlobal::config();
    config->setGroup("Java/JavaScript Settings");
    config->writeEntry("PopupBlockerPassivePopup", enabled);
    config->sync();
}

bool TDEHTMLSettings::jsPopupBlockerPassivePopup() const
{
    return d->m_jsPopupBlockerPassivePopup;
}
