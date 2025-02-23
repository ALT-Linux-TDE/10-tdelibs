
#ifndef __notepad_h__
#define __notepad_h__

#include <tdeparts/genericfactory.h>
#include <tdeparts/part.h>

class TDEAboutData;
class TQMultiLineEdit;

/**
 * Who said writing a part should be complex ? :-)
 * Here is a very simple kedit-like part
 * @internal
 */
class NotepadPart : public KParts::ReadWritePart
{
  TQ_OBJECT
public:
  NotepadPart( TQWidget*, const char* widgetName,
               TQObject* parent, const char* name,
               const TQStringList& args = TQStringList() );
  virtual ~NotepadPart();

  virtual void setReadWrite( bool rw );

  static TDEAboutData* createAboutData();

protected:
  virtual bool openFile();
  virtual bool saveFile();

protected slots:
  void slotSearchReplace();

protected:
  TQMultiLineEdit * m_edit;
};

typedef KParts::GenericFactory<NotepadPart> NotepadFactory;

#endif
