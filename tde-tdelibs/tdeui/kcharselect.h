/* This file is part of the KDE libraries

   Copyright (C) 1999 Reginald Stadlbauer <reggie@kde.org>

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

#ifndef kcharselect_h
#define kcharselect_h

#include <tqgridview.h>
#include <tqvbox.h>
#include <tqcombobox.h>
#include <tqspinbox.h>
#include <tqstring.h>
#include <tqpoint.h>
#include <tqstringlist.h>

#include <tdelibs_export.h>

class TQFont;
class TQFontDatabase;
class TQMouseEvent;
class TQSpinBox;
class KCharSelectTablePrivate;
class KCharSelectPrivate;

/**
 * @short Character selection table
 *
 * A table widget which displays the characters of a font. Internally
 * used by KCharSelect. See the KCharSelect documentation for further
 * details.
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 */

class TDEUI_EXPORT KCharSelectTable : public TQGridView
{
    TQ_OBJECT

public:
    KCharSelectTable( TQWidget *parent, const char *name, const TQString &_font,
		      const TQChar &_chr, int _tableNum );

    virtual TQSize sizeHint() const;
    virtual void resizeEvent( TQResizeEvent * );

    virtual void setFont( const TQString &_font );
    virtual void setChar( const TQChar &_chr );
    virtual void setTableNum( int _tableNum );

    virtual TQChar chr() { return vChr; }

protected:
    virtual void paintCell( class TQPainter *p, int row, int col );

    virtual void mousePressEvent( TQMouseEvent *e ) {  mouseMoveEvent( e ); }
    virtual void mouseDoubleClickEvent ( TQMouseEvent *e ){  mouseMoveEvent( e ); emit doubleClicked();}
    virtual void mouseReleaseEvent( TQMouseEvent *e ) { mouseMoveEvent( e ); emit activated( chr() ); emit activated(); }
    virtual void mouseMoveEvent( TQMouseEvent *e );

    virtual void keyPressEvent( TQKeyEvent *e );

    void gotoLeft();
    void gotoRight();
    void gotoUp();
    void gotoDown();

    TQString vFont;
    TQChar vChr;
    int vTableNum;
    TQPoint vPos;
    TQChar focusItem;
    TQPoint focusPos;
    int temp;

signals:
    void highlighted( const TQChar &c );
    void highlighted();
    void activated( const TQChar &c );
    void activated();
    void focusItemChanged();
    void focusItemChanged( const TQChar &c );
    void tableUp();
    void tableDown();
    void doubleClicked();

private:
    virtual void setFont(const TQFont &f) { TQGridView::setFont(f); }
    void setToolTips();
protected:
    virtual void virtual_hook( int id, void* data );
private:
    KCharSelectTablePrivate* const d;
};

/**
 * @short Character selection widget
 *
 * This widget allows the user to select a character of a
 * specified font in a table
 *
 * \image html kcharselect.png "Character Selection Widget"
 *
 * You can specify the font whose characters should be displayed via
 * setFont() or in the constructor. Using enableFontCombo() you can allow the
 * user to choose the font from a combob-box. As only 256 characters
 * are displayed at once in the table, using the spinbox on the top
 * the user can choose starting from which character the table
 * displays them. This spinbox also can be enabled or disabled using
 * enableTableSpinBox().
 *
 * KCharSelect supports keyboard and mouse navigation. Click+Move
 * always selects the character below the mouse cursor. Using the
 * arrow keys moves the focus mark around and pressing RETURN
 * or SPACE selects the cell which contains the focus mark.
 *
 * To get the current selected character, use the chr()
 * method. You can set the character which should be displayed with
 * setChar() and the table number which should be displayed with
 * setTableNum().
 *
 * @author Reginald Stadlbauer <reggie@kde.org>
 */

class TDEUI_EXPORT KCharSelect : public TQVBox
{
    TQ_OBJECT
    TQ_PROPERTY( TQString fontFamily READ font WRITE setFont )
    TQ_PROPERTY( int tableNum READ tableNum WRITE setTableNum )
    TQ_PROPERTY( bool fontComboEnabled READ isFontComboEnabled WRITE enableFontCombo )
    TQ_PROPERTY( bool tableSpinBoxEnabled READ isTableSpinBoxEnabled WRITE enableTableSpinBox )

public:
    /**
     * Constructor. @p font specifies which font should be displayed, @p
     * chr which character should be selected and @p tableNum specifies
     * the number of the table which should be displayed.
     */
    KCharSelect( TQWidget *parent, const char *name,
		 const TQString &font = TQString::null, const TQChar &chr = ' ', int tableNum = 0 );
    ~KCharSelect();
    /**
     * Reimplemented.
     */
    virtual TQSize sizeHint() const;

    /**
     * Sets the font which is displayed to @p font
     */
    virtual void setFont( const TQString &font );

    /**
     * Sets the currently selected character to @p chr.
     */
    virtual void setChar( const TQChar &chr );

    /**
     * Sets the currently displayed table to @p tableNum.
     */
    virtual void setTableNum( int tableNum );

    /**
     * Returns the currently selected character.
     */
    virtual TQChar chr() const { return charTable->chr(); }

    /**
     * Returns the currently displayed font.
     */
    virtual TQString font() const { return fontCombo->currentText(); }

    /**
     * Returns the currently displayed table
     */
    virtual int tableNum() const { return tableSpinBox->value(); }

    /**
     * If @p e is set to true, the combobox which allows the user to
     * select the font which should be displayed is enabled, else
     * disabled.
     */
    virtual void enableFontCombo( bool e ) { fontCombo->setEnabled( e ); }

    /**

     * If @p e is set to true, the spinbox which allows the user to
     * specify which characters of the font should be displayed, is
     * enabled, else disabled.
     */
    virtual void enableTableSpinBox( bool e ) { tableSpinBox->setEnabled( e ); }

    /**
     * Returns wether the font combobox on the top is enabled or
     * disabled.
     *
     * @see enableFontCombo()
     */
    virtual bool isFontComboEnabled() const { return fontCombo->isEnabled(); }

    /**
     * Returns wether the table spinbox on the top is enabled or
     * disabled.
     *
     * @see enableTableSpinBox()
     */
    virtual bool isTableSpinBoxEnabled() const { return tableSpinBox->isEnabled(); }

protected:
    virtual void fillFontCombo();
    static void cleanupFontDatabase();

    TQComboBox *fontCombo;
    TQSpinBox *tableSpinBox;
    KCharSelectTable *charTable;
    TQStringList fontList;
    static TQFontDatabase * fontDataBase;

protected slots:
    void fontSelected( const TQString &_font );
    void tableChanged( int _value );
    void charHighlighted( const TQChar &c ) { emit highlighted( c ); }
    void charHighlighted() { emit highlighted(); }
    void charActivated( const TQChar &c ) { emit activated( c ); }
    void charActivated() { emit activated(); }
    void charFocusItemChanged() { emit focusItemChanged(); }
    void charFocusItemChanged( const TQChar &c ) { emit focusItemChanged( c ); }
    void charTableUp() { if ( tableNum() < 255 ) setTableNum( tableNum() + 1 ); }
    void charTableDown() { if ( tableNum() > 0 ) setTableNum( tableNum() - 1 ); }
    void slotDoubleClicked() { emit doubleClicked(); }
    void slotUnicodeEntered();
    void slotUpdateUnicode( const TQChar &c );
signals:
    void highlighted( const TQChar &c );
    void highlighted();
    void activated( const TQChar &c );
    void activated();
    void fontChanged( const TQString &_font );
    void focusItemChanged();
    void focusItemChanged( const TQChar &c );
    void doubleClicked();

private:
    virtual void setFont(const TQFont &f) { TQVBox::setFont(f); }
protected:
    virtual void virtual_hook( int id, void* data );
private:
    class KCharSelectPrivate;
    KCharSelectPrivate* const d;
};

#endif
