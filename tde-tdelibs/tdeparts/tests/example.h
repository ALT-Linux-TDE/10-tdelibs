
#ifndef __example_h__
#define __example_h__

#include <tdeparts/partmanager.h>
#include <tdeparts/mainwindow.h>

class TDEAction;
class TQWidget;

class Shell : public KParts::MainWindow
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
  KParts::PartManager *m_manager;
  TQWidget *m_splitter;
};

#endif
