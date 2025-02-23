#include <libtdemid/libtdemid.h>
#include <libtdemid/deviceman.h>
#include <unistd.h>
#include <stdio.h>

int main (int , char **)
{
  printf("Libtdemid Notes Off . (C) 2000 Antonio Larrosa Jimenez . Malaga (Spain)\n");
  printf("Using libtdemid from a simple C++ application\n");

  KMidSimpleAPI::kMidInit();

  kMid.midi->checkInit();
  kMid.midi->openDev();
  kMid.midi->initDev();
  
  kMid.midi->allNotesOff();
  kMid.midi->closeDev();

  KMidSimpleAPI::kMidDestruct();

  return 0;
};

