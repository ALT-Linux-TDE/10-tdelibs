/*
   This file is part of the KDE libraries
   Copyright (C) 2004-2005 Jaroslaw Staniek <js@iidea.pl>

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

#include <tqstring.h>
#include <tqdir.h>
#include <tqfileinfo.h>
#include <tqstringlist.h>

#include <windows.h>
#include <shellapi.h>
#include <tchar.h>

#define FAILURE            \
	{                        \
		if (ok)                \
		{                      \
			*ok = false;         \
		}                      \
		return TQString::null; \
	}

KDEWIN32_EXPORT
TQString getWin32RegistryValue(HKEY key, const TQString &subKey, const TQString &item, bool *ok)
{
	if (!subKey)
	{
		FAILURE;
	}
	HKEY hKey;
	TCHAR *lszValue;
	DWORD dwType = REG_SZ;
	DWORD dwSize;
	if (ERROR_SUCCESS != RegOpenKeyEx(key, subKey.ucs2(), NULL, KEY_READ, &hKey))
	{
		FAILURE;
	}

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, item.ucs2(), NULL, NULL, NULL, &dwSize))
	{
		FAILURE;
	}

	lszValue = new TCHAR[dwSize];

	if (ERROR_SUCCESS != RegQueryValueEx(hKey, item.ucs2(), NULL, &dwType, (LPBYTE)lszValue, &dwSize))
	{
		delete[] lszValue;
		FAILURE;
	}
	RegCloseKey(hKey);

	TQString res = TQString::fromUcs2(lszValue);
	delete[] lszValue;
	return res;
}

KDEWIN32_EXPORT
bool showWin32FilePropertyDialog(const TQString &fileName)
{
	TQString path_ = TQDir::convertSeparators(TQFileInfo(fileName).absFilePath());

	SHELLEXECUTEINFO execInfo;
	memset(&execInfo, 0, sizeof(execInfo));
	execInfo.cbSize = sizeof(execInfo);
	execInfo.fMask  = SEE_MASK_INVOKEIDLIST | SEE_MASK_NOCLOSEPROCESS | SEE_MASK_FLAG_NO_UI;
	const TQString verb("properties");
	execInfo.lpVerb = (TCHAR *)verb.ucs2();
	execInfo.lpFile = (TCHAR *)path_.ucs2();
	return ShellExecuteEx(&execInfo);
}

KDEWIN32_EXPORT
TQCString getWin32LocaleName()
{
	bool ok;
	TQString localeNumber = getWin32RegistryValue(HKEY_CURRENT_USER, "Control Panel\\International",
	                                              "Locale", &ok);
	if (!ok)
	{
		return TQCString();
	}
	TQString localeName = getWin32RegistryValue(HKEY_LOCAL_MACHINE,
	                                            "SYSTEM\\CurrentControlSet\\Control\\Keyboard Layout\\DosKeybCodes", localeNumber,
	                                            &ok);
	if (!ok)
	{
		return TQCString();
	}
	return localeName.latin1();
}

KDEWIN32_EXPORT
TQString convertKFileDialogFilterToQFileDialogFilter(const TQString &filter)
{
	TQString kde_filters = filter;
	int pos;
	// Strip the escape characters from
	// escaped '/' characters.

	TQString copy(kde_filters);
	for (pos = 0; (pos = copy.find("\\/", pos)) != -1; ++pos)
	{
		copy.remove(pos, 1);
	}

	//<js>
	//we need to convert KDE filter format to Qt format
	//Qt format: "some text (*.first *.second)" or "All (*)"
	//KDE format: "*.first *.second" or "*"
	TQStringList filters = TQStringList::split("\n", kde_filters);
	TQString current;
	TQString converted; //finally - converted filter
	for (TQStringList::ConstIterator it = filters.constBegin(); it != filters.constEnd(); ++it)
	{
		current = *it;
		TQString new_f; //filter part
		TQString new_name; //filter name part
		int p = (*it).find('|');
		if (p != -1)
		{
			new_f    = current.left(p);
			new_name = current.mid(p + 1);
		}
		else
		{
			new_f    = current;
			new_name = current; //nothing better
		}
		//remove (.....) from name
		p = new_name.find('(');
		int p2 = new_name.findRev(')');
		TQString new_name1, new_name2;
		if (p != -1)
		{
			new_name1 = new_name.left(p);
		}
		if (p2 != -1)
		{
			new_name2 = new_name.mid(p2 + 1);
		}
		if (!new_name1.isEmpty() || !new_name2.isEmpty())
		{
			new_name = new_name1.stripWhiteSpace() + " " + new_name2.stripWhiteSpace();
		}
		new_name.replace('(', "");
		new_name.replace(')', "");
		new_name = new_name.stripWhiteSpace();

		// make filters unique: remove uppercase extensions (case doesn't matter on win32, BTW)
		TQStringList allfiltersUnique;
		TQStringList origList(TQStringList::split(" ", new_f));
		for (TQStringList::ConstIterator it = origList.constBegin();
		     it != origList.constEnd(); ++it)
		{
			if ((*it) == (*it).lower())
			{
				allfiltersUnique += *it;
			}
		}

		if (!converted.isEmpty())
		{
			converted += ";;";
		}

		converted += (new_name + " (" + allfiltersUnique.join(" ") + ")");
	}
	return converted;
}

