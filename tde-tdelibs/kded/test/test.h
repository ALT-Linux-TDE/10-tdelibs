/* This code is placed in the public domain */
/*               Waldo Bastian - 2001/04/01 */

#ifndef _TEST_H_
#define _TEST_H_

#include "kdedmodule.h"

class TestModule : public KDEDModule
{
   TQ_OBJECT
   K_DCOP
public:
   TestModule(const TQCString &obj);

   void idle();

k_dcop:
   TQString world();
   void registerMe(const TQCString &app);
};

#endif
