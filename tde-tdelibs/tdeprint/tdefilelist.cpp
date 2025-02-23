/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "tdefilelist.h"

#include <tqtoolbutton.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqtooltip.h>
#include <tqheader.h>
#include <tqwhatsthis.h>

#include <tdeio/netaccess.h>
#include <kurldrag.h>
#include <tdefiledialog.h>
#include <tdelocale.h>
#include <kiconloader.h>
#include <tdelistview.h>
#include <krun.h>
#include <kmimetype.h>

KFileList::KFileList(TQWidget *parent, const char *name)
: TQWidget(parent, name)
{
	//WhatsThis strings.... (added by pfeifle@kde.org)
	TQString whatsThisAddFileButton = i18n(  " <qt> <b>Add File button</b>"
						" <p>This button calls the <em>'File Open'</em> dialog to let you"
						" select a file for printing. Note, that "
						" <ul><li>you can select ASCII or International Text, PDF,"
						" PostScript, JPEG, TIFF, PNG, GIF and many other graphic"
						" formats."
						" <li>you can select various files from different paths"
						" and send them as one \"multi-file job\" to the printing"
						" system."
						" </ul>"
					        " </qt>" );

	TQString whatsThisRemoveFileButton = i18n(" <qt> <b>Remove File button</b>"
                                                " <p>This button removes the highlighted file from the"
						" list of to-be-printed files."
					        " </qt>" );

	TQString whatsThisMoveFileUpButton = i18n(" <qt> <b>Move File Up button</b>"
                                                " <p>This button moves the highlighted file up in the list"
						" of files to be printed.</p>"
						" <p>In effect, this changes the order"
						" of the files' printout.</p>"
					        " </qt>" );

	TQString whatsThisMoveFileDownButton = i18n(" <qt> <b>Move File Down button</b>"
                                                " <p>This button moves the highlighted file down in the list"
						" of files to be printed.</p>"
						" <p>In effect, this changes the order"
						" of the files' printout.</p>"
					        " </qt>" );

	TQString whatsThisOpenFileButton = i18n( " <qt> <b>File Open button</b>"
                                                " <p>This button tries to open the highlighted file, so"
						" you can view or edit it before you send it to the printing"
						" system.</p>"
						" <p>If you open"
						" files, TDEPrint will use the application matching the MIME type of"
						" the file.</p>"
					        " </qt>" );

	TQString whatsThisFileSelectionListview = i18n( " <qt> <b>File List view</b>"
                                                " <p>This list displays all the files you selected for printing."
						" You can see the file name(s), file path(s) and the file"
						" (MIME) type(s) as determined by TDEPrint. You may re-arrange the "
						" initial order of the list "
						" with the help of the arrow buttons on the right.</p>"
						" <p>The files will be printed as a single job,"
						" in the same order as displayed in the list.</p>"
						" <p><b>Note:</b> You can select multiple files. The files may be in multiple"
						" locations. The files may be of multiple MIME types. The buttons on the right"
						" side let you add more files, remove already selected files from the list, "
						" re-order the list (by moving files up or down), and open files. If you open"
						" files, TDEPrint will use the application matching the MIME type of"
						" the file.</p>"
					        " </qt>" );

	m_block = false;

	m_files = new TDEListView(this);
	m_files->addColumn(i18n("Name"));
	m_files->addColumn(i18n("Type"));
	m_files->addColumn(i18n("Path"));
	m_files->setAllColumnsShowFocus(true);
	m_files->setSorting(-1);
	m_files->setAcceptDrops(false);
	m_files->setSelectionMode(TQListView::Extended);
	m_files->header()->setStretchEnabled(true, 2);
	TQWhatsThis::add(m_files, whatsThisFileSelectionListview);
	connect(m_files, TQ_SIGNAL(selectionChanged()), TQ_SLOT(slotSelectionChanged()));

	m_add = new TQToolButton(this);
	m_add->setIconSet(SmallIconSet("document-open"));
	connect(m_add, TQ_SIGNAL(clicked()), TQ_SLOT(slotAddFile()));
	TQToolTip::add(m_add, i18n("Add file"));
	TQWhatsThis::add(m_add, whatsThisAddFileButton);

	m_remove = new TQToolButton(this);
	m_remove->setIconSet(SmallIconSet("remove"));
	connect(m_remove, TQ_SIGNAL(clicked()), TQ_SLOT(slotRemoveFile()));
	TQToolTip::add(m_remove, i18n("Remove file"));
	TQWhatsThis::add(m_remove, whatsThisRemoveFileButton);
	m_remove->setEnabled(false);

	m_open = new TQToolButton(this);
	m_open->setIconSet(SmallIconSet("filefind"));
	connect(m_open, TQ_SIGNAL(clicked()), TQ_SLOT(slotOpenFile()));
	TQToolTip::add(m_open, i18n("Open file"));
	TQWhatsThis::add(m_open, whatsThisOpenFileButton);
	m_open->setEnabled(false);

	m_up = new TQToolButton(this);
	m_up->setIconSet(SmallIconSet("go-up"));
	connect(m_up, TQ_SIGNAL(clicked()), TQ_SLOT(slotUp()));
	TQToolTip::add(m_up, i18n("Move up"));
	TQWhatsThis::add(m_up, whatsThisMoveFileUpButton);
	m_up->setEnabled(false);

	m_down = new TQToolButton(this);
	m_down->setIconSet(SmallIconSet("go-down"));
	connect(m_down, TQ_SIGNAL(clicked()), TQ_SLOT(slotDown()));
	TQToolTip::add(m_down, i18n("Move down"));
	TQWhatsThis::add(m_down, whatsThisMoveFileDownButton);
	m_down->setEnabled(false);

	setAcceptDrops(true);

	TQToolTip::add(m_files, i18n(
		"Drag file(s) here or use the button to open a file dialog. "
		"Leave empty for <b>&lt;STDIN&gt;</b>."));

	TQHBoxLayout	*l0 = new TQHBoxLayout(this, 0, KDialog::spacingHint());
	TQVBoxLayout	*l1 = new TQVBoxLayout(0, 0, 1);
	l0->addWidget(m_files);
	l0->addLayout(l1);
	l1->addWidget(m_add);
	l1->addWidget(m_remove);
	l1->addWidget(m_open);
	l1->addSpacing(10);
	l1->addWidget(m_up);
	l1->addWidget(m_down);
	l1->addStretch(1);
}

KFileList::~KFileList()
{
}

void KFileList::dragEnterEvent(TQDragEnterEvent *e)
{
	e->accept(KURLDrag::canDecode(e));
}

void KFileList::dropEvent(TQDropEvent *e)
{
	KURL::List	files;
	if (KURLDrag::decode(e, files))
	{
		addFiles(files);
	}
}

void KFileList::addFiles(const KURL::List& files)
{
	if (files.count() > 0)
	{
		// search last item in current list, to add new ones at the end
		TQListViewItem	*item = m_files->firstChild();
		while (item && item->nextSibling())
			item = item->nextSibling();

		for (KURL::List::ConstIterator it=files.begin(); it!=files.end(); ++it)
		{
			KMimeType::Ptr	mime = KMimeType::findByURL( *it, 0, true, false);
			item = new TQListViewItem(m_files, item, (*it).fileName(), mime->comment(), (*it).url());
			item->setPixmap(0, mime->pixmap(*it, TDEIcon::Small));
		}

		slotSelectionChanged();
		/*
		if (m_files->childCount() > 0)
		{
			m_remove->setEnabled(true);
			m_open->setEnabled(true);
			if (m_files->currentItem() == 0)
				m_files->setSelected(m_files->firstChild(), true);
		}
		*/
	}
}

void KFileList::setFileList(const TQStringList& files)
{
	m_files->clear();
	TQListViewItem *item = 0;
	for (TQStringList::ConstIterator it=files.begin(); it!=files.end(); ++it)
	{
		KURL	url = KURL::fromPathOrURL( *it );
		KMimeType::Ptr	mime = KMimeType::findByURL(url, 0, true, false);
		item = new TQListViewItem(m_files, item, url.fileName(), mime->comment(), url.url());
		item->setPixmap(0, mime->pixmap(url, TDEIcon::Small));
	}
	slotSelectionChanged();
}

TQStringList KFileList::fileList() const
{
	TQStringList	l;
	TQListViewItem	*item = m_files->firstChild();
	while (item)
	{
		l << item->text(2);
		item = item->nextSibling();
	}
	return l;
}

void KFileList::slotAddFile()
{
	KURL::List fnames = KFileDialog::getOpenURLs(TQString::null, TQString::null, this);
	if (!fnames.empty())
		addFiles(fnames);
}

void KFileList::slotRemoveFile()
{
	TQPtrList<TQListViewItem>	l;
	selection(l);
	l.setAutoDelete(true);
	m_block = true;
	l.clear();
	m_block = false;
	slotSelectionChanged();
}

void KFileList::slotOpenFile()
{
	TQListViewItem	*item = m_files->currentItem();
	if (item)
	{
		KURL url( item->text( 2 ) );
		new KRun(url);
	}
}

TQSize KFileList::sizeHint() const
{
	return TQSize(100, 100);
}

void KFileList::selection(TQPtrList<TQListViewItem>& l)
{
	l.setAutoDelete(false);
	TQListViewItem	*item = m_files->firstChild();
	while (item)
	{
		if (item->isSelected())
			l.append(item);
		item = item->nextSibling();
	}
}

void KFileList::slotSelectionChanged()
{
	if (m_block)
		return;

	TQPtrList<TQListViewItem>	l;
	selection(l);
	m_remove->setEnabled(l.count() > 0);
	m_open->setEnabled(l.count() == 1);
	m_up->setEnabled(l.count() == 1 && l.first()->itemAbove());
	m_down->setEnabled(l.count() == 1 && l.first()->itemBelow());
}

void KFileList::slotUp()
{
	TQPtrList<TQListViewItem>	l;
	selection(l);
	if (l.count() == 1 && l.first()->itemAbove())
	{
		TQListViewItem	*item(l.first()), *clone;
		clone = new TQListViewItem(m_files, item->itemAbove()->itemAbove(), item->text(0), item->text(1), item->text(2));
		clone->setPixmap(0, *(item->pixmap(0)));
		delete item;
		m_files->setCurrentItem(clone);
		m_files->setSelected(clone, true);
	}
}

void KFileList::slotDown()
{
	TQPtrList<TQListViewItem>	l;
	selection(l);
	if (l.count() == 1 && l.first()->itemBelow())
	{
		TQListViewItem	*item(l.first()), *clone;
		clone = new TQListViewItem(m_files, item->itemBelow(), item->text(0), item->text(1), item->text(2));
		clone->setPixmap(0, *(item->pixmap(0)));
		delete item;
		m_files->setCurrentItem(clone);
		m_files->setSelected(clone, true);
	}
}

#include "tdefilelist.moc"
