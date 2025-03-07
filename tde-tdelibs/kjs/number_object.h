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

#ifndef _NUMBER_OBJECT_H_
#define _NUMBER_OBJECT_H_

#include "internal.h"
#include "function_object.h"

namespace KJS {

  class NumberInstanceImp : public ObjectImp {
  public:
    NumberInstanceImp(ObjectImp *proto);

    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
  };

  /**
   * @internal
   *
   * The initial value of Number.prototype (and thus all objects created
   * with the Number constructor
   */
  class NumberPrototypeImp : public NumberInstanceImp {
  public:
    NumberPrototypeImp(ExecState *exec,
                       ObjectPrototypeImp *objProto,
                       FunctionPrototypeImp *funcProto);
  };

  /**
   * @internal
   *
   * Class to implement all methods that are properties of the
   * Number.prototype object
   */
  class NumberProtoFuncImp : public InternalFunctionImp {
  public:
    NumberProtoFuncImp(ExecState *exec, FunctionPrototypeImp *funcProto,
                       int i, int len, const Identifier &_ident);

    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    enum { ToString, ToLocaleString, ValueOf, ToFixed, ToExponential, ToPrecision };
  private:
    int id;
  };

  /**
   * @internal
   *
   * The initial value of the the global variable's "Number" property
   */
  class NumberObjectImp : public InternalFunctionImp {
  public:
    NumberObjectImp(ExecState *exec,
                    FunctionPrototypeImp *funcProto,
                    NumberPrototypeImp *numberProto);

    virtual bool implementsConstruct() const;
    virtual Object construct(ExecState *exec, const List &args);

    virtual bool implementsCall() const;
    virtual Value call(ExecState *exec, Object &thisObj, const List &args);

    Value get(ExecState *exec, const Identifier &p) const;
    Value getValueProperty(ExecState *exec, int token) const;
    virtual const ClassInfo *classInfo() const { return &info; }
    static const ClassInfo info;
    enum { NaNValue, NegInfinity, PosInfinity, MaxValue, MinValue };

    Completion execute(const List &);
    Object construct(const List &);
  };

} // namespace

#endif
