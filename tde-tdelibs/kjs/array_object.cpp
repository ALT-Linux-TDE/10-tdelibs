/*
 *  This file is part of the KDE libraries
 *  Copyright (C) 1999-2000 Harri Porten (porten@kde.org)
 *  Copyright (C) 2003 Apple Computer, Inc.
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

#include "value.h"
#include "object.h"
#include "types.h"
#include "interpreter.h"
#include "operations.h"
#include "array_object.h"
#include "internal.h"
#include "error_object.h"

#include "array_object.lut.h"

#include <stdio.h>
#include <string.h>
#include <assert.h>

#define MAX_INDEX 4294967294U // 2^32-2

using namespace KJS;

// ------------------------------ ArrayInstanceImp -----------------------------

const unsigned sparseArrayCutoff = 10000;

const ClassInfo ArrayInstanceImp::info = {"Array", 0, 0, 0};

ArrayInstanceImp::ArrayInstanceImp(ObjectImp *proto, unsigned initialLength)
  : ObjectImp(proto)
  , length(initialLength)
  , storageLength(initialLength < sparseArrayCutoff ? initialLength : 0)
  , capacity(storageLength)
  , storage(capacity ? (ValueImp **)calloc(capacity, sizeof(ValueImp *)) : 0)
{
}

ArrayInstanceImp::ArrayInstanceImp(ObjectImp *proto, const List &list)
  : ObjectImp(proto)
  , length(list.size())
  , storageLength(length)
  , capacity(storageLength)
  , storage(capacity ? (ValueImp **)malloc(sizeof(ValueImp *) * capacity) : 0)
{
  ListIterator it = list.begin();
  unsigned l = length;
  for (unsigned i = 0; i < l; ++i) {
    storage[i] = (it++).imp();
  }
}

ArrayInstanceImp::~ArrayInstanceImp()
{
  free(storage);
}

Value ArrayInstanceImp::get(ExecState *exec, const Identifier &propertyName) const
{
  if (propertyName == lengthPropertyName)
    return Number(length);

  bool ok;
  unsigned index = propertyName.toArrayIndex(&ok);
  if (ok) {
    if (index >= length)
      return Undefined();
    if (index < storageLength) {
      ValueImp *v = storage[index];
      return v ? Value(v) : Undefined();
    }
  }

  return ObjectImp::get(exec, propertyName);
}

Value ArrayInstanceImp::getPropertyByIndex(ExecState *exec,
					   unsigned index) const
{
  if (index > MAX_INDEX)
    return ObjectImp::get(exec, Identifier::from(index));
  if (index >= length)
    return Undefined();
  if (index < storageLength) {
    ValueImp *v = storage[index];
    return v ? Value(v) : Undefined();
  }

  return ObjectImp::get(exec, Identifier::from(index));
}

// Special implementation of [[Put]] - see ECMA 15.4.5.1
void ArrayInstanceImp::put(ExecState *exec, const Identifier &propertyName, const Value &value, int attr)
{
  if (propertyName == lengthPropertyName) {
    unsigned int newLen = value.toUInt32(exec);
    if (value.toNumber(exec) != double(newLen)) {
      Object err = Error::create(exec, RangeError, "Invalid array length.");
      exec->setException(err);
      return;
    }
    setLength(newLen, exec);
    return;
  }

  bool ok;
  unsigned index = propertyName.toArrayIndex(&ok);
  if (ok) {
    putPropertyByIndex(exec, index, value, attr);
    return;
  }

  ObjectImp::put(exec, propertyName, value, attr);
}

void ArrayInstanceImp::putPropertyByIndex(ExecState *exec, unsigned index,
					  const Value &value, int attr)
{
  if (index < sparseArrayCutoff && index >= storageLength) {
    resizeStorage(index + 1);
  }

  if (index >= length && index <= MAX_INDEX) {
    length = index + 1;
  }

  if (index < storageLength) {
    storage[index] = value.imp();
    return;
  }

  assert(index >= sparseArrayCutoff);
  ObjectImp::put(exec, Identifier::from(index), value, attr);
}

bool ArrayInstanceImp::hasProperty(ExecState *exec, const Identifier &propertyName) const
{
  if (propertyName == lengthPropertyName)
    return true;

  bool ok;
  unsigned index = propertyName.toArrayIndex(&ok);
  if (ok) {
    if (index >= length)
      return false;
    if (index < storageLength) {
      ValueImp *v = storage[index];
      return v && v != UndefinedImp::staticUndefined;
    }
  }

  return ObjectImp::hasProperty(exec, propertyName);
}

bool ArrayInstanceImp::hasPropertyByIndex(ExecState *exec, unsigned index) const
{
  if (index > MAX_INDEX)
    return ObjectImp::hasProperty(exec, Identifier::from(index));
  if (index >= length)
    return false;
  if (index < storageLength) {
    ValueImp *v = storage[index];
    return v && v != UndefinedImp::staticUndefined;
  }

  return ObjectImp::hasProperty(exec, Identifier::from(index));
}

bool ArrayInstanceImp::deleteProperty(ExecState *exec, const Identifier &propertyName)
{
  if (propertyName == lengthPropertyName)
    return false;

  bool ok;
  unsigned index = propertyName.toArrayIndex(&ok);
  if (ok) {
    if (index >= length)
      return true;
    if (index < storageLength) {
      storage[index] = 0;
      return true;
    }
  }

  return ObjectImp::deleteProperty(exec, propertyName);
}

bool ArrayInstanceImp::deletePropertyByIndex(ExecState *exec, unsigned index)
{
  if (index > MAX_INDEX)
    return ObjectImp::deleteProperty(exec, Identifier::from(index));
  if (index >= length)
    return true;
  if (index < storageLength) {
    storage[index] = 0;
    return true;
  }

  return ObjectImp::deleteProperty(exec, Identifier::from(index));
}

ReferenceList ArrayInstanceImp::propList(ExecState *exec, bool recursive)
{
  ReferenceList properties = ObjectImp::propList(exec,recursive);

  // avoid fetching this every time through the loop
  ValueImp *undefined = UndefinedImp::staticUndefined;

  for (unsigned i = 0; i < storageLength; ++i) {
    ValueImp *imp = storage[i];
    if (imp && imp != undefined && !ObjectImp::hasProperty(exec,Identifier::from(i))) {
      properties.append(Reference(this, i));
    }
  }
  return properties;
}

void ArrayInstanceImp::resizeStorage(unsigned newLength)
{
    if (newLength < storageLength) {
      memset(storage + newLength, 0, sizeof(ValueImp *) * (storageLength - newLength));
    }
    if (newLength > capacity) {
      unsigned newCapacity;
      if (newLength > sparseArrayCutoff) {
        newCapacity = newLength;
      } else {
        newCapacity = (newLength * 3 + 1) / 2;
        if (newCapacity > sparseArrayCutoff) {
          newCapacity = sparseArrayCutoff;
        }
      }
      storage = (ValueImp **)realloc(storage, newCapacity * sizeof (ValueImp *));
      memset(storage + capacity, 0, sizeof(ValueImp *) * (newCapacity - capacity));
      capacity = newCapacity;
    }
    storageLength = newLength;
}

void ArrayInstanceImp::setLength(unsigned newLength, ExecState *exec)
{
  if (newLength <= storageLength) {
    resizeStorage(newLength);
  }

  if (newLength < length) {
    ReferenceList sparseProperties;

    _prop.addSparseArrayPropertiesToReferenceList(sparseProperties, Object(this));

    ReferenceListIterator it = sparseProperties.begin();
    while (it != sparseProperties.end()) {
      Reference ref = it++;
      bool ok;
      unsigned index = ref.getPropertyName(exec).toArrayIndex(&ok);
      if (ok && index > newLength) {
	ref.deleteValue(exec);
      }
    }
  }

  length = newLength;
}

void ArrayInstanceImp::mark()
{
  ObjectImp::mark();
  unsigned l = storageLength;
  for (unsigned i = 0; i < l; ++i) {
    ValueImp *imp = storage[i];
    if (imp && !imp->marked())
      imp->mark();
  }
}

static ExecState *execForCompareByStringForQSort;

static int compareByStringForQSort(const void *a, const void *b)
{
    ExecState *exec = execForCompareByStringForQSort;
    ValueImp *va = *(ValueImp **)a;
    ValueImp *vb = *(ValueImp **)b;
    if (va->dispatchType() == UndefinedType) {
        return vb->dispatchType() == UndefinedType ? 0 : 1;
    }
    if (vb->dispatchType() == UndefinedType) {
        return -1;
    }
    return compare(va->dispatchToString(exec), vb->dispatchToString(exec));
}

void ArrayInstanceImp::sort(ExecState *exec)
{
    int lengthNotIncludingUndefined = pushUndefinedObjectsToEnd(exec);

    execForCompareByStringForQSort = exec;
    qsort(storage, lengthNotIncludingUndefined, sizeof(ValueImp *), compareByStringForQSort);
    execForCompareByStringForQSort = 0;
}

namespace KJS {

struct CompareWithCompareFunctionArguments {
    CompareWithCompareFunctionArguments(ExecState *e, ObjectImp *cf)
        : exec(e)
        , compareFunction(cf)
        , globalObject(e->dynamicInterpreter()->globalObject())
    {
        arguments.append(Undefined());
        arguments.append(Undefined());
    }

    ExecState *exec;
    ObjectImp *compareFunction;
    List arguments;
    Object globalObject;
};

}

static CompareWithCompareFunctionArguments *compareWithCompareFunctionArguments;

static int compareWithCompareFunctionForQSort(const void *a, const void *b)
{
    CompareWithCompareFunctionArguments *args = compareWithCompareFunctionArguments;

    ValueImp *va = *(ValueImp **)a;
    ValueImp *vb = *(ValueImp **)b;
    if (va->dispatchType() == UndefinedType) {
        return vb->dispatchType() == UndefinedType ? 0 : 1;
    }
    if (vb->dispatchType() == UndefinedType) {
        return -1;
    }

    args->arguments.clear();
    args->arguments.append(va);
    args->arguments.append(vb);
    double compareResult = args->compareFunction->call
	(args->exec, args->globalObject, args->arguments).toNumber(args->exec);
    return compareResult < 0 ? -1 : compareResult > 0 ? 1 : 0;
}

void ArrayInstanceImp::sort(ExecState *exec, Object &compareFunction)
{
    int lengthNotIncludingUndefined = pushUndefinedObjectsToEnd(exec);

    CompareWithCompareFunctionArguments args(exec, compareFunction.imp());
    compareWithCompareFunctionArguments = &args;
    qsort(storage, lengthNotIncludingUndefined, sizeof(ValueImp *), compareWithCompareFunctionForQSort);
    compareWithCompareFunctionArguments = 0;
}

unsigned ArrayInstanceImp::pushUndefinedObjectsToEnd(ExecState *exec)
{
    ValueImp *undefined = UndefinedImp::staticUndefined;

    unsigned o = 0;

    for (unsigned i = 0; i != storageLength; ++i) {
        ValueImp *v = storage[i];
        if (v && v != undefined) {
            if (o != i)
                storage[o] = v;
            o++;
        }
    }

    ReferenceList sparseProperties;
    _prop.addSparseArrayPropertiesToReferenceList(sparseProperties, Object(this));
    unsigned newLength = o + sparseProperties.length();

    if (newLength > storageLength) {
      resizeStorage(newLength);
    }

    ReferenceListIterator it = sparseProperties.begin();
    while (it != sparseProperties.end()) {
      Reference ref = it++;
      storage[o] = ref.getValue(exec).imp();
      ObjectImp::deleteProperty(exec, ref.getPropertyName(exec));
      o++;
    }

    if (newLength != storageLength)
        memset(storage + o, 0, sizeof(ValueImp *) * (storageLength - o));

    return o;
}

// ------------------------------ ArrayPrototypeImp ----------------------------

const ClassInfo ArrayPrototypeImp::info = {"Array", &ArrayInstanceImp::info, &arrayTable, 0};

/* Source for array_object.lut.h
@begin arrayTable 17
  toString       ArrayProtoFuncImp::ToString       DontEnum|Function 0
  toLocaleString ArrayProtoFuncImp::ToLocaleString DontEnum|Function 0
  concat         ArrayProtoFuncImp::Concat         DontEnum|Function 1
  join           ArrayProtoFuncImp::Join           DontEnum|Function 1
  pop            ArrayProtoFuncImp::Pop            DontEnum|Function 0
  push           ArrayProtoFuncImp::Push           DontEnum|Function 1
  reverse        ArrayProtoFuncImp::Reverse        DontEnum|Function 0
  shift          ArrayProtoFuncImp::Shift          DontEnum|Function 0
  slice          ArrayProtoFuncImp::Slice          DontEnum|Function 2
  sort           ArrayProtoFuncImp::Sort           DontEnum|Function 1
  splice         ArrayProtoFuncImp::Splice         DontEnum|Function 2
  unshift        ArrayProtoFuncImp::UnShift        DontEnum|Function 1
@end
*/

// ECMA 15.4.4
ArrayPrototypeImp::ArrayPrototypeImp(ExecState */*exec*/,
                                     ObjectPrototypeImp *objProto)
  : ArrayInstanceImp(objProto, 0)
{
  Value protect(this);
  setInternalValue(Null());
}

Value ArrayPrototypeImp::get(ExecState *exec, const Identifier &propertyName) const
{
  //fprintf( stderr, "[kjs-array_object] ArrayPrototypeImp::get(%s)\n", propertyName.ascii() );
  return lookupGetFunction<ArrayProtoFuncImp, ArrayInstanceImp>( exec, propertyName, &arrayTable, this );
}

// ------------------------------ ArrayProtoFuncImp ----------------------------

ArrayProtoFuncImp::ArrayProtoFuncImp(ExecState *exec, int i, int len)
  : InternalFunctionImp(
    static_cast<FunctionPrototypeImp*>(exec->lexicalInterpreter()->builtinFunctionPrototype().imp())
    ), id(i)
{
  Value protect(this);
  put(exec,lengthPropertyName,Number(len),DontDelete|ReadOnly|DontEnum);
}

bool ArrayProtoFuncImp::implementsCall() const
{
  return true;
}

// ECMA 15.4.4
Value ArrayProtoFuncImp::call(ExecState *exec, Object &thisObj, const List &args)
{
  unsigned int length = thisObj.get(exec,lengthPropertyName).toUInt32(exec);

  Value result;
  switch (id) {
  case ToLocaleString:
  case ToString:

    if (!thisObj.inherits(&ArrayInstanceImp::info)) {
      Object err = Error::create(exec,TypeError);
      exec->setException(err);
      return err;
    }

    // fall through
  case Join: {
    UString separator = ",";
    UString str = "";

    if (id == Join && args.size() > 0 && !args[0].isA(UndefinedType))
      separator = args[0].toString(exec);
    for (unsigned int k = 0; k < length; k++) {
      if (k >= 1)
        str += separator;

      Value element = thisObj.get(exec, k);
      if (element.type() == UndefinedType || element.type() == NullType)
        continue;

      bool fallback = false;
      if (id == ToLocaleString) {
	Object o = element.toObject(exec);
	Object conversionFunction =
	  Object::dynamicCast(o.get(exec, toLocaleStringPropertyName));
	if (conversionFunction.isValid() &&
	    conversionFunction.implementsCall()) {
	  str += conversionFunction.call(exec, o, List()).toString(exec);
	} else {
	  // try toString() fallback
	  fallback = true;
	}
      }
      if (id == ToString || id == Join || fallback) {
	if (element.type() == ObjectType) {
	  Object o = Object::dynamicCast(element);
	  Object conversionFunction =
	    Object::dynamicCast(o.get(exec, toStringPropertyName));
	  if (conversionFunction.isValid() &&
	      conversionFunction.implementsCall()) {
	    str += conversionFunction.call(exec, o, List()).toString(exec);
	  } else {
	    UString msg = "Can't convert " + o.className() +
	      " object to string";
	    Object error = Error::create(exec, RangeError,
					 msg.cstring().c_str());
	    exec->setException(error);
	    return error;
	  }
	} else {
	  str += element.toString(exec);
	}
      }
      if ( exec->hadException() )
        break;
    }
    result = String(str);
    break;
  }
  case Concat: {
    Object arr = Object::dynamicCast(exec->lexicalInterpreter()->builtinArray().construct(exec,List::empty()));
    int n = 0;
    Value curArg = thisObj;
    Object curObj = Object::dynamicCast(thisObj);
    ListIterator it = args.begin();
    for (;;) {
      if (curArg.type() == ObjectType &&
          curObj.inherits(&ArrayInstanceImp::info)) {
        unsigned int k = 0;
        // Older versions tried to optimize out getting the length of thisObj
        // by checking for n != 0, but that doesn't work if thisObj is an empty array.
        length = curObj.get(exec,lengthPropertyName).toUInt32(exec);
        while (k < length) {
          if (curObj.hasProperty(exec,k))
            arr.put(exec, n, curObj.get(exec, k));
          n++;
          k++;
        }
      } else {
        arr.put(exec, n, curArg);
        n++;
      }
      if (it == args.end())
        break;
      curArg = *it;
      curObj = Object::dynamicCast(it++); // may be 0
    }
    arr.put(exec,lengthPropertyName, Number(n), DontEnum | DontDelete);

    result = arr;
    break;
  }
  case Pop:{
    if (length == 0) {
      thisObj.put(exec, lengthPropertyName, Number(length), DontEnum | DontDelete);
      result = Undefined();
    } else {
      result = thisObj.get(exec, length - 1);
      thisObj.put(exec, lengthPropertyName, Number(length - 1), DontEnum | DontDelete);
    }
    break;
  }
  case Push: {
    for (int n = 0; n < args.size(); n++)
      thisObj.put(exec, length + n, args[n]);
    length += args.size();
    thisObj.put(exec,lengthPropertyName, Number(length), DontEnum | DontDelete);
    result = Number(length);
    break;
  }
  case Reverse: {

    unsigned int middle = length / 2;

    for (unsigned int k = 0; k < middle; k++) {
      unsigned lk1 = length - k - 1;
      Value obj = thisObj.get(exec,k);
      Value obj2 = thisObj.get(exec,lk1);
      if (thisObj.hasProperty(exec,lk1)) {
        if (thisObj.hasProperty(exec,k)) {
          thisObj.put(exec, k, obj2);
          thisObj.put(exec, lk1, obj);
        } else {
          thisObj.put(exec, k, obj2);
          thisObj.deleteProperty(exec, lk1);
        }
      } else {
        if (thisObj.hasProperty(exec, k)) {
          thisObj.deleteProperty(exec, k);
          thisObj.put(exec, lk1, obj);
        } else {
          // why delete something that's not there ? Strange.
          thisObj.deleteProperty(exec, k);
          thisObj.deleteProperty(exec, lk1);
        }
      }
    }
    result = thisObj;
    break;
  }
  case Shift: {
    if (length == 0) {
      thisObj.put(exec, lengthPropertyName, Number(length), DontEnum | DontDelete);
      result = Undefined();
    } else {
      result = thisObj.get(exec, 0);
      for(unsigned int k = 1; k < length; k++) {
        if (thisObj.hasProperty(exec, k)) {
          Value obj = thisObj.get(exec, k);
          thisObj.put(exec, k-1, obj);
        } else
          thisObj.deleteProperty(exec, k-1);
      }
      thisObj.deleteProperty(exec, length - 1);
      thisObj.put(exec, lengthPropertyName, Number(length - 1), DontEnum | DontDelete);
    }
    break;
  }
  case Slice: {
    // http://developer.netscape.com/docs/manuals/js/client/jsref/array.htm#1193713 or 15.4.4.10

    // We return a new array
    Object resObj = Object::dynamicCast(exec->lexicalInterpreter()->builtinArray().construct(exec,List::empty()));
    result = resObj;
    int begin = 0;
    if (args[0].type() != UndefinedType) {
      begin = args[0].toInteger(exec);
      if ( begin < 0 )
        begin = maxInt( begin + length, 0 );
      else
        begin = minInt( begin, length );
    }
    int end = length;
    if (args[1].type() != UndefinedType)
    {
      end = args[1].toInteger(exec);
      if ( end < 0 )
        end = maxInt( end + length, 0 );
      else
        end = minInt( end, length );
    }

    //printf( "Slicing from %d to %d \n", begin, end );
    int n = 0;
    for(int k = begin; k < end; k++, n++) {
      if (thisObj.hasProperty(exec, k)) {
        Value obj = thisObj.get(exec, k);
        resObj.put(exec, n, obj);
      }
    }
    resObj.put(exec, lengthPropertyName, Number(n), DontEnum | DontDelete);
    break;
  }
  case Sort:{
#if 0
    printf("KJS Array::Sort length=%d\n", length);
    for ( unsigned int i = 0 ; i<length ; ++i )
      printf("KJS Array::Sort: %d: %s\n", i, thisObj.get(exec, i).toString(exec).ascii() );
#endif
    Object sortFunction;
    bool useSortFunction = (args[0].type() != UndefinedType);
    if (useSortFunction)
      {
        sortFunction = args[0].toObject(exec);
        if (!sortFunction.implementsCall())
          useSortFunction = false;
      }

    if (thisObj.imp()->classInfo() == &ArrayInstanceImp::info) {
      if (useSortFunction)
        ((ArrayInstanceImp *)thisObj.imp())->sort(exec, sortFunction);
      else
        ((ArrayInstanceImp *)thisObj.imp())->sort(exec);
      result = thisObj;
      break;
    }

    if (length == 0) {
      thisObj.put(exec, lengthPropertyName, Number(0), DontEnum | DontDelete);
      result = thisObj;
      break;
    }

    // "Min" sort. Not the fastest, but definitely less code than heapsort
    // or quicksort, and much less swapping than bubblesort/insertionsort.
    for ( unsigned int i = 0 ; i<length-1 ; ++i )
      {
        Value iObj = thisObj.get(exec,i);
        unsigned int themin = i;
        Value minObj = iObj;
        for ( unsigned int j = i+1 ; j<length ; ++j )
          {
            Value jObj = thisObj.get(exec,j);
            double cmp;
            if (jObj.type() == UndefinedType) {
              cmp = 1; // don't check minObj because there's no need to differentiate == (0) from > (1)
            } else if (minObj.type() == UndefinedType) {
              cmp = -1;
            } else if (useSortFunction) {
                List l;
                l.append(jObj);
                l.append(minObj);
                cmp = sortFunction.call(exec, exec->dynamicInterpreter()->globalObject(), l).toNumber(exec);
            } else {
              cmp = (jObj.toString(exec) < minObj.toString(exec)) ? -1 : 1;
            }
            if ( cmp < 0 )
              {
                themin = j;
                minObj = jObj;
              }
          }
        // Swap themin and i
        if ( themin > i )
          {
            //printf("KJS Array::Sort: swapping %d and %d\n", i, themin );
            thisObj.put( exec, i, minObj );
            thisObj.put( exec, themin, iObj );
          }
      }
#if 0
    printf("KJS Array::Sort -- Resulting array:\n");
    for ( unsigned int i = 0 ; i<length ; ++i )
      printf("KJS Array::Sort: %d: %s\n", i, thisObj.get(exec, i).toString(exec).ascii() );
#endif
    result = thisObj;
    break;
  }
  case Splice: {
    // 15.4.4.12 - oh boy this is huge
    Object resObj = Object::dynamicCast(exec->lexicalInterpreter()->builtinArray().construct(exec,List::empty()));
    result = resObj;
    int begin = args[0].toUInt32(exec);
    if ( begin < 0 )
      begin = maxInt( begin + length, 0 );
    else
      begin = minInt( begin, length );
    unsigned int deleteCount = minInt( maxInt( args[1].toUInt32(exec), 0 ), length - begin );

    //printf( "Splicing from %d, deleteCount=%d \n", begin, deleteCount );
    for(unsigned int k = 0; k < deleteCount; k++) {
      if (thisObj.hasProperty(exec,k+begin)) {
        Value obj = thisObj.get(exec, k+begin);
        resObj.put(exec, k, obj);
      }
    }
    resObj.put(exec, lengthPropertyName, Number(deleteCount), DontEnum | DontDelete);

    unsigned int additionalArgs = maxInt( args.size() - 2, 0 );
    if ( additionalArgs != deleteCount )
    {
      if ( additionalArgs < deleteCount )
      {
        for ( unsigned int k = begin; k < length - deleteCount; ++k )
        {
          if (thisObj.hasProperty(exec,k+deleteCount)) {
            Value obj = thisObj.get(exec, k+deleteCount);
            thisObj.put(exec, k+additionalArgs, obj);
          }
          else
            thisObj.deleteProperty(exec, k+additionalArgs);
        }
        for ( unsigned int k = length ; k > length - deleteCount + additionalArgs; --k )
          thisObj.deleteProperty(exec, k-1);
      }
      else
      {
        for ( unsigned int k = length - deleteCount; (int)k > begin; --k )
        {
          if (thisObj.hasProperty(exec,k+deleteCount-1)) {
            Value obj = thisObj.get(exec, k+deleteCount-1);
            thisObj.put(exec, k+additionalArgs-1, obj);
          }
          else
            thisObj.deleteProperty(exec, k+additionalArgs-1);
        }
      }
    }
    for ( unsigned int k = 0; k < additionalArgs; ++k )
    {
      thisObj.put(exec, k+begin, args[k+2]);
    }
    thisObj.put(exec, lengthPropertyName, Number(length - deleteCount + additionalArgs), DontEnum | DontDelete);
    break;
  }
  case UnShift: { // 15.4.4.13
    unsigned int nrArgs = args.size();
    for ( unsigned int k = length; k > 0; --k )
    {
      if (thisObj.hasProperty(exec,k-1)) {
        Value obj = thisObj.get(exec, k-1);
        thisObj.put(exec, k+nrArgs-1, obj);
      } else {
        thisObj.deleteProperty(exec, k+nrArgs-1);
      }
    }
    for ( unsigned int k = 0; k < nrArgs; ++k )
      thisObj.put(exec, k, args[k]);
    result = Number(length + nrArgs);
    thisObj.put(exec, lengthPropertyName, result, DontEnum | DontDelete);
    break;
  }
  default:
    assert(0);
    break;
  }
  return result;
}

// ------------------------------ ArrayObjectImp -------------------------------

ArrayObjectImp::ArrayObjectImp(ExecState *exec,
                               FunctionPrototypeImp *funcProto,
                               ArrayPrototypeImp *arrayProto)
  : InternalFunctionImp(funcProto)
{
  Value protect(this);
  // ECMA 15.4.3.1 Array.prototype
  put(exec,prototypePropertyName, Object(arrayProto), DontEnum|DontDelete|ReadOnly);

  // no. of arguments for constructor
  put(exec,lengthPropertyName, Number(1), ReadOnly|DontDelete|DontEnum);
}

bool ArrayObjectImp::implementsConstruct() const
{
  return true;
}

// ECMA 15.4.2
Object ArrayObjectImp::construct(ExecState *exec, const List &args)
{
  // a single numeric argument denotes the array size (!)
  if (args.size() == 1 && args[0].type() == NumberType) {
    unsigned int n = args[0].toUInt32(exec);
    if (n != args[0].toNumber(exec)) {
      Object error = Error::create(exec, RangeError, "Invalid array length.");
      exec->setException(error);
      return error;
    }
    return Object(new ArrayInstanceImp(exec->lexicalInterpreter()->builtinArrayPrototype().imp(), n));
  }

  // otherwise the array is constructed with the arguments in it
  return Object(new ArrayInstanceImp(exec->lexicalInterpreter()->builtinArrayPrototype().imp(), args));
}

bool ArrayObjectImp::implementsCall() const
{
  return true;
}

// ECMA 15.6.1
Value ArrayObjectImp::call(ExecState *exec, Object &/*thisObj*/, const List &args)
{
  // equivalent to 'new Array(....)'
  return construct(exec,args);
}
