/***************************************************************************
 *   Copyright (C) 2013 by Timothy Pearson                                 *
 *   kb9vqf@pearsoncomputing.net                                           *
 *                                                                         *
 *   This program is free software; you can redistribute it and/or modify  *
 *   it under the terms of the GNU General Public License as published by  *
 *   the Free Software Foundation; either version 2 of the License, or     *
 *   (at your option) any later version.                                   *
 *                                                                         *
 *   This program is distributed in the hope that it will be useful,       *
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of        *
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the         *
 *   GNU General Public License for more details.                          *
 *                                                                         *
 *   You should have received a copy of the GNU General Public License     *
 *   along with this program; if not, write to the                         *
 *   Free Software Foundation, Inc.,                                       *
 *   59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.             *
 ***************************************************************************/

#include <cstdlib>

#include <tdeapplication.h>
#include <tdestartupinfo.h>
#include <tdecmdlineargs.h>
#include <kuniqueapplication.h>
#include <tdeaboutdata.h>

#include "libisofs/iso_fs.h"

#include "kiso.h"
#include "kisofile.h"
#include "kisodirectory.h"
#include "iso.h"

static const char description[] =
	I18N_NOOP("TDE utility for getting ISO information");

static const char version[] = "v0.0.1";

static const TDECmdLineOptions options[] =
{
	{ "exists <filename>", I18N_NOOP("Returns 0 if the file exists, -1 if it does not"), 0 },
	{ "!+device", I18N_NOOP("The device on which to execute the specified command.  Example: /dev/sr0"), 0 },
	TDECmdLineLastOption // End of options.
};

int main(int argc, char *argv[])
{
	TDEAboutData aboutData( "tdeiso_info", I18N_NOOP("TDE ISO information utility"),
		version, description, TDEAboutData::License_GPL,
		"(c) 2013, Timothy Pearson");
		aboutData.addAuthor("Timothy Pearson",0, "kb9vqf@pearsoncomputing.net");
	TDECmdLineArgs::init(argc, argv, &aboutData);
 	TDECmdLineArgs::addCmdLineOptions(options);
	KUniqueApplication::addCmdLineOptions();
	TDEApplication::disableAutoDcopRegistration();

	TDEApplication app(false, false, false);

	//======================================================================================================================================================
	//
	// Main code follows
	//
	//======================================================================================================================================================

	TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

	if (args->count() > 0) {
		TQString device = TQString(args->arg(0));

		if (args->isSet("exists")) {
			KIso isoFile( device );
			if ( !isoFile.open( IO_ReadOnly ) ) {
				printf("Unable to open device '%s'\n\r", device.ascii());
				return -2;
			}
			TQString fileToFind = args->getOption("exists");

			const KArchiveDirectory* entries = isoFile.directory();
			if (!entries) {
				return -1;
			}
			if (entries->entry(fileToFind)) {
				return 0;
			}
			else {
				return -1;
			}
		}
	}
	else {
		TDECmdLineArgs::usage(i18n("No device was specified"));
		return -3;
	}

	//======================================================================================================================================================

	return 0;
}
