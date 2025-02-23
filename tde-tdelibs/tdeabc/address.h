/*
    This file is part of libtdeabc.
    Copyright (c) 2001 Cornelius Schumacher <schumacher@kde.org>

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

#ifndef KABC_ADDRESS_H
#define KABC_ADDRESS_H

#include <tqmap.h>
#include <tqstring.h>
#include <tqvaluelist.h>

#include <tdelibs_export.h>

// template tags for address formatting localization
#define KABC_FMTTAG_realname   TQString("%n")
#define KABC_FMTTAG_REALNAME   TQString("%N")
#define KABC_FMTTAG_company    TQString("%cm")
#define KABC_FMTTAG_COMPANY    TQString("%CM")
#define KABC_FMTTAG_pobox      TQString("%p")
#define KABC_FMTTAG_street     TQString("%s")
#define KABC_FMTTAG_STREET     TQString("%S")
#define KABC_FMTTAG_zipcode    TQString("%z")
#define KABC_FMTTAG_location   TQString("%l")
#define KABC_FMTTAG_LOCATION   TQString("%L")
#define KABC_FMTTAG_region     TQString("%r")
#define KABC_FMTTAG_REGION     TQString("%R")
#define KABC_FMTTAG_newline    TQString("\\n")
#define KABC_FMTTAG_condcomma  TQString("%,")
#define KABC_FMTTAG_condwhite  TQString("%w")
#define KABC_FMTTAG_purgeempty TQString("%0")

namespace TDEABC {

/**
  @short Postal address information.
  
  This class represents information about a postal address.
*/
class KABC_EXPORT Address
{
    friend KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Address & );
    friend KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Address & );

  public:
    /**
      List of addresses.
    */
    typedef TQValueList<Address> List;
    typedef TQValueList<int> TypeList;
  
    /**
      Address types:
     
      @li @p Dom -    domestic
      @li @p Intl -   international
      @li @p Postal - postal
      @li @p Parcel - parcel
      @li @p Home -   home address
      @li @p Work -   address at work
      @li @p Pref -   preferred address
    */
    enum Type { Dom = 1, Intl = 2, Postal = 4, Parcel = 8, Home = 16, Work = 32,
           Pref = 64 };

    /**
      Constructor that creates an empty Address, which is initialized
      with a unique id (see id()).
    */
    Address();
  
    /**
      This is like Address() just above, with the difference
      that you can specify the type.
    */
    Address( int );

    bool operator==( const Address & ) const;
    bool operator!=( const Address & ) const;
  
    /**
      Returns true, if the address is empty.
    */
    bool isEmpty() const;

    /**
      Clears all entries of the address.
    */
    void clear();

    /**
      Sets the unique id.
    */
    void setId( const TQString & );

    /*
      Returns the unique id.
    */
    TQString id() const;

    /**
      Sets the type of address. See enum for definiton of types. 
     
      @param type type, can be a bitwise or of multiple types.
    */
    void setType( int type );

    /**
      Returns the type of address. Can be a bitwise or of multiple types.
    */
    int type() const;

    /**
      Returns a translated string of all types the address has.
    */
    TQString typeLabel() const;

    /**
      Sets the post office box.
    */
    void setPostOfficeBox( const TQString & );

    /**
      Returns the post office box.
    */
    TQString postOfficeBox() const;

    /**
      Returns the translated label for post office box field.
    */
    static TQString postOfficeBoxLabel();

    /**
      Sets the extended address information.
    */
    void setExtended( const TQString & );

    /**
      Returns the extended address information.
    */
    TQString extended() const;

    /**
      Returns the translated label for extended field.
    */
    static TQString extendedLabel();
    
    /**
      Sets the street (including number).
    */
    void setStreet( const TQString & );

    /**
      Returns the street.
    */
    TQString street() const;

    /**
      Returns the translated label for street field.
    */
    static TQString streetLabel();

    /**
      Sets the locality, e.g. city.
    */
    void setLocality( const TQString & );

    /**
      Returns the locality.
    */
    TQString locality() const;

    /**
      Returns the translated label for locality field.
    */
    static TQString localityLabel();

    /**
      Sets the region, e.g. state.
    */
    void setRegion( const TQString & );

    /**
      Returns the region.
    */
    TQString region() const;

    /**
      Returns the translated label for region field.
    */
    static TQString regionLabel();
 
    /**
      Sets the postal code.
    */
    void setPostalCode( const TQString & );

    /**
      Returns the postal code.
    */
    TQString postalCode() const;

    /**
      Returns the translated label for postal code field.
    */
    static TQString postalCodeLabel();

    /**
      Sets the country.
    */
    void setCountry( const TQString & );

    /**
      Returns the country.
    */
    TQString country() const;

    /**
      Returns the translated label for country field.
    */
    static TQString countryLabel();

    /**
      Sets the delivery label. This is the literal text to be used as label.
    */
    void setLabel( const TQString & );

    /**
      Returns the delivery label.
    */
    TQString label() const;

    /**
      Returns the translated label for delivery label field.
    */
    static TQString labelLabel();

    /**
      Returns the list of available types.
    */
    static TypeList typeList();

    /**
      Returns the translated label for a special type.
    */
    static TQString typeLabel( int type );

    /**
      Used for debug output.
    */
    void dump() const;

    /** 
      Returns this address formatted according to the country-specific
      address formatting rules. The formatting rules applied depend on 
      either the addresses {@link #country country} field, or (if the 
      latter is empty) on the system country setting. If companyName is
      provided, an available business address format will be preferred.
      
      @param realName   the formatted name of the contact
      @param orgaName   the name of the organization or company
      @return           the formatted address (containing newline characters)
    */
    TQString formattedAddress( const TQString &realName=TQString::null
                            , const TQString &orgaName=TQString::null ) const;

    /**
      Returns ISO code for a localized country name. Only localized country
      names will be understood. This might be replaced by a TDELocale method in
      the future.
      @param cname  name of the country
      @return       two digit ISO code
    */
    static TQString countryToISO( const TQString &cname );

    /**
      Returns a localized country name for a ISO code. 
      This might be replaced by a TDELocale method in the future.
      @param ISOname two digit ISO code
      @return        localized name of the country
      @since 3.2
    */
    static TQString ISOtoCountry( const TQString &ISOname );

  private:
    /** 
      Parses a snippet of an address template
      @param tsection   the template string to be parsed
      @param result     TQString reference in which the result will be stored
      @return           true if at least one tag evaluated positively, else false
    */
    bool parseAddressTemplateSection( const TQString &tsection
                                    ,       TQString &result
                                    , const TQString &realName
                                    , const TQString &orgaName ) const;

    /** 
      Finds the balanced closing bracket starting from the opening bracket at 
      pos in tsection.
      @return  position of closing bracket, -1 for unbalanced brackets
    */
    int  findBalancedBracket( const TQString &tsection, int pos ) const;

    bool mEmpty;
  
    TQString mId;
    int mType;
  
    TQString mPostOfficeBox;
    TQString mExtended;
    TQString mStreet;
    TQString mLocality;
    TQString mRegion;
    TQString mPostalCode;
    TQString mCountry;
    TQString mLabel;

    static TQMap<TQString, TQString> *mISOMap;
};

KABC_EXPORT TQDataStream &operator<<( TQDataStream &, const Address & );
KABC_EXPORT TQDataStream &operator>>( TQDataStream &, Address & );

}

#endif
