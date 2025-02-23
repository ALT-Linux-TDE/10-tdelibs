/* This file is part of the KDE libraries
   Copyright (C) 2002,2003 Carsten Pfeiffer <pfeiffer@kde.org>

   library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License as published by the Free Software Foundation, version 2.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#include <iostream>

#include <tqfile.h>
#include <tqptrlist.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdefilemetainfo.h>
#include <tdelocale.h>
#include <kpropertiesdialog.h>

#include "fileprops.h"

#define TDEFILEVERSION "0.2"
#define INDENT "\t"

using namespace std;

static TQString beatifyValue( const TQString& value )
{
    if ( value.isNull() )
        return TQString("(no value for key available)");
    else if ( value.isEmpty() )
        return TQString("(empty)");

    return value;
}

FileProps::FileProps( const TQString& path, const TQStringList& suppliedGroups )
    : m_dirty( false )
{
    m_info = new KFileMetaInfo(path, TQString::null, KFileMetaInfo::Everything);
    m_userSuppliedGroups = !suppliedGroups.isEmpty();
    m_groupsToUse = m_userSuppliedGroups ? suppliedGroups : m_info->groups();
}

FileProps::~FileProps()
{
    sync();
    delete m_info;
}

bool FileProps::sync()
{
    if ( !m_dirty )
        return true;

    return m_info->applyChanges();
}

bool FileProps::isValid() const
{
    return m_info->isValid();
}

TQStringList FileProps::supportedGroups() const
{
    return m_info->supportedGroups();
}

TQStringList FileProps::availableGroups() const
{
    return m_info->groups();
}

TQStringList FileProps::supportedKeys( const TQString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return g.supportedKeys();
}

TQStringList FileProps::availableKeys( const TQString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    TQStringList allKeys = g.keys();
    TQStringList ret;
    TQStringList::ConstIterator it = allKeys.begin();
    for ( ; it != allKeys.end(); ++it )
    {
        if ( g.item( *it ).isValid() )
            ret.append( *it );
    }

    return ret;
}

TQStringList FileProps::preferredKeys( const TQString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return g.preferredKeys();
}

TQString FileProps::getValue( const TQString& group,
                             const TQString& key ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return FileProps::createKeyValue( g, key );
}

bool FileProps::setValue( const TQString& group,
                          const TQString& key, const TQString &value )
{
    KFileMetaInfoGroup g = m_info->group( group );
    bool wasAdded = false;
    if ( !g.isValid() )
    {
        if ( m_info->addGroup( group ) )
        {
            wasAdded = true;
            g = m_info->group( group );
        }
        else
            return false;
    }

    bool ok = g[key].setValue( value );

    if ( !ok && wasAdded ) // remove the created group again
        (void) m_info->removeGroup( group );
        
    m_dirty |= ok;
    return ok;
}

TQStringList FileProps::allValues( const TQString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return FileProps::createKeyValueList( g, g.keys() );
}

TQStringList FileProps::preferredValues( const TQString& group ) const
{
    KFileMetaInfoGroup g = m_info->group( group );
    return FileProps::createKeyValueList( g, g.preferredKeys() );
}

// static helper:
// creates strings like
// "group:       translatedKey:               value"
TQString FileProps::createKeyValue( const KFileMetaInfoGroup& g,
                                   const TQString& key )
{
    static const int MAX_SPACE = 25;
    KFileMetaInfoItem item = g.item( key );

    TQString result("%1");
    result = result.arg( (item.isValid() ? item.translatedKey() : key) + ":",
                         -MAX_SPACE );
    result.append( beatifyValue( item.string() ) );

    TQString group("%1");
    group = group.arg( g.translatedName() + ":", -MAX_SPACE );
    result.prepend( group );

    return result;
}

// static
TQStringList FileProps::createKeyValueList( const KFileMetaInfoGroup& g,
                                           const TQStringList& keys )
{
    TQStringList result;
    TQStringList::ConstIterator it = keys.begin();

    for ( ; it != keys.end(); ++it )
        result.append( FileProps::createKeyValue( g, *it ) );

    return result;
}

///////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////



// tdefile --mimetype --listsupported --listavailable --listpreferred --listwritable --getValue "key" --setValue "key=value" --allValues --preferredValues --dialog --quiet file [file...]
// "key" may be a list of keys, separated by commas
static TDECmdLineOptions options[] =
{
    { "m", 0, 0 }, // short option for --mimetype
    { "nomimetype", I18N_NOOP("Do not print the mimetype of the given file(s)"), 0 },

    { "ls", 0, 0 }, // short option for --listsupported
    { "listsupported <mimetype>",
      I18N_NOOP("List all supported metadata keys of the given file(s). "
                "If mimetype is not specified, the mimetype of the given "
                "files is used." ), "file" },

    { "lp", 0, 0 }, // short option for --listpreferred
    { "listpreferred <mimetype>",
      I18N_NOOP("List all preferred metadata keys of the given file(s). "
                "If mimetype is not specified, the mimetype of the given "
                "files is used." ), "file" },

    { "la", 0, 0 }, // short option for --listavailable
    { "listavailable",
      I18N_NOOP("List all metadata keys which have a value in the given "
                "file(s)."), 0 },

    { "sm", 0, 0 }, // short option for --supportedMimetypes
    { "supportedMimetypes",
      I18N_NOOP("Prints all mimetypes for which metadata support is "
                "available."), 0 },

    { "q", 0, 0 }, // short option for --quiet
    { "quiet",
      I18N_NOOP("Do not print a warning when more than one file was given "
                "and they do not all have the same mimetype."), 0 },

    { "av", 0, 0 }, // short option for --allValues
    { "allValues",
      I18N_NOOP("Prints all metadata values, available in the given "
                "file(s)."), 0 },

    { "pv", 0, 0 }, // short option for --preferredValues
    { "preferredValues",
      I18N_NOOP("Prints the preferred metadata values, available in the "
                "given file(s)."), 0 },

    { "dialog",
      I18N_NOOP("Opens a TDE properties dialog to allow viewing and "
                "modifying of metadata of the given file(s)"), 0 },

    { "getValue <key>",
      I18N_NOOP("Prints the value for 'key' of the given file(s). 'key' "
                "may also be a comma-separated list of keys"), 0 },

    { "setValue <key=value>",
      I18N_NOOP("Attempts to set the value 'value' for the metadata key "
                "'key' for the given file(s)"), 0 },

    { "!groups <arguments>", I18N_NOOP("The group to get values from or set values to"),
      0 },

    { "+[files]",
      I18N_NOOP("The file (or a number of files) to operate on."), 0 },
    TDECmdLineLastOption
};


//
// helper functions
//

static void printSupportedMimeTypes()
{
    TQStringList allMimeTypes = KFileMetaInfoProvider::self()->supportedMimeTypes();
    if ( allMimeTypes.isEmpty() )
    {
        cout <<
            i18n("No support for metadata extraction found.").local8Bit().data()
             << endl;
        return;
    }

    cout << i18n("Supported MimeTypes:").local8Bit().data() << endl;

    TQStringList::ConstIterator it = allMimeTypes.begin();
    for ( ; it != allMimeTypes.end(); it++ )
        cout << (*it).local8Bit().data() << endl;
}

// caller needs to delete the returned list!
static KFileItemList * fileItemList( const TDECmdLineArgs *args )
{
    KFileItemList * items = new KFileItemList();
    items->setAutoDelete( true );
    for ( int i = 0; i < args->count(); i++ )
        items->append( new KFileItem( KFileItem::Unknown,
                                     KFileItem::Unknown,
                                     args->url( i ) ));
    return items;
}

static void showPropertiesDialog( const TDECmdLineArgs *args )
{
    KFileItemList *items = fileItemList( args );
    new KPropertiesDialog( *items, 0L, "props dialog", true );
    delete items;
}

static void printMimeTypes( const TDECmdLineArgs *args )
{
    for ( int i = 0; i < args->count(); i++ )
    {
        KURL url = args->url( i );
        KMimeType::Ptr mt = KMimeType::findByURL( url );
        cout << args->arg(i) << ": " << mt->comment().local8Bit().data() << " ("
             << mt->name().local8Bit().data() << ")" << endl;
    }
}

static void printList( const TQStringList& list )
{
    TQStringList::ConstIterator it = list.begin();
    for ( ; it != list.end(); ++it )
        cout << (*it).local8Bit().data() << endl;
    cout << endl;
}

static void processMetaDataOptions( const TQPtrList<FileProps> propList,
                                    TDECmdLineArgs *args )
{
// tdefile --mimetype --supportedMimetypes --listsupported --listavailable --listpreferred --listwritable --getValue "key" --setValue "key=value" --allValues --preferredValues --dialog --quiet file [file...]
// "key" may be a list of keys, separated by commas

    TQString line("-- -------------------------------------------------------");
    FileProps *props;
    TQPtrListIterator<FileProps> it( propList );
    for ( ; (props = it.current()); ++it )
    {
        TQString file = props->fileName() + " ";
        TQString fileString = line.replace( 3, file.length(), file );
        cout << TQFile::encodeName( fileString ).data() << endl;
            
        if ( args->isSet( "listsupported" ) )
        {
            cout << "=Supported Keys=" << endl;
            printList( props->supportedKeys() );
        }
        if ( args->isSet( "listpreferred" ) )
        {
            cout << "=Preferred Keys=" << endl;
            printList( props->preferredKeys() );
        }
        if ( args->isSet( "listavailable" ) )
        {
            cout << "=Available Keys=" << endl;
            TQStringList groups = props->availableGroups();
            TQStringList::ConstIterator git = groups.begin();
            for ( ; git != groups.end(); ++git )
            {
                cout << "Group: " << (*git).local8Bit().data() << endl;
                printList( props->availableKeys( *git ) );
            }
        }
//         if ( args->isSet( "listwritable" ) )
//         {
//             cout << "TODO :)" << endl;
//         }
        if ( args->isSet( "getValue" ) )
        {
            cout << "=Value=" << endl;
            TQString key = TQString::fromLocal8Bit( args->getOption("getValue"));
            TQStringList::ConstIterator git = props->groupsToUse().begin();
            for ( ; git != props->groupsToUse().end(); ++git )
                cout << props->getValue( *git, key ).local8Bit().data() << endl;
        }

        if ( args->isSet( "setValue" ) )
        {
            // separate key and value from the line "key=value"
            TQString cmd = TQString::fromLocal8Bit( args->getOption("setValue"));
            TQString key = cmd.section( '=', 0, 0 );
            TQString value = cmd.section( '=', 1 );

            // either use supplied groups or all supported groups
            // (not only the available!)
            TQStringList groups = props->userSuppliedGroups() ?
                                 props->groupsToUse() :
                                 props->supportedGroups();

            TQStringList::ConstIterator git = groups.begin();
            for ( ; git != groups.end(); ++git )
                props->setValue( *git, key, value );
        }

        if ( args->isSet( "allValues" ) )
        {
            cout << "=All Values=" << endl;
            TQStringList groups = props->availableGroups();
            TQStringList::ConstIterator group = groups.begin();
            for ( ; group != groups.end(); ++group )
                printList( props->allValues( *group ) );
        }
        if ( args->isSet( "preferredValues" ) && !args->isSet("allValues") )
        {
            cout << "=Preferred Values=" << endl;
            TQStringList groups = props->availableGroups();
            TQStringList::ConstIterator group = groups.begin();
            for ( ; group != groups.end(); ++group )
                printList( props->preferredValues( *group ) );
        }
    }

}

int main( int argc, char **argv )
{
    TDEAboutData about(
	  "tdefile", I18N_NOOP( "tdefile" ), TDEFILEVERSION,
	  I18N_NOOP("A commandline tool to read and modify metadata of files." ),
	  TDEAboutData::License_LGPL, "(c) 2002, Carsten Pfeiffer",
	  0 /*text*/, "http://devel-home.kde.org/~pfeiffer/",
	  "pfeiffer@kde.org" );

    about.addAuthor( "Carsten Pfeiffer", 0, "pfeiffer@kde.org",
		     "http://devel-home.kde.org/~pfeiffer/" );

    TDECmdLineArgs::init( argc, argv, &about );

    TDECmdLineArgs::addCmdLineOptions( options );

    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();
    bool useGUI = args->isSet( "dialog" );

    TDEApplication app( useGUI, useGUI );

    TQPtrList<FileProps> m_props;
    m_props.setAutoDelete( true );

    bool quiet = args->isSet( "quiet" );

    if ( args->isSet( "supportedMimetypes" ) )
        printSupportedMimeTypes();

    int files = args->count();
    if ( files == 0 )
        TDECmdLineArgs::usage( i18n("No files specified") ); // exit()s

    if ( args->isSet( "dialog" ) )
    {
        showPropertiesDialog( args );
        return true;
    }

    TQStringList groupsToUse;
    QCStringList suppliedGroups = args->getOptionList( "groups" );
    QCStringList::ConstIterator it = suppliedGroups.begin();
    for ( ; it != suppliedGroups.end(); ++it )
        groupsToUse.append( TQString::fromLocal8Bit( (*it) ) );

    TQString mimeType;

    for ( int i = 0; i < files; i++ )
    {
        if ( args->isSet( "mimetype" ) )
            printMimeTypes( args );

        FileProps *props = new FileProps( args->url(i).path(), groupsToUse );
        if ( props->isValid() )
            m_props.append( props );
        else
        {
            if ( !quiet )
            {
                cerr << args->arg(i) << ": " <<
                i18n("Cannot determine metadata").local8Bit().data() << endl;
            }
            delete props;
        }
    }


    processMetaDataOptions( m_props, args );

    m_props.clear(); // force destruction/sync of props
    cout.flush();

    return 0;
}
