#include "kdesattest.h"
#include <tdeapplication.h>
#include <kimageeffect.h>
#include <tqpainter.h>
#include <tqdatetime.h>
#include <tqstring.h>

int cols = 3, rows = 3; // how many

KDesatWidget::KDesatWidget(TQWidget *parent, const char *name)
  :TQWidget(parent, name)
{

    image = TQImage("testimage.png");
    slide = new KDoubleNumInput(700, this, "desat");
    slide->setRange(0, 1, 0.001);
    slide->setLabel("Desaturate: ", AlignVCenter | AlignLeft);
    connect(slide,TQ_SIGNAL(valueChanged(double)), this, TQ_SLOT(change(double)));

    resize(image.width()*2, image.height() + slide->height());
    slide->setGeometry(0, image.height(), image.width()*2, slide->height());
}

void KDesatWidget::change(double) {
    desat_value = slide->value();
    repaint(false);
}

void KDesatWidget::paintEvent(TQPaintEvent */*ev*/)
{
    TQTime time;
    int it, ft;
    TQString say;

    TQPainter p(this);
    p.setPen(TQt::black);

    // original image
    time.start();
    it = time.elapsed();
    image = TQImage("testimage.png");
    p.drawImage(0, 0, image);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawText(5 , 15, say);

    // desaturated image
    it = time.elapsed();
    image = KImageEffect::desaturate(image, desat_value);
    p.drawImage(image.width(), 0, image);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawText(15+image.width() , 15, say);
}

int main(int argc, char **argv)
{
    TDEApplication *app = new TDEApplication(argc, argv, "KDesatTest");
    KDesatWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

#include "kdesattest.moc"
