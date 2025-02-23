#include <tqlayout.h>

#include "kdatewidget.h"
#include "kdebug.h"
#include "kdialog.h"
#include "ktimewidget.h"

#include "kdatetimewidget.h"

class KDateTimeWidget::KDateTimeWidgetPrivate
{
public:
  KDateWidget * dateWidget;
  KTimeWidget * timeWidget;
};

KDateTimeWidget::KDateTimeWidget(TQWidget * parent, const char * name)
  : TQWidget(parent, name)
{
  init();
}

KDateTimeWidget::KDateTimeWidget(const TQDateTime & datetime,
			     TQWidget * parent, const char * name)
  : TQWidget(parent, name)
{
  init();

  setDateTime(datetime);
}

KDateTimeWidget::~KDateTimeWidget()
{
  delete d;
}

void KDateTimeWidget::init()
{
  d = new KDateTimeWidgetPrivate;

  TQHBoxLayout *layout = new TQHBoxLayout(this, 0, KDialog::spacingHint());
  layout->setAutoAdd(true);

  d->dateWidget = new KDateWidget(this);
  d->timeWidget = new KTimeWidget(this);

  connect(d->dateWidget, TQ_SIGNAL(changed(TQDate)),
          TQ_SLOT(slotValueChanged()));
  connect(d->timeWidget, TQ_SIGNAL(valueChanged(const TQTime &)),
          TQ_SLOT(slotValueChanged()));
}

void KDateTimeWidget::setDateTime(const TQDateTime & datetime)
{
  d->dateWidget->setDate(datetime.date());
  d->timeWidget->setTime(datetime.time());
}

TQDateTime KDateTimeWidget::dateTime() const
{
  return TQDateTime(d->dateWidget->date(), d->timeWidget->time());
}

void KDateTimeWidget::slotValueChanged()
{
  TQDateTime datetime(d->dateWidget->date(),
                     d->timeWidget->time());

  kdDebug() << "slotValueChanged(): " << datetime << "\n";

  emit valueChanged(datetime);
}

#include "kdatetimewidget.moc"
