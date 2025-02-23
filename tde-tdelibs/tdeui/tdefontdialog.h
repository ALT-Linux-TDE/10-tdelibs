/*
    $Id$

    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 1997 Bernd Johannes Wuebben <wuebben@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999 Mario Weilguni <mweilguni@kde.org>

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
#ifndef _TDE_FONT_DIALOG_H_
#define _TDE_FONT_DIALOG_H_

#include <tqlineedit.h>
#include <tqbutton.h>
#include <kdialogbase.h>

class TQComboBox;
class TQCheckBox;
class TQFont;
class TQGroupBox;
class TQLabel;
class TQStringList;
class TDEListBox;
class KIntNumInput;
/**
 * @short A font selection widget.
 *
 * While TDEFontChooser as an ordinary widget can be embedded in
 * custom dialogs and therefore is very flexible, in most cases
 * it is preferable to use the convenience functions in
 * TDEFontDialog.
 *
 * \image html tdefontchooser.png "KDE Font Chooser"
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 * @version $Id$
 */
class TDEUI_EXPORT TDEFontChooser : public TQWidget
{
  TQ_OBJECT
  TQ_PROPERTY( TQFont font READ font WRITE setFont )

public:
  /**
   *  @li @p FamilyList - Identifies the family (leftmost) list.
   *  @li @p StyleList -  Identifies the style (center) list.
   *  @li @p SizeList -   Identifies the size (rightmost) list.
   */
  enum FontColumn { FamilyList=0x01, StyleList=0x02, SizeList=0x04};

  /**
   *  @li @p FontDiffFamily - Identifies a requested change in the font family.
   *  @li @p FontDiffStyle -  Identifies a requested change in the font style.
   *  @li @p FontDiffSize -   Identifies a requested change in the font size.
   */
  enum FontDiff { FontDiffFamily=0x01, FontDiffStyle=0x02, FontDiffSize=0x04 };

  /**
   * Constructs a font picker widget.
   * It normally comes up with all font families present on the system; the
   * getFont method below does allow some more fine-tuning of the selection of fonts
   * that will be displayed in the dialog.
   * <p>Consider the following code snippet;
   * \code
   *    TQStringList list;
   *    TDEFontChooser::getFontList(list,SmoothScalableFonts);
   *    TDEFontChooser chooseFont = new TDEFontChooser(0, "FontList", false, list);
   * \endcode
   * <p>
   * The above creates a font chooser dialog with only SmoothScaleble fonts.
   *
   * @param parent The parent widget.
   * @param name The widget name.
   * @param onlyFixed Only display fonts which have fixed-width
   *        character sizes.
   * @param fontList A list of fonts to display, in XLFD format.  If
   *        no list is formatted, the internal KDE font list is used.
   *        If that has not been created, X is queried, and all fonts
   *        available on the system are displayed.
   * @param diff Display the difference version dialog.
   *        See TDEFontDialog::getFontDiff().
   * @param makeFrame Draws a frame with titles around the contents.
   * @param visibleListSize The minimum number of visible entries in the
   *        fontlists.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState, user choice may be retrieved by
   *        calling sizeIsRelative().
   */
  TDEFontChooser(TQWidget *parent = 0L, const char *name = 0L,
	       bool onlyFixed = false,
	       const TQStringList &fontList = TQStringList(),
	       bool makeFrame = true, int visibleListSize=8,
               bool diff = false, TQButton::ToggleState *sizeIsRelativeState = 0L );

  /**
   * Destructs the font chooser.
   */
  virtual ~TDEFontChooser();

  /**
   * Enables or disable a font column in the chooser.
   *
   * Use this
   * function if your application does not need or supports all font
   * properties.
   *
   * @param column Specify the columns. An or'ed combination of
   *        @p FamilyList, @p StyleList and @p SizeList is possible.
   * @param state If @p false the columns are disabled.
   */
  void enableColumn( int column, bool state );

  /**
   * Sets the currently selected font in the chooser.
   *
   * @param font The font to select.
   * @param onlyFixed Readjust the font list to display only fixed
   *        width fonts if @p true, or vice-versa.
   */
  void setFont( const TQFont &font, bool onlyFixed = false );

  /**
   * @return The bitmask corresponding to the attributes the user
   *         wishes to change.
   */
  int fontDiffFlags();

  /**
   * @return The currently selected font in the chooser.
   */
  TQFont font() const { return selFont; }

  /**
   * Sets the color to use in the preview.
   */
  void setColor( const TQColor & col );

  /**
   * @return The color currently used in the preview (default: the text
   *         color of the active color group)
   */
  TQColor color() const;

  /**
   * Sets the background color to use in the preview.
   */
  void setBackgroundColor( const TQColor & col );

  /**
   * @return The background color currently used in the preview (default:
   *         the base color of the active colorgroup)
   */
  TQColor backgroundColor() const;

  /**
   * Sets the state of the checkbox indicating whether the font size
   * is to be interpreted as relative size.
   * NOTE: If parameter sizeIsRelative was not set in the constructor
   *       of the widget this setting will be ignored.
   */
  void setSizeIsRelative( TQButton::ToggleState relative );

  /**
   * @return Whether the font size is to be interpreted as relative size
   *         (default: TQButton:Off)
   */
  TQButton::ToggleState sizeIsRelative() const;


  /**
   * @return The current text in the sample text input area.
   */
  TQString sampleText() const { return sampleEdit->text(); }

  /**
   * Sets the sample text.
   *
   * Normally you should not change this
   * text, but it can be better to do this if the default text is
   * too large for the edit area when using the default font of your
   * application.
   *
   * @param text The new sample text. The current will be removed.
   */
  void setSampleText( const TQString &text )
  {
    sampleEdit->setText( text );
  }

  /**
   * Shows or hides the sample text box.
   *
   * @param visible Set it to true to show the box, to false to hide it.
   * @since 3.5
   */
  void setSampleBoxVisible( bool visible )
  {
    sampleEdit->setShown( visible );
  }

  /**
   * Converts a TQFont into the corresponding X Logical Font
   * Description (XLFD).
   *
   * @param theFont The font to convert.
   * @return A string representing the given font in XLFD format.
   */
  static TQString getXLFD( const TQFont &theFont )
    { return theFont.rawName(); }

  /**
   * The selection criteria for the font families shown in the dialog.
   *  @li @p FixedWidthFont when included only fixed-width fonts are returned.
   *        The fonts where the width of every character is equal.
   *  @li @p ScalableFont when included only scalable fonts are returned;
   *        certain configurations allow bitmap fonts to remain unscaled and
   *        thus these fonts have limited number of sizes.
   *  @li @p SmoothScalableFont when included only return smooth scalable fonts.
   *        this will return only non-bitmap fonts which are scalable to any size requested.
   *        Setting this option to true will mean the "scalable" flag is irrelavant.
   */
  enum FontListCriteria { FixedWidthFonts=0x01, ScalableFonts=0x02, SmoothScalableFonts=0x04 };

  /**
   * Creates a list of font strings.
   *
   * @param list The list is returned here.
   * @param fontListCriteria should contain all the restrictions for font selection as OR-ed values
   *        @see TDEFontChooser::FontListCriteria for the individual values
   */
  static void getFontList( TQStringList &list, uint fontListCriteria);

  /**
   *
   * Fill the font family list of the TDEFontChooser
   *
   * @param list The font list.
   */
    void setFamilyList( TQStringList list );

  /**
   * Reimplemented for internal reasons.
   */
  virtual TQSize sizeHint( void ) const;

signals:
  /**
   * Emitted whenever the selected font changes.
   */
  void fontSelected( const TQFont &font );

private slots:
  void toggled_checkbox();
  void family_chosen_slot(const TQString&);
  void size_chosen_slot(const TQString&);
  void style_chosen_slot(const TQString&);
  void displaySample(const TQFont &font);
  void showXLFDArea(bool);
  void size_value_slot(int);
private:
  void fillFamilyListBox(bool onlyFixedFonts = false);
  void fillSizeList();
  TQString style_name(const TQString &name);
  // This one must be static since getFontList( TQStringList, char*) is so
  static void addFont( TQStringList &list, const char *xfont );

  void setupDisplay();

  // pointer to an optinally supplied list of fonts to
  // inserted into the fontdialog font-family combo-box
  TQStringList  fontList;

  KIntNumInput *sizeOfFont;

  TQLineEdit    *sampleEdit;
  TQLineEdit    *xlfdEdit;

  TQLabel       *familyLabel;
  TQLabel       *styleLabel;
  TQCheckBox    *familyCheckbox;
  TQCheckBox    *styleCheckbox;
  TQCheckBox    *sizeCheckbox;
  TQLabel       *sizeLabel;
  TDEListBox     *familyListBox;
  TDEListBox     *styleListBox;
  TDEListBox     *sizeListBox;
  TQComboBox    *charsetsCombo; // BIC: remove in KDE4
  TQCheckBox    *sizeIsRelativeCheckBox;

  TQFont        selFont;

  TQString      selectedFamily;
  TQString      selectedStyle;
  int           selectedSize;
  TQMap<TQString, TQString> currentStyles;

  bool usingFixed;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class TDEFontChooserPrivate;
  TDEFontChooserPrivate *d;
};

/**
 * @short A font selection dialog.
 *
 * The TDEFontDialog provides a dialog for interactive font selection.
 * It is basically a thin wrapper around the TDEFontChooser widget,
 * which can also be used standalone. In most cases, the simplest
 * use of this class is the static method TDEFontDialog::getFont(),
 * which pops up the dialog, allows the user to select a font, and
 * returns when the dialog is closed.
 *
 * Example:
 *
 * \code
 *      TQFont myFont;
 *      int result = TDEFontDialog::getFont( myFont );
 *      if ( result == TDEFontDialog::Accepted )
 *            ...
 * \endcode
 *
 * \image html tdefontdialog.png "KDE Font Dialog"
 *
 * @author Preston Brown <pbrown@kde.org>, Bernd Wuebben <wuebben@kde.org>
 * @version $Id$
 */
class TDEUI_EXPORT TDEFontDialog : public KDialogBase  {
    TQ_OBJECT

public:
  /**
   * Constructs a font selection dialog.
   *
   * @param parent The parent widget of the dialog, if any.
   * @param name The name of the dialog.
   * @param modal Specifies whether the dialog is modal or not.
   * @param onlyFixed only display fonts which have fixed-width
   *        character sizes.
   * @param fontlist a list of fonts to display, in XLFD format.  If
   *        no list is formatted, the internal KDE font list is used.
   *        If that has not been created, X is queried, and all fonts
   *        available on the system are displayed.
   * @param makeFrame Draws a frame with titles around the contents.
   * @param diff Display the difference version dialog. See getFontDiff().
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState, user choice may be retrieved by
   *        calling sizeIsRelative().
   *
   */
  TDEFontDialog( TQWidget *parent = 0L, const char *name = 0,
	       bool onlyFixed = false, bool modal = false,
	       const TQStringList &fontlist = TQStringList(),
	       bool makeFrame = true, bool diff = false,
               TQButton::ToggleState *sizeIsRelativeState = 0L );

  /**
   * Sets the currently selected font in the dialog.
   *
   * @param font The font to select.
   * @param onlyFixed readjust the font list to display only fixed
   *        width fonts if true, or vice-versa
   */
  void setFont( const TQFont &font, bool onlyFixed = false )
    { chooser->setFont(font, onlyFixed); }

  /**
   * @return The currently selected font in the dialog.
   */
  TQFont font() const { return chooser->font(); }

  /**
   * Sets the state of the checkbox indicating whether the font size
   * is to be interpreted as relative size.
   * NOTE: If parameter sizeIsRelative was not set in the constructor
   *       of the dialog this setting will be ignored.
   */
  void setSizeIsRelative( TQButton::ToggleState relative )
    { chooser->setSizeIsRelative( relative ); }

  /**
   * @return Whether the font size is to be interpreted as relative size
   *         (default: false)
   */
  TQButton::ToggleState sizeIsRelative() const
    { return chooser->sizeIsRelative(); }

  /**
   * Creates a modal font dialog, lets the user choose a font,
   * and returns when the dialog is closed.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   *
   * @return TQDialog::result().
   */
  static int getFont( TQFont &theFont, bool onlyFixed = false,
		      TQWidget *parent = 0L, bool makeFrame = true,
                      TQButton::ToggleState *sizeIsRelativeState = 0L );

  /**
   * Creates a modal font difference dialog, lets the user choose a selection
   * of changes that should be made to a set of fonts, and returns when the
   * dialog is closed. Useful for choosing slight adjustments to the font set
   * when the user would otherwise have to manually edit a number of fonts.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param diffFlags a reference to the int into which the chosen
   *        difference selection bitmask should be written.
   *        Check the returned bitmask like:
   *        \code
   *        if ( diffFlags & TDEFontChooser::FontDiffFamily )
   *            [...]
   *        if ( diffFlags & TDEFontChooser::FontDiffStyle )
   *            [...]
   *        if ( diffFlags & TDEFontChooser::FontDiffSize )
   *            [...]
   *        \endcode
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   *
   * @returns TQDialog::result().
   */
  static int getFontDiff( TQFont &theFont, int &diffFlags, bool onlyFixed = false,
		      TQWidget *parent = 0L, bool makeFrame = true,
                      TQButton::ToggleState *sizeIsRelativeState = 0L );

  /**
   * When you are not only interested in the font selected, but also
   * in the example string typed in, you can call this method.
   *
   * @param theFont a reference to the font to write the chosen font
   *        into.
   * @param theString a reference to the example text that was typed.
   * @param onlyFixed if true, only select from fixed-width fonts.
   * @param parent Parent widget of the dialog. Specifying a widget different
   *        from 0 (Null) improves centering (looks better).
   * @param makeFrame Draws a frame with titles around the contents.
   * @param sizeIsRelativeState If not zero the widget will show a
   *        checkbox where the user may choose whether the font size
   *        is to be interpreted as relative size.
   *        Initial state of this checkbox will be set according to
   *        *sizeIsRelativeState and user choice will be returned
   *        therein.
   * @return The result of the dialog.
   */
  static int getFontAndText( TQFont &theFont, TQString &theString,
			     bool onlyFixed = false, TQWidget *parent = 0L,
			     bool makeFrame = true,
                             TQButton::ToggleState *sizeIsRelativeState = 0L );

signals:
  /**
   * Emitted whenever the currently selected font changes.
   * Connect to this to monitor the font as it is selected if you are
   * not running modal.
   */
  void fontSelected( const TQFont &font );

protected:
  TDEFontChooser *chooser;

protected:
  virtual void virtual_hook( int id, void* data );
private:
  class TDEFontDialogPrivate;
  TDEFontDialogPrivate *d;

};

#endif
