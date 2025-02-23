/*
 *
 * $Id$
 * This file is part of the KDE project, module kdesktop.
 * Copyright (C) 1999,2000 Geert Jansen <jansen@kde.org>
 *
 * You can Freely distribute this program under the GNU Library General
 * Public License. See the file "COPYING.LIB" for the exact licensing terms.
 */

#ifndef __KRootPixmap_h_Included__
#define __KRootPixmap_h_Included__

#include <tqobject.h>
#include <tqcolor.h>
#include <tdelibs_export.h>

#ifndef TQ_WS_QWS //FIXME

class TQRect;
class TQWidget;
class TQTimer;
class TDESharedPixmap;
class KRootPixmapData;

/**
 * Creates pseudo-transparent widgets.
 *
 * A pseudo-transparent widget is a widget with its background pixmap set to
 * that part of the desktop background that it is currently obscuring. This
 * gives a transparency effect.
 *
 * To create a transparent widget, construct a KRootPixmap and pass it a
 * pointer to your widget. That's it! Moving, resizing and background changes
 * are handled automatically.
 *
 * Instead of using the default behavior, you can ask KRootPixmap
 * to emit a backgroundUpdated(const TQPixmap &) signal whenever
 * the background needs updating by using setCustomPainting(bool).
 * Alternatively by reimplementing updateBackground(TDESharedPixmap*)
 * you can take complete control of the behavior.
 *
 * @author Geert Jansen <jansen@kde.org>
 * @version $Id$
 */
class TDEUI_EXPORT KRootPixmap: public TQObject
{
    TQ_OBJECT

public:
    /**
     * Constructs a KRootPixmap. The KRootPixmap will be created as a child
     * of the target widget so it will be deleted automatically when the
     * widget is destroyed.
     *
     * @param target A pointer to the widget that you want to make pseudo
     * transparent.
     * @param name The internal name of the pixmap
     */
    KRootPixmap( TQWidget *target, const char *name=0 );

    /**
     * Constructs a KRootPixmap where the parent TQObject and target TQWidget are
     * different.
     */
    KRootPixmap( TQWidget *target, TQObject *parent, const char *name=0 );

    /**
     * Destructs the object.
     */
    virtual ~KRootPixmap();

    /**
     * Checks if pseudo-transparency is available.
     * @return @p true if transparency is available, @p false otherwise.
     */
    bool isAvailable() const;

    /**
     * Returns true if the KRootPixmap is active.
     */
    bool isActive() const { return m_bActive; }

    /**
     * Returns the number of the current desktop.
     */
    int currentDesktop() const;

    /**
     * Returns true if custom painting is enabled, false otherwise.
     * @see setCustomPainting(bool)
     */
    bool customPainting() const { return m_bCustomPaint; }

#ifndef KDE_NO_COMPAT
    /**
     * Deprecated, use isAvailable() instead.
     * @deprecated
     */
    TDE_DEPRECATED bool checkAvailable(bool) { return isAvailable(); }
#endif

    /** @since 3.2
     * @return the fade color.
     */
    const TQColor &color() const { return m_FadeColor; }

    /** @since 3.5
     * @return the blur radius.
     */
    const double &blurRadius() const { return m_BlurRadius; }

    /** @since 3.5
     * @return the blur sigma.
     */
    const double &blurSigma() const { return m_BlurSigma; }

    /** @since 3.2
     * @return the color opacity.
     */
    double opacity() const { return m_Fade; }

public slots:
    /**
     * Starts background handling.
     */
    virtual void start();

    /**
     * Stops background handling.
     */
    virtual void stop();

    /**
     * Sets the fade effect.
     *
     * This effect will fade the background to the
     * specified color.
     * @param opacity A value between 0 and 1, indicating the opacity
     * of the color. A value of 0 will not change the image, a value of 1
     * will use the fade color unchanged.
     * @param color The color to fade to.
     */
    void setFadeEffect(double opacity, const TQColor &color);

    /**
     * Sets the blue effect.
     *
     * This effect will blur the background with the specified values.
     * If both values are set to zero no blur is applied (this is the default).
     * @param radius The radius of the gaussian not counting the
     * center pixel. Use 0 and a suitable radius will be automatically used.
     * @param sigma The standard deviation of the gaussian. Use 1 if you're not
     * sure.
     */
    void setBlurEffect(double radius, double sigma);

    /**
     * Repaints the widget background. Normally, you shouldn't need this
     * as it is handled automatically.
     *
     * @param force Force a repaint, even if the contents did not change.
     */
    void repaint( bool force );

    /**
     * Repaints the widget background. Normally, you shouldn't need this
     * as it is handled automatically. This is equivalent to calling
     * repaint( false ).
     */
    void repaint();

    /**
     * Enables custom handling of the background painting. If custom
     * painting is enabled then KRootPixmap will emit a
     * backgroundUpdated() signal when the background for the
     * target widget changes, instead of applying the new background.
     */
    void setCustomPainting( bool enable ) { m_bCustomPaint = enable; }

    /**
     * Asks KDesktop to export the desktop background as a TDESharedPixmap.
     * This method uses DCOP to call KBackgroundIface/setExport(int).
     */
    void enableExports();

    /**
     * Returns the name of the shared pixmap (only needed for low level access)
     */
    static TQString pixmapName(int desk);
signals:
    /**
     * Emitted when the background needs updating and custom painting
     * (see setCustomPainting(bool) ) is enabled.
     *
     * @param pm A pixmap containing the new background.
     */
    void backgroundUpdated( const TQPixmap &pm );

protected:
    /**
     * Reimplemented to filter the events from the target widget and
     * track its movements.
     */
    virtual bool eventFilter(TQObject *, TQEvent *);

    /**
     * Called when the pixmap has been updated. The default implementation
     * applies the fade effect, then sets the target's background, or emits
     * backgroundUpdated(const TQPixmap &) depending on the painting mode.
     */
    virtual void updateBackground( TDESharedPixmap * );

private slots:
    void slotBackgroundChanged(int);
    void slotDone(bool);
    void desktopChanged(int desktop);
    void desktopChanged( WId window, unsigned int properties );

private:
    bool m_bActive, m_bInit, m_bCustomPaint;
    int m_Desk;

    double m_Fade;
    TQColor m_FadeColor;
    double m_BlurRadius;
    double m_BlurSigma;

    TQRect m_Rect;
    TQWidget *m_pWidget;
    TQTimer *m_pTimer;
    TDESharedPixmap *m_pPixmap;
    KRootPixmapData *d;

    void init();
};

#endif // ! TQ_WS_QWS
#endif // __KRootPixmap_h_Included__

