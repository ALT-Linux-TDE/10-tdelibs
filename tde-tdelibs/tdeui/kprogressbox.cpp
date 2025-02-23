/* This file is part of the KDE libraries
   Copyright (C) 2010 Timothy Pearson
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
 * KProgressBox -- a progress indicator widget for KDE with an expandable textbox provided below the progress bar.
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
#include "ktextedit.h"
#include "kprogressbox.h"

#include <tdeapplication.h>
#include <tdelocale.h>
#include <twin.h>

struct KProgressBoxDialog::KProgressBoxDialogPrivate
{
    KProgressBoxDialogPrivate() : cancelButtonShown(true)
    {
    }

    bool cancelButtonShown;
};

/*
 * KProgressBoxDialog implementation
 */
KProgressBoxDialog::KProgressBoxDialog(TQWidget* parent, const char* name,
                                 const TQString& caption, const TQString& text,
                                 bool modal)
    : KDialogBase(KDialogBase::Plain, caption, KDialogBase::Cancel,
                  KDialogBase::Cancel, parent, name, modal),
      mAutoClose(true),
      mAutoReset(false),
      mCancelled(false),
      mAllowCancel(true),
      mAllowTextEdit(false),
      mShown(false),
      mMinDuration(2000),
      d(new KProgressBoxDialogPrivate)
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
    mTextBox = new KTextEdit(mainWidget);
    layout->addWidget(mTextBox);

    connect(mProgressBar, TQ_SIGNAL(percentageChanged(int)),
            this, TQ_SLOT(slotAutoActions(int)));
    connect(mShowTimer, TQ_SIGNAL(timeout()), this, TQ_SLOT(slotAutoShow()));
    mShowTimer->start(mMinDuration, true);
}

KProgressBoxDialog::~KProgressBoxDialog()
{
    delete d;
}

void KProgressBoxDialog::slotAutoShow()
{
    if (mShown || mCancelled)
    {
        return;
    }

    show();
    kapp->processEvents();
}

void KProgressBoxDialog::slotCancel()
{
    mCancelled = true;

    if (mAllowCancel)
    {
        KDialogBase::slotCancel();
    }
}

bool KProgressBoxDialog::wasCancelled()
{
    return mCancelled;
}

void KProgressBoxDialog::ignoreCancel()
{
    mCancelled = false;
}

bool KProgressBoxDialog::wasCancelled() const
{
    return mCancelled;
}

void KProgressBoxDialog::setMinimumDuration(int ms)
{
    mMinDuration = ms;
    if (!mShown)
    {
        mShowTimer->stop();
        mShowTimer->start(mMinDuration, true);
    }
}

int KProgressBoxDialog::minimumDuration()
{
    return mMinDuration;
}

int KProgressBoxDialog::minimumDuration() const
{
    return mMinDuration;
}

void KProgressBoxDialog::setAllowCancel(bool allowCancel)
{
    mAllowCancel = allowCancel;
    showCancelButton(allowCancel);
}

void KProgressBoxDialog::setAllowTextEdit(bool allowTextEdit)
{
    mAllowTextEdit = allowTextEdit;
    mTextBox->setReadOnly(!allowTextEdit);
}

// ### KDE 4 remove
bool KProgressBoxDialog::allowCancel()
{
    return mAllowCancel;
}

bool KProgressBoxDialog::allowCancel() const
{
    return mAllowCancel;
}

KProgress* KProgressBoxDialog::progressBar()
{
    return mProgressBar;
}

KTextEdit* KProgressBoxDialog::textEdit()
{
    return mTextBox;
}

const KProgress* KProgressBoxDialog::progressBar() const
{
    return mProgressBar;
}

const KTextEdit* KProgressBoxDialog::textEdit() const
{
    return mTextBox;
}

void KProgressBoxDialog::setLabel(const TQString& text)
{
    mLabel->setText(text);
}

// ### KDE 4 remove
TQString KProgressBoxDialog::labelText()
{
    return mLabel->text();
}

TQString KProgressBoxDialog::labelText() const
{
    return mLabel->text();
}

void KProgressBoxDialog::showCancelButton(bool show)
{
    showButtonCancel(show);
}

// ### KDE 4 remove
bool KProgressBoxDialog::autoClose()
{
    return mAutoClose;
}

bool KProgressBoxDialog::autoClose() const
{
    return mAutoClose;
}

void KProgressBoxDialog::setAutoClose(bool autoClose)
{
    mAutoClose = autoClose;
}

// ### KDE 4 remove
bool KProgressBoxDialog::autoReset()
{
    return mAutoReset;
}

bool KProgressBoxDialog::autoReset() const
{
    return mAutoReset;
}

void KProgressBoxDialog::setAutoReset(bool autoReset)
{
    mAutoReset = autoReset;
}

void KProgressBoxDialog::setButtonText(const TQString& text)
{
    mCancelText = text;
    setButtonCancel(text);
}

// ### KDE 4 remove
TQString KProgressBoxDialog::buttonText()
{
    return mCancelText;
}

TQString KProgressBoxDialog::buttonText() const
{
    return mCancelText;
}

void KProgressBoxDialog::slotAutoActions(int percentage)
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

void KProgressBoxDialog::show()
{
    KDialogBase::show();
    mShown = true;
}

void KProgressBoxDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }

#include "kprogressbox.moc"
