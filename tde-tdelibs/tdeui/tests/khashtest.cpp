#include "khashtest.h"
#include <tdeapplication.h>
#include <kpixmapeffect.h>
#include <kimageeffect.h>
#include <tqpainter.h>
#include <tqdatetime.h>
#include <tqstring.h>
#include <tqimage.h>

int cols = 3, rows = 3; // how many

void KHashWidget::paintEvent(TQPaintEvent * /*ev*/)
{
    TQTime time;
    int it, ft;
    TQString say;

    TQColor cb = TQColor(0,70,70), ca = TQColor(80,200,200);

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
    KPixmapEffect::hash(pix,KPixmapEffect::NorthLite);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::HorizontalGradient);
    KPixmapEffect::hash(pix,KPixmapEffect::SouthLite);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // elliptic
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::EllipticGradient);
    KPixmapEffect::hash(pix,KPixmapEffect::NorthLite, 1);
    ft = time.elapsed() ;
    say.setNum( ft - it); say += " ms, Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // diagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::DiagonalGradient);
    KPixmapEffect::hash(pix,KPixmapEffect::EastLite);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KPixmapEffect::gradient(pix,ca, cb, KPixmapEffect::CrossDiagonalGradient);
    KPixmapEffect::hash(pix,KPixmapEffect::EastLite, 2);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, CrossDiagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);

    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);


    TQImage image = TQImage("testimage.png");
    it = time.elapsed();
    KImageEffect::hash(image, KImageEffect::WestLite, 2);
    ft = time.elapsed();
    pix.resize(image.width(), image.height());
    pix.convertFromImage(image);
    pix.resize(width()/cols, height()/rows);
    say.setNum( ft - it); say += " ms, CrossDiagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.setPen(TQt::blue);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);
    p.setPen(TQt::white);


    y++; // next row
    x = 0; // reset the columns

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PyramidGradient);
    KPixmapEffect::hash(pix,KPixmapEffect::WestLite);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // rectangular
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::RectangleGradient);
    KPixmapEffect::hash(pix,KPixmapEffect::NWLite);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KPixmapEffect::gradient(pix, ca, cb, KPixmapEffect::PipeCrossGradient);
    KPixmapEffect::hash(pix,KPixmapEffect::WestLite, 3);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

}

int main(int argc, char **argv)
{

    TDEApplication *app = new TDEApplication(argc, argv, "KHashTest");
    KHashWidget w;
    w.resize(250 * cols, 250 * rows);
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

#include <khashtest.moc>
