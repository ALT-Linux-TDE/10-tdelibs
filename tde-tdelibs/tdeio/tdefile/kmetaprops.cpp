/* This file is part of the KDE libraries
    Copyright (C) 2001,2002 Rolf Magnus <ramagnus@kde.org>

    library is free software; you can redistribute it and/or
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
  
    $Id$
 */

#include "kmetaprops.h"

#include <kdebug.h>
#include <tdefilemetainfowidget.h>
#include <tdefilemetainfo.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tdelocale.h>
#include <kprotocolinfo.h>

#include <tqvalidator.h>
#include <tqlayout.h>
#include <tqlabel.h>
#include <tqfileinfo.h>
#include <tqdatetime.h>
#include <tqstylesheet.h>
#include <tqvgroupbox.h>

#undef Bool

class MetaPropsScrollView : public TQScrollView
{
public:
    MetaPropsScrollView(TQWidget* parent = 0, const char* name = 0)
        : TQScrollView(parent, name)
    {
      setFrameStyle(TQFrame::NoFrame);
      m_frame = new TQFrame(viewport(), "MetaPropsScrollView::m_frame");
      m_frame->setFrameStyle(TQFrame::NoFrame);
      addChild(m_frame, 0, 0);
    };

    TQFrame* frame() {return m_frame;};

protected:
    virtual void viewportResizeEvent(TQResizeEvent* ev)
    {
      TQScrollView::viewportResizeEvent(ev);
      m_frame->resize( kMax(m_frame->sizeHint().width(), ev->size().width()),
                       kMax(m_frame->sizeHint().height(), ev->size().height()));
    };

private:
      TQFrame* m_frame;
};

class KFileMetaPropsPlugin::KFileMetaPropsPluginPrivate
{
public:
    KFileMetaPropsPluginPrivate()  {}
    ~KFileMetaPropsPluginPrivate() {}

    TQFrame*                       m_frame;
    TQGridLayout*                  m_framelayout;
    KFileMetaInfo                 m_info;
//    TQPushButton*                m_add;
    TQPtrList<KFileMetaInfoWidget> m_editWidgets;
};

KFileMetaPropsPlugin::KFileMetaPropsPlugin(KPropertiesDialog* props)
  : KPropsDlgPlugin(props)
{
    d = new KFileMetaPropsPluginPrivate;

    KFileItem * fileitem = properties->item();
    kdDebug(250) << "KFileMetaPropsPlugin constructor" << endl;

    d->m_info  = fileitem->metaInfo();
    if (!d->m_info.isValid())
    {
        d->m_info = KFileMetaInfo(properties->kurl().path(-1));
        fileitem->setMetaInfo(d->m_info);
    }

    if ( properties->items().count() > 1 )
    {
        // not yet supported
        // we should allow setting values for a list of files. Itt makes sense
        // in some cases, like the album of a list of mp3s
        return;
    }

    createLayout();

    setDirty(true);
}

void KFileMetaPropsPlugin::createLayout()
{
    TQFileInfo file_info(properties->item()->url().path());

    kdDebug(250) << "KFileMetaPropsPlugin::createLayout" << endl;

    // is there any valid and non-empty info at all?
    if ( !d->m_info.isValid() || (d->m_info.preferredKeys()).isEmpty() )
        return;

    // now get a list of groups
    KFileMetaInfoProvider* prov = KFileMetaInfoProvider::self();
    TQStringList groupList = d->m_info.preferredGroups();

    const KFileMimeTypeInfo* mtinfo = prov->mimeTypeInfo(d->m_info.mimeType());
    if (!mtinfo) 
    {
        kdDebug(7034) << "no mimetype info there\n";
        return;
    }

    // let the dialog create the page frame
    TQFrame* topframe = properties->addPage(i18n("&Meta Info"));
    topframe->setFrameStyle(TQFrame::NoFrame);
    TQVBoxLayout* tmp = new TQVBoxLayout(topframe);

    // create a scroll view in the page
    MetaPropsScrollView* view = new MetaPropsScrollView(topframe);

    tmp->addWidget(view);

    d->m_frame = view->frame();

    TQVBoxLayout *toplayout = new TQVBoxLayout(d->m_frame);
    toplayout->setSpacing(KDialog::spacingHint());

    for (TQStringList::Iterator git=groupList.begin(); 
            git!=groupList.end(); ++git)
    {
        kdDebug(7033) << *git << endl;

        TQStringList itemList = d->m_info.group(*git).preferredKeys();
        if (itemList.isEmpty())
            continue;

        TQGroupBox *groupBox = new TQGroupBox(2, TQt::Horizontal, 
            TQStyleSheet::escape(mtinfo->groupInfo(*git)->translatedName()), 
            d->m_frame);

        toplayout->addWidget(groupBox);

        TQValueList<KFileMetaInfoItem> readItems;
        TQValueList<KFileMetaInfoItem> editItems;

        for (TQStringList::Iterator iit = itemList.begin(); 
                iit!=itemList.end(); ++iit)
        {
            KFileMetaInfoItem item = d->m_info[*git][*iit];
            if ( !item.isValid() ) continue;

            bool editable = file_info.isWritable() && item.isEditable();

            if (editable)
                editItems.append( item );
            else
                readItems.append( item );
        }

        KFileMetaInfoWidget* w = 0L;
        // then first add the editable items to the layout
        for (TQValueList<KFileMetaInfoItem>::Iterator iit= editItems.begin(); 
                iit!=editItems.end(); ++iit)
        {
            TQLabel* l = new TQLabel((*iit).translatedKey() + ":", groupBox);
            l->setAlignment( AlignAuto | AlignTop | ExpandTabs );
            TQValidator* val = mtinfo->createValidator(*git, (*iit).key());
            if (!val) kdDebug(7033) << "didn't get a validator for " << *git << "/" << (*iit).key() << endl;
            w = new KFileMetaInfoWidget(*iit, val, groupBox);
            d->m_editWidgets.append( w );
            connect(w, TQ_SIGNAL(valueChanged(const TQVariant&)), this, TQ_SIGNAL(changed()));
        }

        // and then the read only items
        for (TQValueList<KFileMetaInfoItem>::Iterator iit= readItems.begin(); 
                iit!=readItems.end(); ++iit)
        {
            TQLabel* l = new TQLabel((*iit).translatedKey() + ":", groupBox);
            l->setAlignment( AlignAuto | AlignTop | ExpandTabs );
            (new KFileMetaInfoWidget(*iit, KFileMetaInfoWidget::ReadOnly, 0L, groupBox));
        }
    }

    toplayout->addStretch(1);

    // the add key (disabled until fully implemented)
/*    d->m_add = new TQPushButton(i18n("&Add"), topframe);
    d->m_add->setSizePolicy(TQSizePolicy(TQSizePolicy::Fixed,
                                        TQSizePolicy::Fixed));
    connect(d->m_add, TQ_SIGNAL(clicked()), this, TQ_SLOT(slotAdd()));
    tmp->addWidget(d->m_add);

    // if nothing can be added, deactivate it
    if ( !d->m_info.supportsVariableKeys() )
    {
        // if supportedKeys() does contain anything not in preferredKeys,
        // we have something addable

        TQStringList sk = d->m_info.supportedKeys();
        d->m_add->setEnabled(false);
        for (TQStringList::Iterator it = sk.begin(); it!=sk.end(); ++it)
        {
                if ( l.find(*it)==l.end() )
                {
                    d->m_add->setEnabled(true);
                    kdDebug(250) << "**first addable key is " << (*it).latin1() << "**" <<endl;
                    break;
                }
                kdDebug(250) << "**already existing key is " << (*it).latin1() << "**" <<endl;
        }
    } */
}

/*void KFileMetaPropsPlugin::slotAdd()
{
    // add a lineedit for the name



    // insert the item in the list

}*/

KFileMetaPropsPlugin::~KFileMetaPropsPlugin()
{
  delete d;
}

bool KFileMetaPropsPlugin::supports( KFileItemList _items )
{
#ifdef _GNUC
#warning TODO: Add support for more than one item
#endif
  if (KExecPropsPlugin::supports(_items) || KURLPropsPlugin::supports(_items))
     return false; // Having both is redundant.

  bool metaDataEnabled = TDEGlobalSettings::showFilePreview(_items.first()->url());
  return _items.count() == 1 && metaDataEnabled;
}

void KFileMetaPropsPlugin::applyChanges()
{
  kdDebug(250) << "applying changes" << endl;
  // insert the fields that changed into the info object

  TQPtrListIterator<KFileMetaInfoWidget> it( d->m_editWidgets );
  KFileMetaInfoWidget* w;
  for (; (w = it.current()); ++it) w->apply();
  d->m_info.applyChanges(properties->kurl().path());
}

#include "kmetaprops.moc"
