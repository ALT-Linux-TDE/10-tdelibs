#ifndef __example_h__
#define __example_h__

#include <tdeparts/mainwindow.h>

class Shell : public KParts::MainWindow
{
  TQ_OBJECT
public:
  Shell();
  virtual ~Shell();

  void openURL( const KURL & url );

protected slots:
  void slotFileOpen();

private:
  KParts::ReadOnlyPart *m_gvpart;
};

#endif
