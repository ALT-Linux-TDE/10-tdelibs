/*  This file is part of the KDE Libraries
 *  Copyright (C) 1998 Thomas Tanghus (tanghus@earthling.net)
 *  Additions 1999-2000 by Espen Sand (espen@kde.org)
 *  Additions 2011 by Timothy Pearson (kb9vqf@pearsoncomputing.net)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
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
 */

#ifndef _KDIALOG_H_
#define _KDIALOG_H_

#include <tqdialog.h>
#include <tdelibs_export.h>

class TQLayoutItem;

/**
 * Dialog with extended non-modal support and methods for %KDE standard
 * compliance.
 *
 * Generally, you should not use this class directly, but KDialogBase
 * which inherits KDialog.
 *
 * If the dialog is non-modal and has a parent, the default keybindings
 * (@p escape = @p reject(), @p enter = @p accept(), etc.) are disabled.
 *
 * The marginHint() and spacingHint() sizes shall be used
 * whenever you layout the interior of a dialog. One special note. If
 * you make your own action buttons (OK, Cancel etc), the space
 * beteween the buttons shall be spacingHint(), whereas the space
 * above, below, to the right and to the left shall be marginHint().
 * If you add a separator line above the buttons, there shall be a
 * marginHint() between the buttons and the separator and a
 * marginHint() above the separator as well.
 *
 * @see KDialogBase
 * @author Thomas Tanghus <tanghus@earthling.net>, Espen Sand <espensa@online.no>
 */
class TDEUI_EXPORT KDialog : public TQDialog
{
  TQ_OBJECT

  public:

    /**
     * Constructor.
     *
     * Takes the same arguments as TQDialog.
     */
    KDialog(TQWidget *parent = 0, const char *name = 0,
	    bool modal = false, WFlags f = 0);

    /**
     * Return the number of pixels you shall use between a
     * dialog edge and the outermost widget(s) according to the KDE standard.
     **/
    static int marginHint();

    /**
     * Return the number of pixels you shall use between
     * widgets inside a dialog according to the KDE standard.
     */
    static int spacingHint();

    /**
     * Resize every layout manager used in @p widget and its nested children.
     *
     * @param widget The widget used.
     * @param margin The new layout margin.
     * @param spacing The new layout spacing.
     */
    static void resizeLayout( TQWidget *widget, int margin, int spacing );

    /**
     * Resize every layout associated with @p lay and its children.
     *
     * @param lay layout to be resized
     * @param margin The new layout margin
     * @param spacing The new layout spacing
     */
    static void resizeLayout( TQLayoutItem *lay, int margin, int spacing );

    /**
     * Centers @p widget on the desktop, taking multi-head setups into
     * account. If @p screen is -1, @p widget will be centered on its
     * current screen (if it was shown already) or on the primary screen.
     * If @p screen is -3, @p widget will be centered on the screen that
     * currently contains the mouse pointer.
     * @p screen will be ignored if a merged display (like Xinerama) is not
     * in use, or merged display placement is not enabled in kdeglobals.
     * @since 3.1
     */
    static void centerOnScreen( TQWidget *widget, int screen = -1 );

    /**
     * Places @p widget so that it doesn't cover a certain @p area of the screen.
     * This is typically used by the "find dialog" so that the match it finds can
     * be read.
     * For @p screen, see centerOnScreen
     * @return true on success (widget doesn't cover area anymore, or never did),
     * false on failure (not enough space found)
     * @since 3.2
     */
    static bool avoidArea( TQWidget *widget, const TQRect& area, int screen = -1 );

  public slots:
    /**
     * If the dialog starts with focus in a TQLineEdit child,
     * then call selectAll() on the child.
     */
    virtual void polish();

    /**
     * Make a KDE compliant caption.
     *
     * @param caption Your caption. Do @p not include the application name
     * in this string. It will be added automatically according to the KDE
     * standard.
     */
    virtual void setCaption( const TQString &caption );

    /**
     * Make a plain caption without any modifications.
     *
     * @param caption Your caption. This is the string that will be
     * displayed in the window title.
     */
    virtual void setPlainCaption( const TQString &caption );


  protected:
    /**
     * @internal
     */
    virtual void keyPressEvent(TQKeyEvent*);


   signals:
    /**
     * Emitted when the margin size and/or spacing size
     * have changed.
     *
     * Use marginHint() and spacingHint() in your slot
     * to get the new values.
     */
    void layoutHintChanged();

  private:
    static const int mMarginSize;
    static const int mSpacingSize;

  protected:
    virtual void virtual_hook( int id, void* data );
  private:
    class KDialogPrivate;
    KDialogPrivate* const d;

};


 /**
  * \brief Queue for showing modal dialogs one after the other.
  *
  * This is useful if you want to show a modal dialog but are not in the
  * position to start a new event loop at that point in your code.
  *
  * The disadvantage is that you will not be able to get any information from
  * the dialog, so it can currently only be used for simple dialogs.
  *
  * You probably want to use KMessageBox::queueMessageBox() instead
  * of this class directly.
  *
  * @author Waldo Bastian <bastian@kde.org>
  */
class KDialogQueuePrivate;
class TDEUI_EXPORT KDialogQueue : public TQObject
{
      TQ_OBJECT

public:

      static void queueDialog(TQDialog *);

      ~KDialogQueue();

protected:
      KDialogQueue();
      static KDialogQueue *self();

private slots:
      void slotShowQueuedDialog();

protected:
      KDialogQueuePrivate* const d;
      static KDialogQueue *_self;
};

 /**
  * \brief The default system modal dialog header
  *
  * Internal use only
  *
  * @author Timothy Pearson <kb9vqf@pearsoncomputing.net>
  */
class TDEUI_EXPORT KSMModalDialogHeader : public TQWidget
{
      TQ_OBJECT

public:
      KSMModalDialogHeader( TQWidget* parent );
      ~KSMModalDialogHeader();
};

 /**
  * \brief The default system modal dialog
  *
  * This displays the special TDE system modal dialog,
  * which is used solely to present noninterruptible 
  * session management status messages to the user.
  *
  * @author Timothy Pearson <kb9vqf@pearsoncomputing.net>
  */
class TQLabel;
class TQPushButton;
class TQHBoxLayout;
class TQFrame;
class TQGridLayout;
class TDEUI_EXPORT KSMModalDialog : public TQWidget
{
      TQ_OBJECT

public:
      KSMModalDialog( TQWidget* parent );
      ~KSMModalDialog();

      void setStatusMessage(TQString message);
      void setStartupPhase(TQString msg);
      void closeSMDialog();

protected slots:
      void keepMeOnTop();
      void closeEvent(TQCloseEvent *e);

protected:
      TQPushButton* m_button1;
      TQPushButton* m_button2;
      TQPushButton* m_button3;
      TQHBoxLayout* m_buttonbox;
      TQFrame* m_buttonframe;
      TQGridLayout* m_gridlayout;

private:
      TQTimer* m_keepOnTopTimer;
      TQLabel* m_statusLabel;
      bool m_allowClose;
};

#endif // __KDIALOG_H
