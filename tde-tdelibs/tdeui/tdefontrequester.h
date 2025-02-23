/*
    Copyright (C) 2003 Nadeem Hasan <nhasan@kde.org>

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

#ifndef TDEFONTREQUESTER_H
#define TDEFONTREQUESTER_H

#include <tqwidget.h>
#include <tqfont.h>
#include <tqstring.h>

#include <tdelibs_export.h>

class TQLabel;
class TQPushButton;

/**
 * This class provides a widget with a lineedit and a button, which invokes
 * a font dialog (TDEFontDialog).
 *
 * The lineedit provides a preview of the selected font. The preview text can
 * be customized. You can also have the font dialog show only the fixed fonts.
 *
 * \image html tdefontrequester.png "KDE Font Requester"
 *
 * @author Nadeem Hasan <nhasan@kde.org>
 *
 */
class TDEUI_EXPORT TDEFontRequester : public TQWidget
{
  TQ_OBJECT

  TQ_PROPERTY( TQString title READ title WRITE setTitle )
  TQ_PROPERTY( TQString sampleText READ sampleText WRITE setSampleText )
  TQ_PROPERTY( TQFont font READ font WRITE setFont )

  public:

    /**
     * Constructs a font requester widget.
     *
     * @param parent The parent widget.
     * @param name The widget name.
     * @param onlyFixed Only display fonts which have fixed-width character
     *        sizes.
     */
    TDEFontRequester( TQWidget *parent=0L, const char *name=0L,
        bool onlyFixed=false );

    /**
     * @return The currently selected font in the requester.
     */
    TQFont font() const { return m_selFont; }

    /**
     * @return Returns true if only fixed fonts are displayed.
     */
    bool isFixedOnly() const { return m_onlyFixed; }

    /**
     * @return The current text in the sample text input area.
     */
    TQString sampleText() const { return m_sampleText; }

    /**
     * @return The current title of the widget.
     */
    TQString title() const { return m_title; }

    /**
     * @return Pointer to the label used for preview.
     */
    TQLabel *label() const { return m_sampleLabel; }

    /**
     * @return Pointer to the pushbutton in the widget.
     */
    TQPushButton *button() const { return m_button; }

    /**
     * Sets the currently selected font in the requester.
     *
     * @param font The font to select.
     * @param onlyFixed Display only fixed-width fonts in the font dialog
     * if @p true, or vice-versa.
     */
    virtual void setFont( const TQFont &font, bool onlyFixed=false );

    /**
     * Sets the sample text.
     *
     * Normally you should not change this
     * text, but it can be better to do this if the default text is
     * too large for the edit area when using the default font of your
     * application. Default text is current font name and size. Setting
     * the text to TQString::null will restore the default.
     *
     * @param text The new sample text. The current will be removed.
     */
    virtual void setSampleText( const TQString &text );

    /**
     * Set the title for the widget that will be used in the tooltip and
     * what's this text.
     *
     * @param title The title to be set.
     */
    virtual void setTitle( const TQString & title );

  signals:
    /**
     * Emitted when a new @p font has been selected in the underlying dialog
     */
    void fontSelected( const TQFont &font );

  protected:

    void displaySampleText();
    void setToolTip();

  protected slots:

    virtual void buttonClicked();

  protected:

    bool m_onlyFixed;
    TQString m_sampleText, m_title;
    TQLabel *m_sampleLabel;
    TQPushButton *m_button;
    TQFont m_selFont;

  private:

    class TDEFontRequesterPrivate;
    TDEFontRequesterPrivate *d;
};

#endif // TDEFONTREQUESTER_H
