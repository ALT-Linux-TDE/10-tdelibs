
/*

    Requires the Qt widget libraries, available at no cost at
    http://www.troll.no

    Copyright (C) 1996 Bernd Johannes Wuebben  <wuebben@kde.org>
    Copyright (c) 1999 Preston Brown <pbrown@kde.org>
    Copyright (c) 1999 Mario Weilguni <mweilguni@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License as published by the Free Software Foundation; either
    version 2 of the License, or (at your option) any later version.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <config.h>

#include <stdio.h>
#include <stdlib.h>

#include <tqcombobox.h>
#include <tqcheckbox.h>
#include <tqfile.h>
#include <tqfont.h>
#include <tqgroupbox.h>
#include <tqlabel.h>
#include <tqlayout.h>
#include <tqscrollbar.h>
#include <tqstringlist.h>
#include <tqfontdatabase.h>
#include <tqwhatsthis.h>
#include <tqtooltip.h>

#include <tdeapplication.h>
#include <kcharsets.h>
#include <tdeconfig.h>
#include <kdialog.h>
#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include <tqlineedit.h>
#include <tdelistbox.h>
#include <tdelocale.h>
#include <kstandarddirs.h>
#include <kdebug.h>
#include <knuminput.h>

#include "tdefontdialog.moc"

static int minimumListWidth( const TQListBox *list )
{
  int w=0;
  for( uint i=0; i<list->count(); i++ )
  {
    int itemWidth = list->item(i)->width(list);
    w = TQMAX(w,itemWidth);
  }
  if( w == 0 ) { w = 40; }
  w += list->frameWidth() * 2;
  w += list->verticalScrollBar()->sizeHint().width();
  return w;
}

static int minimumListHeight( const TQListBox *list, int numVisibleEntry )
{
  int w = list->count() > 0 ? list->item(0)->height(list) :
    list->fontMetrics().lineSpacing();

  if( w < 0 ) { w = 10; }
  if( numVisibleEntry <= 0 ) { numVisibleEntry = 4; }
  return ( w * numVisibleEntry + 2 * list->frameWidth() );
}

class TDEFontChooser::TDEFontChooserPrivate
{
public:
    TDEFontChooserPrivate()
        { m_palette.setColor(TQPalette::Active, TQColorGroup::Text, TQt::black);
          m_palette.setColor(TQPalette::Active, TQColorGroup::Base, TQt::white); }
    TQPalette m_palette;
};

TDEFontChooser::TDEFontChooser(TQWidget *parent, const char *name,
			   bool onlyFixed, const TQStringList &fontList,
			   bool makeFrame, int visibleListSize, bool diff,
                           TQButton::ToggleState *sizeIsRelativeState )
  : TQWidget(parent, name), usingFixed(onlyFixed)
{
  charsetsCombo = 0;

  TQString mainWhatsThisText =
    i18n( "Here you can choose the font to be used." );
  TQWhatsThis::add( this, mainWhatsThisText );

  d = new TDEFontChooserPrivate;
  TQVBoxLayout *topLayout = new TQVBoxLayout( this, 0, KDialog::spacingHint() );
  int checkBoxGap = KDialog::spacingHint() / 2;

  TQWidget *page;
  TQGridLayout *gridLayout;
  int row = 0;
  if( makeFrame )
  {
    page = new TQGroupBox( i18n("Requested Font"), this );
    topLayout->addWidget(page);
    gridLayout = new TQGridLayout( page, 5, 3, KDialog::marginHint(), KDialog::spacingHint() );
    gridLayout->addRowSpacing( 0, fontMetrics().lineSpacing() );
    row = 1;
  }
  else
  {
    page = new TQWidget( this );
    topLayout->addWidget(page);
    gridLayout = new TQGridLayout( page, 4, 3, 0, KDialog::spacingHint() );
  }

  //
  // first, create the labels across the top
  //
  TQHBoxLayout *familyLayout = new TQHBoxLayout();
  familyLayout->addSpacing( checkBoxGap );
  if (diff) {
    familyCheckbox = new TQCheckBox(i18n("Font"), page);
    connect(familyCheckbox, TQ_SIGNAL(toggled(bool)), TQ_SLOT(toggled_checkbox()));
    familyLayout->addWidget(familyCheckbox, 0, TQt::AlignLeft);
    TQString familyCBToolTipText =
      i18n("Change font family?");
    TQString familyCBWhatsThisText =
      i18n("Enable this checkbox to change the font family settings.");
    TQWhatsThis::add( familyCheckbox, familyCBWhatsThisText );
    TQToolTip::add(   familyCheckbox, familyCBToolTipText );
    familyLabel = 0;
  } else {
    familyCheckbox = 0;
    familyLabel = new TQLabel( i18n("Font:"), page, "familyLabel" );
    familyLayout->addWidget(familyLabel, 1, TQt::AlignLeft);
  }
  gridLayout->addLayout(familyLayout, row, 0 );

  TQHBoxLayout *styleLayout = new TQHBoxLayout();
  if (diff) {
     styleCheckbox = new TQCheckBox(i18n("Font style"), page);
     connect(styleCheckbox, TQ_SIGNAL(toggled(bool)), TQ_SLOT(toggled_checkbox()));
     styleLayout->addWidget(styleCheckbox, 0, TQt::AlignLeft);
    TQString styleCBToolTipText =
      i18n("Change font style?");
    TQString styleCBWhatsThisText =
      i18n("Enable this checkbox to change the font style settings.");
    TQWhatsThis::add( styleCheckbox, styleCBWhatsThisText );
    TQToolTip::add(   styleCheckbox, styleCBToolTipText );
    styleLabel = 0;
  } else {
    styleCheckbox = 0;
    styleLabel = new TQLabel( i18n("Font style:"), page, "styleLabel");
    styleLayout->addWidget(styleLabel, 1, TQt::AlignLeft);
  }
  styleLayout->addSpacing( checkBoxGap );
  gridLayout->addLayout(styleLayout, row, 1 );

  TQHBoxLayout *sizeLayout = new TQHBoxLayout();
  if (diff) {
    sizeCheckbox = new TQCheckBox(i18n("Size"),page);
    connect(sizeCheckbox, TQ_SIGNAL(toggled(bool)), TQ_SLOT(toggled_checkbox()));
    sizeLayout->addWidget(sizeCheckbox, 0, TQt::AlignLeft);
    TQString sizeCBToolTipText =
      i18n("Change font size?");
    TQString sizeCBWhatsThisText =
      i18n("Enable this checkbox to change the font size settings.");
    TQWhatsThis::add( sizeCheckbox, sizeCBWhatsThisText );
    TQToolTip::add(   sizeCheckbox, sizeCBToolTipText );
    sizeLabel = 0;
  } else {
    sizeCheckbox = 0;
    sizeLabel = new TQLabel( i18n("Size:"), page, "sizeLabel");
    sizeLayout->addWidget(sizeLabel, 1, TQt::AlignLeft);
  }
  sizeLayout->addSpacing( checkBoxGap );
  sizeLayout->addSpacing( checkBoxGap ); // prevent label from eating border
  gridLayout->addLayout(sizeLayout, row, 2 );

  row ++;

  //
  // now create the actual boxes that hold the info
  //
  familyListBox = new TDEListBox( page, "familyListBox");
  familyListBox->setEnabled( !diff );
  gridLayout->addWidget( familyListBox, row, 0 );
  TQString fontFamilyWhatsThisText =
    i18n("Here you can choose the font family to be used." );
  TQWhatsThis::add( familyListBox, fontFamilyWhatsThisText );
  TQWhatsThis::add(diff?(TQWidget *) familyCheckbox:(TQWidget *) familyLabel, fontFamilyWhatsThisText );
  connect(familyListBox, TQ_SIGNAL(highlighted(const TQString &)),
	  TQ_SLOT(family_chosen_slot(const TQString &)));
  if(!fontList.isEmpty())
  {
    familyListBox->insertStringList(fontList);
  }
  else
  {
    fillFamilyListBox(onlyFixed);
  }

  familyListBox->setMinimumWidth( minimumListWidth( familyListBox ) );
  familyListBox->setMinimumHeight(
    minimumListHeight( familyListBox, visibleListSize  ) );

  styleListBox = new TDEListBox( page, "styleListBox");
  styleListBox->setEnabled( !diff );
  gridLayout->addWidget(styleListBox, row, 1);
  TQString fontStyleWhatsThisText =
    i18n("Here you can choose the font style to be used." );
  TQWhatsThis::add( styleListBox, fontStyleWhatsThisText );
  TQWhatsThis::add(diff?(TQWidget *)styleCheckbox:(TQWidget *)styleLabel, fontFamilyWhatsThisText );
  styleListBox->insertItem(i18n("Regular"));
  styleListBox->insertItem(i18n("Italic"));
  styleListBox->insertItem(i18n("Bold"));
  styleListBox->insertItem(i18n("Bold Italic"));
  styleListBox->setMinimumWidth( minimumListWidth( styleListBox ) );
  styleListBox->setMinimumHeight(
    minimumListHeight( styleListBox, visibleListSize  ) );

  connect(styleListBox, TQ_SIGNAL(highlighted(const TQString &)),
	  TQ_SLOT(style_chosen_slot(const TQString &)));


  sizeListBox = new TDEListBox( page, "sizeListBox");
  sizeOfFont = new KIntNumInput( page, "sizeOfFont");
  sizeOfFont->setMinValue(4);

  sizeListBox->setEnabled( !diff );
  sizeOfFont->setEnabled( !diff );
  if( sizeIsRelativeState ) {
    TQString sizeIsRelativeCBText =
      i18n("Relative");
    TQString sizeIsRelativeCBToolTipText =
      i18n("Font size<br><i>fixed</i> or <i>relative</i><br>to environment");
    TQString sizeIsRelativeCBWhatsThisText =
      i18n("Here you can switch between fixed font size and font size "
           "to be calculated dynamically and adjusted to changing "
           "environment (e.g. widget dimensions, paper size)." );
    sizeIsRelativeCheckBox = new TQCheckBox( sizeIsRelativeCBText,
                                            page,
                                           "sizeIsRelativeCheckBox" );
    sizeIsRelativeCheckBox->setTristate( diff );
    TQGridLayout *sizeLayout2 = new TQGridLayout( 3,2, KDialog::spacingHint()/2, "sizeLayout2" );
    gridLayout->addLayout(sizeLayout2, row, 2);
    sizeLayout2->setColStretch( 1, 1 ); // to prevent text from eating the right border
    sizeLayout2->addMultiCellWidget( sizeOfFont, 0, 0, 0, 1);
    sizeLayout2->addMultiCellWidget(sizeListBox, 1,1, 0,1);
    sizeLayout2->addWidget(sizeIsRelativeCheckBox, 2, 0, TQt::AlignLeft);
    TQWhatsThis::add( sizeIsRelativeCheckBox, sizeIsRelativeCBWhatsThisText );
    TQToolTip::add(   sizeIsRelativeCheckBox, sizeIsRelativeCBToolTipText );
  }
  else {
    sizeIsRelativeCheckBox = 0L;
    TQGridLayout *sizeLayout2 = new TQGridLayout( 2,1, KDialog::spacingHint()/2, "sizeLayout2" );
    gridLayout->addLayout(sizeLayout2, row, 2);
    sizeLayout2->addWidget( sizeOfFont, 0, 0);
    sizeLayout2->addMultiCellWidget(sizeListBox, 1,1, 0,0);
  }
  TQString fontSizeWhatsThisText =
    i18n("Here you can choose the font size to be used." );
  TQWhatsThis::add( sizeListBox, fontSizeWhatsThisText );
  TQWhatsThis::add( diff?(TQWidget *)sizeCheckbox:(TQWidget *)sizeLabel, fontSizeWhatsThisText );

  fillSizeList();
  sizeListBox->setMinimumWidth( minimumListWidth(sizeListBox) +
    sizeListBox->fontMetrics().maxWidth() );
  sizeListBox->setMinimumHeight(
    minimumListHeight( sizeListBox, visibleListSize  ) );

  connect( sizeOfFont, TQ_SIGNAL( valueChanged(int) ),
           TQ_SLOT(size_value_slot(int)));

  connect( sizeListBox, TQ_SIGNAL(highlighted(const TQString&)),
	   TQ_SLOT(size_chosen_slot(const TQString&)) );
  sizeListBox->setSelected(sizeListBox->findItem(TQString::number(10)), true); // default to 10pt.

  row ++;

  row ++;
  sampleEdit = new TQLineEdit( page, "sampleEdit");
  TQFont tmpFont( TDEGlobalSettings::generalFont().family(), 64, TQFont::Black );
  sampleEdit->setFont(tmpFont);
  //i18n: This is a classical test phrase. (It contains all letters from A to Z.)
  sampleEdit->setText(i18n("The Quick Brown Fox Jumps Over The Lazy Dog"));
  sampleEdit->setMinimumHeight( sampleEdit->fontMetrics().lineSpacing() );
  sampleEdit->setAlignment(TQt::AlignCenter);
  gridLayout->addMultiCellWidget(sampleEdit, 4, 4, 0, 2);
  TQString sampleEditWhatsThisText =
    i18n("This sample text illustrates the current settings. "
         "You may edit it to test special characters." );
  TQWhatsThis::add( sampleEdit, sampleEditWhatsThisText );
  connect(this, TQ_SIGNAL(fontSelected(const TQFont &)),
	  TQ_SLOT(displaySample(const TQFont &)));

  TQVBoxLayout *vbox;
  if( makeFrame )
  {
    page = new TQGroupBox( i18n("Actual Font"), this );
    topLayout->addWidget(page);
    vbox = new TQVBoxLayout( page, KDialog::spacingHint() );
    vbox->addSpacing( fontMetrics().lineSpacing() );
  }
  else
  {
    page = new TQWidget( this );
    topLayout->addWidget(page);
    vbox = new TQVBoxLayout( page, 0, KDialog::spacingHint() );
    TQLabel *label = new TQLabel( i18n("Actual Font"), page );
    vbox->addWidget( label );
  }

  xlfdEdit = new TQLineEdit( page, "xlfdEdit" );
  vbox->addWidget( xlfdEdit );

  // lets initialize the display if possible
  setFont( TDEGlobalSettings::generalFont(), usingFixed );
  // check or uncheck or gray out the "relative" checkbox
  if( sizeIsRelativeState && sizeIsRelativeCheckBox )
    setSizeIsRelative( *sizeIsRelativeState );

  TDEConfig *config = TDEGlobal::config();
  TDEConfigGroupSaver saver(config, TQString::fromLatin1("General"));
  showXLFDArea(config->readBoolEntry(TQString::fromLatin1("fontSelectorShowXLFD"), false));
}

TDEFontChooser::~TDEFontChooser()
{
  delete d;
}

void TDEFontChooser::fillSizeList() {
  if(! sizeListBox) return; //assertion.

  static const int c[] =
  {
    4,  5,  6,  7,
    8,  9,  10, 11,
    12, 13, 14, 15,
    16, 17, 18, 19,
    20, 22, 24, 26,
    28, 32, 48, 64,
    0
  };
  for(int i = 0; c[i]; ++i)
  {
    sizeListBox->insertItem(TQString::number(c[i]));
  }
}

void TDEFontChooser::setColor( const TQColor & col )
{
  d->m_palette.setColor( TQPalette::Active, TQColorGroup::Text, col );
  TQPalette pal = sampleEdit->palette();
  pal.setColor( TQPalette::Active, TQColorGroup::Text, col );
  sampleEdit->setPalette( pal );
}

TQColor TDEFontChooser::color() const
{
  return d->m_palette.color( TQPalette::Active, TQColorGroup::Text );
}

void TDEFontChooser::setBackgroundColor( const TQColor & col )
{
  d->m_palette.setColor( TQPalette::Active, TQColorGroup::Base, col );
  TQPalette pal = sampleEdit->palette();
  pal.setColor( TQPalette::Active, TQColorGroup::Base, col );
  sampleEdit->setPalette( pal );
}

TQColor TDEFontChooser::backgroundColor() const
{
  return d->m_palette.color( TQPalette::Active, TQColorGroup::Base );
}

void TDEFontChooser::setSizeIsRelative( TQButton::ToggleState relative )
{
  // check or uncheck or gray out the "relative" checkbox
  if( sizeIsRelativeCheckBox ) {
    if( TQButton::NoChange == relative )
      sizeIsRelativeCheckBox->setNoChange();
    else
      sizeIsRelativeCheckBox->setChecked(  TQButton::On == relative );
  }
}

TQButton::ToggleState TDEFontChooser::sizeIsRelative() const
{
  return sizeIsRelativeCheckBox
       ? sizeIsRelativeCheckBox->state()
       : TQButton::NoChange;
}

TQSize TDEFontChooser::sizeHint( void ) const
{
  return minimumSizeHint();
}


void TDEFontChooser::enableColumn( int column, bool state )
{
  if( column & FamilyList )
  {
    familyListBox->setEnabled(state);
  }
  if( column & StyleList )
  {
    styleListBox->setEnabled(state);
  }
  if( column & SizeList )
  {
    sizeListBox->setEnabled(state);
  }
}


void TDEFontChooser::setFont( const TQFont& aFont, bool onlyFixed )
{
  selFont = aFont;
  selectedSize=aFont.pointSize();
  if (selectedSize == -1)
     selectedSize = TQFontInfo(aFont).pointSize();

  if( onlyFixed != usingFixed)
  {
    usingFixed = onlyFixed;
    fillFamilyListBox(usingFixed);
  }
  setupDisplay();
  displaySample(selFont);
}


int TDEFontChooser::fontDiffFlags() {
   int diffFlags = 0;
   if (familyCheckbox && styleCheckbox && sizeCheckbox) {
      diffFlags = (int)(familyCheckbox->isChecked() ? FontDiffFamily : 0)
                | (int)( styleCheckbox->isChecked() ? FontDiffStyle  : 0)
                | (int)(  sizeCheckbox->isChecked() ? FontDiffSize   : 0);
   }
   return diffFlags;
}

void TDEFontChooser::toggled_checkbox()
{
  familyListBox->setEnabled( familyCheckbox->isChecked() );
  styleListBox->setEnabled( styleCheckbox->isChecked() );
  sizeListBox->setEnabled( sizeCheckbox->isChecked() );
  sizeOfFont->setEnabled( sizeCheckbox->isChecked() );
}

TQString TDEFontChooser::style_name(const TQString &style)
{
  return i18n(
    TQString(style).replace("Plain",  "Regular")
                   .replace("Normal", "Regular")
                   .replace("Oblique", "Italic")
                   .utf8());
}

void TDEFontChooser::family_chosen_slot(const TQString& family)
{
    TQString currentFamily;
    if (family.isEmpty())
    {
      currentFamily = familyListBox->currentText();
    }
    else
    {
      currentFamily = family;
    }

    TQFontDatabase dbase;
    TQStringList styles = dbase.styles(currentFamily);
    styleListBox->clear();
    currentStyles.clear();
    for ( TQStringList::Iterator it = styles.begin(); it != styles.end(); ++it ) {
        TQString style = style_name(*it);
        if(!styleListBox->findItem(style)) {
            styleListBox->insertItem(style);
            currentStyles.insert(style, *it);
        }
    }
    if(styleListBox->count()==0) {
        styleListBox->insertItem(i18n("Regular"));
        currentStyles.insert(i18n("Regular"), "Normal");
    }

    styleListBox->blockSignals(true);
    TQListBoxItem *item = styleListBox->findItem(selectedStyle);
    if (item)
    {
       styleListBox->setSelected(styleListBox->findItem(selectedStyle), true);
    }
    else
    {
       styleListBox->setSelected(0, true);
    }
    styleListBox->blockSignals(false);

    style_chosen_slot(TQString::null);

    if (!family.isEmpty() )
    {
      selectedFamily = family;
    }
}

void TDEFontChooser::size_chosen_slot(const TQString& size){

  selectedSize=size.toInt();
  sizeOfFont->setValue(selectedSize);
  selFont.setPointSize(selectedSize);
  emit fontSelected(selFont);
}

void TDEFontChooser::size_value_slot(int val) {
  selFont.setPointSize(val);
  emit fontSelected(selFont);
}

void TDEFontChooser::style_chosen_slot(const TQString& style)
{
    TQString currentStyle;
    if (style.isEmpty())
       currentStyle = styleListBox->currentText();
    else
       currentStyle = style;

    int diff=0; // the difference between the font size requested and what we can show.

    sizeListBox->clear();
    TQFontDatabase dbase;
    if(dbase.isSmoothlyScalable(familyListBox->currentText(), currentStyles[currentStyle])) {  // is vector font
        //sampleEdit->setPaletteBackgroundPixmap( VectorPixmap ); // TODO
        fillSizeList();
    } else {                                // is bitmap font.
        //sampleEdit->setPaletteBackgroundPixmap( BitmapPixmap ); // TODO
        TQValueList<int> sizes = dbase.smoothSizes(familyListBox->currentText(), currentStyles[currentStyle]);
        if(sizes.count() > 0) {
            TQValueList<int>::iterator it;
            diff=1000;
            for ( it = sizes.begin(); it != sizes.end(); ++it ) {
                if(*it <= selectedSize || diff > *it - selectedSize) diff = selectedSize - *it;
                sizeListBox->insertItem(TQString::number(*it));
            }
        } else // there are times QT does not provide the list..
            fillSizeList();
    }
    sizeListBox->blockSignals(true);
    sizeListBox->setSelected(sizeListBox->findItem(TQString::number(selectedSize)), true);
    sizeListBox->blockSignals(false);
    sizeListBox->ensureCurrentVisible();

    //kdDebug() << "Showing: " << familyListBox->currentText() << ", " << currentStyles[currentStyle] << ", " << selectedSize-diff << endl;
    selFont = dbase.font(familyListBox->currentText(), currentStyles[currentStyle], selectedSize-diff);
    emit fontSelected(selFont);
    if (!style.isEmpty())
        selectedStyle = style;
}

void TDEFontChooser::displaySample(const TQFont& font)
{
  sampleEdit->setFont(font);
  sampleEdit->setCursorPosition(0);
  xlfdEdit->setText(font.rawName());
  xlfdEdit->setCursorPosition(0);

  //TQFontInfo a = TQFontInfo(font);
  //kdDebug() << "font: " << a.family () << ", " << a.pointSize () << endl;
  //kdDebug() << "      (" << font.toString() << ")\n";
}

void TDEFontChooser::setupDisplay()
{
  // Calling familyListBox->setCurrentItem() causes the value of selFont
  // to change, so we save the family, style and size beforehand.
  TQFontDatabase dbase;
  TQString family = TQString(selFont.family()).lower();
  TQString style = style_name(dbase.styleString(selFont));
  int size = selFont.pointSize();
  if (size == -1)
     size = TQFontInfo(selFont).pointSize();
  TQString sizeStr = TQString::number(size);

  int numEntries, i;

  numEntries = familyListBox->count();
  for (i = 0; i < numEntries; i++) {
    if (family == familyListBox->text(i).lower()) {
      familyListBox->setCurrentItem(i);
      break;
    }
  }

  // 1st Fallback
  if ( (i == numEntries) )
  {
    if (family.contains('['))
    {
      family = family.left(family.find('[')).stripWhiteSpace();
      for (i = 0; i < numEntries; i++) {
        if (family == familyListBox->text(i).lower()) {
          familyListBox->setCurrentItem(i);
          break;
        }
      }
    }
  }

  // 2nd Fallback
  if ( (i == numEntries) )
  {
    TQString fallback = family+" [";
    for (i = 0; i < numEntries; i++) {
      if (familyListBox->text(i).lower().startsWith(fallback)) {
        familyListBox->setCurrentItem(i);
        break;
      }
    }
  }

  // 3rd Fallback
  if ( (i == numEntries) )
  {
    for (i = 0; i < numEntries; i++) {
      if (familyListBox->text(i).lower().startsWith(family)) {
        familyListBox->setCurrentItem(i);
        break;
      }
    }
  }

  // Fall back in case nothing matched. Otherwise, diff doesn't work
  if ( i == numEntries )
    familyListBox->setCurrentItem( 0 );

  int item = 0;
  for (int i = 0; i < (int)styleListBox->count(); ++i)
  {
    if (styleListBox->text(i) == style)
    {
      item = i;
      break;
    }
  }
  styleListBox->setCurrentItem(item);

  numEntries = sizeListBox->count();
  for (i = 0; i < numEntries; i++){
    if (sizeStr == sizeListBox->text(i)) {
      sizeListBox->setCurrentItem(i);
      break;
    }
  }

  sizeOfFont->setValue(size);
}


void TDEFontChooser::getFontList( TQStringList &list, uint fontListCriteria)
{
  TQFontDatabase dbase;
  TQStringList lstSys(dbase.families());

  // if we have criteria; then check fonts before adding
  if (fontListCriteria)
  {
    TQStringList lstFonts;
    for (TQStringList::Iterator it = lstSys.begin(); it != lstSys.end(); ++it)
    {
        if ((fontListCriteria & FixedWidthFonts) > 0 && !dbase.isFixedPitch(*it)) continue;
        if (((fontListCriteria & (SmoothScalableFonts | ScalableFonts)) == ScalableFonts) &&
                !dbase.isBitmapScalable(*it)) continue;
        if ((fontListCriteria & SmoothScalableFonts) > 0 && !dbase.isSmoothlyScalable(*it)) continue;
        lstFonts.append(*it);
    }

    if((fontListCriteria & FixedWidthFonts) > 0) {
        // Fallback.. if there are no fixed fonts found, it's probably a
        // bug in the font server or Qt.  In this case, just use 'fixed'
        if (lstFonts.count() == 0)
          lstFonts.append("fixed");
    }

    lstSys = lstFonts;
  }

  lstSys.sort();

  list = lstSys;
}

void TDEFontChooser::setFamilyList( TQStringList list )
{
  familyListBox->blockSignals(true);
  familyListBox->clear();
  familyListBox->insertStringList(list);
  setFont( TDEGlobalSettings::generalFont(), usingFixed );
  familyListBox->blockSignals(false);
}

void TDEFontChooser::addFont( TQStringList &list, const char *xfont )
{
  const char *ptr = strchr( xfont, '-' );
  if ( !ptr )
    return;

  ptr = strchr( ptr + 1, '-' );
  if ( !ptr )
    return;

  TQString font = TQString::fromLatin1(ptr + 1);

  int pos;
  if ( ( pos = font.find( '-' ) ) > 0 ) {
    font.truncate( pos );

    if ( font.find( TQString::fromLatin1("open look"), 0, false ) >= 0 )
      return;

    TQStringList::Iterator it = list.begin();

    for ( ; it != list.end(); ++it )
      if ( *it == font )
	return;
    list.append( font );
  }
}

void TDEFontChooser::fillFamilyListBox(bool onlyFixedFonts)
{
  TQStringList fontList;
  getFontList(fontList, onlyFixedFonts?FixedWidthFonts:0);
  familyListBox->clear();
  familyListBox->insertStringList(fontList);
}

void TDEFontChooser::showXLFDArea(bool show)
{
  if( show )
  {
    xlfdEdit->parentWidget()->show();
  }
  else
  {
    xlfdEdit->parentWidget()->hide();
  }
}

///////////////////////////////////////////////////////////////////////////////

TDEFontDialog::TDEFontDialog( TQWidget *parent, const char* name,
			  bool onlyFixed, bool modal,
			  const TQStringList &fontList, bool makeFrame, bool diff,
                          TQButton::ToggleState *sizeIsRelativeState )
  : KDialogBase( parent, name, modal, i18n("Select Font"), Ok|Cancel, Ok )
{
  chooser = new TDEFontChooser( this, "fontChooser",
                              onlyFixed, fontList, makeFrame, 8,
                              diff, sizeIsRelativeState );
  setMainWidget(chooser);
}


int TDEFontDialog::getFontDiff( TQFont &theFont, int &diffFlags, bool onlyFixed,
                             TQWidget *parent, bool makeFrame,
                             TQButton::ToggleState *sizeIsRelativeState )
{
  TDEFontDialog dlg( parent, "Font Selector", onlyFixed, true, TQStringList(),
		   makeFrame, true, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont = dlg.chooser->font();
    diffFlags = dlg.chooser->fontDiffFlags();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return result;
}

int TDEFontDialog::getFont( TQFont &theFont, bool onlyFixed,
                          TQWidget *parent, bool makeFrame,
                          TQButton::ToggleState *sizeIsRelativeState )
{
  TDEFontDialog dlg( parent, "Font Selector", onlyFixed, true, TQStringList(),
		   makeFrame, false, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont = dlg.chooser->font();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return result;
}


int TDEFontDialog::getFontAndText( TQFont &theFont, TQString &theString,
				 bool onlyFixed, TQWidget *parent,
				 bool makeFrame,
                                 TQButton::ToggleState *sizeIsRelativeState )
{
  TDEFontDialog dlg( parent, "Font and Text Selector", onlyFixed, true,
		   TQStringList(), makeFrame, false, sizeIsRelativeState );
  dlg.setFont( theFont, onlyFixed );

  int result = dlg.exec();
  if( result == Accepted )
  {
    theFont   = dlg.chooser->font();
    theString = dlg.chooser->sampleText();
    if( sizeIsRelativeState )
      *sizeIsRelativeState = dlg.chooser->sizeIsRelative();
  }
  return result;
}

void TDEFontChooser::virtual_hook( int, void* )
{ /*BASE::virtual_hook( id, data );*/ }

void TDEFontDialog::virtual_hook( int id, void* data )
{ KDialogBase::virtual_hook( id, data ); }
