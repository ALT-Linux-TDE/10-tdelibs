/***************************************************************************
 *   Copyright (C) 2011-2014 Timothy Pearson <kb9vqf@pearsoncomputing.net> *
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
 *   51 Franklin Street, Fifth Floor, Boston, MA  02110-1301, USA.         *
 ***************************************************************************/

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <tdelocale.h>
#include <kdebug.h>
#include <tdeconfig.h>

#include <pwd.h>
#include <signal.h>

static const char description[] =
    I18N_NOOP("TDE composition manager detection utility");

static const char version[] = "0.1";

static TDECmdLineOptions options[] =
{
    TDECmdLineLastOption
};

int main(int argc, char **argv)
{
    TDEAboutData about("kdetcompmgr", I18N_NOOP("kdetcompmgr"), version, description,
		     TDEAboutData::License_GPL, "(C) 2011-2014 Timothy Pearson", 0, 0, "kb9vqf@pearsoncomputing.net");
    about.addAuthor( "Timothy Pearson", 0, "kb9vqf@pearsoncomputing.net" );
    TDECmdLineArgs::init(argc, argv, &about);
    TDECmdLineArgs::addCmdLineOptions( options );

    TDEApplication app;

    TDEConfig config("twinrc", true);
    config.setGroup( "Notification Messages" );
    if (!config.readBoolEntry("UseTranslucency",false)) {
        // Attempt to load the compton-tde pid file
        char *filename;
        const char *pidfile = "compton-tde.pid";
        char uidstr[sizeof(uid_t)*8+1];
        sprintf(uidstr, "%d", getuid());
        int n = strlen(P_tmpdir)+strlen(uidstr)+strlen(pidfile)+3;
        filename = (char*)malloc(n*sizeof(char)+1);
        memset(filename,0,n);
        strcat(filename, P_tmpdir);
        strcat(filename, "/.");
        strcat(filename, uidstr);
        strcat(filename, "-");
        strcat(filename, pidfile);

        // Now that we did all that by way of introduction...read the file!
        FILE *pFile;
        char buffer[255];
        pFile = fopen(filename, "r");
        int kompmgrpid = 0;
        if (pFile) {
            printf("[kdetcompmgr] Using '%s' as compton-tde pidfile\n", filename);
            // obtain file size
            fseek (pFile , 0 , SEEK_END);
            unsigned long lSize = ftell (pFile);
            if (lSize > 254)
                lSize = 254;
            rewind (pFile);
            fclose(pFile);
            kompmgrpid = atoi(buffer);
        }

        free(filename);
        filename = NULL;

        if (kompmgrpid) {
            kill(kompmgrpid, SIGTERM);
        }
    }

     if (app.detectCompositionManagerAvailable(false, false)) {		// Perform a shallow check for the composite extension (a deep check would cause noticeable flicker)
	TDEConfig config2("twinrc", true);
	config2.setGroup( "Notification Messages" );
	if (config2.readBoolEntry("UseTranslucency",false)) {
		app.detectCompositionManagerAvailable(true, true);
		return 2;
	}
	else {
		app.detectCompositionManagerAvailable(true, false);
		return 0;
	}
     }
     else {
	app.detectCompositionManagerAvailable(true, false);
	return 1;
     }
}

