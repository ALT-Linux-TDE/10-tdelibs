/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2002 Michael Goffioul <tdeprint@swing.be>
 *  Complete rewrite on Sat Jun 15 2002 (c) Anders Lund <anders@alweb.dk>
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
 
#ifndef __KATE_PRINTER_H__
#define __KATE_PRINTER_H__

#include <kprintdialogpage.h>

class KateDocument;

class KColorButton;
class KPrinter;
class TQCheckBox;
class TQComboBox;
class TQGroupBox;
class TQLabel;
class TQLineEdit;
class TQSpinBox;

class KatePrinter
{
  public:
    static bool print (KateDocument *doc);
};

#ifndef TQ_WS_WIN //TODO: reenable
//BEGIN Text settings
/*
  Text settings page:
  - [ ] Print Selection (enabled if there is a selection in the view)
  - Print Line Numbers
    () Smart () Yes () No
*/
class KatePrintTextSettings : public KPrintDialogPage
{
  TQ_OBJECT
  public:
    KatePrintTextSettings( KPrinter *printer, TQWidget *parent=0, const char *name=0 );
    ~KatePrintTextSettings(){};

    void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
    void setOptions(const TQMap<TQString,TQString>& opts);
    
    /* call if view has a selection, enables the seelction checkbox according to the arg */
    void enableSelection( bool );
  
  private:
    TQCheckBox *cbSelection, *cbLineNumbers, *cbGuide;
};
//END Text Settings

//BEGIN Header/Footer
/*
  Header & Footer page:
  - enable header/footer
  - header/footer props
    o formats
    o colors
*/

class KatePrintHeaderFooter : public KPrintDialogPage
{
  TQ_OBJECT
  public:
    KatePrintHeaderFooter( KPrinter *printer, TQWidget *parent=0, const char *name=0 );
    ~KatePrintHeaderFooter(){};

    void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
    void setOptions(const TQMap<TQString,TQString>& opts);

  public slots:  
    void setHFFont();
        
  private:
    TQCheckBox *cbEnableHeader, *cbEnableFooter;
    TQLabel *lFontPreview;
    TQString strFont;
    TQGroupBox *gbHeader, *gbFooter;
    TQLineEdit *leHeaderLeft, *leHeaderCenter, *leHeaderRight;
    KColorButton *kcbtnHeaderFg, *kcbtnHeaderBg;
    TQCheckBox *cbHeaderEnableBgColor;
    TQLineEdit *leFooterLeft, *leFooterCenter, *leFooterRight;
    KColorButton *kcbtnFooterFg, *kcbtnFooterBg;
    TQCheckBox *cbFooterEnableBgColor;    
};

//END Header/Footer

//BEGIN Layout
/*
  Layout page:
  - Color scheme
  - Use Box
  - Box properties
    o Width
    o Margin
    o Color
*/
class KatePrintLayout : public KPrintDialogPage
{
  TQ_OBJECT
  public:
    KatePrintLayout( KPrinter *printer, TQWidget *parent=0, const char *name=0 );
    ~KatePrintLayout(){};

    void getOptions(TQMap<TQString,TQString>& opts, bool incldef = false);
    void setOptions(const TQMap<TQString,TQString>& opts);
  
  private:
    TQComboBox *cmbSchema;
    TQCheckBox *cbEnableBox, *cbDrawBackground;
    TQGroupBox *gbBoxProps;
    TQSpinBox *sbBoxWidth, *sbBoxMargin;
    KColorButton* kcbtnBoxColor;
};
//END Layout
#endif //!TQ_WS_WIN

#endif
