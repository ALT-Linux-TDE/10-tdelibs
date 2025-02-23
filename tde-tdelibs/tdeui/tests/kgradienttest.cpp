#include "kgradienttest.h"
#include <tdeapplication.h>
#include <kpixmapeffect.h>
#include <tqpainter.h>
#include <tqdatetime.h>
#include <tqstring.h>

int cols = 3, rows = 3; // how many

void KGradientWidget::paintEvent(TQPaintEvent */*ev*/)
{
    TQTime time;
    int it, ft;
    TQString say;

    TQColor ca = TQt::black, cb = TQt::blue;

    int x = 0, y = 0;

    pix.resize(width()/cols, height()/rows);
    TQPainter p(this);
    p.setPen(TQt::white);

    // draw once, so that the benchmarking be fair :-)
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::VerticalGradient);

    // vertical
    time.start();
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::VerticalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::HorizontalGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // elliptic
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::EllipticGradient);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // diagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::DiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::CrossDiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, CrossDiagonal";
    p.drawPixmap(width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PyramidGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // pattern
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::RectangleGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PipeCrossGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);
}

int main(int argc, char **argv)
{
    TDEApplication *app = new TDEApplication(argc, argv, "KGradientTest");
    KGradientWidget w;
    w.resize(250 * cols, 250 * rows);
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

