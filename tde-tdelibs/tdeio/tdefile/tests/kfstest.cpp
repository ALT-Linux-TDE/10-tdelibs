/* This file is part of the KDE libraries
    Copyright (C) 1997, 1998 Richard Moore <rich@kde.org>
                  1998 Stephan Kulow <coolo@kde.org>

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

#include <unistd.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <tqdir.h>
#include <tqlayout.h>
#include <tqstringlist.h>
#include <tqwidget.h>

#include <tdefiledialog.h>
#include <tdefileiconview.h>
#include <tdemessagebox.h>
#include <tdeconfig.h>
#include <tdeapplication.h>
#include <kurl.h>
#include <kurlbar.h>
#include <tdediroperator.h>
#include <tdefile.h>
#include <kdebug.h>
#include <kicondialog.h>

#include "kfdtest.h"

int main(int argc, char **argv)
{
    TDEApplication a(argc, argv, "kfstest", true);
    TQString name1;
    TQStringList names;

    TQString argv1;
    TQString startDir;
    if (argc > 1)
	argv1 = TQString::fromLatin1(argv[1]);
    if ( argc > 2 )
        startDir = TQString::fromLatin1( argv[2]);

    if (argv1 == TQString::fromLatin1("diroperator")) {
	KDirOperator *op = new KDirOperator(startDir, 0, "operator");
	op->setViewConfig( TDEGlobal::config(), "TestGroup" );
	op->setView(KFile::Simple);
	op->show();
	a.setMainWidget(op);
	a.exec();
    }

    else if (argv1 == TQString::fromLatin1("justone")) {
        TQString name = KFileDialog::getOpenFileName(startDir);
        tqDebug("filename=%s",name.latin1());
    }

    else if (argv1 == TQString::fromLatin1("existingURL")) {
        KURL url = KFileDialog::getExistingURL();
        tqDebug("URL=%s",url.url().latin1());
        name1 = url.url();
    }

    else if (argv1 == TQString::fromLatin1("preview")) {
        KURL u =  KFileDialog::getImageOpenURL();
        tqDebug("filename=%s", u.url().latin1());
    }

    else if (argv1 == TQString::fromLatin1("preselect")) {
        names = KFileDialog::getOpenFileNames(TQString::fromLatin1("/etc/passwd"));
        TQStringList::Iterator it = names.begin();
        while ( it != names.end() ) {
            tqDebug("selected file: %s", (*it).latin1());
            ++it;
        }
    }

    else if (argv1 == TQString::fromLatin1("dirs"))
	name1 = KFileDialog::getExistingDirectory();

    else if (argv1 == TQString::fromLatin1("heap")) {
	KFileDialog *dlg = new KFileDialog( startDir, TQString::null, 0L,
					    "file dialog", true );
	dlg->setMode( KFile::File);
    dlg->setOperationMode( KFileDialog::Saving );
    TQStringList filter;
    filter << "all/allfiles" << "text/plain";
    dlg->setMimeFilter( filter, "all/allfiles" );
    KURLBar *urlBar = dlg->speedBar();
    if ( urlBar )
    {
        urlBar->insertDynamicItem( KURL("ftp://ftp.kde.org"), 
                                   TQString::fromLatin1("KDE FTP Server") );
    }

	if ( dlg->exec() == KDialog::Accepted )
	    name1 = dlg->selectedURL().url();
    }

    else if ( argv1 == TQString::fromLatin1("eventloop") )
    {
        KFDTest *test = new KFDTest( startDir );
        return a.exec();
    }

    else if (argv1 == TQString::fromLatin1("save")) {
        KURL u = KFileDialog::getSaveURL();
//        TQString(TQDir::homeDirPath() + TQString::fromLatin1("/testfile")),
//        TQString::null, 0L);
        name1 = u.url();
    }

    else if (argv1 == TQString::fromLatin1("icon")) {
    	TDEIconDialog dlg;
	TQString icon = dlg.selectIcon();
	kdDebug() << icon << endl;
    }

//     else if ( argv1 == TQString::fromLatin1("dirselect") ) {
//         KURL url;
//         url.setPath( "/" );
//         KURL selected = KDirSelectDialog::selectDirectory( url );
//         name1 = selected.url();
//         tqDebug("*** selected: %s", selected.url().latin1());
//     }

    else {
	KFileDialog dlg(startDir,
			TQString::fromLatin1("*|All Files\n"
					    "*.lo *.o *.la|All libtool Files"),
			0, 0, true);
//    dlg.setFilter( "*.tdevelop" );
	dlg.setMode( (KFile::Mode) (KFile::Files |
                                    KFile::Directory |
                                    KFile::ExistingOnly |
                                    KFile::LocalOnly) );
//         TQStringList filter;
//         filter << "text/plain" << "text/html" << "image/png";
//        dlg.setMimeFilter( filter );
//    KMimeType::List types;
//    types.append( KMimeType::mimeType( "text/plain" ) );
//    types.append( KMimeType::mimeType( "text/html" ) );
//    dlg.setFilterMimeType( "Filetypes:", types, types.first() );
	if ( dlg.exec() == TQDialog::Accepted ) {
	    KURL::List list = dlg.selectedURLs();
	    KURL::List::ConstIterator it = list.begin();
            tqDebug("*** selectedURLs(): ");
	    while ( it != list.end() ) {
		name1 = (*it).url();
		tqDebug("  -> %s", name1.latin1());
		++it;
            }
            tqDebug("*** selectedFile: %s", dlg.selectedFile().latin1());
            tqDebug("*** selectedURL: %s", dlg.selectedURL().url().latin1());
            tqDebug("*** selectedFiles: ");
            TQStringList l = dlg.selectedFiles();
            TQStringList::Iterator it2 = l.begin();
            while ( it2 != l.end() ) {
                tqDebug("  -> %s", (*it2).latin1());
                ++it2;
            }
	}
    }

    if (!(name1.isNull()))
	KMessageBox::information(0, TQString::fromLatin1("You selected the file " ) + name1,
				 TQString::fromLatin1("Your Choice"));
    return 0;
}
