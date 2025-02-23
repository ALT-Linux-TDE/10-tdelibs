#ifndef _OPT_KDECVS_SRC_KDELIBS_DCOP_TEST_TESTER_H
#define _OPT_KDECVS_SRC_KDELIBS_DCOP_TEST_TESTER_H
#include "test_stub.h"
#include <tqobject.h>



class Driver : public TQObject, public Test_stub
{
	TQ_OBJECT

	public:
		Driver(const char*);
	public slots:
		void test();

	private:
	int count;
};

#endif // _OPT_KDE-CVS_SRC_KDELIBS_DCOP_TEST_TESTER_H
