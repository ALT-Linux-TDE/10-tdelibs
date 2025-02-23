
#ifndef __parts_h__
#define __parts_h__

#include <tdeparts/part.h>

class TQMultiLineEdit;
namespace KParts {
class GUIActivateEvent;
};

class Part1 : public KParts::ReadOnlyPart
{
  TQ_OBJECT
public:
  Part1( TQObject *parent, TQWidget * parentWidget );
  virtual ~Part1();

protected:
  virtual bool openFile();

protected:
  TQMultiLineEdit * m_edit;
  TDEInstance *m_instance;
};

class Part2 : public KParts::Part
{
  TQ_OBJECT
public:
  Part2( TQObject *parent, TQWidget * parentWidget );
  virtual ~Part2();

protected:
  // This is not mandatory - only if you care about setting the
  // part caption when the part is used in a multi-part environment
  // (i.e. in a part manager)
  // There is a default impl for ReadOnlyPart...
  virtual void guiActivateEvent( KParts::GUIActivateEvent * );
  TDEInstance *m_instance;
};

#endif
