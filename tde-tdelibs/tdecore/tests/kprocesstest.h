//
//  DUMMY -- A dummy class with a slot to demonstrate TDEProcess signals
//
//  version 0.2, Aug 2nd 1997
//
//  (C) Christian Czezatke
//  e9025461@student.tuwien.ac.at
//


#ifndef __DUMMY_H__
#define __DUMMY_H__

#include <stdio.h>
#include <tqobject.h>
#include "kprocess.h"

class Dummy : public TQObject
{
 TQ_OBJECT

 public slots:
   void printMessage(TDEProcess *proc)
   {
     printf("Process %d exited!\n", (int)proc->getPid()); 
   } 
 
   void gotOutput(TDEProcess*, char *buffer, int len)
   {
    char result[1025];             // this is ugly since it relys on the internal buffer size of TDEProcess,
    memcpy(result, buffer, len);   // NEVER do that in your own application... ;-) 
    result[len] = '\0';
    printf("OUTPUT>>%s", result);
   }

   void outputDone(TDEProcess *proc)
     /*
       Slot Procedure for the "sort" example. -- If it is indicated that the "sort" command has
       absorbed all its input, we send an "EOF" to it to indicate that there is no more
       data to be processed. 
      */
   {
       proc->closeStdin();
   }

};

#endif


