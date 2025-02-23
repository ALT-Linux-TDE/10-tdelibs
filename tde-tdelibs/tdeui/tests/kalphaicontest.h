#include <kdebug.h>
#include <tqwidget.h>
#include <tqimage.h>
#include <tqpixmap.h>
#include <kimageeffect.h>
#include <kalphapainter.h>

class KAlphaTest : public TQWidget
{
  public:
    KAlphaTest();
  protected:
    void paintEvent(TQPaintEvent *);
  private:
    TQImage m_image;
    TQPixmap m_pixmap;
    TQImage m_bg;

  public:
    static bool m_useDblBuffer;
    static bool m_usePixmap;
    static bool m_correctOverlapping;
};

