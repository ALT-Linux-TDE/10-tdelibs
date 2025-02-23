/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001-2002 Michael Goffioul <tdeprint@swing.be>
 *  Complete rewrite on Sat Jun 15 2002 (c) Anders Lund <anders@alweb.dk>
 *  Copyright (c) 2002, 2003 Anders Lund <anders@alweb.dk>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include "kateprinter.h"

#include <kateconfig.h>
#include <katedocument.h>
#include <katefactory.h>
#include <katehighlight.h>
#include <katelinerange.h>
#include <katerenderer.h>
#include <kateschema.h>
#include <katetextline.h>

#include <tdeapplication.h>
#include <kcolorbutton.h>
#include <kdebug.h>
#include <kdialog.h> // for spacingHint()
#include <tdefontdialog.h>
#include <tdelocale.h>
#include <kprinter.h>
#include <kurl.h>
#include <kuser.h> // for loginName

#include <tqpainter.h>
#include <tqpopupmenu.h>
#include <tqpaintdevicemetrics.h>
#include <tqcheckbox.h>
#include <tqcombobox.h>
#include <tqgroupbox.h>
#include <tqhbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqlineedit.h>
#include <tqspinbox.h>
#include <tqstringlist.h>
#include <tqwhatsthis.h>

//BEGIN KatePrinter
bool KatePrinter::print (KateDocument *doc)
{
#ifndef TQ_WS_WIN //TODO: reenable
  KPrinter printer;

  // docname is now always there, including the right Untitled name
  printer.setDocName(doc->docName());

  KatePrintTextSettings *kpts = new KatePrintTextSettings(&printer, NULL);
  kpts->enableSelection( doc->hasSelection() );
  printer.addDialogPage( kpts );
  printer.addDialogPage( new KatePrintHeaderFooter(&printer, NULL) );
  printer.addDialogPage( new KatePrintLayout(&printer, NULL) );

   if ( printer.setup( kapp->mainWidget(), i18n("Print %1").arg(printer.docName()) ) )
   {
     KateRenderer renderer(doc);
     //renderer.config()->setSchema (1);
     renderer.setPrinterFriendly(true);

     TQPainter paint( &printer );
     TQPaintDeviceMetrics pdm( &printer );
     /*
        We work in tree cycles:
        1) initialize variables and retrieve print settings
        2) prepare data according to those settings
        3) draw to the printer
     */
     uint pdmWidth = pdm.width();
     uint y = 0;
     uint xstart = 0; // beginning point for painting lines
     uint lineCount = 0;
     uint maxWidth = pdmWidth;
     uint headerWidth = pdmWidth;
     int startCol = 0;
     int endCol = 0;
     bool needWrap = true;
     bool pageStarted = true;

     // Text Settings Page
     bool selectionOnly = ( doc->hasSelection() &&
                           ( printer.option("app-kate-printselection") == "true" ) );
     int selStartCol = 0;
     int selEndCol = 0;

     bool useGuide = ( printer.option("app-kate-printguide") == "true" );
     int guideHeight = 0;
     int guideCols = 0;

     bool printLineNumbers = ( printer.option("app-kate-printlinenumbers") == "true" );
     uint lineNumberWidth( 0 );

     // Header/Footer Page
     TQFont headerFont; // used for header/footer
     TQString f = printer.option("app-kate-hffont");
     if (!f.isEmpty())
       headerFont.fromString( f );

     bool useHeader = (printer.option("app-kate-useheader") == "true");
     TQColor headerBgColor(printer.option("app-kate-headerbg"));
     TQColor headerFgColor(printer.option("app-kate-headerfg"));
     uint headerHeight( 0 ); // further init only if needed
     TQStringList headerTagList; // do
     bool headerDrawBg = false; // do

     bool useFooter = (printer.option("app-kate-usefooter") == "true");
     TQColor footerBgColor(printer.option("app-kate-footerbg"));
     TQColor footerFgColor(printer.option("app-kate-footerfg"));
     uint footerHeight( 0 ); // further init only if needed
     TQStringList footerTagList = 0; // do
     bool footerDrawBg = 0; // do

     // Layout Page
     renderer.config()->setSchema( KateFactory::self()->schemaManager()->number(
           printer.option("app-kate-colorscheme") ) );
     bool useBackground = ( printer.option("app-kate-usebackground") == "true" );
     bool useBox = (printer.option("app-kate-usebox") == "true");
     int boxWidth(printer.option("app-kate-boxwidth").toInt());
     TQColor boxColor(printer.option("app-kate-boxcolor"));
     int innerMargin = useBox ? printer.option("app-kate-boxmargin").toInt() : 6;

     // Post initialization
     uint maxHeight = (useBox ? pdm.height()-innerMargin : pdm.height());
     uint currentPage( 1 );
     uint lastline = doc->lastLine(); // nessecary to print selection only
     uint firstline( 0 );

     KateHlItemDataList ilist;

     if (useGuide)
       doc->highlight()->getKateHlItemDataListCopy (renderer.config()->schema(), ilist);

     /*
        Now on for preparations...
        during preparations, variable names starting with a "_" means
        those variables are local to the enclosing block.
     */
     {
       if ( selectionOnly )
       {
         // set a line range from the first selected line to the last
         firstline = doc->selStartLine();
         selStartCol = doc->selStartCol();
         lastline = doc->selEndLine();
         selEndCol = doc->selEndCol();

         lineCount = firstline;
       }

       if ( printLineNumbers )
       {
         // figure out the horiizontal space required
         TQString s( TQString("%1 ").arg( doc->numLines() ) );
         s.fill('5', -1); // some non-fixed fonts haven't equally wide numbers
                          // FIXME calculate which is actually the widest...
         lineNumberWidth = renderer.currentFontMetrics()->width( s );
         // a small space between the line numbers and the text
         int _adj = renderer.currentFontMetrics()->width( "5" );
         // adjust available width and set horizontal start point for data
         maxWidth -= (lineNumberWidth + _adj);
         xstart += lineNumberWidth + _adj;
       }

       if ( useHeader || useFooter )
       {
         // Set up a tag map
         // This retrieves all tags, ued or not, but
         // none of theese operations should be expensive,
         // and searcing each tag in the format strings is avoided.
         TQDateTime dt = TQDateTime::currentDateTime();
         TQMap<TQString,TQString> tags;

         KUser u (KUser::UseRealUserID);
         tags["u"] = u.loginName();

         tags["d"] = TDEGlobal::locale()->formatDateTime(dt, true, false);
         tags["D"] =  TDEGlobal::locale()->formatDateTime(dt, false, false);
         tags["h"] =  TDEGlobal::locale()->formatTime(dt.time(), false);
         tags["y"] =  TDEGlobal::locale()->formatDate(dt.date(), true);
         tags["Y"] =  TDEGlobal::locale()->formatDate(dt.date(), false);
         tags["f"] =  doc->url().fileName();
         tags["U"] =  doc->url().prettyURL();
         if ( selectionOnly )
         {
           TQString s( i18n("(Selection of) ") );
           tags["f"].prepend( s );
           tags["U"].prepend( s );
         }

         TQRegExp reTags( "%([dDfUhuyY])" ); // TODO tjeck for "%%<TAG>"

         if (useHeader)
         {
           headerDrawBg = ( printer.option("app-kate-headerusebg") == "true" );
           headerHeight = TQFontMetrics( headerFont ).height();
           if ( useBox || headerDrawBg )
             headerHeight += innerMargin * 2;
           else
             headerHeight += 1 + TQFontMetrics( headerFont ).leading();

           TQString headerTags = printer.option("app-kate-headerformat");
           int pos = reTags.search( headerTags );
           TQString rep;
           while ( pos > -1 )
           {
             rep = tags[reTags.cap( 1 )];
             headerTags.replace( (uint)pos, 2, rep );
             pos += rep.length();
             pos = reTags.search( headerTags, pos );
           }
           headerTagList = TQStringList::split('|', headerTags, true);

           if (!headerBgColor.isValid())
             headerBgColor = TQt::lightGray;
           if (!headerFgColor.isValid())
             headerFgColor = TQt::black;
         }

         if (useFooter)
         {
           footerDrawBg = ( printer.option("app-kate-footerusebg") == "true" );
           footerHeight = TQFontMetrics( headerFont ).height();
           if ( useBox || footerDrawBg )
             footerHeight += 2*innerMargin;
           else
             footerHeight += 1; // line only

           TQString footerTags = printer.option("app-kate-footerformat");
           int pos = reTags.search( footerTags );
           TQString rep;
           while ( pos > -1 )
           {
             rep = tags[reTags.cap( 1 )];
             footerTags.replace( (uint)pos, 2, rep );
             pos += rep.length();
             pos = reTags.search( footerTags, pos );
           }

           footerTagList = TQStringList::split('|', footerTags, true);
           if (!footerBgColor.isValid())
             footerBgColor = TQt::lightGray;
           if (!footerFgColor.isValid())
             footerFgColor = TQt::black;
           // adjust maxheight, so we can know when/where to print footer
           maxHeight -= footerHeight;
         }
       } // if ( useHeader || useFooter )

       if ( useBackground )
       {
         if ( ! useBox )
         {
           xstart += innerMargin;
           maxWidth -= innerMargin * 2;
         }
       }

       if ( useBox )
       {
         if (!boxColor.isValid())
           boxColor = TQt::black;
         if (boxWidth < 1) // shouldn't be pssible no more!
           boxWidth = 1;
         // set maxwidth to something sensible
         maxWidth -= ( ( boxWidth + innerMargin )  * 2 );
         xstart += boxWidth + innerMargin;
         // maxheight too..
         maxHeight -= boxWidth;
       }
       else
         boxWidth = 0;

       if ( useGuide )
       {
         // calculate the height required
         // the number of columns is a side effect, saved for drawing time
         // first width is needed
         int _w = pdmWidth - innerMargin * 2;
         if ( useBox )
           _w -= boxWidth * 2;
         else
         {
           if ( useBackground )
             _w -= ( innerMargin * 2 );
           _w -= 2; // 1 px line on each side
         }

         // base of height: margins top/bottom, above and below tetle sep line
         guideHeight = ( innerMargin * 4 ) + 1;

         // get a title and add the height required to draw it
         TQString _title = i18n("Typographical Conventions for %1").arg(doc->highlight()->name());
         guideHeight += paint.boundingRect( 0, 0, _w, 1000, TQt::AlignTop|TQt::AlignHCenter, _title ).height();

         // see how many columns we can fit in
         int _widest( 0 );

         TQPtrListIterator<KateHlItemData> it( ilist );
         KateHlItemData *_d;

         int _items ( 0 );
         while ( ( _d = it.current()) != 0 )
         {
           _widest = kMax( _widest, ((TQFontMetrics)(
                                _d->bold() ?
                                  _d->italic() ?
                                    renderer.config()->fontStruct()->myFontMetricsBI :
                                    renderer.config()->fontStruct()->myFontMetricsBold :
                                  _d->italic() ?
                                    renderer.config()->fontStruct()->myFontMetricsItalic :
                                    renderer.config()->fontStruct()->myFontMetrics
                                    ) ).width( _d->name ) );
           _items++;
           ++it;
         }
         guideCols = _w/( _widest + innerMargin );
         // add height for required number of lines needed given columns
         guideHeight += renderer.fontHeight() * ( _items/guideCols );
         if ( _items%guideCols )
           guideHeight += renderer.fontHeight();
       }

       // now that we know the vertical amount of space needed,
       // it is possible to calculate the total number of pages
       // if needed, that is if any header/footer tag contains "%P".
       if ( headerTagList.grep("%P").count() || footerTagList.grep("%P").count() )
       {
         kdDebug(13020)<<"'%P' found! calculating number of pages..."<<endl;
         uint _pages = 0;
         uint _ph = maxHeight;
         if ( useHeader )
           _ph -= ( headerHeight + innerMargin );
         if ( useFooter )
           _ph -= innerMargin;
         int _lpp = _ph / renderer.fontHeight();
         uint _lt = 0, _c=0;

         // add space for guide if required
         if ( useGuide )
           _lt += (guideHeight + (renderer.fontHeight() /2)) / renderer.fontHeight();
         long _lw;
         for ( uint i = firstline; i < lastline; i++ )
         {
           _lw = renderer.textWidth( doc->kateTextLine( i ), -1 );
           while ( _lw >= 0 )
           {
             _c++;
             _lt++;
             if ( (int)_lt  == _lpp )
             {
               _pages++;
               _lt = 0;
             }
             _lw -= maxWidth;
             if ( ! _lw ) _lw--; // skip lines matching exactly!
           }
         }
         if ( _lt ) _pages++; // last page

         // substitute both tag lists
         TQString re("%P");
         TQStringList::Iterator it;
         for ( it=headerTagList.begin(); it!=headerTagList.end(); ++it )
           (*it).replace( re, TQString( "%1" ).arg( _pages ) );
         for ( it=footerTagList.begin(); it!=footerTagList.end(); ++it )
           (*it).replace( re, TQString( "%1" ).arg( _pages ) );
       }
     } // end prepare block

     /*
        On to draw something :-)
     */
     uint _count = 0;
     while (  lineCount <= lastline  )
     {
       startCol = 0;
       endCol = 0;
       needWrap = true;

       while (needWrap)
       {
         if ( y + renderer.fontHeight() >= (uint)(maxHeight) )
         {
           kdDebug(13020)<<"Starting new page, "<<_count<<" lines up to now."<<endl;
           printer.newPage();
           currentPage++;
           pageStarted = true;
           y=0;
         }

         if ( pageStarted )
         {

           if ( useHeader )
           {
             paint.setPen(headerFgColor);
             paint.setFont(headerFont);
             if ( headerDrawBg )
                paint.fillRect(0, 0, headerWidth, headerHeight, headerBgColor);
             if (headerTagList.count() == 3)
             {
               int valign = ( (useBox||headerDrawBg||useBackground) ?
                              TQt::AlignVCenter : TQt::AlignTop );
               int align = valign|TQt::AlignLeft;
               int marg = ( useBox || headerDrawBg ) ? innerMargin : 0;
               if ( useBox ) marg += boxWidth;
               TQString s;
               for (int i=0; i<3; i++)
               {
                 s = headerTagList[i];
                 if (s.find("%p") != -1) s.replace("%p", TQString::number(currentPage));
                 paint.drawText(marg, 0, headerWidth-(marg*2), headerHeight, align, s);
                 align = valign|(i == 0 ? TQt::AlignHCenter : TQt::AlignRight);
               }
             }
             if ( ! ( headerDrawBg || useBox || useBackground ) ) // draw a 1 px (!?) line to separate header from contents
             {
               paint.drawLine( 0, headerHeight-1, headerWidth, headerHeight-1 );
               //y += 1; now included in headerHeight
             }
             y += headerHeight + innerMargin;
           }

           if ( useFooter )
           {
             if ( ! ( footerDrawBg || useBox || useBackground ) ) // draw a 1 px (!?) line to separate footer from contents
               paint.drawLine( 0, maxHeight + innerMargin - 1, headerWidth, maxHeight + innerMargin - 1 );
             if ( footerDrawBg )
                paint.fillRect(0, maxHeight+innerMargin+boxWidth, headerWidth, footerHeight, footerBgColor);
             if (footerTagList.count() == 3)
             {
               int align = TQt::AlignVCenter|TQt::AlignLeft;
               int marg = ( useBox || footerDrawBg ) ? innerMargin : 0;
               if ( useBox ) marg += boxWidth;
               TQString s;
               for (int i=0; i<3; i++)
               {
                 s = footerTagList[i];
                 if (s.find("%p") != -1) s.replace("%p", TQString::number(currentPage));
                 paint.drawText(marg, maxHeight+innerMargin, headerWidth-(marg*2), footerHeight, align, s);
                 align = TQt::AlignVCenter|(i == 0 ? TQt::AlignHCenter : TQt::AlignRight);
               }
             }
           } // done footer

           if ( useBackground )
           {
             // If we have a box, or the header/footer has backgrounds, we want to paint
             // to the border of those. Otherwise just the contents area.
             int _y = y, _h = maxHeight - y;
             if ( useBox )
             {
               _y -= innerMargin;
               _h += 2 * innerMargin;
             }
             else
             {
               if ( headerDrawBg )
               {
                 _y -= innerMargin;
                 _h += innerMargin;
               }
               if ( footerDrawBg )
               {
                 _h += innerMargin;
               }
             }
             paint.fillRect( 0, _y, pdmWidth, _h, renderer.config()->backgroundColor());
           }

           if ( useBox )
           {
             paint.setPen(TQPen(boxColor, boxWidth));
             paint.drawRect(0, 0, pdmWidth, pdm.height());
             if (useHeader)
               paint.drawLine(0, headerHeight, headerWidth, headerHeight);
             else
               y += innerMargin;

             if ( useFooter ) // drawline is not trustable, grr.
               paint.fillRect( 0, maxHeight+innerMargin, headerWidth, boxWidth, boxColor );
           }

           if ( useGuide && currentPage == 1 )
           {  // FIXME - this may span more pages...
             // draw a box unless we have boxes, in which case we end with a box line

             // use color of dsNormal for the title string and the hline
             KateAttributeList _dsList;
             KateHlManager::self()->getDefaults ( renderer.config()->schema(), _dsList );
             paint.setPen( _dsList.at(0)->textColor() );
             int _marg = 0; // this could be available globally!??
             if ( useBox )
             {
               _marg += (2*boxWidth) + (2*innerMargin);
               paint.fillRect( 0, y+guideHeight-innerMargin-boxWidth, headerWidth, boxWidth, boxColor );
             }
             else
             {
               if ( useBackground )
                 _marg += 2*innerMargin;
               paint.drawRect( _marg, y, pdmWidth-(2*_marg), guideHeight );
               _marg += 1;
               y += 1 + innerMargin;
             }
             // draw a title string
             paint.setFont( renderer.config()->fontStruct()->myFontBold );
             TQRect _r;
             paint.drawText( _marg, y, pdmWidth-(2*_marg), maxHeight - y,
                TQt::AlignTop|TQt::AlignHCenter,
                i18n("Typographical Conventions for %1").arg(doc->highlight()->name()), -1, &_r );
             int _w = pdmWidth - (_marg*2) - (innerMargin*2);
             int _x = _marg + innerMargin;
             y += _r.height() + innerMargin;
             paint.drawLine( _x, y, _x + _w, y );
             y += 1 + innerMargin;
             // draw attrib names using their styles

             TQPtrListIterator<KateHlItemData> _it( ilist );
             KateHlItemData *_d;
             int _cw = _w/guideCols;
             int _i(0);

             while ( ( _d = _it.current() ) != 0 )
             {
               paint.setPen( renderer.attribute(_i)->textColor() );
               paint.setFont( renderer.attribute(_i)->font( *renderer.currentFont() ) );
               paint.drawText(( _x + ((_i%guideCols)*_cw)), y, _cw, renderer.fontHeight(),
                        TQt::AlignVCenter|TQt::AlignLeft, _d->name, -1, &_r );
               _i++;
               if ( _i && ! ( _i%guideCols ) ) y += renderer.fontHeight();
               ++_it;
             }
             if ( _i%guideCols ) y += renderer.fontHeight();// last row not full
             y += ( useBox ? boxWidth : 1 ) + (innerMargin*2);
           }

           pageStarted = false;
         } // pageStarted; move on to contents:)

         if ( printLineNumbers && ! startCol ) // don't repeat!
         {
           paint.setFont( renderer.config()->fontStruct()->font( false, false ) );
           paint.setPen( renderer.config()->lineNumberColor() );
           paint.drawText( (( useBox || useBackground ) ? innerMargin : 0), y,
                        lineNumberWidth, renderer.fontHeight(),
                        TQt::AlignRight, TQString("%1").arg( lineCount + 1 ) );
         }
         endCol = renderer.textWidth(doc->kateTextLine(lineCount), startCol, maxWidth, &needWrap);

         if ( endCol < startCol )
         {
           //kdDebug(13020)<<"--- Skipping garbage, line: "<<lineCount<<" start: "<<startCol<<" end: "<<endCol<<" real EndCol; "<< buffer->line(lineCount)->length()<< " !?"<<endl;
           lineCount++;
           continue; // strange case...
                     // Happens if the line fits exactly.
                     // When it happens, a line of garbage would be printed.
                     // FIXME Most likely this is an error in textWidth(),
                     // failing to correctly set needWrap to false in this case?
         }

         // if we print only selection:
         // print only selected range of chars.
         bool skip = false;
         if ( selectionOnly )
         {
           bool inBlockSelection = ( doc->blockSelectionMode() && lineCount >= firstline && lineCount <= lastline );
           if ( lineCount == firstline || inBlockSelection )
           {
             if ( startCol < selStartCol )
               startCol = selStartCol;
           }
           if ( lineCount == lastline  || inBlockSelection )
           {
             if ( endCol > selEndCol )
             {
               endCol = selEndCol;
               skip = true;
             }
           }
         }

         // HA! this is where we print [part of] a line ;]]
         // FIXME Convert this function + related functionality to a separate KatePrintView
         KateLineRange range;
         range.line = lineCount;
         range.startCol = startCol;
         range.endCol = endCol;
         range.wrap = needWrap;
         paint.translate(xstart, y);
         renderer.paintTextLine(paint, &range, 0, maxWidth);
         paint.resetXForm();
         if ( skip )
         {
           needWrap = false;
           startCol = 0;
         }
         else
         {
           startCol = endCol;
         }

         y += renderer.fontHeight();
         _count++;
       } // done while ( needWrap )

       lineCount++;
     } // done lineCount <= lastline
     return true;
  }

#endif //!TQ_WS_WIN
  return false;
}
//END KatePrinter

#ifndef TQ_WS_WIN //TODO: reenable
//BEGIN KatePrintTextSettings
KatePrintTextSettings::KatePrintTextSettings( KPrinter * /*printer*/, TQWidget *parent, const char *name )
  : KPrintDialogPage( parent, name )
{
  setTitle( i18n("Te&xt Settings") );

  TQVBoxLayout *lo = new TQVBoxLayout ( this );
  lo->setSpacing( KDialog::spacingHint() );

  cbSelection = new TQCheckBox( i18n("Print &selected text only"), this );
  lo->addWidget( cbSelection );

  cbLineNumbers = new TQCheckBox( i18n("Print &line numbers"), this );
  lo->addWidget( cbLineNumbers );

  cbGuide = new TQCheckBox( i18n("Print syntax &guide"), this );
  lo->addWidget( cbGuide );

  lo->addStretch( 1 );

  // set defaults - nothing to do :-)

  // whatsthis
  TQWhatsThis::add( cbSelection, i18n(
        "<p>This option is only available if some text is selected in the document.</p>"
        "<p>If available and enabled, only the selected text is printed.</p>") );
  TQWhatsThis::add( cbLineNumbers, i18n(
        "<p>If enabled, line numbers will be printed on the left side of the page(s).</p>") );
  TQWhatsThis::add( cbGuide, i18n(
        "<p>Print a box displaying typographical conventions for the document type, as "
        "defined by the syntax highlighting being used.") );
}

void KatePrintTextSettings::getOptions( TQMap<TQString,TQString>& opts, bool )
{
  opts["app-kate-printselection"] = cbSelection->isChecked() ? "true" : "false";
  opts["app-kate-printlinenumbers"] = cbLineNumbers->isChecked() ? "true" : "false";
  opts["app-kate-printguide"] = cbGuide->isChecked() ? "true" : "false" ;
}

void KatePrintTextSettings::setOptions( const TQMap<TQString,TQString>& opts )
{
  TQString v;
  v = opts["app-kate-printselection"];
  if ( ! v.isEmpty() )
    cbSelection->setChecked( v == "true" );
  v = opts["app-kate-printlinenumbers"];
  if ( ! v.isEmpty() )
    cbLineNumbers->setChecked( v == "true" );
  v = opts["app-kate-printguide"];
  if ( ! v.isEmpty() )
    cbGuide->setChecked( v == "true" );
}

void KatePrintTextSettings::enableSelection( bool enable )
{
  cbSelection->setEnabled( enable );
}

//END KatePrintTextSettings

//BEGIN KatePrintHeaderFooter
KatePrintHeaderFooter::KatePrintHeaderFooter( KPrinter * /*printer*/, TQWidget *parent, const char *name )
  : KPrintDialogPage( parent, name )
{
  setTitle( i18n("Hea&der && Footer") );

  TQVBoxLayout *lo = new TQVBoxLayout ( this );
  uint sp = KDialog::spacingHint();
  lo->setSpacing( sp );

  // enable
  TQHBoxLayout *lo1 = new TQHBoxLayout ( lo );
  cbEnableHeader = new TQCheckBox( i18n("Pr&int header"), this );
  lo1->addWidget( cbEnableHeader );
  cbEnableFooter = new TQCheckBox( i18n("Pri&nt footer"), this );
  lo1->addWidget( cbEnableFooter );

  // font
  TQHBoxLayout *lo2 = new TQHBoxLayout( lo );
  lo2->addWidget( new TQLabel( i18n("Header/footer font:"), this ) );
  lFontPreview = new TQLabel( this );
  lFontPreview->setFrameStyle( TQFrame::Panel|TQFrame::Sunken );
  lo2->addWidget( lFontPreview );
  lo2->setStretchFactor( lFontPreview, 1 );
  TQPushButton *btnChooseFont = new TQPushButton( i18n("Choo&se Font..."), this );
  lo2->addWidget( btnChooseFont );
  connect( btnChooseFont, TQ_SIGNAL(clicked()), this, TQ_SLOT(setHFFont()) );
  // header
  gbHeader = new TQGroupBox( 2, TQt::Horizontal, i18n("Header Properties"), this );
  lo->addWidget( gbHeader );

  TQLabel *lHeaderFormat = new TQLabel( i18n("&Format:"), gbHeader );
  TQHBox *hbHeaderFormat = new TQHBox( gbHeader );
  hbHeaderFormat->setSpacing( sp );
  leHeaderLeft = new TQLineEdit( hbHeaderFormat );
  leHeaderCenter = new TQLineEdit( hbHeaderFormat );
  leHeaderRight = new TQLineEdit( hbHeaderFormat );
  lHeaderFormat->setBuddy( leHeaderLeft );
  new TQLabel( i18n("Colors:"), gbHeader );
  TQHBox *hbHeaderColors = new TQHBox( gbHeader );
  hbHeaderColors->setSpacing( sp );
  TQLabel *lHeaderFgCol = new TQLabel( i18n("Foreground:"), hbHeaderColors );
  kcbtnHeaderFg = new KColorButton( hbHeaderColors );
  lHeaderFgCol->setBuddy( kcbtnHeaderFg );
  cbHeaderEnableBgColor = new TQCheckBox( i18n("Bac&kground"), hbHeaderColors );
  kcbtnHeaderBg = new KColorButton( hbHeaderColors );

  gbFooter = new TQGroupBox( 2, TQt::Horizontal, i18n("Footer Properties"), this );
  lo->addWidget( gbFooter );

  // footer
  TQLabel *lFooterFormat = new TQLabel( i18n("For&mat:"), gbFooter );
  TQHBox *hbFooterFormat = new TQHBox( gbFooter );
  hbFooterFormat->setSpacing( sp );
  leFooterLeft = new TQLineEdit( hbFooterFormat );
  leFooterCenter = new TQLineEdit( hbFooterFormat );
  leFooterRight = new TQLineEdit( hbFooterFormat );
  lFooterFormat->setBuddy( leFooterLeft );

  new TQLabel( i18n("Colors:"), gbFooter );
  TQHBox *hbFooterColors = new TQHBox( gbFooter );
  hbFooterColors->setSpacing( sp );
  TQLabel *lFooterBgCol = new TQLabel( i18n("Foreground:"), hbFooterColors );
  kcbtnFooterFg = new KColorButton( hbFooterColors );
  lFooterBgCol->setBuddy( kcbtnFooterFg );
  cbFooterEnableBgColor = new TQCheckBox( i18n("&Background"), hbFooterColors );
  kcbtnFooterBg = new KColorButton( hbFooterColors );

  lo->addStretch( 1 );

  // user friendly
  connect( cbEnableHeader, TQ_SIGNAL(toggled(bool)), gbHeader, TQ_SLOT(setEnabled(bool)) );
  connect( cbEnableFooter, TQ_SIGNAL(toggled(bool)), gbFooter, TQ_SLOT(setEnabled(bool)) );
  connect( cbHeaderEnableBgColor, TQ_SIGNAL(toggled(bool)), kcbtnHeaderBg, TQ_SLOT(setEnabled(bool)) );
  connect( cbFooterEnableBgColor, TQ_SIGNAL(toggled(bool)), kcbtnFooterBg, TQ_SLOT(setEnabled(bool)) );

  // set defaults
  cbEnableHeader->setChecked( true );
  leHeaderLeft->setText( "%y" );
  leHeaderCenter->setText( "%f" );
  leHeaderRight->setText( "%p" );
  kcbtnHeaderFg->setColor( TQColor("black") );
  cbHeaderEnableBgColor->setChecked( true );
  kcbtnHeaderBg->setColor( TQColor("lightgrey") );

  cbEnableFooter->setChecked( true );
  leFooterRight->setText( "%U" );
  kcbtnFooterFg->setColor( TQColor("black") );
  cbFooterEnableBgColor->setChecked( true );
  kcbtnFooterBg->setColor( TQColor("lightgrey") );

  // whatsthis
  TQString  s = i18n("<p>Format of the page header. The following tags are supported:</p>");
  TQString s1 = i18n(
      "<ul><li><tt>%u</tt>: current user name</li>"
      "<li><tt>%d</tt>: complete date/time in short format</li>"
      "<li><tt>%D</tt>: complete date/time in long format</li>"
      "<li><tt>%h</tt>: current time</li>"
      "<li><tt>%y</tt>: current date in short format</li>"
      "<li><tt>%Y</tt>: current date in long format</li>"
      "<li><tt>%f</tt>: file name</li>"
      "<li><tt>%U</tt>: full URL of the document</li>"
      "<li><tt>%p</tt>: page number</li>"
      "</ul><br>"
      "<u>Note:</u> Do <b>not</b> use the '|' (vertical bar) character.");
  TQWhatsThis::add(leHeaderRight, s + s1 );
  TQWhatsThis::add(leHeaderCenter, s + s1 );
  TQWhatsThis::add(leHeaderLeft, s + s1 );
  s = i18n("<p>Format of the page footer. The following tags are supported:</p>");
  TQWhatsThis::add(leFooterRight, s + s1 );
  TQWhatsThis::add(leFooterCenter, s + s1 );
  TQWhatsThis::add(leFooterLeft, s + s1 );


}

void KatePrintHeaderFooter::getOptions(TQMap<TQString,TQString>& opts, bool )
{
  opts["app-kate-hffont"] = strFont;

  opts["app-kate-useheader"] = (cbEnableHeader->isChecked() ? "true" : "false");
  opts["app-kate-headerfg"] = kcbtnHeaderFg->color().name();
  opts["app-kate-headerusebg"] = (cbHeaderEnableBgColor->isChecked() ? "true" : "false");
  opts["app-kate-headerbg"] = kcbtnHeaderBg->color().name();
  opts["app-kate-headerformat"] = leHeaderLeft->text() + "|" + leHeaderCenter->text() + "|" + leHeaderRight->text();

  opts["app-kate-usefooter"] = (cbEnableFooter->isChecked() ? "true" : "false");
  opts["app-kate-footerfg"] = kcbtnFooterFg->color().name();
  opts["app-kate-footerusebg"] = (cbFooterEnableBgColor->isChecked() ? "true" : "false");
  opts["app-kate-footerbg"] = kcbtnFooterBg->color().name();
  opts["app-kate-footerformat"] = leFooterLeft->text() + "|" + leFooterCenter->text() + "|" + leFooterRight->text();
}

void KatePrintHeaderFooter::setOptions( const TQMap<TQString,TQString>& opts )
{
  TQString v;
  v = opts["app-kate-hffont"];
  strFont = v;
  TQFont f = font();
  if ( ! v.isEmpty() )
  {
    if (!strFont.isEmpty())
      f.fromString( strFont );

    lFontPreview->setFont( f );
  }
  lFontPreview->setText( (f.family() + ", %1pt").arg( f.pointSize() ) );

  v = opts["app-kate-useheader"];
  if ( ! v.isEmpty() )
    cbEnableHeader->setChecked( v == "true" );
  v = opts["app-kate-headerfg"];
  if ( ! v.isEmpty() )
    kcbtnHeaderFg->setColor( TQColor( v ) );
  v = opts["app-kate-headerusebg"];
  if ( ! v.isEmpty() )
    cbHeaderEnableBgColor->setChecked( v == "true" );
  v = opts["app-kate-headerbg"];
  if ( ! v.isEmpty() )
    kcbtnHeaderBg->setColor( TQColor( v ) );

  TQStringList tags = TQStringList::split('|', opts["app-kate-headerformat"], "true");
  if (tags.count() == 3)
  {
    leHeaderLeft->setText(tags[0]);
    leHeaderCenter->setText(tags[1]);
    leHeaderRight->setText(tags[2]);
  }

  v = opts["app-kate-usefooter"];
  if ( ! v.isEmpty() )
    cbEnableFooter->setChecked( v == "true" );
  v = opts["app-kate-footerfg"];
  if ( ! v.isEmpty() )
    kcbtnFooterFg->setColor( TQColor( v ) );
  v = opts["app-kate-footerusebg"];
  if ( ! v.isEmpty() )
    cbFooterEnableBgColor->setChecked( v == "true" );
  v = opts["app-kate-footerbg"];
  if ( ! v.isEmpty() )
    kcbtnFooterBg->setColor( TQColor( v ) );

  tags = TQStringList::split('|', opts["app-kate-footerformat"], "true");
  if (tags.count() == 3)
  {
    leFooterLeft->setText(tags[0]);
    leFooterCenter->setText(tags[1]);
    leFooterRight->setText(tags[2]);
  }
}

void KatePrintHeaderFooter::setHFFont()
{
  TQFont fnt( lFontPreview->font() );
  // display a font dialog
  if ( TDEFontDialog::getFont( fnt, false, this ) == TDEFontDialog::Accepted )
  {
    // change strFont
    strFont = fnt.toString();
    // set preview
    lFontPreview->setFont( fnt );
    lFontPreview->setText( (fnt.family() + ", %1pt").arg( fnt.pointSize() ) );
  }
}

//END KatePrintHeaderFooter

//BEGIN KatePrintLayout

KatePrintLayout::KatePrintLayout( KPrinter * /*printer*/, TQWidget *parent, const char *name )
  : KPrintDialogPage( parent, name )
{
  setTitle( i18n("L&ayout") );

  TQVBoxLayout *lo = new TQVBoxLayout ( this );
  lo->setSpacing( KDialog::spacingHint() );

  TQHBox *hb = new TQHBox( this );
  lo->addWidget( hb );
  TQLabel *lSchema = new TQLabel( i18n("&Schema:"), hb );
  cmbSchema = new TQComboBox( false, hb );
  lSchema->setBuddy( cmbSchema );

  cbDrawBackground = new TQCheckBox( i18n("Draw bac&kground color"), this );
  lo->addWidget( cbDrawBackground );

  cbEnableBox = new TQCheckBox( i18n("Draw &boxes"), this );
  lo->addWidget( cbEnableBox );

  gbBoxProps = new TQGroupBox( 2, TQt::Horizontal, i18n("Box Properties"), this );
  lo->addWidget( gbBoxProps );

  TQLabel *lBoxWidth = new TQLabel( i18n("W&idth:"), gbBoxProps );
  sbBoxWidth = new TQSpinBox( 1, 100, 1, gbBoxProps );
  lBoxWidth->setBuddy( sbBoxWidth );

  TQLabel *lBoxMargin = new TQLabel( i18n("&Margin:"), gbBoxProps );
  sbBoxMargin = new TQSpinBox( 0, 100, 1, gbBoxProps );
  lBoxMargin->setBuddy( sbBoxMargin );

  TQLabel *lBoxColor = new TQLabel( i18n("Co&lor:"), gbBoxProps );
  kcbtnBoxColor = new KColorButton( gbBoxProps );
  lBoxColor->setBuddy( kcbtnBoxColor );

  connect( cbEnableBox, TQ_SIGNAL(toggled(bool)), gbBoxProps, TQ_SLOT(setEnabled(bool)) );

  lo->addStretch( 1 );
  // set defaults:
  sbBoxMargin->setValue( 6 );
  gbBoxProps->setEnabled( false );
  cmbSchema->insertStringList (KateFactory::self()->schemaManager()->list ());
  cmbSchema->setCurrentItem( 1 );

  // whatsthis
  // FIXME uncomment when string freeze is over
//   TQWhatsThis::add ( cmbSchema, i18n(
//         "Select the color scheme to use for the print." ) );
  TQWhatsThis::add( cbDrawBackground, i18n(
        "<p>If enabled, the background color of the editor will be used.</p>"
        "<p>This may be useful if your color scheme is designed for a dark background.</p>") );
  TQWhatsThis::add( cbEnableBox, i18n(
        "<p>If enabled, a box as defined in the properties below will be drawn "
        "around the contents of each page. The Header and Footer will be separated "
        "from the contents with a line as well.</p>") );
  TQWhatsThis::add( sbBoxWidth, i18n(
        "The width of the box outline" ) );
  TQWhatsThis::add( sbBoxMargin, i18n(
        "The margin inside boxes, in pixels") );
  TQWhatsThis::add( kcbtnBoxColor, i18n(
        "The line color to use for boxes") );
}

void KatePrintLayout::getOptions(TQMap<TQString,TQString>& opts, bool )
{
  opts["app-kate-colorscheme"] = cmbSchema->currentText();
  opts["app-kate-usebackground"] = cbDrawBackground->isChecked() ? "true" : "false";
  opts["app-kate-usebox"] = cbEnableBox->isChecked() ? "true" : "false";
  opts["app-kate-boxwidth"] = sbBoxWidth->cleanText();
  opts["app-kate-boxmargin"] = sbBoxMargin->cleanText();
  opts["app-kate-boxcolor"] = kcbtnBoxColor->color().name();
}

void KatePrintLayout::setOptions( const TQMap<TQString,TQString>& opts )
{
  TQString v;
  v = opts["app-kate-colorscheme"];
  if ( ! v.isEmpty() )
    cmbSchema->setCurrentItem( KateFactory::self()->schemaManager()->number( v ) );
  v = opts["app-kate-usebackground"];
  if ( ! v.isEmpty() )
    cbDrawBackground->setChecked( v == "true" );
  v = opts["app-kate-usebox"];
  if ( ! v.isEmpty() )
    cbEnableBox->setChecked( v == "true" );
  v = opts["app-kate-boxwidth"];
  if ( ! v.isEmpty() )
    sbBoxWidth->setValue( v.toInt() );
  v = opts["app-kate-boxmargin"];
  if ( ! v.isEmpty() )
    sbBoxMargin->setValue( v.toInt() );
  v = opts["app-kate-boxcolor"];
  if ( ! v.isEmpty() )
    kcbtnBoxColor->setColor( TQColor( v ) );
}
//END KatePrintLayout

#include "kateprinter.moc"
#endif //!TQ_WS_WIN
