// $Id$

//
// configtest.cpp: libKDEcore example
//
// demonstrates use of KSimpleConfig class

#include <ksimpleconfig.h>
#include <tdeapplication.h>
#include <unistd.h>
#include <stdio.h>

#ifdef HAVE_PATHS_H
#include <paths.h>
#endif

#ifndef _PATH_TMP
#define _PATH_TMP "/tmp/"
#endif

int main( int argc, char **argv )
{
  TDEApplication  a( argc, argv, TQCString("tdeconfigtest") );

  KSimpleConfig aConfig( _PATH_TMP"/simpleconfig.cfg" );

  aConfig.setGroup( "Test group" );
  aConfig.writeEntry( "key1", "value1" );
  aConfig.writeEntry( "key2", "value2" );

  aConfig.setGroup( "Another Test group" );
  aConfig.writeEntry( "akey1", "avalue1" );
  aConfig.writeEntry( "akey2", "avalue2" );

  fprintf( stderr, "[ksimpleconfigtest] entries written, sleeping for 10 seconds\n" );
  aConfig.sync();
  sleep( 10 );

  aConfig.setGroup( "Test group" );
  aConfig.deleteEntry( "key2", false );
  fprintf( stderr, "[ksimpleconfigtest] Deleted on entry from Test group, sleeping\n" );
  aConfig.sync();
  sleep( 10 );

  aConfig.deleteGroup( "Another Test group", true );
  fprintf( stderr, "[ksimpleconfigtest] Deleted Another Test group\n" );
}

