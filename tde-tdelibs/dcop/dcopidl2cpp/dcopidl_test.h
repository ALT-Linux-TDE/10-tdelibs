#include <tdelistview.h>
#include "main.h"

#ifndef TEST_H
#define TEST_H

// still todo:
// stub:
//    - includes to super relationship, a bit much to test, needs multiple files?

class TDEUI_EXPORT DefaultTest : public TQObject, virtual public DCOPObject
{
   TQ_OBJECT
   K_DCOP
public:
   DefaultTest();
   void nonDcopFunction( TQString filename, TQString url, TQString text, TQString address, TQString icon );
k_dcop:
   void noArgsTest();
   void argsTest( TQString filename, TQString url );
   void unNamedArgsTest( TQString, TQString );

   void constTest( TQString, TQString ) const;
   TQStringList writeTypeTest( const TQString &, TQPtrList<int> );

   void voidReturnType( TQString filename, TQString url, TQString text, TQString address, TQString icon );
   TQString nonVoidReturnType( TQString filename, TQString text, TQString address );
   int intReturnType( TQString filename, TQString text, TQString address );
   bool boolReturnType( TQString filename, TQString text, TQString address );

   ASYNC asyncTest( TQString filename, TQString text, TQString address );
};

namespace TestNamespace {
   class NamespaceTest
   {
      K_DCOP
   public:
      NamespaceTest();
   k_dcop:
      void function1( TQString filename, TQString url, TQString text, TQString address, TQString icon );
   };
}

class NoSuper
{
   K_DCOP
public:
   NoSuper();
k_dcop:
   void function1( TQString filename, TQString url, TQString text, TQString address, TQString icon );
};

class NonDCOPObject : public MyDCOPObjectBase
{
   K_DCOP
public:
   NonDCOPObject();
k_dcop:
   void function1( TQString filename, TQString url, TQString text, TQString address, TQString icon );
};

class NoFunctions : public DCOPObject
{
   K_DCOP
public:
   NonDCOPObject();
   void nonDcopFunction( TQString filename, TQString url, TQString text, TQString address, TQString icon );
};

class NonHashingTest : public TQObject, virtual public DCOPObject
{
   TQ_OBJECT
   K_DCOP
public:
   NonHashingTest();
k_dcop:
   void function1( TQString );
};

class HashingTest : public TQObject, virtual public DCOPObject
{
   TQ_OBJECT
   K_DCOP
public:
   HashingTest();
k_dcop:
   void function1( TQString );
   void function2( TQString, TQString );
   void function3( TQString, TQString, TQString );
   void function4( TQString, TQString, TQString, TQString );
   void function5( TQString, TQString, TQString, TQString, TQString );
   void function6( TQString, TQString, TQString, TQString, TQString, TQString );
   void function7( TQString, TQString, TQString, TQString, TQString, TQString, TQString );
   void function8( TQString, TQString, TQString, TQString, TQString, TQString, TQString, TQString );
};

class SignalTest : virtual public DCOPObject
{
   K_DCOP
public:
   SignalTest(TQCString objId = "KBookmarkNotifier") : DCOPObject(objId) {}
k_dcop_signals:
   void signal1( TQString filename, TQString url, TQString text, TQString address, TQString icon );
   void signal2( TQString filename, TQString text, TQString address );
   void signal3( TQString filename, TQString url );
};

#endif // end
