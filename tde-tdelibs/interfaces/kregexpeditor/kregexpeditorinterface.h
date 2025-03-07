#ifndef __kregexpeditorinterface_h__
#define __kregexpeditorinterface_h__

#include <tqstring.h>

/**
 *  A graphical editor for regular expressions.
 * 
 * @author Jesper K. Pedersen blackie@kde.org
 *
 * The actual editor is located in tdeutils, with an interface in
 * tdelibs. This means that it is a bit more complicated to create an
 * instance of the editor, but only a little bit more complicated.
 *
 * To check if kregexpeditor in tdeutils is installed and available use this line:
 * 
 * \code
 * bool installed=!TDETrader::self()->query("KRegExpEditor/KRegExpEditor").isEmpty();
 * \endcode
 *
 * The following is a template for what you need to do to create an instance of the
 * regular expression dialog:
 *
 * \code
 * TQDialog *editorDialog = KParts::ComponentFactory::createInstanceFromQuery<TQDialog>( "KRegExpEditor/KRegExpEditor" );
 * if ( editorDialog ) {
 *   // tdeutils was installed, so the dialog was found fetch the editor interface
 *   KRegExpEditorInterface *editor = ::tqt_cast<KRegExpEditorInterface *>( editorDialog );
 *   Q_ASSERT( editor ); // This should not fail!
 *   
 *   // now use the editor.
 *   editor->setRegExp("^kde$");
 *
 *   // Finally exec the dialog
 *   editorDialog->exec();
 * }
 * else {
 *   // Don't offer the dialog.
 * }
 * \endcode
 *
 * Note: signals and slots must be connected to the editorDialog object, not to the editor object:
 * \code
 * connect( editorDialog, TQ_SIGNAL( canUndo( bool ) ), undoBut, TQ_SLOT( setEnabled( bool ) ) );
 * \endcode
 *
 * If you want to create an instance of the editor widget, i.e. not the
 * dialog, then you must do it in the following way:
 *
 * \code
 * TQWidget *editorWidget =
 * KParts::ComponentFactory::createInstanceFromQuery<TQWidget>( 
 *     "KRegExpEditor/KRegExpEditor", TQString::null, parent );
 * if ( editorWidget ) {
 *   // tdeutils was installed, so the widget was found fetch the editor interface
 *   KRegExpEditorInterface *editor = ::tqt_cast<KRegExpEditorInterface *>( editorWidget );
 *   Q_ASSERT( editor ); // This should not fail!
 *   
 *   // now use the editor.
 *   editor->setRegExp("^kde$");

 *   // Finally insert the widget into the layout of its parent
 *   layout->addWidget( editorWidget );
 * }
 * else {
 *   // Don't offer the editor widget.
 * }
 * \endcode
 *
 */
class KRegExpEditorInterface
{
public:
  /**
   * returns the regular expression of the editor in Qt3 QRegExp
   * syntax. Note, there is also a 'regexp' Qt property available.
   */
  virtual TQString regExp() const = 0;

protected:
// These are signals: in classes that actually implement the interface.

  /**
   * This signal tells whether undo is available.
   */
  virtual void canUndo( bool ) = 0;

  /**
   * This signal tells whether redo is available.
   */
  virtual void canRedo( bool ) = 0;

  /**
   * This signal is emited whenever the regular expression changes.
   * The argument is true when the regular expression is different from
   * the loaded regular expression and false when it is equal to the
   * loaded regular expression.
   */
  virtual void changes( bool ) = 0;

public:
// These are public slots: in classes that implement the interface.

 /**
  * Set the regular expression for the editor. The syntax must be Qt3
  * TQRegExp syntax.
  */
  virtual void setRegExp( const TQString &regexp ) = 0;
  virtual void redo() = 0;
  virtual void undo() = 0;

  /**
   * Set text to use when showing matches. NOT IMPLEMENTED YET!
   *   
   * This method is not yet implemented. In later version of the widget
   * this method will be used to give the widget a text to show matches of
   * the regular expression on.
   */
  virtual void setMatchText( const TQString& ) = 0;
  
  /**
   * This method allows for future changes that will not break binary
   * compatibility. DONT USE!
   *   
   * KDE has a policy of keeping binary compatibility for all major
   * version of KDE. This means that new methods can not be added to this
   * API before KDE version 4.0.
   *
   * This method is an escape door for that.
   *   
   * Conclusion: You should not use this method in this version of KDE!
   */
  virtual void doSomething( TQString method, void* arguments ) = 0;
};

#endif

