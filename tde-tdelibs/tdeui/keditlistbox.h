/* This file is part of the KDE libraries
    Copyright (C) 2000 David Faure <faure@kde.org>, Alexander Neundorf <neundorf@kde.org>

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

#ifndef KEDITLISTBOX_H
#define KEDITLISTBOX_H

#include <tqgroupbox.h>
#include <tqlistbox.h>

#include <tdelibs_export.h>

class KLineEdit;
class KComboBox;
class TQPushButton;

class KEditListBoxPrivate;
/**
 * An editable listbox
 *
 * This class provides an editable listbox ;-), this means
 * a listbox which is accompanied by a line edit to enter new
 * items into the listbox and pushbuttons to add and remove
 * items from the listbox and two buttons to move items up and down.
 *
 * \image html keditlistbox.png "KDE Edit List Box Widget"
 *
 */
class TDEUI_EXPORT KEditListBox : public TQGroupBox
{
   TQ_OBJECT
   

   TQ_SETS( Button )
   TQ_PROPERTY( Button buttons READ buttonsProp WRITE setButtonsProp )
   TQ_PROPERTY( TQStringList items READ items WRITE setItems )

public:
   class CustomEditor;

   public:

      /**
       * Enumeration of the buttons, the listbox offers. Specify them in the
       * constructor in the buttons parameter, or in setButtons.
       */
      enum Button { Add = 1, Remove = 2, UpDown = 4 };
      enum { All = Add|Remove|UpDown }; // separated so that it doesn't appear in Qt designer

      /**
       * Create an editable listbox.
       *
       * If @p checkAtEntering is true, after every character you type
       * in the line edit KEditListBox will enable or disable
       * the Add-button, depending whether the current content of the
       * line edit is already in the listbox. Maybe this can become a
       * performance hit with large lists on slow machines.
       * If @p checkAtEntering is false,
       * it will be checked if you press the Add-button. It is not
       * possible to enter items twice into the listbox.
       */
      KEditListBox(TQWidget *parent = 0, const char *name = 0,
		   bool checkAtEntering=false, int buttons = All );
      /**
       * Create an editable listbox.
       *
       * The same as the other constructor, additionally it takes
       * @p title, which will be the title of the frame around the listbox.
       */
      KEditListBox(const TQString& title, TQWidget *parent = 0,
		   const char *name = 0, bool checkAtEntering=false,
		   int buttons = All );

      /**
       * Another constructor, which allows to use a custom editing widget
       * instead of the standard KLineEdit widget. E.g. you can use a
       * KURLRequester or a KComboBox as input widget. The custom
       * editor must consist of a lineedit and optionally another widget that
       * is used as representation. A KComboBox or a KURLRequester have a
       * KLineEdit as child-widget for example, so the KComboBox is used as
       * the representation widget.
       *
       * @see KURLRequester::customEditor()
       * @since 3.1
       */
      KEditListBox( const TQString& title,
                    const CustomEditor &customEditor,
                    TQWidget *parent = 0, const char *name = 0,
                    bool checkAtEntering = false, int buttons = All );

      virtual ~KEditListBox();

      /**
       * Return a pointer to the embedded TQListBox.
       */
      TQListBox* listBox() const     { return m_listBox; }
      /**
       * Return a pointer to the embedded TQLineEdit.
       */
      KLineEdit* lineEdit() const     { return m_lineEdit; }
      /**
       * Return a pointer to the Add button
       */
      TQPushButton* addButton() const     { return servNewButton; }
      /**
       * Return a pointer to the Remove button
       */
      TQPushButton* removeButton() const     { return servRemoveButton; }
      /**
       * Return a pointer to the Up button
       */
      TQPushButton* upButton() const     { return servUpButton; }
      /**
       * Return a pointer to the Down button
       */
      TQPushButton* downButton() const     { return servDownButton; }

      /**
       * See TQListBox::count()
       */
      int count() const   { return int(m_listBox->count()); }
      /**
       * See TQListBox::insertStringList()
       */
      void insertStringList(const TQStringList& list, int index=-1);
      /**
       * See TQListBox::insertStringList()
       */
      void insertStrList(const TQStrList* list, int index=-1);
      /**
       * See TQListBox::insertStrList()
       */
      void insertStrList(const TQStrList& list, int index=-1);
      /**
       * See TQListBox::insertStrList()
       */
      void insertStrList(const char ** list, int numStrings=-1, int index=-1);
      /**
       * See TQListBox::insertItem()
       */
      void insertItem(const TQString& text, int index=-1) {m_listBox->insertItem(text,index);}
      /**
       * Clears both the listbox and the line edit.
       */
      void clear();
      /**
       * See TQListBox::text()
       */
      TQString text(int index) const { return m_listBox->text(index); }
      /**
       * See TQListBox::currentItem()
       */
      int currentItem() const;
      /**
       * See TQListBox::currentText()
       */
      TQString currentText() const  { return m_listBox->currentText(); }

      /**
       * @returns a stringlist of all items in the listbox
       */
      TQStringList items() const;

      /**
       * Clears the listbox and sets the contents to @p items
       *
       * @since 3.4
       */
      void setItems(const TQStringList& items);

      /**
       * Returns which buttons are visible
       */
      int buttons() const;
      inline Button buttonsProp() const { return (Button)buttons(); }

      /**
       * Specifies which buttons should be visible
       */
      void setButtons( uint buttons );
      inline void setButtonsProp( Button buttons ) { setButtons((uint)buttons); }

   signals:
      void changed();

      /**
       * This signal is emitted when the user adds a new string to the list.
       * @param text is the added string.
       * @since 3.2
       * @see added( int item, const TQString & text )
       */
      void added( const TQString & text );

      /**
       * This signal is emitted when the user adds a new string to the list.
       * @param item is the added item's position in the list.
       * @param text is the added string.
       * @since R14.1.0
       * @see added( const TQString & text )
       */
      void added( int item, const TQString & text );

      /**
       * This signal is emitted when the user removes a string from the list.
       * @param text is the removed string.
       * @since 3.2
       * @see removed( int item, const TQString & text )
       */
      void removed( const TQString & text );

      /**
       * This signal is emitted when the user removes a string from the list.
       * @param item is the removed item's position in the list.
       * @param text is the removed string.
       * @since R14.1.0
       * @see removed( const TQString & text )
       */
      void removed( int item, const TQString & text );

      /**
       * This signal is emitted when the user renames a list item.
       * @param from is the original item's text.
       * @param to is the new text of the item.
       * @since R14.1.0
       * @see renamed( int item, const TQString &from, const TQString &to )
       */
      void renamed( const TQString &from, const TQString &to );

      /**
       * This signal is emitted when the user renames a list item.
       * @param item is the renamed item's position in the list.
       * @param from is the original item's text.
       * @param to is the new text of the item.
       * @since R14.1.0
       * @see renamed( const TQString &from, const TQString &to )
       */
      void renamed( int item, const TQString &from, const TQString &to );

   protected slots:
      //the names should be self-explaining
      void moveItemUp();
      void moveItemDown();
      void addItem();
      void removeItem();
      void enableMoveButtons(int index);
      void typedSomething(const TQString& text);

   private:
      TQListBox *m_listBox;
      TQPushButton *servUpButton, *servDownButton;
      TQPushButton *servNewButton, *servRemoveButton;
      KLineEdit *m_lineEdit;

      //this is called in both ctors, to avoid code duplication
      void init( bool checkAtEntering, int buttons,
                 TQWidget *representationWidget = 0L );

   protected:
      virtual void virtual_hook( int id, void* data );
   private:
      //our lovely private d-pointer
      KEditListBoxPrivate* const d;

    /**
     * Custom editor class
     *
     * @since 3.1
     **/
    // ### KDE4: add virtual destructor
    public:
    class CustomEditor
    {
    public:
        TDEUI_EXPORT CustomEditor()
            : m_representationWidget( 0L ),
              m_lineEdit( 0L ) {}
        TDEUI_EXPORT CustomEditor( TQWidget *repWidget, KLineEdit *edit )
            : m_representationWidget( repWidget ),
              m_lineEdit( edit ) {}
        TDEUI_EXPORT CustomEditor( KComboBox *combo );

        TDEUI_EXPORT void setRepresentationWidget( TQWidget *repWidget ) {
            m_representationWidget = repWidget;
        }
        TDEUI_EXPORT void setLineEdit( KLineEdit *edit ) {
            m_lineEdit = edit;
        }

        TDEUI_EXPORT virtual TQWidget   *representationWidget() const {
            return m_representationWidget;
        }
        TDEUI_EXPORT  virtual KLineEdit *lineEdit() const {
            return m_lineEdit;
        }

    protected:
        TQWidget *m_representationWidget;
        KLineEdit *m_lineEdit;
    };
};

#endif

