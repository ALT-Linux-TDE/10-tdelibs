#include <kservice.h>

#include <tdeapplication.h>

int main(int argc, char *argv[])
{
   TDEApplication k(argc,argv,"whatever",true,true); // KMessageBox needs KApp for makeStdCaption

   KService::rebuildKSycoca(0);
   return 0;
}
