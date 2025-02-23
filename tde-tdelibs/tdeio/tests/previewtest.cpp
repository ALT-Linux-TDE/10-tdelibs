
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqpushbutton.h>

#include <tdeapplication.h>
#include <kdebug.h>
#include <klineedit.h>

#include <tdeio/previewjob.h>

#include "previewtest.moc"

PreviewTest::PreviewTest()
    :TQWidget()
{
    TQGridLayout *layout = new TQGridLayout(this, 2, 2);
    m_url = new KLineEdit(this);
    m_url->setText("/home/malte/gore_bush.jpg");
    layout->addWidget(m_url, 0, 0);
    TQPushButton *btn = new TQPushButton("Generate", this);
    connect(btn, TQ_SIGNAL(clicked()), TQ_SLOT(slotGenerate()));
    layout->addWidget(btn, 0, 1);
    m_preview = new TQLabel(this);
    m_preview->setMinimumSize(400, 300);
    layout->addMultiCellWidget(m_preview, 1, 1, 0, 1);
}

void PreviewTest::slotGenerate()
{
    KURL::List urls;
    urls.append(m_url->text());
    TDEIO::PreviewJob *job = TDEIO::filePreview(urls, m_preview->width(), m_preview->height(), true, 48);
    connect(job, TQ_SIGNAL(result(TDEIO::Job*)), TQ_SLOT(slotResult(TDEIO::Job*)));
    connect(job, TQ_SIGNAL(gotPreview(const KFileItem *, const TQPixmap &)), TQ_SLOT(slotPreview(const KFileItem *, const TQPixmap &)));
    connect(job, TQ_SIGNAL(failed(const KFileItem *)), TQ_SLOT(slotFailed()));
}

void PreviewTest::slotResult(TDEIO::Job*)
{
    kdDebug() << "PreviewTest::slotResult(...)" << endl;
}

void PreviewTest::slotPreview(const KFileItem *, const TQPixmap &pix)
{
    kdDebug() << "PreviewTest::slotPreview()" << endl;
    m_preview->setPixmap(pix);
}

void PreviewTest::slotFailed()
{
    kdDebug() << "PreviewTest::slotFailed()" << endl;
    m_preview->setText("failed");
}

int main(int argc, char **argv)
{
    TDEApplication app(argc, argv, "previewtest", true, true);
    PreviewTest *w = new PreviewTest;
    w->show();
    app.setMainWidget(w);
    return app.exec();
}

