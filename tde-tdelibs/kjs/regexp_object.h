/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License as published by the Free Software Foundation; either
 *  version 2 of the License, or (at your option) any later version.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
 *
 */

#ifndef _REGEXP_OBJECT_H_
#define _REGEXP_OBJECT_H_

#include "internal.h"
#include "function_object.h"
#include "regexp.h"

namespace KJS {
  class ExecState;
  class RegExpPrototypeImp : public ObjectImp {
  public:
    RegExpPrototypeImp(ExecState *exec,
                       ObjectPrototypeImp *objProto,
                       FunctionPrototypeImp *funcProto);
    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  };

  class RegExpProtoFuncImp : public InternalFunctionImp {
  public:
    RegExpProtoFuncImp(ExecState *exec, FunctionPrototypeImp *funcProto, int i, int len,
                       const Identifier &_ident);

    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    enum { Exec, Test, ToString, Compile };
  private:
    int id;
  };

  class RegExpImp : public ObjectImp {
  public:
    RegExpImp(RegExpPrototypeImp *regexpProto);
    ~RegExpImp();
    void setRegExp(RegExp *r);
    RegExp* regExp() { return reg; }

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  private:
    RegExp *reg;
  };

  class RegExpObjectImp : public InternalFunctionImp {
  public:
    RegExpObjectImp(ExecState *exec,
                    FunctionPrototypeImp *funcProto,
                    RegExpPrototypeImp *regProto);
    virtual ~RegExpObjectImp();
    virtual bool implementsConstruct() const;
    virtual Object construct(ExecState *exec, const List &args);
    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    Value get(ExecState *exec, const Identifier &p) const;
    bool hasProperty(ExecState *exec, const Identifier &propertyName) const;
    int ** registerRegexp( const RegExp* re, const UString& s );
    void setSubPatterns(int num) { lastNrSubPatterns = num; }
    Object arrayOfMatches(ExecState *exec, const UString &result) const;

    /*
     Attempts to create a new regular expression engine for the string p
     and the flags stored in flagsInput. If this succeeds, it returns the 
     engine. If not, it returns 0, and raises an exception in exec
    */
    static RegExp* makeEngine(ExecState *exec, const UString &p, const Value &flagsInput);
  private:
    UString lastString;
    int *lastOvector;
    unsigned int lastNrSubPatterns;
  };

} // namespace

#endif
