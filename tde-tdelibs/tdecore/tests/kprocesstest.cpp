//
//  MAIN -- a little demo of the capabilities of the "TDEProcess" class
//
//  version 0.2, Aug 2nd 1997
//  $Id$
//
//  (C) Christian Czezatke
//  e9025461@student.tuwien.ac.at
//


#include "kprocess.h"

#include <stdio.h>
#include <string.h>
#include <tdeapplication.h>

#include <signal.h>

#include "kprocesstest.h"

#define PROCNO 10


//
// A nice input for "sort"... ;- )
//
static const char txt[] = "hat\nder\nalte\nhexenmeister\nsich\ndoch\neinmal\nwegbegeben\n\
und\nnun\nsollen\nseine\ngeister\nsich\nnach\nmeinem\nwillen\nregen\nseine\nwort\nund\n\
werke\nmerkt\nich\nund\nden\nbrauch\nund\nmit\ngeistesstaerke\ntu\nich\nwunder\nauch\n";


int main(int argc, char *argv[])
{
 TDEProcess p1, p2, p3, p4;
 Dummy dummy; 
 TDEApplication app(argc, argv, TQCString("kprocesstest"));


 printf("Welcome to the TDEProcess Demo Application!\n");

 //
 // The kghostview demo -- Starts a kghostview instance blocking. -- After
 // kghostview has exited, kghostview is restarted non-blocking. When the process exits, 
 // the signal "processExited" will be emitted.
 //

 p1 << "kghostview";
 TQObject::connect(&p1, TQ_SIGNAL(processExited(TDEProcess *)),  &dummy, TQ_SLOT(printMessage(TDEProcess *)));
 
 printf("starting kghostview blocking (close to continue)\n");
 p1.start(TDEProcess::Block);
 printf("restarting kghostview non blocking\n");
 p1.start();


 //
 // A konsole with tcsh to demonstrate how to pass command line options to a process
 // with "TDEProcess" (is run blocking)
 //

 printf("Starting konsole with /bin/tcsh as shell (close to continue)\n");
 p2 << "konsole" << "-e" << "/bin/tcsh";
 p2.setWorkingDirectory("/tmp");
 TQObject::connect(&p2, TQ_SIGNAL(processExited(TDEProcess *)),  &dummy, TQ_SLOT(printMessage(TDEProcess *)));
 p2.start(TDEProcess::Block);

 //
 // Getting the output from a process. "ls" with parameter "-l" is called and it output is captured
 //

 p3 << "ls" << "-l";
 TQObject::connect(&p3, TQ_SIGNAL(processExited(TDEProcess *)), 
		  &dummy, TQ_SLOT(printMessage(TDEProcess *)));

 TQObject::connect(&p3, TQ_SIGNAL(receivedStdout(TDEProcess *, char *, int)),
		  &dummy, TQ_SLOT(gotOutput(TDEProcess *, char *, int)));
 TQObject::connect(&p3, TQ_SIGNAL(receivedStderr(TDEProcess *, char *, int)),
		  &dummy, TQ_SLOT(gotOutput(TDEProcess *, char *, int)));

 p3.start(TDEProcess::NotifyOnExit, TDEProcess::AllOutput);
 

 //
 // An even more advanced example of communicating with a child proces. -- A "sort" command
 // is started. After it has been started a list of words (as stored in "txt") is written
 // to its stdin. When the sort command has absorbed all its input it will emit the signal
 // "inputSent". -- This signal is connected to "outputDone" in the Dummy object. 
 // 
 // "OutputDone" will do a "sendEof" to p4. -- This will cause "sort" to perform its task.
 // The output of sort is then captured once more by connecting to the signal "outputWaiting"
 //
 //

 p4 << "sort";
 TQObject::connect(&p4, TQ_SIGNAL(processExited(TDEProcess *)), 
		  &dummy, TQ_SLOT(printMessage(TDEProcess *)));

 TQObject::connect(&p4, TQ_SIGNAL(receivedStdout(TDEProcess *, char *, int)),
		  &dummy, TQ_SLOT(gotOutput(TDEProcess *, char *, int)));
 TQObject::connect(&p4, TQ_SIGNAL(receivedStderr(TDEProcess *, char *, int)),
		  &dummy, TQ_SLOT(gotOutput(TDEProcess *, char *, int)));

 TQObject::connect(&p4, TQ_SIGNAL(wroteStdin(TDEProcess *)),
		  &dummy, TQ_SLOT(outputDone(TDEProcess *)));

 p4.start(TDEProcess::NotifyOnExit, TDEProcess::All);
 printf("after p4.start");
 p4.writeStdin(txt, strlen(txt)); 

 printf("Entering man Qt event loop -- press <CTRL><C> to abort\n");
 app.exec();

 return 0;
}
#include "kprocesstest.moc"
