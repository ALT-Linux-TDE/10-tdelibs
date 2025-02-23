/* This file is part of the KDE libraries
   Copyright (C) 1996 Martynas Kunigelis

   This library is free software; you can redistribute it and/or
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
*/
/**
 * KProgress -- a progress indicator widget for KDE.
 */

#include <stdlib.h>
#include <limits.h>

#include <tqpainter.h>
#include <tqpixmap.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>
#include <tqstring.h>
#include <tqregexp.h>
#include <tqstyle.h>
#include <tqtimer.h>

#include "kprogress.h"

#include <tdeapplication.h>
#include <tdelocale.h>
#include <twin.h>

KProgress::KProgress(TQWidget *parent, const char *name, WFlags f)
  : TQProgressBar(parent, name, f),
    mFormat("%p%")
{
    setProgress(0);
}

KProgress::KProgress(int totalSteps, TQWidget *parent, const char *name, WFlags f)
  : TQProgressBar(totalSteps, parent, name, f),
    mFormat("%p%")
{
    setProgress(0);
}

KProgress::~KProgress()
{
}

void KProgress::advance(int offset)
{
    setProgress(progress() + offset);
}

void KProgress::setTotalSteps(int totalSteps)
{
    TQProgressBar::setTotalSteps(totalSteps);

    if (totalSteps)
    {
        emit percentageChanged((progress() * 100) / totalSteps);
    }
}

void KProgress::setProgress(int progress)
{
    TQProgressBar::setProgress(progress);

    if (totalSteps())
    {
        emit percentageChanged((progress * 100) / totalSteps());
    }
}

// ### KDE 4 remove
void KProgress::setValue(int progress)
{
    setProgress(progress); 
}

// ### KDE 4 remove
void KProgress::setRange(int /*min*/, int max)
{
    setTotalSteps(max);
}

// ### KDE 4 remove
int KProgress::maxValue()
{
    return totalSteps();
}

void KProgress::setTextEnabled(bool enable)
{
    setPercentageVisible(enable);
}

bool KProgress::textEnabled() const
{
    return percentageVisible();
}

void KProgress::setFormat(const TQString & format)
{
    mFormat = format;
    if (mFormat != "%p%")
        setCenterIndicator(true);
}

TQString KProgress::format() const
{
    return mFormat;
}

// ### KDE 4 remove
int KProgress::value() const
{
    return progress();
}

bool KProgress::setIndicator(TQString &indicator, int progress, int totalSteps)
{
    if (!totalSteps)
        return false;
    TQString newString(mFormat);
    newString.replace(TQString::fromLatin1("%v"),
                      TQString::number(progress));
    newString.replace(TQString::fromLatin1("%m"),
                      TQString::number(totalSteps));

    if (totalSteps > INT_MAX / 1000) {
        progress /= 1000;
        totalSteps /= 1000;
    }

    newString.replace(TQString::fromLatin1("%p"),
                      TQString::number((progress * 100) / totalSteps)); 

    if (newString != indicator)
    {
        indicator = newString;
        return true;
    }

    return false;
}

struct KProgressDialog::KProgressDialogPrivate
{
    KProgressDialogPrivate() : cancelButtonShown(true)
    {
    }

    bool cancelButtonShown;
};

/*
 * KProgressDialog implementation
 */
KProgressDialog::KProgressDialog(TQWidget* parent, const char* name,
                                 const TQString& caption, const TQString& text,
                                 bool modal)
    : KDialogBase(KDialogBase::Plain, caption, KDialogBase::Cancel,
                  KDialogBase::Cancel, parent, name, modal),
      mAutoClose(true),
      mAutoReset(false),
      mCancelled(false),
      mAllowCancel(true),
      mShown(false),
      mMinDuration(2000),
      d(new KProgressDialogPrivate)
{
#ifdef TQ_WS_X11
    KWin::setIcons(winId(), kapp->icon(), kapp->miniIcon());
#endif
    mShowTimer = new TQTimer(this);
    
    showButton(KDialogBase::Close, false);
    mCancelText = actionButton(KDialogBase::Cancel)->text();

    TQFrame* mainWidget = plainPage();
    TQVBoxLayout* layout = new TQVBoxLayout(mainWidget, 10);

    mLabel = new TQLabel(text, mainWidget);
    layout->addWidget(mLabel);

    mProgressBar = new KProgress(mainWidget);
    layout->addWidget(mProgressBar);

    connect(mProgressBar, TQ_SIGNAL(percentageChanged(int)),
            this, TQ_SLOT(slotAutoActions(int)));
    connect(mShowTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotAutoShow()));
    mShowTimer->start(mMinDuration, true);
}

KProgressDialog::~KProgressDialog()
{
    delete d;
}

void KProgressDialog::slotAutoShow()
{
    if (mShown || mCancelled)
    {
        return;
    }

    show();
    kapp->processEvents();
}

void KProgressDialog::slotCancel()
{
    mCancelled = true;

    if (mAllowCancel)
    {
        KDialogBase::slotCancel();
    }
}

bool KProgressDialog::wasCancelled()
{
    return mCancelled;
}

void KProgressDialog::ignoreCancel()
{
    mCancelled = false;
}

bool KProgressDialog::wasCancelled() const
{
    return mCancelled;
}

void KProgressDialog::setMinimumDuration(int ms)
{
    mMinDuration = ms;
    if (!mShown)
    {
        mShowTimer->stop();
        mShowTimer->start(mMinDuration, true);
    }
}

int KProgressDialog::minimumDuration()
{
    return mMinDuration;
}

int KProgressDialog::minimumDuration() const
{
    return mMinDuration;
}

void KProgressDialog::setAllowCancel(bool allowCancel)
{
    mAllowCancel = allowCancel;
    showCancelButton(allowCancel);
}

// ### KDE 4 remove
bool KProgressDialog::allowCancel()
{
    return mAllowCancel;
}

bool KProgressDialog::allowCancel() const
{
    return mAllowCancel;
}

KProgress* KProgressDialog::progressBar()
{
    return mProgressBar;
}

const KProgress* KProgressDialog::progressBar() const
{
    return mProgressBar;
}

void KProgressDialog::setLabel(const TQString& text)
{
    mLabel->setText(text);
}

// ### KDE 4 remove
TQString KProgressDialog::labelText()
{
    return mLabel->text();
}

TQString KProgressDialog::labelText() const
{
    return mLabel->text();
}

void KProgressDialog::showCancelButton(bool show)
{
    showButtonCancel(show);
}

// ### KDE 4 remove
bool KProgressDialog::autoClose()
{
    return mAutoClose;
}

bool KProgressDialog::autoClose() const
{
    return mAutoClose;
}

void KProgressDialog::setAutoClose(bool autoClose)
{
    mAutoClose = autoClose;
}

// ### KDE 4 remove
bool KProgressDialog::autoReset()
{
    return mAutoReset;
}

bool KProgressDialog::autoReset() const
{
    return mAutoReset;
}

void KProgressDialog::setAutoReset(bool autoReset)
{
    mAutoReset = autoReset;
}

void KProgressDialog::setButtonText(const TQString& text)
{
    mCancelText = text;
    setButtonCancel(text);
}

// ### KDE 4 remove
TQString KProgressDialog::buttonText()
{
    return mCancelText;
}

TQString KProgressDialog::buttonText() const
{
    return mCancelText;
}

void KProgressDialog::slotAutoActions(int percentage)
{
    if (percentage < 100)
    {
        if (!d->cancelButtonShown)
        {
            setButtonCancel(mCancelText);
            d->cancelButtonShown = true;
        }
        return;
    }

    mShowTimer->stop();

    if (mAutoReset)
    {
        mProgressBar->setProgress(0);
    }
    else
    {
        setAllowCancel(true);
        setButtonCancel(KStdGuiItem::close());
        d->cancelButtonShown = false;
    }

    if (mAutoClose)
    {
        if (mShown)
        {
            hide();
        }
        else
        {
            emit finished();
        }
    }
}

void KProgressDialog::show()
{
    KDialogBase::show();
    mShown = true;
}


void KProgress::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void KProgressDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kprogress.moc"
