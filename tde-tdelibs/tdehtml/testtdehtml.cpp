// program to test the new tdehtml implementation

#include <stdlib.h>
#include "decoder.h"
#include "tdeapplication.h"
#include "html_document.h"
#include "htmltokenizer.h"
// to be able to delete a static protected member pointer in kbrowser...
// just for memory debugging
#define protected public
#include "tdehtml_part.h"
#include "tdehtmlview.h"
#undef protected
#include "testtdehtml.h"
#include "testtdehtml.moc"
#include "misc/loader.h"
#include <tqcursor.h>
#include <dom_string.h>
#include "dom/dom2_range.h"
#include "dom/html_document.h"
#include "dom/dom_exception.h"
#include <stdio.h>
#define protected public
#include "tdehtml_factory.h"
#undef protected
#include "css/cssstyleselector.h"
#include "html/html_imageimpl.h"
#include "rendering/render_style.h"
#include <tdemainwindow.h>
#include <tdecmdlineargs.h>
#include <tdeaction.h>
#include "domtreeview.h"
#include <tdefiledialog.h>

static TDECmdLineOptions options[] = { { "+file", "url to open", 0 } , TDECmdLineLastOption };

int main(int argc, char *argv[])
{

    TDECmdLineArgs::init(argc, argv, "testtdehtml", "Testtdehtml",
            "a basic web browser using the TDEHTML library", "1.0");
    TDECmdLineArgs::addCmdLineOptions(options);

    TDEApplication a;
    TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs( );
    if ( args->count() == 0 ) {
	TDECmdLineArgs::usage();
	::exit( 1 );
    }

#ifndef __TDE_HAVE_GCC_VISIBILITY
    TDEHTMLFactory *fac = new TDEHTMLFactory(true);
#endif

    TDEMainWindow *toplevel = new TDEMainWindow();
    TDEHTMLPart *doc = new TDEHTMLPart( toplevel, 0, toplevel, 0, TDEHTMLPart::BrowserViewGUI );

    Dummy *dummy = new Dummy( doc );
    TQObject::connect( doc->browserExtension(), TQ_SIGNAL( openURLRequest( const KURL &, const KParts::URLArgs & ) ),
		      dummy, TQ_SLOT( slotOpenURL( const KURL&, const KParts::URLArgs & ) ) );

    TQObject::connect( doc, TQ_SIGNAL(completed()), dummy, TQ_SLOT(handleDone()) );

    if (args->url(0).url().right(4).find(".xml", 0, false) == 0) {
        KParts::URLArgs ags(doc->browserExtension()->urlArgs());
        ags.serviceType = "text/xml";
        doc->browserExtension()->setURLArgs(ags);
    }

    doc->openURL( args->url(0) );

//     DOMTreeView * dtv = new DOMTreeView(0, doc, "DomTreeView");
//     dtv->show();

    toplevel->setCentralWidget( doc->widget() );
    toplevel->resize( 800, 600);

//     dtv->resize(toplevel->width()/2, toplevel->height());

    TQDomDocument d = doc->domDocument();
    TQDomElement viewMenu = d.documentElement().firstChild().childNodes().item( 2 ).toElement();
    TQDomElement e = d.createElement( "action" );
    e.setAttribute( "name", "debugRenderTree" );
    viewMenu.appendChild( e );
    e = d.createElement( "action" );
    e.setAttribute( "name", "debugDOMTree" );
    viewMenu.appendChild( e );


    e = d.createElement( "action" );
    e.setAttribute( "name", "debugDoBenchmark" );
    viewMenu.appendChild( e );

    TQDomElement toolBar = d.documentElement().firstChild().nextSibling().toElement();
    e = d.createElement( "action" );
    e.setAttribute( "name", "editable" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "navigable" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "reload" );
    toolBar.insertBefore( e, toolBar.firstChild() );
    e = d.createElement( "action" );
    e.setAttribute( "name", "print" );
    toolBar.insertBefore( e, toolBar.firstChild() );

    (void)new TDEAction( "Reload", "reload", TQt::Key_F5, dummy, TQ_SLOT( reload() ), doc->actionCollection(), "reload" );
    (void)new TDEAction( "Benchmark...", 0, 0, dummy, TQ_SLOT( doBenchmark() ), doc->actionCollection(), "debugDoBenchmark" );
    TDEAction* kprint = new TDEAction( "Print", "print", 0, doc->browserExtension(), TQ_SLOT( print() ), doc->actionCollection(), "print" );
    kprint->setEnabled(true);
    TDEToggleAction *ta = new TDEToggleAction( "Navigable", "edit-clear", 0, doc->actionCollection(), "navigable" );
    ta->setChecked(doc->isCaretMode());
    TQWidget::connect(ta, TQ_SIGNAL(toggled(bool)), dummy, TQ_SLOT( toggleNavigable(bool) ));
    ta = new TDEToggleAction( "Editable", "edit", 0, doc->actionCollection(), "editable" );
    ta->setChecked(doc->isEditable());
    TQWidget::connect(ta, TQ_SIGNAL(toggled(bool)), dummy, TQ_SLOT( toggleEditable(bool) ));
    toplevel->guiFactory()->addClient( doc );

    doc->setJScriptEnabled(true);
    doc->setJavaEnabled(true);
    doc->setPluginsEnabled( true );
    doc->setURLCursor(TQCursor(TQt::PointingHandCursor));
    a.setTopWidget(doc->widget());
    TQWidget::connect(doc, TQ_SIGNAL(setWindowCaption(const TQString &)),
		     doc->widget()->topLevelWidget(), TQ_SLOT(setCaption(const TQString &)));
    doc->widget()->show();
    toplevel->show();
    ((TQScrollView *)doc->widget())->viewport()->show();


    int ret = a.exec();
#ifndef __TDE_HAVE_GCC_VISIBILITY
    fac->deref();
#endif
    return ret;
}

void Dummy::doBenchmark()
{
    TDEConfigGroup settings(TDEGlobal::config(), "bench");
    results.clear();

    TQString directory = KFileDialog::getExistingDirectory(settings.readPathEntry("path"), m_part->view(), 
            TQString::fromLatin1("Please select directory with tests"));

    if (!directory.isEmpty()) {
        settings.writePathEntry("path", directory);

        TQDir dirListing(directory, "*.html");
        for (int i = 0; i < dirListing.count(); ++i) {
            filesToBenchmark.append(dirListing.absFilePath(dirListing[i]));
        }
    }

    benchmarkRun = 0;

    if (!filesToBenchmark.isEmpty())
        nextRun();
}

const int COLD_RUNS = 2;
const int HOT_RUNS  = 5;

void Dummy::nextRun()
{
    if (benchmarkRun == (COLD_RUNS + HOT_RUNS)) {
        filesToBenchmark.remove(filesToBenchmark.begin());
        benchmarkRun = 0;
    }

    if (!filesToBenchmark.isEmpty()) {
        loadTimer.start();
        m_part->openURL(filesToBenchmark[0]);
    } else {
        //Generate HTML for report.
        m_part->begin();
        m_part->write("<table border=1>");

        for (TQMap<TQString, TQValueList<int> >::iterator i = results.begin(); i != results.end(); ++i) {
            m_part->write("<tr><td>" + i.key() + "</td>");
            TQValueList<int> timings = i.data();
            int total = 0;
            for (int pos = 0; pos < timings.size(); ++pos) {
                int t = timings[pos];
                if (pos < COLD_RUNS)
		    m_part->write(TQString::fromLatin1("<td>(Cold):") + TQString::number(t) + "</td>");
                else {
                    total += t;
                    m_part->write(TQString::fromLatin1("<td><i>") + TQString::number(t) + "</i></td>");
                }
            }

            m_part->write(TQString::fromLatin1("<td>Average:<b>") + TQString::number(double(total) / HOT_RUNS) + "</b></td>");

            m_part->write("</tr>");
        }

        m_part->end();
    }
}

void Dummy::handleDone() 
{
    if (filesToBenchmark.isEmpty()) return;

    results[filesToBenchmark[0]].append(loadTimer.elapsed());
    ++benchmarkRun;
    nextRun();
}
