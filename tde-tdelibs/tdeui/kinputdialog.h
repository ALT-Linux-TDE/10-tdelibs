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

#ifndef KINPUTDIALOG_H
#define KINPUTDIALOG_H

// #include <tqt.h>

#include <kdialogbase.h>

class TQValidator;

class KLineEdit;
class KIntSpinBox;
class KDoubleSpinBox;
class KComboBox;
class KTextEdit;
class KInputDialogPrivate;

/**
 * The KInputDialog class provides a simple dialog to get a single value
 * from the user. The value can be a string, a number (either an integer or
 * a float) or an item from a list. This class is designed to be source
 * compatible with QInputDialog.
 *
 * Five static convenience functions are provided: getText(), getInteger().
 * getDouble(), getItem() and getItemList().
 *
 * @since 3.2
 * @author Nadeem Hasan <nhasan@kde.org>
 */
class TDEUI_EXPORT KInputDialog : public KDialogBase
{
  TQ_OBJECT

  private:

    /**
     * Constructor. This class is not designed to be instantiated except
     * from the static member functions.
     */
    KInputDialog( const TQString &caption, const TQString &label,
      const TQString &value, TQWidget *parent, const char *name,
      TQValidator *validator, const TQString &mask );
    KInputDialog( const TQString &caption, const TQString &label,
      const TQString &value, TQWidget *parent, const char *name );
    KInputDialog( const TQString &caption, const TQString &label, int value,
      int minValue, int maxValue, int step, int base, TQWidget *parent,
      const char *name );
    KInputDialog( const TQString &caption, const TQString &label, double value,
      double minValue, double maxValue, double step, int decimals,
      TQWidget *parent, const char *name );
    KInputDialog( const TQString &caption, const TQString &label,
      const TQStringList &list, int current, bool editable, TQWidget *parent,
      const char *name );
    KInputDialog( const TQString &caption, const TQString &label,
      const TQStringList &list, const TQStringList &select, bool editable,
      TQWidget *parent, const char *name );

    ~KInputDialog();

    KLineEdit *lineEdit() const;
    KIntSpinBox *intSpinBox() const;
    KDoubleSpinBox *doubleSpinBox() const;
    KComboBox *comboBox() const;
    TDEListBox *listBox() const;
    KTextEdit *textEdit() const;

  private slots:

    void slotEditTextChanged( const TQString& );
    void slotUpdateButtons( const TQString& );

  public:

    /**
     * Static convenience function to get a string from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as a label for the line edit. value is the initial
     * value of the line edit. ok will be set to true if user pressed Ok
     * and false if user pressed Cancel.
     *
     * If you provide a validator, the Ok button is disabled as long as
     * the validator doesn't return Acceptable. If there is no validator,
     * the Ok button is enabled whenever the line edit isn't empty. If you
     * want to accept empty input, create a trivial TQValidator that
     * always returns acceptable, e.g. TQRegExpValidator with a regexp
     * of ".*".
     *
     * @param caption   Caption of the dialog
     * @param label     Text of the label for the line edit
     * @param value     Initial value of the line edit
     * @param ok        This bool would be set to true if user pressed Ok
     * @param parent    Parent of the dialog widget
     * @param name      Name of the dialog widget
     * @param validator A @ref TQValidator to be associated with the line edit
     * @param mask      Mask associated with the line edit. See the
     *                  documentation for @ref TQLineEdit about masks.
     *
     * @return String user entered if Ok was pressed, else a null string
     */
    static TQString getText( const TQString &caption, const TQString &label,
        const TQString &value=TQString::null, bool *ok=0, TQWidget *parent=0,
        const char *name=0, TQValidator *validator=0,
        const TQString &mask=TQString::null );

    /** 
     * Same as @ref getText except it provides an extra parameter to specify 
     * a TQWhatsThis text for the input widget.
     *
     * ### KDE4: Merge with getText.
     *
     * @since KDE 3.3
     **/
    static TQString text( const TQString &caption, const TQString &label, 
        const TQString &value=TQString::null, bool *ok=0, TQWidget *parent=0, 
        const char *name=0, TQValidator *validator=0,
        const TQString &mask=TQString::null,
        const TQString& whatsThis=TQString::null );

    /**
     * Static convenience function to get a multiline string from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as a label for the line edit. value is the initial
     * value of the line edit. ok will be set to true if user pressed Ok
     * and false if user pressed Cancel.
     *
     * @param caption   Caption of the dialog
     * @param label     Text of the label for the line edit
     * @param value     Initial value of the line edit
     * @param ok        This bool would be set to true if user pressed Ok
     * @param parent    Parent of the dialog widget
     * @param name      Name of the dialog widget
     *
     * @return String user entered if Ok was pressed, else a null string
     * @since 3.3
     */
    static TQString getMultiLineText( const TQString &caption,
        const TQString &label, const TQString &value=TQString::null,
        bool *ok=0, TQWidget *parent=0, const char *name=0 );

    /**
     * Static convenience function to get an integer from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as the label for the spin box. value is the initial
     * value for the spin box. minValue and maxValue are the minimum and
     * maximum allowable values the user may choose. step is the amount by
     * which the value will change as the user presses the increment and
     * decrement buttons of the spin box. Base is the base of the number.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the spin box
     * @param value    Initial value of the spin box
     * @param minValue Minimum value user can input
     * @param maxValue Maximum value user can input
     * @param step     Amount by which value is incremented or decremented
     * @param base     Base of the number
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     * @param name     Name of the dialog widget
     *
     * @return Number user entered if Ok was pressed, else 0
     */

    static int getInteger( const TQString &caption, const TQString &label,
        int value=0, int minValue=-2147483647, int maxValue=2147483647,
        int step=1, int base=10, bool *ok=0, TQWidget *parent=0,
        const char *name=0 );

    /**
     * This is an overloaded convenience function. It behaves exactly same as
     * above except it assumes base to be 10, i.e. accepts decimal numbers.
     */
    static int getInteger( const TQString &caption, const TQString &label,
        int value=0, int minValue=-2147483647, int maxValue=2147483647,
        int step=1, bool *ok=0, TQWidget *parent=0, const char *name=0 );

    /**
     * Static convenience function to get a floating point number from the user.
     *
     * caption is the text that is displayed in the title bar. label is the
     * text that appears as the label for the spin box. value is the initial
     * value for the spin box. minValue and maxValue are the minimum and
     * maximum allowable values the user may choose. step is the amount by
     * which the value will change as the user presses the increment and
     * decrement buttons of the spin box.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the spin box
     * @param value    Initial value of the spin box
     * @param minValue Minimum value user can input
     * @param maxValue Maximum value user can input
     * @param step     Amount by which value is incremented or decremented
     * @param decimals Number of digits after the decimal point
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     * @param name     Name of the dialog widget
     *
     * @return Number user entered if Ok was pressed, else 0
     */
    static double getDouble( const TQString &caption, const TQString &label,
        double value=0, double minValue=-2147483647, 
        double maxValue=2147483647, double step=0.1, int decimals=1,
        bool *ok=0, TQWidget *parent=0, const char *name=0 );

    /**
     * This is an overloaded convenience function. It behaves exctly like
     * the above function.
     */
    static double getDouble( const TQString &caption, const TQString &label,
        double value=0, double minValue=-2147483647, 
        double maxValue=2147483647, int decimals=1, bool *ok=0,
        TQWidget *parent=0, const char *name=0 );

    /**
     * Static convenience function to let the user select an item from a
     * list. caption is the text that is displayed in the title bar.
     * label is the text that appears as the label for the list. list
     * is the string list which is inserted into the list, and current
     * is the number of the item which should be the selected item. If 
     * editable is true, the user can enter their own text.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the spin box
     * @param list     List of item for user to choose from
     * @param current  Index of the selected item
     * @param editable If true, user can enter own text
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     * @param name     Name of the dialog widget
     *
     * @return Text of the selected item. If @p editable is true this can be
     *         a text entered by the user.
     */
    static TQString getItem( const TQString &caption, const TQString &label,
        const TQStringList &list, int current=0, bool editable=false,
        bool *ok=0, TQWidget *parent=0, const char *name=0 );

    /**
     * Static convenience function to let the user select one or more
     * items from a listbox. caption is the text that is displayed in the
     * title bar. label is the text that appears as the label for the listbox.
     * list is the string list which is inserted into the listbox, select
     * is the list of item(s) that should be the selected. If multiple is 
     * true, the user can select multiple items.
     *
     * @param caption  Caption of the dialog
     * @param label    Text of the label for the spin box
     * @param list     List of item for user to choose from
     * @param select   List of item(s) that should be selected
     * @param multiple If true, user can select multiple items
     * @param ok       This bool would be set to true if user pressed Ok
     * @param parent   Parent of the dialog widget
     * @param name     Name of the dialog widget
     *
     * @return List of selected items if multiple is true, else currently
     *         selected item as a QStringList
     */
    static TQStringList getItemList( const TQString &caption,
        const TQString &label, const TQStringList &list=TQStringList(),
        const TQStringList &select=TQStringList(), bool multiple=false,
        bool *ok=0, TQWidget *parent=0, const char *name=0 );

  private:

    KInputDialogPrivate* const d;
    friend class KInputDialogPrivate;
};

#endif // KINPUTDIALOG_H
