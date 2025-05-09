/* This file is part of the KDE libraries
    Copyright (C) 2003 Joseph Wenninger <jowenn@kde.org>
                  2003 Andras Mantia <amantia@freemail.hu>

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

#include "config-tdefile.h"

#include "kencodingfiledialog.h"
#include <kcombobox.h>
#include <tdetoolbar.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kcharsets.h>
#include <tqtextcodec.h>
#include <tdediroperator.h>
#include <tderecentdocument.h>

struct KEncodingFileDialogPrivate
{
    KComboBox *encoding;
};

KEncodingFileDialog::KEncodingFileDialog(const TQString& startDir, const TQString& encoding , const TQString& filter,
			 const TQString& caption, KFileDialog::OperationMode type, TQWidget *parent, const char* name, bool modal)
   : KFileDialog(startDir,filter,parent,name,modal), d(new KEncodingFileDialogPrivate)
{
  setCaption(caption);
  
  setOperationMode( type );
    
  TDEToolBar *tb = toolBar();
  tb->insertSeparator();
  int index = tb->insertCombo(TQStringList(), -1 /*id*/, false /*writable*/, 0 /*signal*/, 0 /*receiver*/, 0 /*slot*/ );
  d->encoding = tb->getCombo( tb->idAt( index ) );
  if ( !d->encoding )
      return;

  d->encoding->clear ();
  TQString sEncoding = encoding;
  if (sEncoding.isEmpty())
     sEncoding = TQString::fromLatin1(TDEGlobal::locale()->encoding());
  
  TQStringList encodings (TDEGlobal::charsets()->availableEncodingNames());
  int insert = 0;
  for (uint i=0; i < encodings.count(); i++)
  {
    bool found = false;
    TQTextCodec *codecForEnc = TDEGlobal::charsets()->codecForName(encodings[i], found);

    if (found)
    {
      d->encoding->insertItem (encodings[i]);
      if ( (codecForEnc->name() == sEncoding) || (encodings[i] == sEncoding) )
      {
        d->encoding->setCurrentItem(insert);
      }

      insert++;
    }
  }
        
     
}

KEncodingFileDialog::~KEncodingFileDialog()
{
    delete d;
}


TQString KEncodingFileDialog::selectedEncoding() const
{
  if (d->encoding)
     return d->encoding->currentText();
  else
    return TQString::null;     
}


KEncodingFileDialog::Result KEncodingFileDialog::getOpenFileNameAndEncoding(const TQString& encoding,
 				     const TQString& startDir,
                                     const TQString& filter,
                                     TQWidget *parent, const TQString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent, 
	"filedialog", true);

    dlg.setMode( KFile::File | KFile::LocalOnly );
    dlg.ops->clearHistory();
    dlg.exec();
 
    Result res;
    res.fileNames<<dlg.selectedFile();
    res.encoding=dlg.selectedEncoding();	
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenFileNamesAndEncoding(const TQString& encoding,
					  const TQString& startDir,
                                          const TQString& filter,
                                          TQWidget *parent,
                                          const TQString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent, 
	"filedialog", true);
    dlg.setMode(KFile::Files | KFile::LocalOnly);
    dlg.ops->clearHistory();
    dlg.exec();

    Result res;
    res.fileNames=dlg.selectedFiles();
    res.encoding=dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenURLAndEncoding(const TQString& encoding, const TQString& startDir, 
				const TQString& filter, TQWidget *parent, const TQString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent, 
		"filedialog", true);

    dlg.setMode( KFile::File );
    dlg.ops->clearHistory();
    dlg.exec();

    Result res;
    res.URLs<<dlg.selectedURL();
    res.encoding=dlg.selectedEncoding();
    return res;
}

KEncodingFileDialog::Result KEncodingFileDialog::getOpenURLsAndEncoding(const TQString& encoding, const TQString& startDir,
                                          const TQString& filter,
                                          TQWidget *parent,
                                          const TQString& caption)
{
    KEncodingFileDialog dlg(startDir, encoding,filter,caption.isNull() ? i18n("Open") : caption,Opening,parent, 
	"filedialog", true);

    dlg.setMode(KFile::Files);
    dlg.ops->clearHistory();
    dlg.exec();

    Result res;
    res.URLs=dlg.selectedURLs();
    res.encoding=dlg.selectedEncoding();
    return res;
}


KEncodingFileDialog::Result KEncodingFileDialog::getSaveFileNameAndEncoding(const TQString& encoding,
			             const TQString& dir, 
				     const TQString& filter,
                                     TQWidget *parent,
                                     const TQString& caption)
{
    bool specialDir = dir.at(0) == ':';
    KEncodingFileDialog dlg(specialDir?dir:TQString::null, encoding,filter,caption.isNull() ? i18n("Save As") : caption,
	Saving,parent, "filedialog", true);

    if ( !specialDir )
        dlg.setSelection( dir ); // may also be a filename
    dlg.exec();

    TQString filename = dlg.selectedFile();
    if (!filename.isEmpty())
        TDERecentDocument::add(filename);

    Result res;
    res.fileNames<<filename;
    res.encoding=dlg.selectedEncoding();
    return res;
}


KEncodingFileDialog::Result  KEncodingFileDialog::getSaveURLAndEncoding(const TQString& encoding,
			     const TQString& dir, const  TQString& filter,
                             TQWidget *parent, const TQString& caption)
{
    bool specialDir = dir.at(0) == ':';
    KEncodingFileDialog dlg(specialDir?dir:TQString::null, encoding,filter,caption.isNull() ? i18n("Save As") : 
	caption, Saving,parent, "filedialog", true);

    if ( !specialDir )
    dlg.setSelection( dir ); // may also be a filename

    dlg.exec();

    KURL url = dlg.selectedURL();
    if (url.isValid())
        TDERecentDocument::add( url );

    Result res;
    res.URLs<<url;
    res.encoding=dlg.selectedEncoding();
    return res;
}



void KEncodingFileDialog::virtual_hook( int id, void* data ) 
{
 KFileDialog::virtual_hook( id, data ); 
}


#include "kencodingfiledialog.moc"
