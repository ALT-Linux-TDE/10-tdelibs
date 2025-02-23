//
// $Id$
//
// Author: Jacek Konieczny <jajcus@zeus.polsl.gliwice.pl>
//

#include <stdlib.h>
#include <stdio.h>

#include <tqdatetime.h>
#include <tqlabel.h>

#include <tdeglobal.h>
#include <tdeglobalsettings.h>
#include "tdelocale.h"
#include <tdeapplication.h>
#include <kcharsets.h>
#include <kdebug.h>

#include "klocaletest.h"

bool check(TQString txt, TQString a, TQString b)
{
  if (a.isEmpty())
     a = TQString::null;
  if (b.isEmpty())
     b = TQString::null;
  if (a == b) {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a << "' against expected value '" << b << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

bool checkDate(TQString txt, TQDate a, TQDate b)
{
  if (a == b) {
    kdDebug() << txt << " : checking '" << a.toString() << "' against expected value '" << b.toString() << "'... " << "ok" << endl;
  }
  else {
    kdDebug() << txt << " : checking '" << a.toString() << "' against expected value '" << b.toString() << "'... " << "KO !" << endl;
    exit(1);
  }
  return true;
}

Test::Test( TQWidget *parent, const char *name )
  : TQWidget( parent, name )
{
  setCaption("Testing TDELocale");

  TQWidget *d = tqApp->desktop();
  setGeometry((d->width()-320)>>1, (d->height()-160)>>1, 420, 420);

  createFields();
  show();
}

Test::~Test()
{
  ;
}

void Test::createFields()
{
  TQString string;

  string+="Selected languages: ";
  string+=TDEGlobal::locale()->languages()+"\n";

  // This will show nothing, as there is no klocaletest.mo
  // but you can copy other *.mo file
  string+="Used language: ";
  string+=TDEGlobal::locale()->language()+"\n";
  string+="Locale encoding: ";
  string+=TQString::fromLatin1(TDEGlobal::locale()->encoding())+"\n";

  string+="Localized date and time: ";
  string+=TDEGlobal::locale()->formatDateTime(TQDateTime::currentDateTime());
  string+="\nLocalized monetary numbers: ";
  string+=TDEGlobal::locale()->formatMoney(1234567.89) + " / \n" +TDEGlobal::locale()->formatMoney(-1234567.89);
  // This will not work
  // but you can copy other *.mo file
  string+="\nSome localized strings:\n";
  string+=TQString::fromLatin1("Yes = ")+i18n("Yes")+"\n";
  string+=TQString::fromLatin1("No = ")+i18n("No")+"\n";
  string+=TQString::fromLatin1("Help = ")+i18n("Help")+"\n";
  string+=TQString::fromLatin1("Cancel = ")+i18n("Cancel")+"\n";

  label=new TQLabel(string,this,"Label");
  label->setGeometry(10,10,400,400);
  label->setFont(TDEGlobalSettings::generalFont());
  label->show();
}

int main( int argc, char ** argv )
{
  TDELocale::setMainCatalogue("tdelibs");
  TDEApplication a( argc, argv, TQCString("klocaletest") );

  TDEGlobal::locale()->setLanguage(TQString::fromLatin1("en_US"));
  TDEGlobal::locale()->setCountry(TQString::fromLatin1("C"));
  TDEGlobal::locale()->setThousandsSeparator(TQString::fromLatin1(","));

  TQString formatted;
  formatted = TDEGlobal::locale()->formatNumber( 70 ); check("formatNumber(70)",formatted,"70.00");
  formatted = TDEGlobal::locale()->formatNumber( 70, 0 ); check("formatNumber(70, 0)",formatted,"70");
  formatted = TDEGlobal::locale()->formatNumber( 70.2 ); check("formatNumber(70.2)",formatted,"70.20");
  formatted = TDEGlobal::locale()->formatNumber( 70.24 ); check("formatNumber(70.24)",formatted,"70.24");
  formatted = TDEGlobal::locale()->formatNumber( 70.245 ); check("formatNumber(70.245)",formatted,"70.25"); /*rounded*/
  formatted = TDEGlobal::locale()->formatNumber(1234567.89123456789,8); check("formatNumber(1234567.89123456789,8)",formatted,"1,234,567.89123457");

  formatted = TDEGlobal::locale()->formatNumber("70"); check("formatNumber(\"70\")",formatted,"70.00");
  formatted = TDEGlobal::locale()->formatNumber("70", true, 2); check("formatNumber(\"70\", true, 2)",formatted,"70.00");
  formatted = TDEGlobal::locale()->formatNumber("70", true, 0); check("formatNumber(\"70\", true, 0)",formatted,"70");
  formatted = TDEGlobal::locale()->formatNumber("70.9123", true, 0); check("formatNumber(\"70.9123\", true, 0)",formatted,"71"); /* rounded */
  formatted = TDEGlobal::locale()->formatNumber("-70.2", true, 2); check("formatNumber(\"-70.2\", true, 2)",formatted,"-70.20");
  formatted = TDEGlobal::locale()->formatNumber("+70.24", true, 2); check("formatNumber(\"+70.24\", true, 2)",formatted,"70.24");
  formatted = TDEGlobal::locale()->formatNumber("70.245", true, 2); check("formatNumber(\"70.245\", true, 2)",formatted,"70.25"); /*rounded*/
  formatted = TDEGlobal::locale()->formatNumber("99.996", true, 2); check("formatNumber(\"99.996\", true, 2)",formatted,"100.00"); /*rounded*/
  formatted = TDEGlobal::locale()->formatNumber("12345678901234567.89123456789", false, 0); check("formatNumber(\"12345678901234567.89123456789\", false, 0)",formatted,"12,345,678,901,234,567.89123456789");



  double num;
  bool ok;
  num = TDEGlobal::locale()->readNumber( "12,1", &ok ); check("readNumber(12,1)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,100", &ok ); check("readNumber(12,100)",ok?"yes":"no","yes");
  num = TDEGlobal::locale()->readNumber( "12,100000,000", &ok ); check("readNumber(12,100000,000)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,100000000", &ok ); check("readNumber(12,100000000)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,100000,000", &ok ); check("readNumber(12,100000,000)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,,100,000", &ok ); check("readNumber(12,,100,000)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,1000,000", &ok ); check("readNumber(12,1000,000)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,0000000,000", &ok ); check("readNumber(12,0000000,000)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,0000000", &ok ); check("readNumber(12,0000000)",ok?"yes":"no","no");
  num = TDEGlobal::locale()->readNumber( "12,146,131.12", &ok ); check("readNumber(12,146,131.12)",ok?"yes":"no","yes");
  num = TDEGlobal::locale()->readNumber( "1.12345678912", &ok );
        tqDebug( "%s", TQString::number( num, 'g', 12 ).latin1() ); // warning this is the only way to see all decimals
        check("readNumber(1.12345678912)",ok && num==1.12345678912?"yes":"no","yes");
  // bug 95511
  TDELocale locale(*TDEGlobal::locale());
  locale.setCurrencySymbol("$$");
  num = locale.readMoney("1,234,567.89$$", &ok);
  check("readMoney(1,234,567.89$$)",ok?"yes":"no","yes");
  num = locale.readMoney("-1,234,567.89$$", &ok);
  check("readMoney(-1,234,567.89$$)",ok?"yes":"no","yes");

  TQDate date;
  date.setYMD( 2002, 5, 3 );
  checkDate("readDate( 3, 5, 2002 )",date,TDEGlobal::locale()->readDate( TDEGlobal::locale()->formatDate( date ) ) );
  date = TQDate::currentDate();
  checkDate("readDate( TQDate::currentDate() )",date,TDEGlobal::locale()->readDate( TDEGlobal::locale()->formatDate( date ) ) );

  TQTime time;
  time = TDEGlobal::locale()->readTime( "11:22:33", &ok );
  check("readTime(\"11:22:33\")", (ok && time == TQTime(11, 22, 33)) ?
        "yes" : "no", "yes");
  time = TDEGlobal::locale()->readTime( "11:22", &ok );
  check("readTime(\"11:22\")", (ok && time == TQTime(11, 22, 0)) ?
        "yes" : "no", "yes");
  time = TDEGlobal::locale()->readTime( "foo", &ok );
  check("readTime(\"foo\")", (!ok && !time.isValid()) ?
        "invalid" : "valid", "invalid");

  time = TDEGlobal::locale()->readTime( "11:22:33", TDELocale::WithoutSeconds, &ok );
  check("readTime(\"11:22:33\", WithoutSeconds)", (!ok && !time.isValid()) ?
        "invalid" : "valid", "invalid");
  time = TDEGlobal::locale()->readTime( "11:22", TDELocale::WithoutSeconds, &ok );
  check("readTime(\"11:22\", WithoutSeconds)", (ok && time == TQTime(11, 22, 0)) ?
        "yes" : "no", "yes");

  TDEGlobal::locale()->setTimeFormat( "%H:%M %p" );
  time = TQTime( 0, 22, 33 );
  TQString timeStr = TDEGlobal::locale()->formatTime( time, true /*seconds*/, false /*duration*/ );
  check("formatTime(\"0:22\", as time)", timeStr, "00:22 am" );
  timeStr = TDEGlobal::locale()->formatTime( time, true /*seconds*/, true /*duration*/ );
  check("formatTime(\"0:22\", as duration)", timeStr, "00:22" );

  kdDebug() << "setLanguage C\n";
  TDEGlobal::locale()->setLanguage(TQString::fromLatin1("C"));
  kdDebug() << "C: " << i18n("yes") << " " << i18n("TQAccel", "Space") << endl;

  kdDebug() << "setLanguage de\n";
  TDEGlobal::locale()->setLanguage(TQString::fromLatin1("de"));
  kdDebug() << "de: " << i18n("yes") << " " << i18n("TQAccel", "Space") << endl;


  Test m;
  a.setMainWidget( &m );
  m.show();

  return a.exec();
}

#include "klocaletest.moc"
