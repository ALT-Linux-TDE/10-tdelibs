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
/*****************************************************************************
*                                                                            *
*  KProgressBox -- progress indicator widget for KDE                         *
*  Original QRangeControl-based version written by Martynas Kunigelis        *
*  Current TQProgressBar based version by Aaron Seigo                         *
*  Textbox extension by Timothy Pearson                                      * 
*                                                                            *
*****************************************************************************/

#ifndef _KProgressBox_H
#define _KProgressBox_H "$Id: KProgressBox.h 589356 2006-09-28 00:58:09Z tpearson $"

#include <tqprogressbar.h>
#include <kdialogbase.h>
#include <kprogress.h>
#include <ktextedit.h>

/**
 * KProgressBoxDialog provides a dialog with a text label, a progress bar
 * and an optional cancel button with a KDE look 'n feel.
 *
 * Since knowing how long it can take to complete an action and it is
 * undesirable to show a dialog for a split second before hiding it,
 * there are a few ways to control the timing behavior of KProgressBoxDialog.
 * There is a time out that can be set before showing the dialog as well
 * as an option to autohide or keep displaying the dialog once complete.
 *
 * All the functionality of KProgressBox is available through direct access
 * to the progress bar widget via progressBar();
 *
 * Also, an expandable textbox provided below the progress bar.
 *
 * @short A dialog with a progress bar and text box.
 * @author Timothy Pearson
 */
class TDEUI_EXPORT KProgressBoxDialog : public KDialogBase
{
    TQ_OBJECT

    public:
        /**
         * Constructs a KProgressBoxDialog
         *
         * @param parent Parent of the widget
         * @param name Widget name
         * @param caption Text to display in window title bar
         * @param text Text to display in the dialog
         * @param modal Set to true to make the dialog modal
         */
        KProgressBoxDialog(TQWidget* parent = 0, const char* name = 0,
                        const TQString& caption = TQString::null,
                        const TQString& text = TQString::null,
                        bool modal = false);

        /**
         * Destructor
         */
        ~KProgressBoxDialog();

        /**
         * Returns the KProgressBox used in this dialog.
         * To set the number of steps or other progress bar related
         * settings, access the KProgressBox object directly via this method.
         */
        KProgress* progressBar();

        /**
         * Returns the KTextEdit used in this dialog.
         * To set the number of lines or other text box related
         * settings, access the KTextEdit object directly via this method.
         */
        KTextEdit* textEdit();

        /**
         * Returns the KProgressBox used in this dialog.
         * To set the number of steps or other progress bar related
         * settings, access the KProgressBox object directly via this method.
         */
        const KProgress* progressBar() const;

        /**
         * Returns the KTextEdit used in this dialog.
         * To set the number of lines or other text box related
         * settings, access the KTextEdit object directly via this method.
         */
        const KTextEdit* textEdit() const;

        /**
         * Sets the text in the dialog
         *
         * @param text the text to display
         */
        void    setLabel(const TQString & text);

        /**
         * Returns the current dialog text
         * @deprecated
         */
        // ### Remove this KDE 4.0
        TQString labelText() TDE_DEPRECATED;

        /**
         * Returns the current dialog text
         */
        TQString labelText() const;

        /**
         * Sets whether or not the user can cancel the process.
         * If the dialog is cancellable, the Cancel button will be shown
         * and the user can close the window using the window decorations.
         * If the process is not (or should not be) interuptable,
         * set the dialog to be modal and not cancellable.
         *
         * @param allowCancel Set to true to make the dialog non-closable
         */
        void setAllowCancel(bool allowCancel);

        /**
         * Sets whether or not the user can edit the text shown in the textbox.
         *
         * @param allowTextEdit Set to true to make the text editable
         */
        void setAllowTextEdit(bool allowTextEdit);

        /**
         * Returns true if the dialog can be canceled, false otherwise
         * @deprecated
         */
        // ### Remove this KDE 4.0
        bool allowCancel() TDE_DEPRECATED;

        /**
         * Returns true if the dialog can be canceled, false otherwise
         */
        bool allowCancel() const;

        /**
         * Sets whether the cancel button is visible. setAllowCancel(false)
         * implies showCancelButton(false)
         *
         * @param show Whether or not the cancel button should be shown
         */
        void showCancelButton(bool show);

        /**
         * Sets whether the dialog should close automagically when
         * all the steps in the KProgressBox have been completed.
         */
        void setAutoClose(bool close);

        /**
         * Returns true if the dialog will close upon completion,
         * or false otherwise
         */
        // ### Remove this KDE 4.0
        bool autoClose();

        /**
         * Returns true if the dialog will close upon completion,
         * or false otherwise
         */
        bool autoClose() const;

        /**
         * Sets whether the dialog should reset the KProgressBox dialog
         * back to 0 steps compelete when all steps have been completed.
         * This is useful for KProgressBoxDialogs that will be reused.
         */
        void setAutoReset(bool autoReset);

        /**
         * Returns true if the KProgressBox widget will be reset
         * upon completion, or false otherwise
         */
        // ### Remove this KDE 4.0
        bool autoReset();

        /**
         * Returns true if the KProgressBox widget will be reset
         * upon completion, or false otherwise
         */
        bool autoReset() const;

        /**
         * Returns true if the dialog was closed or canceled
         * before completion. If the dialog is not cancellable
         * it will always return false.
         */
        // ### Remove this KDE 4.0
        bool wasCancelled();

        /**
         * Returns true if the dialog was closed or canceled
         * before completion. If the dialog is not cancellable
         * it will always return false.
         */
        bool wasCancelled() const;

        /**
         * Ignores the last cancel action if the cancel button was 
         * pressed. Useful for kdialog when combined with a KMessageBox
         * to display a message like "Are you sure you want to cancel?" 
	 * @since 3.5.5
         */
        void ignoreCancel();

        /**
         * Sets the text to appear on the cancel button.
         */
        void setButtonText(const TQString&);

        /**
         * Returns the text on the cancel button
         * @deprecated
         */
        // ### Remove this KDE 4.0
        TQString buttonText() TDE_DEPRECATED;

        /**
         * Returns the text on the cancel button
         */
        TQString buttonText() const;

        /**
         * Set the minimum number of milliseconds to wait before
         * actually showing the dialog
         */
        void setMinimumDuration(int ms);

        /**
         * Returns the wait duration in milliseconds
         * @deprecated
         */
        // ### Remove this KDE 4.0
        int  minimumDuration() TDE_DEPRECATED;

        /**
         * Returns the wait duration in milliseconds
         */
        int  minimumDuration() const;

 	/**
	 * Reimplemented for internal reasons, the API is not affected.
	 */
        virtual void show();

    protected slots:
        void slotAutoShow();
        void slotAutoActions(int percentage);
        void slotCancel();

    private:
        // ### Move these member variables to d in KDE 4.0
        bool       mAutoClose;
        bool       mAutoReset;
        bool       mCancelled;
        bool       mAllowCancel;
        bool       mAllowTextEdit;
        bool       mShown;
        TQString    mCancelText;
        TQLabel*    mLabel;
        KProgress* mProgressBar;
        KTextEdit* mTextBox;
        TQTimer*    mShowTimer;
        int        mMinDuration;
    protected:
	virtual void virtual_hook( int id, void* data );
    private:
        struct KProgressBoxDialogPrivate;
        KProgressBoxDialogPrivate *d;
};

#endif
