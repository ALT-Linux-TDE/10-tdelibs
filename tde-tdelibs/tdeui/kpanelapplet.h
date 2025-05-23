/*****************************************************************

Copyright (c) 2000 Matthias Elter

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

#ifndef __kpanelapplet_h__
#define __kpanelapplet_h__

#include <tqframe.h>
#include <tdeconfig.h>

class TDEConfig;
class TQPopupMenu;

/**
 * @short %TDE Panel Applet class
 *
 * Panel applets
 *   @li Are small applications living in the %TDE panel.
 *   @li Are implemented as DSOs (Dynamic Shared Objects).
 *
 * Note: For security and stability reasons the panel won't load
 * untrusted third party applets directly into its namespace but via an
 * external wrapper process.
 *
 * The panel locates available applets by searching for applet desktop
 * files in (ALL_DIRS)/share/apps/kicker/applets. Every panel applet should
 * install a desktop file there to be recognized by the panel.
 *
 * Besides standard keys like "Name", "Comment" and "Icon" there are
 * two panel applet specific keys:
 *
 * \b X-TDE-Library \n
 *
 * Used by the panel to locate the applet DSO (Dynamic Shared Object)
 * Example: X-TDE-Library=libexampleapplet
 *
 * \b X-TDE-UniqueApplet \n
 *
 * Similar to TDEApplication and KUniqueApplication there are
 * two types of panel applets. Use unique applets when it makes no
 * sence to run more than one instance of a applet in the panel. A
 * good example for unique applets is the taskbar applet.  Use normal
 * applets when you need instance specific configuration. An example
 * is the koolclock applet where you might want to run two instances
 * in your panel, one configured as analog clock, the other one as
 * digital clock. X-TDE-UniqueApplet is a boolean key which defaults
 * to "false".  Example: X-TDE-UniqueApplet=true
 *
 * Back to panel applet DSOs, the following conventions are used for %KDE:
 * Name:    lib<appletname>applet.la
 * LDFLAGS: -module -no-undefined
 *
 * To implement a panel applet it is not enough to write a class
 * inheriting from KPanelApplet but you also have to provide a
 * factory function in your DSO.  A sample factory function could look
 * like this:
 *
 * \code
 *
 * extern "C"
 * {
 *     KPanelApplet* init(TQWidget *parent, const TQString& configFile)
 *     {
 *         TDEGlobal::locale()->insertCatalogue("exampleapplet");
 *         return new ExampleApplet(configFile, KPanelApplet::Normal,
 *                       KPanelApplet::About | KPanelApplet::Help | KPanelApplet::Preferences,
 *                       parent, "exampleapplet");
 *     }
 * }
 *
 * \endcode
 *
 * Note: Don't change the factory function signature or the panel will
 * fail to load your applet.
 *
 * @author Matthias Elter <elter@kde.org>
 **/
class TDEUI_EXPORT KPanelApplet : public TQFrame
{
    TQ_OBJECT

public:

    /**
     * This enum describes the type of the applet.
     */
    enum Type { Normal = 0, Stretch };
    /**
     * This enum holds flags which can be ORed together to describe
     * which items the context menu over the applet handle contains.
     */
    // KDE4: Merge these with KPanelExtension's enums
    enum Action { About = 1, Help = 2, Preferences = 4, ReportBug = 8 };
    enum Position { pLeft = 0, pRight, pTop, pBottom };
    enum Alignment { LeftTop = 0, Center, RightBottom };
    enum Direction { Up = 0, Down, Left, Right };

    /**
     * Constructs a KPanelApplet just like any other widget.
     *
     * @param configFile The configFile handed over in the factory function.
     * @param t The applet type().
     * @param actions Standard RMB menu actions supported by the applet (see action() ).
     * @param parent The pointer to the parent widget handed over in the factory function.
     * @param name A Qt object name for your applet.
     * @param f Window control flags
     **/
    KPanelApplet(const TQString& configFile, Type t = Normal,
                 int actions = 0, TQWidget *parent = 0, const char *name = 0,
                 WFlags f = 0);

    /**
     * Destructor.
     **/
    ~KPanelApplet();

    /**
     * Returns a suggested width for a given height.
     *
     * Every applet should reimplement this function.
     *
     * Depending on the panel orientation the height (horizontal panel) or the
     * width (vertical panel) of the applets is fixed.
     * The exact values of the fixed size component depend on the panel size.
     *
     * On a horizontal panel the applet height is fixed, the panel will
     * call widthForHeight(int height) with @p height
     * equal to 'the fixed applet height'
     * when laying out the applets.
     *
     * The applet can now choose the other size component (width)
     * based on the given height.
     *
     * The width you return is granted.
     **/
    virtual int widthForHeight(int height) const { return height; }

    /**
     * @return A suggested height for a given width.
     *
     * Every applet should reimplement this function.
     *
     * Depending on the panel orientation the height (horizontal panel) or the
     * width (vertical panel) of the applets is fixed.
     * The exact values of the fixed size component depend on the panel size.
     *
     * On a vertical panel the applet width is fixed, the panel will
     * call heightForWidth(int width) with @p width
     * equal to 'the fixed applet width'
     * when laying out the applets.
     *
     * The applet can now choose the other size component (height)
     * based on the given width.
     *
     * The height you return is granted.
     **/
    virtual int heightForWidth(int width) const { return width; }

    /**
     * Always use this TDEConfig object to save/load your applet's configuration.
     *
     * For unique applets this config object will write to a config file called
     * \<appletname\>rc in the user's local %KDE directory.
     *
     * For normal applets this config object will write to a instance specific config file
     * called \<appletname\>\<instanceid\>rc in the user's local %KDE directory.
     **/
    TDEConfig* config() const { return _config; }
    TDESharedConfig::Ptr sharedConfig() const;

    /**
     * @return Type indicating the applet's type.
     * Type
     **/
    Type type() const { return _type; }

    /**
     * @return int indicating the supported RMB menu actions.
     * Action
     **/
    int actions() const { return _actions; }

    /**
     * Generic action dispatcher. Called  when the user selects an item
     * from the applet's RMB menu.
     *
     * Reimplement this function to handle actions.
     *
     * For About, Help, Preferences and ReportBug, use the convenience handlers
     * ref about(), help(), preferences(), reportBug()
     *
     **/
    virtual void action( Action a );

    /**
     * @return the applet's custom menu, usually the same as the context menu, or 0 if none
     * see setCustomMenu(TQPopupMenu*)
     */
    const TQPopupMenu* customMenu() const;

    /**
     * @internal
     **/
    void setPosition( Position p );
    /**
     * @internal
     **/
    void setAlignment( Alignment a );

signals:
    /**
     * Emit this signal to make the panel relayout all applets, when
     * you want to change your width (horizontal panel) or
     * height (vertical panel).
     *
     * The panel is going to relayout all applets based on their
     * widthForHeight(int height) (horizontal panel) or
     * heightForWidth(int width) (vertical panel).
     *
     * Please note that the panel may change the applet's location
     * if the new widthForHeight(int height) (horizontal panel) or
     * heightForWidth(int width) (vertical panel) does not fit into the
     * current panel layout.
     **/
    void updateLayout();

    /**
     * Request keyboard focus from the panel.
     * @deprecated
     **/
    void requestFocus();

    /**
     * Request keyboard focus from the panel. Applets should never call this directly
     * but rather call needsFocus(bool)
     * @see needsFocus
     * @param focus activate the window and ensure the panel remains visible when true
     * Each and ever time a requestFocus(true) is emitted, it MUST be paired eventually
     * with a requestFocus(false) otherwise the panel may end up never hiding
     * @since 3.4
     **/
    void requestFocus(bool focus);

protected:

    /**
     * Is called when the user selects "About" from the applet's RMB menu.
     * Reimplement this function to launch a about dialog.
     *
     * Note that this is called only when your applet supports the About action.
     * See Action and KPanelApplet().
     **/
    virtual void about() {}

    /**
     * Is called when the user selects "Help" from the applet's RMB menu.
     * Reimplement this function to launch a manual or help page.
     *
     * Note that this is called only when your applet supports the Help action.
     * See Action and KPanelApplet().
     **/
    virtual void help() {}

    /**
     * Is called when the user selects "Preferences" from the applet's RMB menu.
     * Reimplement this function to launch a preferences dialog or kcontrol module.
     *
     * Note that this is called only when your applet supports the preferences action.
     * See Action and KPanelApplet().
     **/
    virtual void preferences() {}

    /**
     * Is called when the user selects "Report bug" from the applet's RMB menu.
     * Reimplement this function to launch a bug reporting dialog.
     *
     * Note that this is called only when your applet supports the ReportBug
     * action.
     * See Action and KPanelApplet()
     **/
   virtual void reportBug() {}

    /**
     * @return the applet's orientation. (horizontal or vertical)
     **/
    Orientation orientation() const;
    /**
     * @return the applet's position. (top, left, bottom, or right)
     **/
    Position position() const { return _position; }
    /**
     * @return the applet's alignment. (top/left, center, or bottom/right)
     **/
    Alignment alignment() const { return _alignment; }

    /**
     * The panel on which this applet resides has changed its position.
     * Reimplement this change handler in order to adjust the look of your
     * applet.
     **/
    virtual void positionChange( Position p );

    /**
     * The panel on which this applet resides has changed its alignment.
     * Reimplement this change handler in order to adjust the look of your
     * applet.
     **/
    virtual void alignmentChange( Alignment /*a*/ ) {}

    /**
     * Use this method to set the custom menu for this applet so that it can be shown
     * in the applet handle menu and other appropriate places that the applet many not itself
     * be aware of. The applet itself is still responsible for deleting and managing the
     * the menu.
     *
     * If the menu is deleted during the life of the applet, be sure to call this method again
     * with the new menu (or 0) to avoid crashes
     */
    void setCustomMenu(const TQPopupMenu*);

    /**
     * Register widgets that can receive keyboard focus with this this method
     * This call results in an eventFilter being places on the widget.
     * @param widget the widget to watch for keyboard focus
     * @param watch whether to start watching the widget, or to stop doing so
     * @since 3.4
     */
    void watchForFocus(TQWidget* widget, bool watch = true);

    /**
     * Call this whenever focus is needed or not needed. You do not have to call this method
     * for widgets that have been registered with watchForFocus
     * @param focus whether to or not to request focus
     * @since 3.4
     */
    void needsFocus(bool focus);

    /**
     * The orientation changed to @p orientation. Reimplement this
     * change handler in order to adjust the look of your applet.
     *
     * @deprecated Reimplement positionChange instead.
     **/
    // FIXME: Remove for KDE 4
    virtual TDE_DEPRECATED void orientationChange( Orientation /* orientation*/) {}

    /**
     * A convenience method that translates the position of the applet into which
     * direction to show a popup.
     **/
    Direction popupDirection();

    /**
     * The popup direction changed to @p direction. Reimplement this
     * change handler in order to adjust the look of your applet.
     *
     * @deprecated Reimplement positionChange instead.
     **/
    // FIXME: Remove for KDE 4
    virtual TDE_DEPRECATED void popupDirectionChange( Direction /*direction*/ ) {}

    bool eventFilter(TQObject *, TQEvent *);

private:
    Type         _type;
    Position     _position;
    Alignment    _alignment;
    TDEConfig*     _config;
    int          _actions;
protected:
    virtual void virtual_hook( int id, void* data );
    class KPanelAppletPrivate;
    KPanelAppletPrivate *d;
};

#endif
