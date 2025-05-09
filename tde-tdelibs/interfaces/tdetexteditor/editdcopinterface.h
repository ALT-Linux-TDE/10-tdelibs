#ifndef EDIT_DCOP_INTERFACE_H
#define EDIT_DCOP_INTERFACE_H

#include <dcopobject.h>
#include <dcopref.h>
#include <tqstringlist.h>
#include <tqcstring.h>
//#include "editdcopinterface.moc"
namespace KTextEditor
{
  class EditInterface;
  /**
  This is the main interface to the EditInterface of KTextEdit.
  This will provide a consistant dcop interface to all KDE applications that use it.
  @short DCOP interface to EditInterface.
  @author Ian Reinhart Geiser <geiseri@kde.org>
  */
  class KTEXTEDITOR_EXPORT EditDCOPInterface : virtual public DCOPObject
  {
  K_DCOP

  public:
    /**
    Construct a new interface object for the text editor.
    @param Parent the parent EditInterface object
    that will provide us with the functions for the interface.
    @param name the TQObject's name
    */
    EditDCOPInterface( EditInterface *Parent, const char *name );
    /**
    Destructor
    Cleans up the object.
    **/
    virtual ~EditDCOPInterface();
  k_dcop:
    /**
    * @return the complete document as a single TQString
    */
    virtual TQString text ();

    /**
    * @return All the text from the requested line.
    */
    virtual TQString textLine ( uint line );

    /**
    * @return The current number of lines in the document
    */
    virtual int numLines ();

    /**
    * @return the number of characters in the document
    */
    virtual int length ();

    /**
    * Set the given text into the view.
    * Warning: This will overwrite any data currently held in this view.
    */
    virtual void setText (const TQString &text );

    /**
    *  Inserts text at line "line", column "col"
    *  returns true if success
    */
    virtual bool insertText ( uint line, uint col, const TQString &text );

    /**
    *  remove text at line "line", column "col"
    *  returns true if success
    */
    virtual bool removeText ( uint startLine, uint startCol, uint endLine, uint endCol) ;

    /**
    * Insert line(s) at the given line number.
    */
    virtual bool insertLine ( uint line, const TQString &text );

    /**
    * Insert line(s) at the given line number.
    * If only one line is in the current document, removeLine will fail (return false)
    */
    virtual bool removeLine ( uint line );
  private:
    EditInterface *m_parent;
  };
}
#endif


