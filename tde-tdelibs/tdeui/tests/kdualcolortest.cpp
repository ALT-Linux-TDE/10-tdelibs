#include "kdualcolortest.h"
#include <kdualcolorbutton.h>
#include <tdeapplication.h>
#include <tdelocale.h>
#include <tqlayout.h>
#include <tqpalette.h>

KDualColorWidget::KDualColorWidget(TQWidget *parent, const char *name)
    : TQWidget(parent, name)
{
    lbl = new TQLabel("Testing, testing, 1, 2, 3...", this);
    KDualColorButton *colorBtn =
        new KDualColorButton(lbl->colorGroup().text(),
                             lbl->colorGroup().background(), this);
    connect(colorBtn, TQ_SIGNAL(fgChanged(const TQColor &)),
            TQ_SLOT(slotFgChanged(const TQColor &)));
    connect(colorBtn, TQ_SIGNAL(bgChanged(const TQColor &)),
            TQ_SLOT(slotBgChanged(const TQColor &)));
    connect(colorBtn, TQ_SIGNAL(currentChanged(KDualColorButton::DualColor)),
            TQ_SLOT(slotCurrentChanged(KDualColorButton::DualColor)));
    
    TQHBoxLayout *layout = new TQHBoxLayout(this, 5);
    layout->addWidget(colorBtn, 0);
    layout->addWidget(lbl, 1);
    layout->activate();
    resize(sizeHint());
}

void KDualColorWidget::slotFgChanged(const TQColor &c)
{
    TQPalette p = lbl->palette();
    p.setColor(TQColorGroup::Text, c);
    lbl->setPalette(p);
}

void KDualColorWidget::slotBgChanged(const TQColor &c)
{
    TQPalette p = lbl->palette();
    TQBrush b(c, SolidPattern);
    p.setBrush(TQColorGroup::Background, b);
    setPalette(p);
}

void KDualColorWidget::slotCurrentChanged(KDualColorButton::DualColor current)
{
    if(current == KDualColorButton::Foreground)
        tqDebug("Foreground Button Selected.");
    else
        tqDebug("Background Button Selected.");
}

int main(int argc, char **argv)
{
    TDEApplication *app = new TDEApplication(argc, argv, "KDualColorTest");
    KDualColorWidget w;
    app->setMainWidget(&w);
    w.show();
    return(app->exec());
}

#include "kdualcolortest.moc"


