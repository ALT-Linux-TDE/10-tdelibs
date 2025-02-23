#include <kuserprofile.h>
#include <ktrader.h>
#include <kservice.h>
#include <kmimetype.h>
#include <assert.h>
#include <kstandarddirs.h>
#include <kservicegroup.h>
#include <kimageio.h>
#include <kprotocolinfo.h>
#include <kprocess.h>
#include <tqtimer.h>

#include "kdcopcheck.h"
#include <dcopclient.h>

#include <tdeapplication.h>

#include <stdio.h>
#include <stdlib.h>

void debug(TQString txt)
{
 fprintf(stderr, "%s\n", txt.ascii());
}

void debug(const char *txt)
{
 fprintf(stderr, "%s\n", txt);
}
void debug(const char *format, const char *txt)
{
 fprintf(stderr, format, txt);
 fprintf(stderr, "\n");
}

TestService::TestService(const TQString &exec)
{
   m_exec = exec;
   proc << exec;

   proc.start();

   connect(kapp->dcopClient(), TQ_SIGNAL( applicationRegistered(const TQCString&)),
           this, TQ_SLOT(newApp(const TQCString&)));
   connect(kapp->dcopClient(), TQ_SIGNAL( applicationRemoved(const TQCString&)),
           this, TQ_SLOT(endApp(const TQCString&)));
   connect(&proc, TQ_SIGNAL(processExited(TDEProcess *)),
           this, TQ_SLOT(appExit()));

   TQTimer::singleShot(20*1000, this, TQ_SLOT(stop()));
   result = KService::DCOP_None;
}

void TestService::newApp(const TQCString &appId)
{
   TQString id = appId;
   if (id == m_exec)
   {
      result = KService::DCOP_Unique;
      stop();
   }
   else if (id.startsWith(m_exec))
   {
      result = KService::DCOP_Multi;
      stop();
   }
   tqWarning("Register %s", appId.data());
}

void TestService::endApp(const TQCString &appId)
{
   tqWarning("Unegister %s", appId.data());
}

void TestService::appExit()
{
   tqWarning("Exit");
}

void TestService::stop()
{
   kapp->exit_loop();
}

int TestService::exec()
{
   kapp->enter_loop();
   return result;
}

int main(int argc, char *argv[])
{
   setenv("IGNORE_SYCOCA_VERSION", "true", true);
   TDEApplication k(argc,argv,"whatever",false/*nostyle*/,false/*noGUI*/); // KMessageBox needs KApp for makeStdCaption

   k.dcopClient()->setNotifications(true);

   KService::List list = KService::allServices();

   tqWarning("I found %ld services.", list.count());
   int i = 0;
   for(KService::List::ConstIterator it = list.begin(); it != list.end(); ++it)
   {
      if (((*it)->DCOPServiceType() == KService::DCOP_None) &&
          !(*it)->desktopEntryPath().startsWith("SuSE") &&
           (*it)->hasServiceType("Application"))
      {
         if ((*it)->exec().startsWith((*it)->desktopEntryName()))
         {
            i++;

            TestService *test = new TestService((*it)->desktopEntryName());
            int n = test->exec();
            delete test;

            TQString result;
            if (n == KService::DCOP_None)
               result = "None";
            else if (n == KService::DCOP_Unique)
               result = "Unique";
            else if (n == KService::DCOP_Multi)
               result = "Multi";
           
            tqWarning("%s %s", (*it)->desktopEntryPath().latin1(),
                              result.latin1());
         }
      }
   }
   tqWarning("%d left after filtering.", i);
}

#include "kdcopcheck.moc"
