/*
  Copyright (c) 2002 Maksim Orlovich <mo002j@mail.rochester.edu>

  Permission is hereby granted, free of charge, to any person obtaining a
  copy of this software and associated documentation files (the "Software"),
  to deal in the Software without restriction, including without limitation
  the rights to use, copy, modify, merge, publish, distribute, sublicense,
  and/or sell copies of the Software, and to permit persons to whom the
  Software is furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL
  THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
  DEALINGS IN THE SOFTWARE.
*/

#include <tqfileinfo.h>
#include <tqmap.h>
#include <tqstringlist.h>
#include <tqsettings.h>

#include <tdeapplication.h>
#include <tdecmdlineargs.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <ksimpleconfig.h>
#include <kstandarddirs.h>

static const char desc[] = I18N_NOOP("TDE Tool to build a cache list of all pixmap themes installed");
static const char ver[] = "0.9.1";

int main(int argc, char **argv)
{
    TDECmdLineArgs::init(argc, argv, "kinstalltheme", I18N_NOOP("KInstalltheme"), desc, ver);
    TDEApplication qapp(false, false); //We don't  allow styles.. Kind of ironic, isn't it?

    TDEGlobal::dirs()->addResourceType("themercs", TDEGlobal::dirs()->kde_default("data")+TQString("tdestyle/themes"));
    TQStringList themercs = TDEGlobal::dirs()->findAllResources("themercs","*.themerc");

    TQMap <TQString, TQString> themes; //Name->file mapping..

    for (TQStringList::iterator i = themercs.begin(); i!=themercs.end(); ++i)
    {
        TQString file=*i;
        KSimpleConfig config(file, true);
        TQString name = TQFileInfo(file).baseName(); //This is nice and static...
        //So we don't have to worry about our key changing when the language does.

        config.setGroup( "KDE" );

        if (config.readEntry( "widgetStyle" ) == "basicstyle.la")
        {
            //OK, emit a style entry...
            if (!themes.contains(name)) //Only add first occurrence, i.e. user local one.
                themes[name] = file;
        }
    }

    KSimpleConfig cache( TDEGlobal::dirs()->saveLocation("config")+"kthemestylerc");

#if 0
//Doesn't seem to work with present Qt..
	TQStringList existing = cache.subkeyList("/kthemestyle");
	for (TQStringList::iterator i = existing.begin(); i != existing.end(); i++)
	{
		cout<<"Have:"<<(*i).latin1()<<"\n";
		cache.removeEntry("/ktmthestyle"+(*i));
	}
#endif

    TQStringList themeNames; //A list of names, each occurring once - the keys of the themes map..

    for (TQMap<TQString, TQString>::Iterator  i = themes.begin(); i!=themes.end(); ++i)
    {
        cache.setGroup(i.key().lower());
        cache.writePathEntry("file",TQFileInfo(i.data()).fileName());
        themeNames.push_back(i.key());
    }

    cache.setGroup("General");
    cache.writeEntry("themes", themeNames.join("^e")+"^e");

    return 0;
}
