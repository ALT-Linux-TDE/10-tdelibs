/* This file is part of the KDE project
   Copyright (C) 1998, 1999 Torben Weis <weis@kde.org>

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

#ifndef __parse_h__
#define __parse_h__

/*
 * Functions definition for yacc
 */
void TDETraderParse_mainParse( const char *_code );
void TDETraderParse_setParseTree( void *_ptr1 );
void TDETraderParse_error( const char* err );
void* TDETraderParse_newOR( void *_ptr1, void *_ptr2 );
void* TDETraderParse_newAND( void *_ptr1, void *_ptr2 );
void* TDETraderParse_newCMP( void *_ptr1, void *_ptr2, int _i );
void* TDETraderParse_newIN( void *_ptr1, void *_ptr2 );
void* TDETraderParse_newMATCH( void *_ptr1, void *_ptr2 );
void* TDETraderParse_newCALC( void *_ptr1, void *_ptr2, int _i );
void* TDETraderParse_newBRACKETS( void *_ptr1 );
void* TDETraderParse_newNOT( void *_ptr1 );
void* TDETraderParse_newEXIST( char *_ptr1 );
void* TDETraderParse_newID( char *_ptr1 );
void* TDETraderParse_newSTRING( char *_ptr1 );
void* TDETraderParse_newNUM( int _i );
void* TDETraderParse_newFLOAT( float _f );
void* TDETraderParse_newBOOL( char _b );

void* TDETraderParse_newWITH( void *_ptr1 );
void* TDETraderParse_newMAX( void *_ptr1 );
void* TDETraderParse_newMIN( void *_ptr1 );
void* TDETraderParse_newMAX2( char *_id );
void* TDETraderParse_newMIN2( char *_id );
void* TDETraderParse_newFIRST();
void* TDETraderParse_newRANDOM();

#endif
