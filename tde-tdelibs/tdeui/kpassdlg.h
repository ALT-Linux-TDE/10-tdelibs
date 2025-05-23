/* This file is part of the KDE libraries
   Copyright (C) 1998 Pietro Iglio <iglio@fub.it>
   Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
   Copyright (C) 2004,2005 Andrew Coles <andrew_coles@yahoo.co.uk>

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
#ifndef __KPassDlg_h_included__
#define __KPassDlg_h_included__

#include <tqstring.h>
#include <tqlineedit.h>
#include <kdialogbase.h>

class TQLabel;
class TQGridLayout;
class TQWidget;

/**
 * @short A safe password input widget.
 * @author Geert Jansen <geertj@kde.org>
 *
 * The widget uses the user's global "echo mode" setting.
 */

class TDEUI_EXPORT KPasswordEdit
    : public TQLineEdit
{
    TQ_OBJECT

public:
    enum EchoModes { OneStar, ThreeStars, NoEcho };

    /**
     * Constructs a password input widget using the user's global "echo mode" setting.
     */
    KPasswordEdit(TQWidget *parent=0, const char *name=0);
    // KDE4: either of the two must go! add default values for parameters

    /**
     * Constructs a password input widget using echoMode as "echo mode".
     * Note that echoMode is a TQLineEdit::EchoMode.
     * @since 3.0
     */
    KPasswordEdit(EchoMode echoMode, TQWidget *parent, const char *name);

     /**
     * Constructs a password input widget using echoMode as "echo mode".
     * Note that echoMode is a KPasswordEdit::EchoModes.
     * @since 3.2
     */
    KPasswordEdit(EchoModes echoMode, TQWidget *parent, const char *name);

    /**
     * @deprecated, will be removed in KDE 4.0
     * Creates a password input widget using echoMode as "echo mode".
     */
    KPasswordEdit(TQWidget *parent, const char *name, int echoMode) TDE_DEPRECATED;

    /**
     * Destructs the widget.
     */
    ~KPasswordEdit();

    /**
     * Returns the password.
     */
    TQString password() const;

    /**
     * Erases the current password.
     */
    void erase();

    static const int PassLen;

    /**
     * Set the current maximum password length.  If a password longer than the limit
     * specified is currently entered, it is truncated accordingly.
     *
     * @param newLength: The new maximum password length
     * @since 3.4
     */
    void setMaxPasswordLength(int newLength);

    /**
     * Returns the current maximum password length.
     * @since 3.4
     */
    int maxPasswordLength() const;

public slots:
    /**
     * Reimplementation
     */
    virtual void insert( const TQString &);

protected:
    virtual void keyPressEvent(TQKeyEvent *);
    virtual void focusInEvent(TQFocusEvent *e);
    virtual bool event(TQEvent *e);

private:
    void init();
};


/**
 * @short A password input dialog.
 *
 * This dialog asks the user to enter a password. The functions you're
 * probably interested in are the static methods, getPassword() and
 * getNewPassword().
 *
 * <b>Usage example</b>\n
 *
 * \code
 * TQString password;
 * int result = KPasswordDialog::getPassword(password, i18n("Prompt message"));
 * if (result == KPasswordDialog::Accepted)
 *     use(password);
 * \endcode
 *
 * \image html kpassworddialog.png "KDE Password Dialog"
 *
 * <b>Security notes:</b>\n
 *
 * Keeping passwords in memory can be a potential security hole. You should
 * handle this situation with care.
 *
 * @li You may want to use disableCoreDump() to disable core dumps.
 * Core dumps are dangerous because they are an image of the process memory,
 * and thus include any passwords that were in memory.
 *
 * @author Geert Jansen <jansen@kde.org>
 */

class TDEUI_EXPORT KPasswordDialog
    : public KDialogBase
{
    TQ_OBJECT

public:
    /**
     * This enum distinguishes the two operation modes of this dialog:
     */
    enum Types {
        /**
         * The user is asked to enter a password.
         */
        Password,

        /**
         * The user is asked to enter a password and to confirm it
         * a second time. This is usually used when the user
         * changes his password.
         */
        NewPassword
    };

    /**
     * Constructs a password dialog.
     *
     * @param type: if NewPassword is given here, the dialog contains two
     *        input fields, so that the user must confirm his password
     *        and possible typos are detected immediately.
     * @param enableKeep: if true, a check box is shown in the dialog
     *        which allows the user to keep his password input for later.
     * @param extraBttn: allows to show additional buttons, KDialogBase.
     * @param parent Passed to lower level constructor.
     * @param name Passed to lower level constructor
     *
     * @since 3.0
     */
    KPasswordDialog(Types type, bool enableKeep, int extraBttn,
                    TQWidget *parent=0, const char *name=0);

    /**
     * @deprecated Variant of the previous constructor without the
     * possibility to specify a parent. Will be removed in KDE 4.0
     */
    KPasswordDialog(int type, TQString prompt, bool enableKeep=false,
                    int extraBttn=0) TDE_DEPRECATED;
    // note that this implicitly deprecates the 'prompt' variants of
    // getPassword() below. i guess the above constructor needs to be extended.

    /**
     * Construct a password dialog. Essentially the same as above but allows the
     * icon in the password dialog to be set via @p iconName.
     * @param type if NewPassword is given here, the dialog contains two
     * input fields, so that the user must confirm his password
     * and possible typos are detected immediately
     * @param enableKeep: if true, a check box is shown in the dialog
     *        which allows the user to keep his password input for later.
     * @param extraBttn: allows to show additional buttons.
     * @param iconName the name of the icon to be shown in the dialog. If empty,
     * a default icon is used
     * @param parent Passed to lower level constructor.
     * @param name Passed to lower level constructor
     * @since 3.3
     */
    KPasswordDialog(Types type, bool enableKeep, int extraBttn, const TQString& iconName,
                    TQWidget *parent = 0, const char *name = 0);

    /**
     * Destructs the password dialog.
     */
    virtual ~KPasswordDialog();

    /**
     * Sets the password prompt.
     */
    void setPrompt(TQString prompt);

    /**
     * Returns the password prompt.
     */
    TQString prompt() const;

    /**
     * Sets the text to be dynamically displayed when the keep checkbox is checked
     */
    void setKeepWarning(TQString warn);

    /**
     * Adds a line of information to the dialog.
     */
    void addLine(TQString key, TQString value);

    /**
     * Allow empty passwords? - Default: false
     * @since 3.4
     */
    void setAllowEmptyPasswords(bool allowed);

    /**
     * Allow empty passwords?
     * @since 3.4
     */
    bool allowEmptyPasswords() const;

    /**
     * Minimum acceptable password length.
     * Default: If empty passwords are forbidden, 1;
     *          Otherwise, 0.
     *
     * @param minLength: The new minimum password length
     * @since 3.4
     */
    void setMinimumPasswordLength(int minLength);

    /**
     * Minimum acceptable password length.
     * @since 3.4
     */
    int minimumPasswordLength() const;

    /**
     * Maximum acceptable password length.  Limited to 199.
     * Default: No limit, i.e. -1
     *
     * @param maxLength: The new maximum password length.
     * @since 3.4
     */
    void setMaximumPasswordLength(int maxLength);

    /**
     * Maximum acceptable password length.
     * @since 3.4
     */
    int maximumPasswordLength() const;

    /**
     * Password length that is expected to be reasonably safe.
     *
     * Default: 8 - the standard UNIX password length
     *
     * @param reasonableLength: The new reasonable password length.
     * @since 3.4
     */
    void setReasonablePasswordLength(int reasonableLength);

    /**
     * Password length that is expected to be reasonably safe.
     * @since 3.4
     */
    int reasonablePasswordLength() const;

    /**
     * Set the password strength level below which a warning is given
     * Value is in the range 0 to 99. Empty passwords score 0;
     * non-empty passwords score up to 100, depending on their length and whether they
     * contain numbers, mixed case letters and punctuation.
     *
     * Default: 1 - warn if the password has no discernable strength whatsoever
     * @param warningLevel: The level below which a warning should be given.
     * @since 3.4
     */
    void setPasswordStrengthWarningLevel(int warningLevel);

    /**
     * Password strength level below which a warning is given
     * @since 3.4
     */
    int passwordStrengthWarningLevel() const;

    /**
     * Returns the password entered.
     */
    TQString password() const { return m_pEdit->password(); }

    /**
     * Clears the password input field. You might want to use this after the
     * user failed to enter the correct password.
     * @since 3.3
     */
    void clearPassword();

    /**
     * Returns true if the user wants to keep the password.
     */
    bool keep() const { return m_Keep; }

    /**
     * Pops up the dialog, asks the user for a password, and returns it.
     *
     * @param password The password is returned in this reference parameter.
     * @param prompt A prompt for the password. This can be a few lines of
     * information. The text is word broken to fit nicely in the dialog.
     * @param keep Enable/disable a checkbox controlling password keeping.
     * If you pass a null pointer, or a pointer to the value 0, the checkbox
     * is not shown. If you pass a pointer to a nonzero value, the checkbox
     * is shown and the result is stored in *keep.
     * @return Result code: Accepted or Rejected.
     */
    static int getPassword(TQString &password, TQString prompt, int *keep=0L);

    /**
     * Pops up the dialog, asks the user for a password and returns it. The
     * user has to enter the password twice to make sure it was entered
     * correctly.
     *
     * @param password The password is returned in this reference parameter.
     * @param prompt A prompt for the password. This can be a few lines of
     * information. The text is word broken to fit nicely in the dialog.
     * @return Result code: Accepted or Rejected.
     */
    static int getNewPassword(TQString &password, TQString prompt);

    /**
     * Static helper function that disables core dumps.
     */
    static void disableCoreDumps();

protected slots:
    void slotOk();
    void slotCancel();
    void slotKeep(bool);
    void slotLayout();

protected:

    /**
     * Virtual function that can be overridden to provide password
     * checking in derived classes. It should return @p true if the
     * password is valid, @p false otherwise.
     */
    virtual bool checkPassword(const TQString&) { return true; }

private slots:
  void enableOkBtn();

private:
    void init();
    void erase();

    int m_Keep;
    int m_Type;
    int m_Row;
    TQLabel *m_pHelpLbl;
    TQLabel *m_keepWarnLbl;
    TQGridLayout *m_pGrid;
    TQWidget *m_pMain;
    KPasswordEdit *m_pEdit;
    KPasswordEdit *m_pEdit2;

protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KPasswordDialogPrivate;
    KPasswordDialogPrivate* const d;
};


#endif // __KPassDlg_h_included__
