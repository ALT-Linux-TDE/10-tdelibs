
#ifndef __normalktm_h__
#define __normalktm_h__

#include <tdeparts/part.h>
#include <tdemainwindow.h>

class TDEAction;
class TQWidget;

class Shell : public TDEMainWindow
{
  TQ_OBJECT
public:
  Shell();
  virtual ~Shell();

protected slots:
  void slotFileOpen();
  void slotFileOpenRemote();
  void slotFileEdit();
  void slotFileCloseEditor();

protected:
  void embedEditor();

private:

  TDEAction * m_paEditFile;
  TDEAction * m_paCloseEditor;

  KParts::ReadOnlyPart *m_part1;
  KParts::Part *m_part2;
  KParts::ReadWritePart *m_editorpart;
  TQWidget *m_splitter;
};

#endif
