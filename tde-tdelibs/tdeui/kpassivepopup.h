/*
 *   Copyright            : (C) 2001-2002 by Richard Moore
 *   Copyright            : (C) 2004-2005 by Sascha Cunz
 *   License              : This file is released under the terms of the LGPL, version 2.
 *   Email                : rich@kde.org
 *   Email                : sascha.cunz@tiscali.de
 */

#ifndef KPASSIVEPOPUP_H
#define KPASSIVEPOPUP_H

#include <tqframe.h>

#include <tdelibs_export.h>

class TQBoxLayout;
class TQTimer;
class TQLabel;
class TQVBox;

/**
 * @short A dialog-like popup that displays messages without interupting the user.
 *
 * The simplest uses of KPassivePopup are by using the various message() static
 * methods. The position the popup appears at depends on the type of the parent window:
 *
 * @li Normal Windows: The popup is placed adjacent to the icon of the window.
 * @li System Tray Windows: The popup is placed adjact to the system tray window itself.
 * @li Skip Taskbar Windows: The popup is placed adjact to the window
 *     itself if it is visible, and at the edge of the desktop otherwise.
 *
 * You also have the option of calling show with a TQPoint as a parameter that
 * removes the automatic placing of KPassivePopup and shows it in the point you want.
 *
 * The most basic use of KPassivePopup displays a popup containing a piece of text:
 * \code
 *    KPassivePopup::message( "This is the message", this );
 * \endcode
 * We can create popups with titles and icons too, as this example shows:
 * \code
 *    TQPixmap px;
 *    px.load( "hi32-app-logtracker.png" );
 *    KPassivePopup::message( "Some title", "This is the main text", px, this );
 * \endcode
 * For more control over the popup, you can use the setView(TQWidget *) method
 * to create a custom popup.
 * \code
 *    KPassivePopup *pop = new KPassivePopup( parent );
 *
 *    TQVBox *vb = new TQVBox( pop );
 *    (void) new TQLabel( vb, "<b>Isn't this great?</b>" );
 *
 *    TQHBox *box = new TQHBox( vb );
 *    (void) new TQPushButton( box, "Yes" );
 *    (void) new TQPushButton( box, "No" );
 *
 *    pop->setView( vb );
 *    pop->show();
 * \endcode
 *
 * @version $Id$
 * @since 3.1
 * @author Richard Moore, rich@kde.org
 * @author Sascha Cunz, sascha.cunz@tiscali.de
 */
class TDEUI_EXPORT KPassivePopup : public TQFrame
{
    TQ_OBJECT
    TQ_PROPERTY (bool autoDelete READ autoDelete WRITE setAutoDelete )
    TQ_PROPERTY (int timeout READ timeout WRITE setTimeout )

public:
    /**
     * Styles that a KPassivePopup can have.
     * @since 3.5
     */
    enum PopupStyle
    {
        Boxed,             ///< Information will appear in a framed box (default)
        Balloon,           ///< Information will appear in a comic-alike balloon
	CustomStyle=128    ///< Ids greater than this are reserved for use by subclasses
    };

    /**
     * Creates a popup for the specified widget.
     */
    KPassivePopup( TQWidget *parent=0, const char *name=0, WFlags f=0 );

    /**
     * Creates a popup for the specified window.
     */
    KPassivePopup( WId parent, const char *name=0, WFlags f=0 );

    /**
     * Creates a popup for the specified widget.
     * @since 3.5
     */
    KPassivePopup( int popupStyle, TQWidget *parent=0, const char *name=0, WFlags f=0 );

    /**
     * Creates a popup for the specified window.
     * @since 3.5
     */
    KPassivePopup( int popupStyle, WId parent, const char *name=0, WFlags f=0 );

    /**
     * Cleans up.
     */
    virtual ~KPassivePopup();

    /**
     * Sets the main view to be the specified widget (which must be a child of the popup).
     */
    void setView( TQWidget *child );

    /**
     * Creates a standard view then calls setView(TQWidget*) .
     */
    void setView( const TQString &caption, const TQString &text = TQString::null );

    /**
     * Creates a standard view then calls setView(TQWidget*) .
     */
    virtual void setView( const TQString &caption, const TQString &text, const TQPixmap &icon );

    /**
     * Returns a widget that is used as standard view if one of the 
     * setView() methods taking the TQString arguments is used.
     * You can use the returned widget to customize the passivepopup while 
     * keeping the look similar to the "standard" passivepopups.
     *
     * After customizing the widget, pass it to setView( TQWidget* )
     *
     * @param caption The window caption (title) on the popup
     * @param text The text for the popup
     * @param icon The icon to use for the popup
     * @param parent The parent widget used for the returned TQVBox. If left 0L,
     * then "this", i.e. the passive popup object will be used.
     *
     * @return a TQVBox containing the given arguments, looking like the
     * standard passivepopups.
     * @see setView( TQWidget * )
     * @see setView( const TQString&, const TQString& )
     * @see setView( const TQString&, const TQString&, const TQPixmap& )
     */
    TQVBox * standardView( const TQString& caption, const TQString& text,
                          const TQPixmap& icon, TQWidget *parent = 0L );

    /**
     * Returns the main view.
     */
    TQWidget *view() const { return msgView; }

    /**
     * Returns the delay before the popup is removed automatically.
     */
    int timeout() const { return hideDelay; }

    /**
     * Enables / disables auto-deletion of this widget when the timeout
     * occurs.
     * The default is false. If you use the class-methods message(),
     * auto-delection is turned on by default.
     */
    virtual void setAutoDelete( bool autoDelete );

    /**
     * @returns true if the widget auto-deletes itself when the timeout occurs.
     * @see setAutoDelete
     */
    bool autoDelete() const { return m_autoDelete; }

    /**
     * Sets the anchor of this balloon. The balloon tries automatically to adjust
     * itself somehow around the point.
     * @since 3.5
     */
    void setAnchor( const TQPoint& anchor );

    // TODO KDE4: give all the statics method a const TQPoint p = TQPoint() that in 
    // case the point is not null calls the show(cosnt TQPoint &p) method instead
    // the show() one.
    /**
     * Convenience method that displays popup with the specified  message  beside the
     * icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const TQString &text, TQWidget *parent, const char *name=0 );

    /**
     * Convenience method that displays popup with the specified caption and message
     * beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const TQString &caption, const TQString &text,
				   TQWidget *parent, const char *name=0 );

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const TQString &caption, const TQString &text,
				   const TQPixmap &icon,
				   TQWidget *parent, const char *name=0, int timeout = -1 );

    /**
     * Convenience method that displays popup with the specified icon, caption and
     * message beside the icon of the specified window.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( const TQString &caption, const TQString &text,
				   const TQPixmap &icon,
				   WId parent, const char *name=0, int timeout = -1 );

    /**
     * Convenience method that displays popup with the specified popup-style and message beside the
     * icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const TQString &text, TQWidget *parent,
				   const char *name=0 );

    /**
     * Convenience method that displays popup with the specified popup-style, caption and message
     * beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const TQString &caption, const TQString &text,
				   TQWidget *parent, const char *name=0 );

    /**
     * Convenience method that displays popup with the specified popup-style, icon, caption and
     * message beside the icon of the specified widget.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const TQString &caption, const TQString &text,
				   const TQPixmap &icon,
				   TQWidget *parent, const char *name=0, int timeout = -1 );

    /**
     * Convenience method that displays popup with the specified popup-style, icon, caption and
     * message beside the icon of the specified window.
     * Note that the returned object is destroyed when it is hidden.
     * @see setAutoDelete
     */
    static KPassivePopup *message( int popupStyle, const TQString &caption, const TQString &text,
				   const TQPixmap &icon,
				   WId parent, const char *name=0, int timeout = -1 );


public slots:
    /**
     * Sets the delay for the popup is removed automatically. Setting the delay to 0
     * disables the timeout, if you're doing this, you may want to connect the
     * clicked() signal to the hide() slot.
     * Setting the delay to -1 makes it use the default value.
     *
     * @see timeout
     */
    void setTimeout( int delay );

    /**
     * Reimplemented to reposition the popup.
     */
    virtual void show();

    /**
     * Shows the popup in the given point
     * @since 3.5
     */
    void show(const TQPoint &p);

signals:
    /**
     * Emitted when the popup is clicked.
     */
    void clicked();

    /**
     * Emitted when the popup is clicked.
     */
    void clicked( TQPoint pos );

    /**
     * Emitted when the popup is hidden.
     */
    void hidden(KPassivePopup*);

protected:
    /**
     * This method positions the popup.
     */
    virtual void positionSelf();

    /**
     * Reimplemented to destroy the object when autoDelete() is
     * enabled.
     */
    virtual void hideEvent( TQHideEvent * );

    /**
     * Moves the popup to be adjacent to the icon of the specified rectangle.
     */
    void moveNear( TQRect target );

    /**
     * Reimplemented to detect mouse clicks.
     */
    virtual void mouseReleaseEvent( TQMouseEvent *e );

    /**
     * If no relative window (eg taskbar button, system tray window) is
     * available, use this rectangle (pass it to moveNear()).
     * Basically KWinModule::workArea() with width and height set to 0
     * so that moveNear uses the upper-left position.
     * @return The TQRect to be passed to moveNear() if no other is
     * available.
     */
    TQRect defaultArea() const;

    /**
     * Updates the transparency mask. Unused if PopupStyle == Boxed
     * @since 3.5
     */
    void updateMask();

    /**
     * Overwrite to paint the border when PopupStyle == Balloon.
     * Unused if PopupStyle == Boxed
     */
    virtual void paintEvent( TQPaintEvent* pe );

private:
    void init( int popupStyle );

    WId window;
    TQWidget *msgView;
    TQBoxLayout *topLayout;
    int hideDelay;
    TQTimer *hideTimer;

    TQLabel *ttlIcon;
    TQLabel *ttl;
    TQLabel *msg;

    bool m_autoDelete;

    /* @internal */
    class Private;
    Private *d;
};

#endif // KPASSIVEPOPUP_H
