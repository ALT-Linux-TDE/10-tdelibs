/* This file is part of the KDE libraries
    Copyright (C) 2000 Wilco Greven <greven@kde.org>

    library is free software; you can redistribute it and/or
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


#ifndef KURLREQUESTERDIALOG_H
#define KURLREQUESTERDIALOG_H

#include <kdialogbase.h>
#include <kurl.h>

class KURLCompletion;
class KURLRequester;
class KFileDialog;
/**
 * Dialog in which a user can enter a filename or url. It is a dialog
 * encapsulating KURLRequester. The API is derived from
 * KFileDialog.
 *
 * @short Simple dialog to enter a filename/url.
 * @author Wilco Greven <greven@kde.org>
 */
class TDEIO_EXPORT KURLRequesterDlg : public KDialogBase
{
    TQ_OBJECT

public:
    /**
     * Constructs a KURLRequesterDlg.
     *
     * @param url    The url of the directory to start in. Use TQString::null
     *               to start in the current working directory, or the last
     *               directory where a file has been selected.
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     * @param modal  Specifies whether the dialog should be opened as modal
     *               or not.
     */
    KURLRequesterDlg( const TQString& url, TQWidget *parent,
                      const char *name, bool modal = true );

    /**
     * Constructs a KURLRequesterDlg.
     *
     * @param url    The url of the directory to start in. Use TQString::null
     *               to start in the current working directory, or the last
     *               directory where a file has been selected.
     * @param text   Text of the label
     * @param parent The parent object of this widget.
     * @param name The name of this widget.
     * @param modal  Specifies whether the dialog should be opened as modal
     *                  or not.
     */
    KURLRequesterDlg( const TQString& url, const TQString& text,
                      TQWidget *parent, const char *name, bool modal=true );
    /**
     * Destructs the dialog.
     */
    ~KURLRequesterDlg();

    /**
     * Returns the fully qualified filename.
     */
    KURL selectedURL() const;

    /**
     * Creates a modal dialog, executes it and returns the selected URL.
     *
     * @param url This specifies the initial path of the input line.
     * @param parent The widget the dialog will be centered on initially.
     * @param caption The caption to use for the dialog.
     */
    static KURL getURL(const TQString& url = TQString::null,
            TQWidget *parent= 0, const TQString& caption = TQString::null);

    /**
     * Returns a pointer to the file dialog used by the KURLRequester.
     */
    KFileDialog * fileDialog();
    /**
     * Returns a pointer to the KURLRequester.
     */
    KURLRequester *urlRequester();

private slots:
    void slotClear();
    void slotTextChanged(const TQString &);
private:
    void initDialog(const TQString &text, const TQString &url);
    KURLRequester *urlRequester_;

    class KURLRequesterDlgPrivate;
    KURLRequesterDlgPrivate *d;

};

#endif // KURLREQUESTERDIALOG_H
