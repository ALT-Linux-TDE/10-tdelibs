
#include "kcolortest.h"
#include <tdeapplication.h>
#include <kimageeffect.h>
#include <stdio.h>
#include <tqdatetime.h>

bool fullscreen = false, oldway = false, intvsfade = false;
int max = 20; // how many steps

KColorWidget::KColorWidget(TQWidget *parent, const char *name)
    : TQWidget(parent, name)
{

  if (fullscreen || intvsfade) {
    TQPixmap shot = TQPixmap::grabWindow(TQApplication::desktop()->winId());
    original = shot.convertToImage();
  }
  else
    original = TQImage("testimage.png");
  resize(original.width(), original.height());
}

void KColorWidget::paintEvent(TQPaintEvent *)
{
  if(!pixmap.isNull())
    bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
	   TQt::CopyROP, true);
}

void KColorWidget::doIntensityLoop()
{
    int count;

    int start, stop;
    TQTime t;

    t.start();

    image = original; image.detach();

    if (fullscreen){
      start = t.elapsed();
      for(count=0; count < max; ++count){
	if (!oldway)
	  KImageEffect::intensity(image, -1./max);
	else {
	  uint *qptr=(uint *)image.bits();
	  TQRgb qrgb;
	  int size=pixmap.width()*pixmap.height();
	  for (int i=0;i<size; i++, qptr++)
	    {
	      qrgb=*(TQRgb *)qptr;
	      *qptr=tqRgb((int)(tqRed(qrgb)*1./max),
			 (int)(tqGreen(qrgb)*1./max),
			 (int)(tqBlue(qrgb)*1./max));
	    }
	}
	pixmap.convertFromImage(image);
	bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
	       TQt::CopyROP, true);
      }
      stop = t.elapsed();
      tqDebug ("Total fullscreen %s dim time for %d steps : %f s",
	       oldway?"(antonio)":"(intensity)", count, (stop - start)*1e-3);

      if (intvsfade) {
	image = original; image.detach();
	start = t.elapsed();
	for(count=0; count < max; ++count){
	  KImageEffect::fade(image, 1./max, black);
	  pixmap.convertFromImage(image);
	  bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
		 TQt::CopyROP, true);
	}
      }
      stop = t.elapsed();
      tqDebug ("Total fullscreen (fade) dim time for %d steps : %f s",
	       count, (stop - start)*1e-3);

    }

    else {
      image = original; image.detach();
      tqDebug("Intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::intensity(image, 1./max);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }

      for(count=0; count < max; ++count){
        KImageEffect::intensity(image, -1./max);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }

      image = original; image.detach();
      tqDebug("Red channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Red);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Red);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }

      image = original; image.detach();
      tqDebug("Green channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Green);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Green);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }

      image = original; image.detach();
      tqDebug("Blue channel intensity test");
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, -1./max, KImageEffect::Blue);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }
      for(count=0; count < max; ++count){
        KImageEffect::channelIntensity(image, 1./max, KImageEffect::Blue);
        pixmap.convertFromImage(image);
        bitBlt(this, 0, 0, &pixmap, 0, 0, pixmap.width(), pixmap.height(),
               TQt::CopyROP, true);
      }
    }
}

int main(int argc, char **argv)
{
    if (argc > 1) {
      if (!strcmp(argv[1], "fullscreen"))
	{
	  fullscreen = true;
	  if (!strcmp(argv[2], "old_way"))
	    oldway = true;
	}
      else if (!strcmp(argv[1], "int_vs_fade")) {
	intvsfade = fullscreen = true;
	oldway = false;
      }
      else
	printf("Usage: %s [int_vs_fade | fullscreen [old_way]]\n ", argv[0]);
    }
    TDEApplication *app = new TDEApplication(argc, argv, "KColorTest");
    KColorWidget w;
    app->setMainWidget(&w);
    w.show();
    w.doIntensityLoop();
    return(app->exec());
}
