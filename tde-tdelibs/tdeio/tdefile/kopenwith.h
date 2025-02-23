//
/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>

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
#ifndef __open_with_h__
#define __open_with_h__

#include <tqdialog.h>

#include <kurl.h>
#include <krun.h>
#include <kservice.h>

class TDEApplicationTree;
class KURLRequester;

class TQWidget;
class TQCheckBox;
class TQPushButton;
class TQLabel;

class KOpenWithDlgPrivate;

/* ------------------------------------------------------------------------- */
/**
 * "Open with" dialog box.
 * Used automatically by KRun, and used by libkonq.
 *
 * @author David Faure <faure@kde.org>
 */
class TDEIO_EXPORT KOpenWithDlg : public TQDialog //#TODO: Use KDialogBase for KDE4
{
    TQ_OBJECT
public:

    /**
     * Create a dialog that asks for a application to open a given
     * URL(s) with.
     *
     * @param urls   the URLs that should be opened. The list can be empty,
     * if the dialog is used to choose an application but not for some particular URLs.
     * @param parent parent widget
     */
    KOpenWithDlg( const KURL::List& urls, TQWidget *parent = 0L );

    /**
     * Create a dialog that asks for a application to open a given
     * URL(s) with.
     *
     * @param urls   is the URL that should be opened
     * @param text   appears as a label on top of the entry box.
     * @param value  is the initial value of the line
     * @param parent parent widget
     */
    KOpenWithDlg( const KURL::List& urls, const TQString& text, const TQString& value,
                  TQWidget *parent = 0L );

    /**
     * Create a dialog to select a service for a given service type.
     * Note that this dialog doesn't apply to URLs.
     *
     * @param serviceType the service type we want to choose an application for.
     * @param value  is the initial value of the line
     * @param parent parent widget
     */
    KOpenWithDlg( const TQString& serviceType, const TQString& value,
                  TQWidget *parent = 0L );

    /**
     * Create a dialog to select an application
     * Note that this dialog doesn't apply to URLs.
     *
     * @param parent parent widget
     * @since 3.1
     */
    KOpenWithDlg( TQWidget *parent = 0L );

    /**
     * Destructor
     */
    ~KOpenWithDlg();

    /**
     * @return the text the user entered
     */
    TQString text() const;
    /**
     * Hide the "Do not &close when command exits" Checkbox
     */
    void hideNoCloseOnExit();
    /**
     * Hide the "Run in &terminal" Checkbox
     */
    void hideRunInTerminal();
    /**
     * @return the chosen service in the application tree
     * Can be null, if the user typed some text and didn't select a service.
     */
    KService::Ptr service() const { return m_pService; }
    /**
     * Set whether a new .desktop file should be created if the user selects an
     * application for which no corresponding .desktop file can be found.
     *
     * Regardless of this setting a new .desktop file may still be created if
     * the user has chosen to remember the file association.
     *
     * The default is false: no .desktop files are created.
     * @since 3.2
     */
    void setSaveNewApplications(bool b);

public slots:
    /**
    * The slot for clearing the edit widget
    */
    void slotClear();
    void slotSelected( const TQString&_name, const TQString& _exec, const TQString& _desktopPath );
    void slotHighlighted( const TQString& _name, const TQString& _exec, const TQString& _desktopPath );
    void slotTextChanged();
    void slotTerminalToggled(bool);
    void slotDbClick();
    void slotOK();

protected slots:
    /**
     * Reimplemented from TQDialog::accept() to save history of the combobox
     */
    virtual void accept();

protected:

    /**
     * Determine service type from URLs
     */
    void setServiceType( const KURL::List& _urls );

    /**
     * Create a dialog that asks for a application to open a given
     * URL(s) with.
     *
     * @param text   appears as a label on top of the entry box.
     * @param value  is the initial value of the line
     */
    void init( const TQString& text, const TQString& value );

    KURLRequester * edit;
    TQString m_command;

    TDEApplicationTree* m_pTree;
    TQLabel *label;

    TQString qName, qServiceType;
    bool m_terminaldirty;
    TQCheckBox   *terminal, *remember, *nocloseonexit;
    TQPushButton *UNUSED;
    TQPushButton *UNUSED2;

    KService::Ptr m_pService;

    KOpenWithDlgPrivate *d;
};

/* ------------------------------------------------------------------------- */

#ifndef KDE_NO_COMPAT
/**
 * This class handles the displayOpenWithDialog call, made by KRun
 * when it has no idea what to do with a URL.
 * It displays the open-with dialog box.
 *
 * If you use KRun you _need_ to create an instance of KFileOpenWithHandler
 * (except if you can make sure you only use it for executables or
 *  Type=Application desktop files)
 *
 *
 */
class TDEIO_EXPORT_DEPRECATED KFileOpenWithHandler : public KOpenWithHandler
{
public:
  KFileOpenWithHandler() : KOpenWithHandler() {}
  virtual ~KFileOpenWithHandler() {}

  /**
   * Opens an open-with dialog box for @p urls
   * @returns true if the operation succeeded
   */
  virtual bool displayOpenWithDialog( const KURL::List& urls );
};
#endif


/* ------------------------------------------------------------------------- */

#endif
