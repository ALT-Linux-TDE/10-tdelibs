#include <stdlib.h>
#include <stdio.h>

#include <tdeapplication.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <tqstring.h>

#include "tdewalletbackend.h"

#define CHECK_RETURN(func, test, test_str) { \
    int rc = (func); \
    test_cnt++;\
    if (test) {\
        printf("%-20s returned %d as expected  (should be %s)\n", #func, rc, test_str);\
    } else {\
        printf("%-20s returned %d UNEXPECTEDLY (should be %s)\n", #func, rc, test_str);\
        test_failed++;\
    }\
}

int main(int argc, char **argv) {
   TDEAboutData aboutData( "tdewalletbackendtest", "tdewallet backend testing routine", "0.1" );

   TDECmdLineArgs::init( argc, argv, &aboutData );
   TDEApplication a(false, false);

   TDEWallet::Backend be("ktestwallet");
   printf("TDEWalletBackend constructed\n");

   TQByteArray apass, bpass, cpass;

   apass.duplicate("apassword", 9);
   bpass.duplicate("bpassword", 9);
   cpass.duplicate("cpassword", 9);


   int test_cnt = 0;
   int test_failed = 0;

   printf("Passwords initialised.\n");

   CHECK_RETURN(be.close(apass), rc==-255,       "-255");
   CHECK_RETURN(be.open(bpass),  rc==0 || rc==1, "0 or 1");
   CHECK_RETURN(be.close(bpass), rc==0,          "0 or 1");
   CHECK_RETURN(be.open(apass),  rc<0,           "negative");
   CHECK_RETURN(be.open(bpass),  rc==0,          "0");

   printf ("===========================================\n");
   printf ("%d test failed out of %d\n", test_failed, test_cnt);

   return test_failed == 0 ? EXIT_SUCCESS : EXIT_FAILURE;
}


