/*
    This file is part of KOrganizer.
    Copyright (c) 2002 Cornelius Schumacher <schumacher@kde.org>

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

#include <tdeapplication.h>
#include <kdebug.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>
#include <tdeaboutdata.h>

#include "downloaddialog.h"

static const TDECmdLineOptions op[] =
{
	{"type <type>", I18N_NOOP("Display only media of this type"), 0},
	{"+[providerlist]", I18N_NOOP("Provider list to use"), 0},
	TDECmdLineLastOption
};

int main(int argc, char **argv)
{
	TDEAboutData about("tdehotnewstuff", "TDEHotNewStuff", "0.2");
	TDECmdLineArgs *args;

	TDECmdLineArgs::init(argc, argv, &about);
	TDECmdLineArgs::addCmdLineOptions(op);
	args = TDECmdLineArgs::parsedArgs();

	TDEApplication i;

	KNS::DownloadDialog d;
	if(args->isSet("type")) d.setType(args->getOption("type"));
	if(args->count() == 1) d.setProviderList(args->arg(0));
	d.load();
	d.exec();

	return 0;
}

