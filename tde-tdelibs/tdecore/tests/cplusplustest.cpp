/* This file is part of the KDE libraries
    Copyright (c) 1999 Waldo Bastian <bastian@kde.org>

    This library is free software; you can redistribute it and/or
    modify it under the terms of the GNU Library General Public
    License version 2 as published by the Free Software Foundation.

    This library is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
    Library General Public License for more details.

    You should have received a copy of the GNU Library General Public License
    along with this library; see the file COPYING.LIB.  If not, write to
    the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
    Boston, MA 02110-1301, USA.
*/

#include <tqstring.h>
#include <tqdict.h>

class A { int foo; };
class B { int bar; };
class C : public A, public B { int foobar; };

TQDict<A> dictA;
TQDict<B> dictB;

int main(int , char *[])
{
  C obj;
  A *pA = &obj;
  B *pB = &obj;
  C *pC = &obj;
tqWarning("pA = %p, pB = %p, pC = %p", pA, pB, pC);
  if (pA == pC) tqWarning("pA == pC");
  if (pB == pC) tqWarning("pB == pC");

  dictA.insert("hello", pC);
  dictB.insert("hello", pC);

  if (dictA["hello"] == pC) tqWarning("dictA['hello'] == pC");
  if (dictB["hello"] == pC) tqWarning("dictB['hello'] == pC");
}
