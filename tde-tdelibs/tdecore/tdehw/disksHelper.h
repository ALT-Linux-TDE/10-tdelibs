/* This file is part of the TDE libraries
   Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>
             (C) 2013 Golubev Alexander <fatzer2@gmail.com>

   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Library General Public
   License version 2 as published by the Free Software Foundation.

   This library is distributed in the hope that it will be useful,
   but WITHOUT ANY WARRANTY; without even the implied warranty of
   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
   Library General Public License for more details.

   You should have received a copy of the GNU Library General Public License
   along with this library; see the file COPYING.LIB.  If not, write to
   the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
   Boston, MA 02110-1301, USA.
*/

#ifndef _DISKS_HELPER_H
#define _DISKS_HELPER_H

#if defined(WITH_UDISKS) || defined(WITH_UDISKS2)
	#include "tqstringlist.h"
	#include "tqvariant.h"

	class TQString;
	class TDEStorageDevice;
#endif

#ifdef WITH_UDISKS
//-------------------------------
//  UDisks
//-------------------------------
TQStringVariantMap udisksEjectDrive(TDEStorageDevice *sdevice);
TQStringVariantMap udisksMountDrive(const TQString &deviceNode, const TQString &fileSystemType, TQStringList mountOptions);
TQStringVariantMap udisksUnmountDrive(const TQString &deviceNode, TQStringList unmountOptions);
#endif

#ifdef WITH_UDISKS2
//-------------------------------
//  UDisks2
//-------------------------------
TQStringVariantMap udisks2EjectDrive(TDEStorageDevice *sdevice);
TQStringVariantMap udisks2MountDrive(const TQString &deviceNode, const TQString &fileSystemType, const TQString &mountOptions);
TQStringVariantMap udisks2UnmountDrive(const TQString &deviceNode, const TQString &unmountOptions);
TQStringVariantMap udisks2UnlockDrive(const TQString &deviceNode, const TQString &passphrase);
TQStringVariantMap udisks2LockDrive(const TQString &deviceNode);
#endif

#endif
