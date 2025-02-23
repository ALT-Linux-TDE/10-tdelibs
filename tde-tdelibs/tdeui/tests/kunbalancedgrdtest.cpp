#include "kunbalancedgrdtest.h"
#include <tdeapplication.h>
#include <kpixmapeffect.h>
#include <tqpainter.h>
#include <tqstring.h>
#include <tqlayout.h>

int cols = 3, rows = 3; // how many


KGradientWidget::KGradientWidget(TQWidget *parent, const char *name)
  :TQWidget(parent, name)
{
  time.start();
  setMinimumSize(250 * cols, 250 * rows);
}

void KGradientWidget::paintEvent(TQPaintEvent *)
{
    int it, ft;
    TQString say;

    TQColor ca = TQt::black, cb = TQt::cyan;

    int x = 0, y = 0;

    pix.resize(width()/cols, height()/rows);
    TQPainter p(this);
    p.setPen(TQt::white);

    // draw once, so that the benchmarking be fair :-)
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::VerticalGradient,
				      xbalance, ybalance);

    // vertical
    time.start();
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::VerticalGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Vertical";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5 + (x++)*width()/cols, 15+y*height()/rows, say); // augment x

    // horizontal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::HorizontalGradient,
				      xbalance, ybalance);
    ft = time.elapsed() ;
    say.setNum( ft - it); say = "Horizontal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // elliptic
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::EllipticGradient,
				      xbalance, ybalance);
    ft = time.elapsed() ;
    say.setNum( ft - it); say = "Elliptic";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // diagonal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::DiagonalGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Diagonal";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // crossdiagonal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix,ca, cb,
				      KPixmapEffect::CrossDiagonalGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "CrossDiagonal";
    p.drawPixmap(width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    y++; // next row
    x = 0; // reset the columns

    // pyramidal
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::PyramidGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Pyramid";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/cols, 15+y*height()/rows, say);

    // rectangular
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::RectangleGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "Rectangle";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);

    // crosspipe
    it = time.elapsed();
    KPixmapEffect::unbalancedGradient(pix, ca, cb,
				      KPixmapEffect::PipeCrossGradient,
				      xbalance, ybalance);
    ft = time.elapsed();
    say.setNum( ft - it); say = "PipeCross";
    p.drawPixmap(x*width()/cols, y*height()/rows, pix);
    p.drawText(5+(x++)*width()/rows, 15+y*height()/rows, say);
}

myTopWidget::myTopWidget (TQWidget *parent, const char *name)
  :TQWidget(parent, name)
{
  TQGridLayout *lay = new TQGridLayout (this, 2, 3, 0);

  grds = new KGradientWidget(this);
  lay->addMultiCellWidget(grds, 0, 0 ,0, 2);

  bLabel = new TQLabel("Balance: X = 000; Y = 000", this);
  lay->addWidget(bLabel, 1, 0);

  xSlider = new TQSlider ( -200, 200, 1, 100, TQSlider::Horizontal, this);
  lay->addWidget(xSlider, 1, 1);

  ySlider = new TQSlider ( -200, 200, 1, 100, TQSlider::Horizontal, this);
  lay->addWidget(ySlider, 1, 2);

  connect(xSlider, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(rebalance()));
  connect(ySlider, TQ_SIGNAL(valueChanged(int)), this, TQ_SLOT(rebalance()));

  rebalance();

  itime = otime = 0;
  time.start();
}

void myTopWidget::rebalance()
{
  otime = time.elapsed();

  TQString val; val.sprintf("Balance: X = %3d; Y = %3d",
			   xSlider->value(), ySlider->value());

  bLabel->setText(val);
  grds->setBalance(xSlider->value(), ySlider->value());

  if ((otime - itime )> 500)
    {
      grds->repaint(false);
      itime = time.elapsed();
    }
}

int main(int argc, char **argv)
{
    TDEApplication *app = new TDEApplication(argc, argv, "KUnbalancedGradientTest");
    myTopWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

#include "kunbalancedgrdtest.moc"
