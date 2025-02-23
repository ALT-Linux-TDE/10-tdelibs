/* This file is part of the KDE project
 * Copyright (C) 2012 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 * This program is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public
 * License as published by the Free Software Foundation version 2.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; see the file COPYING.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 *
 */

#include <config.h>
#include "tdefile_elf.h"

#include <kprocess.h>
#include <tdelocale.h>
#include <kgenericfactory.h>
#include <kstringvalidator.h>
#include <kdebug.h>

#include <tqdict.h>
#include <tqvalidator.h>
#include <tqcstring.h>
#include <tqfile.h>
#include <tqdatetime.h>

#include "tdelficon.h"

#if !defined(__osf__)
#include <inttypes.h>
#else
typedef unsigned long uint32_t;
typedef unsigned short uint16_t;
#endif

typedef KGenericFactory<KElfPlugin> ElfFactory;

K_EXPORT_COMPONENT_FACTORY(tdefile_elf, ElfFactory( "tdefile_elf" ))

KElfPlugin::KElfPlugin(TQObject *parent, const char *name,
                       const TQStringList &args)

    : KFilePlugin(parent, name, args)
{
    const TQCString elfMimeTypes[]= {
        "application/x-executable",
        "application/x-pie-executable",
        "application/x-sharedlib"
    };
    const int elfMimeTypesCount = sizeof(elfMimeTypes)/sizeof(elfMimeTypes[0]);

    for (int i = 0; i < elfMimeTypesCount; i++)
    {
        KFileMimeTypeInfo* info = addMimeTypeInfo(elfMimeTypes[i]);

        KFileMimeTypeInfo::GroupInfo* group = 0L;
        KFileMimeTypeInfo::GroupInfo* group2 = 0L;

        group = addGroupInfo(info, "Technical", i18n("Embedded Metadata"));
        group2 = addGroupInfo(info, "Icon", i18n("Embedded Icon(s)"));

        KFileMimeTypeInfo::ItemInfo* item;

        item = addItemInfo(group, "Name", i18n("Internal Name"), TQVariant::String);
        item = addItemInfo(group, "Description", i18n("Description"), TQVariant::String);
        item = addItemInfo(group, "License", i18n("License"), TQVariant::String);
        item = addItemInfo(group, "Copyright", i18n("Copyright"), TQVariant::String);
        item = addItemInfo(group, "Authors", i18n("Author(s)"), TQVariant::String);
        item = addItemInfo(group, "Product", i18n("Product"), TQVariant::String);
        item = addItemInfo(group, "Organization", i18n("Organization"), TQVariant::String);
        item = addItemInfo(group, "Version", i18n("Version"), TQVariant::String);
        item = addItemInfo(group, "DateTime", i18n("Compilation Date/Time"), TQVariant::String);
        item = addItemInfo(group, "SystemIcon", i18n("Requested Icon"), TQVariant::String);
        item = addItemInfo(group, "SCMModule", i18n("SCM Module"), TQVariant::String);
        item = addItemInfo(group, "SCMRevision", i18n("SCM Revision"), TQVariant::String);
        item = addItemInfo(group, "Notes", i18n("Comments"), TQVariant::String);

        item = addItemInfo(group2, "EmbeddedIcon", i18n("Icon Name(s)"), TQVariant::String);
    }
}


bool KElfPlugin::readInfo( KFileMetaInfo& info, uint what)
{
	libr_icon *icon = NULL;
	libr_file *handle = NULL;
	libr_access_t access = LIBR_READ;

	if((handle = libr_open(const_cast<char*>(info.path().ascii()), access)) == NULL)
	{
		kdWarning() << "failed to open file" << info.path() << endl;
		return false;
	}

	KFileMetaInfoGroup group = appendGroup(info, "Technical");
	KFileMetaInfoGroup group2 = appendGroup(info, "Icon");

	appendItem(group, "Name", elf_get_resource(handle, ".metadata_name"));
	appendItem(group, "Description", elf_get_resource(handle, ".metadata_description"));
	appendItem(group, "License", elf_get_resource(handle, ".metadata_license"));
	appendItem(group, "Copyright", elf_get_resource(handle, ".metadata_copyright"));
	appendItem(group, "Authors", elf_get_resource(handle, ".metadata_authors"));
	appendItem(group, "Product", elf_get_resource(handle, ".metadata_product"));
	appendItem(group, "Organization", elf_get_resource(handle, ".metadata_organization"));
	appendItem(group, "Version", elf_get_resource(handle, ".metadata_version"));
	appendItem(group, "DateTime", elf_get_resource(handle, ".metadata_datetime"));
	appendItem(group, "SystemIcon", elf_get_resource(handle, ".metadata_sysicon"));
	appendItem(group, "SCMModule", elf_get_resource(handle, ".metadata_scmmodule"));
	appendItem(group, "SCMRevision", elf_get_resource(handle, ".metadata_scmrevision"));
	appendItem(group, "Notes", elf_get_resource(handle, ".metadata_notes"));

	TQString iconListing;

	iconentry *entry = NULL;
	iconlist icons;
	if(!get_iconlist(handle, &icons))
	{
		// Failed to obtain a list of ELF icons
	}
	else {
		while((entry = get_nexticon(&icons, entry)) != NULL)
		{
			if (iconListing.isEmpty()) {
				iconListing = entry->name;
			}
			else {
				iconListing = iconListing.append("\n").append(entry->name);
			}
			break;
		}
	}

	appendItem(group2, "EmbeddedIcon", iconListing);

	libr_close(handle);

	return true;
}

#include "tdefile_elf.moc"
