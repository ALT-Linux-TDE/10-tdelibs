#include "kblendtest.h"
#include <tdeapplication.h>
#include <kimageeffect.h>
#include <tqpainter.h>
#include <tqdatetime.h>
#include <tqstring.h>

int cols = 3, rows = 3; // how many

KBlendWidget::KBlendWidget(TQWidget *parent, const char *name)
  :TQWidget(parent, name)
{

    bgnd = tqRgb(255, 255, 255);

    image = TQImage("testimage.png");

    resize(image.width()*2+60, image.height()*3+80);
    setBackgroundColor(bgnd);
}

void KBlendWidget::paintEvent(TQPaintEvent */*ev*/)
{
    TQTime time;
    int it, ft;
    TQString say;

    image = TQImage("testimage.png");

    TQPainter p(this);
    p.setPen(TQt::black);

    // you see here use of anti_dir param (blend from down to up, here)
    time.start();
    it = time.elapsed();
    KImageEffect::blend(image, 0.2, bgnd, KImageEffect::VerticalGradient,true);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Vertical";
    p.drawImage(20, 20, image);
    p.drawText(5 , 15, say);

    image = TQImage("testimage.png");

    // here a negative initial intensity is used (1/2 of image is unaffected)
    it = time.elapsed();
    KImageEffect::blend(image, -0.5, bgnd, KImageEffect::HorizontalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Horizontal";
    p.drawImage(40+image.width(), 20, image);
    p.drawText(15+image.width() , 15, say);

    image = TQImage("testimage.png");

    it = time.elapsed();
    KImageEffect::blend(image, 0., bgnd, KImageEffect::DiagonalGradient,true);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Diagonal";
    p.drawImage(20, 40+image.height(), image);
    p.drawText(5 , 35+image.height(), say);

    image = TQImage("testimage.png");

    it = time.elapsed();
    KImageEffect::blend(image, 0.1, bgnd, KImageEffect::CrossDiagonalGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, CrossDiagonal";
    p.drawImage(40+image.width(), 40+image.height(), image);
    p.drawText(25+image.width() , 35 + image.height(), say);

    image = TQImage("testimage.png");

    it = time.elapsed();
    KImageEffect::blend(image, -0.6, bgnd, KImageEffect::RectangleGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Rectangle";
    p.drawImage(20, 60+2*image.height(), image);
    p.drawText(5 , 55+2*image.height(), say);

    image = TQImage("testimage.png");

    it = time.elapsed();
    KImageEffect::blend(image, 0.2, bgnd, KImageEffect::EllipticGradient);
    ft = time.elapsed();
    say.setNum( ft - it); say += " ms, Elliptic";
    p.drawImage(40+image.width(), 60+2*image.height(), image);
    p.drawText(25+image.width(), 55+2*image.height(), say);
    p.end();
}

int main(int argc, char **argv)
{
    TDEApplication app(argc, argv, "KBlendTest");
    KBlendWidget w;
    app.setMainWidget(&w);
    w.show();
    return(app.exec());
}

