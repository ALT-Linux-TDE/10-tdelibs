// Simplest example using two kde calendar systems (gregorian and hijri)
// Carlos Moro <cfmoro@correo.uniovi.es>
// GNU-GPL v.2

#include "kcalendarsystemfactory.h"
#include "kcalendarsystem.h"

#include <tqstringlist.h>

#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include <kdebug.h>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <tdecmdlineargs.h>

class TDELocale;

void test(TQDate & date);

static const char description[] = "KCalendarTest";

static TDECmdLineOptions options[] =
{
  { "help", I18N_NOOP("Prints this help"), 0 },
  { "type hijri|gregorian|jalali|hebrew", I18N_NOOP("Supported calendar types"), 0 },
  { "date <date>", I18N_NOOP("Show day info"), 0 },
};

int main(int argc, char **argv) {

	TQDate date;
	TQString calType, option;
	
        TDEAboutData aboutData( "kcalendartest", "KCalendarTest" ,
                        "0.1", description, TDEAboutData::License_GPL,
                        "(c) 2002, Carlos Moro", 0, 0,
                        "cfmoro@correo.uniovi.es");
  	aboutData.addAuthor("Carlos Moro",0, "cfmoro@correo.uniovi.es");
	

        TDECmdLineArgs::init( argc, argv, &aboutData );
        TDECmdLineArgs::addCmdLineOptions( options ); // Add our own options.

        TDECmdLineArgs *args = TDECmdLineArgs::parsedArgs();

	TDEApplication app(false, false);

        TQStringList lst = KCalendarSystemFactory::calendarSystems();
	kdDebug() << "Supported calendar types: " << endl;
	for (TQStringList::Iterator it = lst.begin(); it != lst.end(); ++it)
            kdDebug() << *it << endl;
        kdDebug() << endl;

	
        if ( args->isSet("type") )
		calType = args->getOption("type");
	
	
	TDEGlobal::locale()->setCalendar(calType);

  /*
   *  If we like to see some date
   *
   */
      	if ( args->isSet("date") ) {
    		option = args->getOption("date");
                date = TDEGlobal::locale()->readDate(option);
  	} else 
    		date = TQDate::currentDate();

	args->clear(); // Free up some memory.
	
	test(date);

	return 0;	
  
	

}

void test(TQDate & date) {

        kdDebug() << "(TDELocale) readDate" << endl;

        kdDebug() << "Created calendar: " << TDEGlobal::locale()->calendar()->calendarName() << endl;

	kdDebug() << "Day name for first day of week is " << TDEGlobal::locale()->calendar()->weekDayName(1) << endl;
	kdDebug() << "Short month name for second month is " << TDEGlobal::locale()->calendar()->weekDayName(1, true) << endl;

	kdDebug() << "Month name for second month is " << TDEGlobal::locale()->calendar()->monthName(2, TDEGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name for second month is " << TDEGlobal::locale()->calendar()->monthName(2, TDEGlobal::locale()->calendar()->year(date), true) << endl;
	kdDebug() << "Month name possessive for second month is " << TDEGlobal::locale()->calendar()->monthNamePossessive(2, TDEGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name possessive for second month is " << TDEGlobal::locale()->calendar()->monthNamePossessive(2, TDEGlobal::locale()->calendar()->year(date), true) << endl;
	kdDebug() << "Month name for fifth month is " << TDEGlobal::locale()->calendar()->monthName(5, TDEGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name for fifth month is " << TDEGlobal::locale()->calendar()->monthName(5, TDEGlobal::locale()->calendar()->year(date), true) << endl;
	kdDebug() << "Month name possessive for fifth month is " << TDEGlobal::locale()->calendar()->monthNamePossessive(5, TDEGlobal::locale()->calendar()->year(date)) << endl;
	kdDebug() << "Short month name possessive for fifth month is " << TDEGlobal::locale()->calendar()->monthNamePossessive(5, TDEGlobal::locale()->calendar()->year(date), true) << endl;

	kdDebug() << "Day for date " << date.toString() << " is " << TDEGlobal::locale()->calendar()->day(date) << endl;
	kdDebug() << "Month for date " << date.toString() << " is " << TDEGlobal::locale()->calendar()->month(date) << endl;
	kdDebug() << "Year for date " << date.toString() << " is " << TDEGlobal::locale()->calendar()->year(date) << endl;

	kdDebug() << "Day for date " << date.toString() << " as a string is " << TDEGlobal::locale()->calendar()->dayString(date, true) << endl;
	kdDebug() << "Month for date " << date.toString() << " as a string is " << TDEGlobal::locale()->calendar()->monthString(date, true) << endl;
	kdDebug() << "Year for date " << date.toString() << " as a string is " << TDEGlobal::locale()->calendar()->yearString(date, true) << endl;

	kdDebug() << "Day of week for date " << date.toString() << " is number " << TDEGlobal::locale()->calendar()->dayOfWeek(date) << endl;
	kdDebug() << "Week name for date " << date.toString() << " is " << TDEGlobal::locale()->calendar()->weekDayName(date) << endl;
	kdDebug() << "Short week name for date " << date.toString() << " is " << TDEGlobal::locale()->calendar()->weekDayName(date, true) << endl;

	kdDebug() << "Month name for date " << date.toString() <<  " is "  << TDEGlobal::locale()->calendar()->monthName(date) << endl;
	kdDebug() << "Short month name for date " << date.toString() << " is "  << TDEGlobal::locale()->calendar()->monthName(date, true) << endl;
	kdDebug() << "Month name possessive for date " << date.toString() <<  " is "  << TDEGlobal::locale()->calendar()->monthNamePossessive(date) << endl;
	kdDebug() << "Short month name possessive for date " << date.toString() << " is "  << TDEGlobal::locale()->calendar()->monthNamePossessive(date, true) << endl;

 	kdDebug() << "It's week number " << TDEGlobal::locale()->calendar()->weekNumber(date) << endl;


	kdDebug() << "(TDELocale) Formatted date: " << TDEGlobal::locale()->formatDate(date) << endl;
	kdDebug() << "(TDELocale) Short formatted date: " << TDEGlobal::locale()->formatDate(date, true) << endl;

	kdDebug() << "That month have : " << TDEGlobal::locale()->calendar()->daysInMonth(date) << " days" << endl;

	kdDebug() << "That year has " << TDEGlobal::locale()->calendar()->monthsInYear(date) << " months" << endl;
	kdDebug() << "There are " << TDEGlobal::locale()->calendar()->weeksInYear(TDEGlobal::locale()->calendar()->year(date)) << " weeks that year" << endl;
	kdDebug() << "There are " << TDEGlobal::locale()->calendar()->daysInYear(date) << " days that year" << endl;
	
	kdDebug() << "The day of pray is number " << TDEGlobal::locale()->calendar()->weekDayOfPray() << endl;
	
	kdDebug() << "Max valid year supported is " << TDEGlobal::locale()->calendar()->maxValidYear() << endl;
	kdDebug() << "Min valid year supported is " << TDEGlobal::locale()->calendar()->minValidYear() << endl;
	
	kdDebug() << "It's the day number " << TDEGlobal::locale()->calendar()->dayOfYear(date) << " of year" << endl;
	
	kdDebug() << "Add 3 days" << endl;
	date = TDEGlobal::locale()->calendar()->addDays(date, 3);
	kdDebug() << "It's " << TDEGlobal::locale()->formatDate(date) << endl;

	kdDebug() << "Then add 3 months" << endl;
	date = TDEGlobal::locale()->calendar()->addMonths(date, 3);
	kdDebug() << "It's " << TDEGlobal::locale()->formatDate(date) << endl;

	kdDebug() << "And last, add -3 years" << endl;
	date = TDEGlobal::locale()->calendar()->addYears(date, -3);
	kdDebug() << "It's " << TDEGlobal::locale()->formatDate(date) << endl;
	
	kdDebug() << "Is lunar based: " << TDEGlobal::locale()->calendar()->isLunar() << endl;
	kdDebug() << "Is lunisolar based: " << TDEGlobal::locale()->calendar()->isLunisolar() << endl;
	kdDebug() << "Is solar based: " << TDEGlobal::locale()->calendar()->isSolar() << endl;

}
