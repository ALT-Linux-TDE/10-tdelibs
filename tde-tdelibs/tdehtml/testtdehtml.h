#ifndef TESTTDEHTML_H
#define TESTTDEHTML_H

#include <kdebug.h>
#include <tqvaluelist.h>
#include <tqdatetime.h>

/**
 * @internal
 */
class Dummy : public TQObject
{
  TQ_OBJECT
public:
  Dummy( TDEHTMLPart *part ) : TQObject( part ) { m_part = part; };

private slots:
  void slotOpenURL( const KURL &url, const KParts::URLArgs &args )
  {
    m_part->browserExtension()->setURLArgs( args );
    m_part->openURL( url );
  }
  void reload()
  {
      KParts::URLArgs args; args.reload = true;
      m_part->browserExtension()->setURLArgs( args );
      m_part->openURL( m_part->url() );
  }
  
  void toggleNavigable(bool s)
  {
      m_part->setCaretMode(s);
  }

  void toggleEditable(bool s)
  {
  kdDebug() << "editable: " << s << endl;
      m_part->setEditable(s);
  }

  void doBenchmark();

  void handleDone();

private:
  TDEHTMLPart *m_part;
  TQValueList<TQString> filesToBenchmark;
  TQMap<TQString, TQValueList<int> > results;
  int                 benchmarkRun;
  TQTime               loadTimer;

  void nextRun();
};

#endif
