/*****************************************************************

Copyright (c) 2000-2003 Matthias Hoelzer-Kluepfel <mhk@kde.org>
                        Tobias Koenig <tokoe@kde.org>
                        Daniel Molkentin <molkentin@kde.org>

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in
all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.  IN NO EVENT SHALL THE
AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN
AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN
CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.

******************************************************************/

#include <tqcheckbox.h>
#include <tqfile.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqregexp.h>
#include <tqtextstream.h>
#include <tqimage.h>

#include <tdeaboutdata.h>
#include <tdeapplication.h>
#include <tdeconfig.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <kiconloader.h>
#include <tdelocale.h>
#include <kpushbutton.h>
#include <kseparator.h>
#include <kstandarddirs.h>
#include <kstdguiitem.h>
#include <ktextbrowser.h>
#include <kiconeffect.h>
#include <tdeglobalsettings.h>

#ifdef TQ_WS_X11
#include <twin.h>
#endif

#include "ktip.h"


KTipDatabase::KTipDatabase(const TQString &_tipFile)
{
    TQString tipFile = _tipFile;
    if (tipFile.isEmpty())
	tipFile = TQString::fromLatin1(TDEGlobal::instance()->aboutData()->appName()) + "/tips";

    loadTips(tipFile);

    if (!mTips.isEmpty())
	mCurrent = kapp->random() % mTips.count();
}


KTipDatabase::KTipDatabase( const TQStringList& tipsFiles )
{
   if ( tipsFiles.isEmpty() || ( ( tipsFiles.count() == 1 ) && tipsFiles.first().isEmpty() ) )
   {
       addTips(TQString::fromLatin1(TDEGlobal::instance()->aboutData()->appName()) + "/tips");
   }
   else
   {
       for (TQStringList::ConstIterator it = tipsFiles.begin(); it != tipsFiles.end(); ++it)
           addTips( *it );
   }
    if (!mTips.isEmpty())
	mCurrent = kapp->random() % mTips.count();

}

void KTipDatabase::loadTips(const TQString &tipFile)
{
    mTips.clear();
    addTips(tipFile);
}

// if you change something here, please update the script
// preparetips, which depends on extracting exactly the same
// text as done here.
void KTipDatabase::addTips(const TQString& tipFile )
{
    TQString fileName = locate("data", tipFile);

    if (fileName.isEmpty())
    {
	kdDebug() << "KTipDatabase::addTips: can't find '" << tipFile << "' in standard dirs" << endl;
        return;
    }

    TQFile file(fileName);
    if (!file.open(IO_ReadOnly))
    {
	kdDebug() << "KTipDatabase::addTips: can't open '" << fileName << "' for reading" << endl;
	return;
    }

    TQByteArray data = file.readAll();
    TQString content = TQString::fromUtf8(data.data(), data.size());
    const TQRegExp rx("\\n+");

    int pos = -1;
    while ((pos = content.find("<html>", pos + 1, false)) != -1)
    {
       // to make translations work, tip extraction here must exactly 
       // match what is done by the preparetips script 
       TQString tip = content 
           .mid(pos + 6, content.find("</html>", pos, false) - pos - 6)
           .replace(rx, "\n");
       if (!tip.endsWith("\n"))
           tip += "\n";
       if (tip.startsWith("\n")) 
            tip = tip.mid(1); 
        if (tip.isEmpty())
        {
            kdDebug() << "Empty tip found! Skipping! " << pos << endl;
            continue;
        }
	mTips.append(tip);
    }

    file.close();

}

void KTipDatabase::nextTip()
{
    if (mTips.isEmpty())
	return ;
    mCurrent += 1;
    if (mCurrent >= (int) mTips.count())
	mCurrent = 0;
}


void KTipDatabase::prevTip()
{
    if (mTips.isEmpty())
	return ;
    mCurrent -= 1;
    if (mCurrent < 0)
	mCurrent = mTips.count() - 1;
}


TQString KTipDatabase::tip() const
{
    if (mTips.isEmpty())
	return TQString::null;
    return mTips[mCurrent];
}

KTipDialog *KTipDialog::mInstance = 0;


KTipDialog::KTipDialog(KTipDatabase *db, TQWidget *parent, const char *name)
  : KDialog(parent, name)
{
    /**
     * Parent is 0L when TipDialog is used as a mainWidget. This should
     * be the case only in ktip, so let's use the ktip layout.
     */
    bool isTipDialog = (parent);

    TQImage img;
    int h,s,v;

    mBlendedColor = TDEGlobalSettings::activeTitleColor();
    mBlendedColor.hsv(&h,&s,&v);
    mBlendedColor.setHsv(h, int(s*(71/76.0)), int(v*(67/93.0)));

    if (!isTipDialog)
    {
	img = TQImage(locate("data", "tdewizard/pics/wizard_small.png"));
	// colorize and check to figure the correct color
	TDEIconEffect::colorize(img, mBlendedColor, 1.0);
	TQRgb colPixel( img.pixel(0,0) );

	mBlendedColor = TQColor(tqRed(colPixel),tqGreen(colPixel),tqBlue(colPixel));
    }

    mBaseColor = TDEGlobalSettings::alternateBackgroundColor();
    mBaseColor.hsv(&h,&s,&v);
    mBaseColor.setHsv(h, int(s*(10/6.0)), int(v*(93/99.0)));

    mTextColor = TDEGlobalSettings::textColor();


    mDatabase = db;

    setCaption(i18n("Tip of the Day"));
#ifdef TQ_WS_X11
    KWin::setIcons( winId(),
                    TDEGlobal::iconLoader()->loadIcon( "idea", TDEIcon::NoGroup, 32 ),
                    TDEGlobal::iconLoader()->loadIcon( "idea", TDEIcon::NoGroup, 16 ) );
#endif
    TQVBoxLayout *vbox = new TQVBoxLayout(this, marginHint(), spacingHint());

   if (isTipDialog)
    {
	TQHBoxLayout *pl = new TQHBoxLayout(vbox, 0, 0);

	TQLabel *bulb = new TQLabel(this);
	bulb->setPixmap(locate("data", "tdeui/pics/ktip-bulb.png"));
	pl->addWidget(bulb);

	TQLabel *titlePane = new TQLabel(this);
	titlePane->setBackgroundPixmap(locate("data", "tdeui/pics/ktip-background.png"));
	titlePane->setText(i18n("Did you know...?\n"));
	titlePane->setFont(TQFont(TDEGlobalSettings::generalFont().family(), 20, TQFont::Bold));
	titlePane->setAlignment(TQLabel::AlignCenter);
	pl->addWidget(titlePane, 100);
    }

    TQHBox *hbox = new TQHBox(this);
    hbox->setSpacing(0);
    hbox->setFrameStyle(TQFrame::Panel | TQFrame::Sunken);
    vbox->addWidget(hbox);

    TQHBox *tl = new TQHBox(hbox);
    tl->setMargin(7);
    tl->setBackgroundColor(mBlendedColor);

    TQHBox *topLeft = new TQHBox(tl);
    topLeft->setMargin(15);
    topLeft->setBackgroundColor(mBaseColor);

    mTipText = new KTextBrowser(topLeft);

    mTipText->setWrapPolicy( TQTextEdit::AtWordOrDocumentBoundary );
    mTipText->mimeSourceFactory()->addFilePath(
	TDEGlobal::dirs()->findResourceDir("data", "tdewizard/pics")+"tdewizard/pics/");
    mTipText->setFrameStyle(TQFrame::NoFrame | TQFrame::Plain);
    mTipText->setHScrollBarMode(TQScrollView::AlwaysOff);
    mTipText->setLinkUnderline(false);

    TQStyleSheet *sheet = mTipText->styleSheet();
    TQStyleSheetItem *item = sheet->item("a");
    item->setFontWeight(TQFont::Bold);
    mTipText->setStyleSheet(sheet);
    TQPalette pal = mTipText->palette();
    pal.setColor( TQPalette::Active, TQColorGroup::Link, mBlendedColor );
    pal.setColor( TQPalette::Inactive, TQColorGroup::Link, mBlendedColor );
    mTipText->setPalette(pal);

    TQStringList icons = TDEGlobal::dirs()->resourceDirs("icon");
    TQStringList::Iterator it;
    for (it = icons.begin(); it != icons.end(); ++it)
        mTipText->mimeSourceFactory()->addFilePath(*it);

    if (!isTipDialog)
    {
	TQLabel *l = new TQLabel(hbox);
	l->setPixmap(img);
	l->setBackgroundColor(mBlendedColor);
	l->setAlignment(TQt::AlignRight | TQt::AlignBottom);

	resize(550, 230);
        TQSize sh = size();

        TQRect rect = TDEGlobalSettings::splashScreenDesktopGeometry();

        move(rect.x() + (rect.width() - sh.width())/2,
	rect.y() + (rect.height() - sh.height())/2);
    }

    KSeparator* sep = new KSeparator( KSeparator::HLine, this);
    vbox->addWidget(sep);

    TQHBoxLayout *hbox2 = new TQHBoxLayout(vbox, 4);

    mTipOnStart = new TQCheckBox(i18n("&Show tips on startup"), this);
    hbox2->addWidget(mTipOnStart, 1);

    KPushButton *prev = new KPushButton( KStdGuiItem::back(
            KStdGuiItem::UseRTL ), this );
    prev->setText( i18n("&Previous") );
    hbox2->addWidget(prev);

    KPushButton *next = new KPushButton( KStdGuiItem::forward(
            KStdGuiItem::UseRTL ), this );
    next->setText( i18n("Opposite to Previous","&Next") );
    hbox2->addWidget(next);

    KPushButton *ok = new KPushButton(KStdGuiItem::close(), this);
    ok->setDefault(true);
    hbox2->addWidget(ok);

    TDEConfigGroup config(kapp->config(), "TipOfDay");
    mTipOnStart->setChecked(config.readBoolEntry("RunOnStart", true));

    connect(next, TQ_SIGNAL(clicked()), this, TQ_SLOT(nextTip()));
    connect(prev, TQ_SIGNAL(clicked()), this, TQ_SLOT(prevTip()));
    connect(ok, TQ_SIGNAL(clicked()), this, TQ_SLOT(accept()));
    connect(mTipOnStart, TQ_SIGNAL(toggled(bool)), this, TQ_SLOT(showOnStart(bool)));

    ok->setFocus();

    nextTip();
}

KTipDialog::~KTipDialog()
{
    if( mInstance==this )
        mInstance = 0L;
}

void KTipDialog::showTip(const TQString &tipFile, bool force)
{
    showTip(kapp->mainWidget(), tipFile, force);
}

void KTipDialog::showTip(TQWidget *parent, const TQString &tipFile, bool force)
{
  showMultiTip( parent, TQStringList(tipFile), force );
}

void KTipDialog::showMultiTip(TQWidget *parent, const TQStringList &tipFiles, bool force)
{
    TDEConfigGroup configGroup(kapp->config(), "TipOfDay");

    const bool runOnStart = configGroup.readBoolEntry("RunOnStart", true);

    if (!force)
    {
        if (!runOnStart)
	    return;

        bool hasLastShown = configGroup.hasKey("TipLastShown");
        if (hasLastShown)
        {
           const int oneDay = 24*60*60;
           TQDateTime lastShown = configGroup.readDateTimeEntry("TipLastShown");
           // Show tip roughly once a week
           if (lastShown.secsTo(TQDateTime::currentDateTime()) < (oneDay + (kapp->random() % (10*oneDay))))
               return;
        }
        configGroup.writeEntry("TipLastShown", TQDateTime::currentDateTime());
        kapp->config()->sync();
        if (!hasLastShown)
           return; // Don't show tip on first start
    }

    if (!mInstance)
	mInstance = new KTipDialog(new KTipDatabase(tipFiles), parent);
    else
	// The application might have changed the RunOnStart option in its own
	// configuration dialog, so we should update the checkbox.
      mInstance->mTipOnStart->setChecked(runOnStart);

      mInstance->show();
      mInstance->raise();
  }

static TQString fixTip(TQString tip)
{
    TQRegExp iconRegExp("<img src=\"(.*)\">");
    iconRegExp.setMinimal(true);
    if (iconRegExp.search(tip)>-1) {
      TQString iconName = iconRegExp.cap(1);
      if (!iconName.isEmpty())
         if (TDEGlobal::dirs()->findResource("icon", iconName).isEmpty())
           tip.replace("crystalsvg","hicolor");
    }

    return tip;
}

  void KTipDialog::prevTip()
  {
      mDatabase->prevTip();
      TQString currentTip = TQString::fromLatin1(
     "<qt text=\"%1\" bgcolor=\"%2\">%3</qt>")
     .arg(mTextColor.name())
     .arg(mBaseColor.name())
     .arg(i18n(mDatabase->tip().utf8()));


      currentTip = fixTip(currentTip);
      mTipText->setText(currentTip);
      mTipText->setContentsPos(0, 0);
  }

  void KTipDialog::nextTip()
  {
      mDatabase->nextTip();
      TQString currentTip = TQString::fromLatin1(
        "<qt text=\"%1\" bgcolor=\"%2\">%3</qt>")
        .arg(mTextColor.name())
        .arg(mBaseColor.name())
        .arg(i18n(mDatabase->tip().utf8()));


      currentTip = fixTip(currentTip);
      mTipText->setText(currentTip);
      mTipText->setContentsPos(0, 0);
  }

  void KTipDialog::showOnStart(bool on)
  {
      setShowOnStart(on);
  }

  void KTipDialog::setShowOnStart(bool on)
  {
      TDEConfigGroup config(kapp->config(), "TipOfDay");
      config.writeEntry("RunOnStart", on);
      config.sync();
  }

  bool KTipDialog::eventFilter(TQObject *o, TQEvent *e)
  {
    if (o == mTipText && e->type()== TQEvent::KeyPress &&
		(((TQKeyEvent *)e)->key() == Key_Return ||
		((TQKeyEvent *)e)->key() == Key_Space ))
		accept();

	// If the user presses Return or Space, we close the dialog as if the
	// default button was pressed even if the KTextBrowser has the keyboard
	// focus. This could have the bad side-effect that the user cannot use the
	// keyboard to open urls in the KTextBrowser, so we just let it handle
	// the key event _additionally_. (Antonio)

	return TQWidget::eventFilter( o, e );
}

void KTipDialog::virtual_hook( int id, void* data )
{
	KDialog::virtual_hook( id, data );
}

#include "ktip.moc"
