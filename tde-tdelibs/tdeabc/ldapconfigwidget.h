/*
    This file is part of libtdeabc.
    Copyright (c) 2004 Szombathelyi György <gyurco@freemail.hu>

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

#ifndef LDAPCONFIGWIDGET_H
#define LDAPCONFIGWIDGET_H

#include <tqwidget.h>
#include <tqmap.h>
#include <tqstring.h>

#include <tdeabc/ldapurl.h>
#include <tdeabc/ldif.h>
#include <tdeio/job.h>

class TQGridLayout;
class TQSpinBox;
class TQPushButton;
class TQCheckBox;
class TQRadioButton;
class KComboBox;
class KLineEdit;
class KProgressDialog;

namespace TDEABC {

  /**
    @short LDAP Configuration widget
 
    This class can be used to query the user for LDAP connection parameters. 
    It's TDEConfigXT compatible, using widget names starting with kcfg_
  */

  class KABC_EXPORT LdapConfigWidget : public TQWidget
  {
    TQ_OBJECT
    
    TQ_PROPERTY( LCW_Flags flags READ flagsProp WRITE setFlagsProp )
    TQ_PROPERTY( TQString user READ user WRITE setUser )
    TQ_PROPERTY( TQString password READ password WRITE setPassword )
    TQ_PROPERTY( TQString bindDN READ bindDN WRITE setBindDN )
    TQ_PROPERTY( TQString realm READ realm WRITE setRealm )
    TQ_PROPERTY( TQString host READ host WRITE setHost )
    TQ_PROPERTY( int port READ port WRITE setPort )
    TQ_PROPERTY( int ver READ ver WRITE setVer )
    TQ_PROPERTY( TQString dn READ dn WRITE setDn )
    TQ_PROPERTY( TQString filter READ filter WRITE setFilter )
    TQ_PROPERTY( TQString mech READ mech WRITE setMech )
    TQ_PROPERTY( bool secNO READ isSecNO WRITE setSecNO )
    TQ_PROPERTY( bool secSSL READ isSecSSL WRITE setSecSSL )
    TQ_PROPERTY( bool secTLS READ isSecSSL WRITE setSecTLS )
    TQ_PROPERTY( bool authAnon READ isAuthAnon WRITE setAuthAnon )
    TQ_PROPERTY( bool authSimple READ isAuthSimple WRITE setAuthSimple )
    TQ_PROPERTY( bool authSASL READ isAuthSASL WRITE setAuthSASL )
    TQ_PROPERTY( int sizeLimit READ sizeLimit WRITE setSizeLimit )
    TQ_PROPERTY( int timeLimit READ timeLimit WRITE setTimeLimit )
    TQ_SETS ( LCW_Flags )
        
    public:
    
      enum LCW_Flags {
        W_USER = 0x1,
        W_PASS = 0x2,
        W_BINDDN = 0x4,
        W_REALM = 0x8,
        W_HOST = 0x10,
        W_PORT = 0x20,
        W_VER = 0x40,
        W_DN = 0x80,
        W_FILTER = 0x100,
        W_SECBOX = 0x400,
        W_AUTHBOX = 0x800,
        W_TIMELIMIT = 0x1000,
        W_SIZELIMIT = 0x2000,
        W_ALL = 0xFFFFFFF
      };
      
      /** Constructs an empty configuration widget.
       * You need to call setFlags() after this.
       */
      LdapConfigWidget( TQWidget* parent = 0, 
        const char* name = 0, WFlags fl = 0 );
      /** Constructs a configuration widget */
      LdapConfigWidget( int flags, TQWidget* parent = 0,
        const char* name = 0, WFlags fl = 0 );
      /** Destructs a configuration widget */
      virtual ~LdapConfigWidget();

      /** Sets the user name. Kconfig widget name: kcfg_ldapuser */
      void setUser( const TQString &user );
      /** Gets the user name. Kconfig widget name: kcfg_ldapuser */
      TQString user() const;

      /** Sets the password. Kconfig widget name: kcfg_ldappassword */
      void setPassword( const TQString &password );
      /** Gets the password. Kconfig widget name: kcfg_ldappassword */
      TQString password() const;

      /**
       * Sets the bind dn. Useful for SASL proxy auth.
       * Kconfig widget name: kcfg_ldapbinddn
       */
      void setBindDN( const TQString &binddn );
      /** Gets the bind dn. Kconfig widget name: kcfg_ldapbinddn*/
      TQString bindDN() const;

      /** Sets the SASL realm. Kconfig widget name: kcfg_ldaprealm */
      void setRealm( const TQString &realm );
      /** Gets the SASL realm. Kconfig widget name: kcfg_ldaprealm */
      TQString realm() const;

      /** Sets the host name. Kconfig widget name: kcfg_ldaphost */
      void setHost( const TQString &host );
      /** Gets the host name. Kconfig widget name: kcfg_ldaphost */
      TQString host() const;

      /** Sets the LDAP port. Kconfig widget name: kcfg_ldapport */
      void setPort( int port );
      /** Gets the LDAP port. Kconfig widget name: kcfg_ldapport */
      int port() const;

      /** Sets the LDAP protocol version. Kconfig widget name: kcfg_ldapver */
      void setVer( int ver );
      /** Gets the LDAP protocol version. Kconfig widget name: kcfg_ldapver */
      int ver() const;

      /** Sets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      void setDn( const TQString &dn );
      /** Gets the LDAP Base DN. Kconfig widget name: kcfg_ldapdn */
      TQString dn() const;

      /** Sets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      void setFilter( const TQString &filter );
      /** Gets the LDAP Filter. Kconfig widget name: kcfg_ldapfilter */
      TQString filter() const;

      /** Sets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      void setMech( const TQString &mech );
      /** Gets the SASL Mechanism. Kconfig widget name: kcfg_ldapsaslmech */
      TQString mech() const;

      /**
       * Sets the configuration to no transport security.
       * Kconfig widget name: kcfg_ldapnosec
       */
      void setSecNO( bool b = true );
      /**
       * Returns true if no transport security selected.
       * Kconfig widget name: kcfg_ldapnosec
       */
      bool isSecNO() const;

      /**
       * Sets the configuration to TLS.
       * Kconfig widget name: kcfg_ldaptls
       */
      void setSecTLS( bool b = true );
      /**
       * Returns true if TLS selected.
       * Kconfig widget name: kcfg_ldaptls
       */
      bool isSecTLS() const;

      /**
       * Sets the configuration to SSL.
       * Kconfig widget name: kcfg_ldapssl
       */
      void setSecSSL( bool b = true );
      /**
       * Returns true if SSL selected.
       * Kconfig widget name: kcfg_ldapssl
       */
      bool isSecSSL() const;

      /**
       * Sets the authentication to anonymous.
       * Kconfig widget name: kcfg_ldapanon
       */
      void setAuthAnon( bool b = true );
      /**
       * Returns true if Anonymous authentication selected.
       * Kconfig widget name: kcfg_ldapanon
       */
      bool isAuthAnon() const;

      /**
       * Sets the authentication to simple.
       * Kconfig widget name: kcfg_ldapsimple
       */
      void setAuthSimple( bool b = true );
      /**
       * Returns true if Simple authentication selected.
       * Kconfig widget name: kcfg_ldapsimple
       */
      bool isAuthSimple() const;

      /**
       * Sets the authentication to SASL.
       * Kconfig widget name: kcfg_ldapsasl
       */
      void setAuthSASL( bool b = true );
      /**
       * Returns true if SASL authentication selected.
       * Kconfig widget name: kcfg_ldapsasl
       */
      bool isAuthSASL() const;

      /**
       * Sets the size limit.
       * TDEConfig widget name: kcfg_ldapsizelimit
       */
      void setSizeLimit( int sizelimit );
      /**
       * Returns the size limit.
       * TDEConfig widget name: kcfg_ldapsizelimit
       */
      int sizeLimit() const;

      /**
       * Sets the time limit.
       * TDEConfig widget name: kcfg_ldaptimelimit
       */
      void setTimeLimit( int timelimit );
      /**
       * Returns the time limit.
       * TDEConfig widget name: kcfg_ldaptimelimit
       */
      int timeLimit() const;

      int flags() const;
      void setFlags( int flags );
      inline LCW_Flags flagsProp() const { return (LCW_Flags)flags(); }
      inline void setFlagsProp( LCW_Flags flags ) { setFlags((int)flags); }

      /**
       * Returns a LDAP Url constructed from the settings given.
       * Extensions are filled for use in the LDAP ioslave
       */
      TDEABC::LDAPUrl url() const;

    private slots:
      void setLDAPPort();
      void setLDAPSPort();
      void setAnonymous( int state );
      void setSimple( int state );
      void setSASL( int state );
      void mQueryDNClicked();
      void mQueryMechClicked();
      void loadData( TDEIO::Job*, const TQByteArray& );
      void loadResult( TDEIO::Job* );  
    private:

      int mFlags;
      LDIF mLdif;
      TQStringList mQResult;
      TQString mAttr;

      KLineEdit *mUser;
      KLineEdit *mPassword;
      KLineEdit *mHost;
      TQSpinBox  *mPort, *mVer, *mSizeLimit, *mTimeLimit;
      KLineEdit *mDn, *mBindDN, *mRealm;
      KLineEdit *mFilter;
      TQRadioButton *mAnonymous,*mSimple,*mSASL;
      TQCheckBox *mSubTree;
      TQPushButton *mEditButton;
      TQPushButton *mQueryMech;
      TQRadioButton *mSecNO,*mSecTLS,*mSecSSL;
      KComboBox *mMech;

      TQString mErrorMsg;
      bool mCancelled;
      KProgressDialog *mProg;

      TQGridLayout *mainLayout;
      class LDAPConfigWidgetPrivate;
      LDAPConfigWidgetPrivate *d;

      void sendQuery();
      void initWidget();
  };
}

#endif
