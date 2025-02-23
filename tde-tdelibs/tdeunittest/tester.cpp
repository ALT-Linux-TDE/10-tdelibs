/*
 * Copyright (C)  2005  Jeroen Wijnhout <Jeroen.Wijnhout@kdemail.net>
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *   notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *   notice, this list of conditions and the following disclaimer in the
 *   documentation and/or other materials provided with the distribution.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR
 * IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES
 * OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED.
 * IN NO EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE,
 * DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
 * THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF
 * THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#include <iostream>
using namespace std;

#include <tqmetaobject.h>

#include "tester.h"

namespace KUnitTest
{
    SlotTester::SlotTester(const char *name) : Tester(name)
    {
        m_resultsList.setAutoDelete(true);
        m_total = m_results;
    }

    void SlotTester::invokeMember(const TQString &str)
    {
        TQString slotname = TQString::number(TQ_SLOT_CODE) + str;
        connect(this, TQ_SIGNAL(invoke()), this, slotname.ascii());
        emit invoke();
        disconnect(this, TQ_SIGNAL(invoke()), this, slotname.ascii());
    }
    
    void SlotTester::allTests()
    {
        TQStrList allSlots = metaObject()->slotNames();
        
        if ( allSlots.contains("setUp()") > 0 ) invokeMember("setUp()");

        for ( char *sl = allSlots.first(); sl; sl = allSlots.next() ) 
        {
            TQString str = sl;
           
            if ( str.startsWith("test") )
            {
                m_results = results(sl);
                m_results->clear();

                cout << "KUnitTest_Debug_BeginSlot[" << sl << "]" << endl;
                invokeMember(str);
                cout << "KUnitTest_Debug_EndSlot[" << sl << "]" << endl;
            }
        }

        if ( allSlots.contains("tearDown()") > 0 ) invokeMember("tearDown()");
    
        m_total->clear();        
    }
    
    TestResults *SlotTester::results(const char *sl) 
    {
        if ( m_resultsList.find(sl) == 0L ) m_resultsList.insert(sl, new TestResults());
    
        return m_resultsList[sl]; 
    }
}

TQTextStream& operator<<( TQTextStream& str, const TQRect& r ) {
    str << "[" << r.x() << "," << r.y() << " - " << r.width() << "x" << r.height() << "]";
    return str;
}

TQTextStream& operator<<( TQTextStream& str, const TQPoint& r ) {
    str << "(" << r.x() << "," << r.y() << ")";
    return str;
}

TQTextStream& operator<<( TQTextStream& str, const TQSize& r ) {
    str << "[" << r.width() << "x" << r.height() << "]";
    return str;
}

#include "tester.moc"

