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

#ifndef _KJSLOOKUP_H_
#define _KJSLOOKUP_H_

#include "identifier.h"
#include "value.h"
#include "object.h"
#include "interpreter.h"
#include <stdio.h>

namespace KJS {

  /**
   * An entry in a hash table.
   */
  struct HashEntry {
    /**
     * s is the offset to the string key (e.g. a property name)
     */
    unsigned short soffset;
    /**
     * value is the result value (usually an enum value)
     */
    short int value;
    /**
     * attr is a set for flags (e.g. the property flags, see object.h)
     */
    unsigned char attr;
    /**
     * params is another number. For property hashtables, it is used to
     * denote the number of argument of the function
     */
    unsigned char params;
    /**
     * next is the index to the next entry for the same hash value
     */
    short next;
  };

  /**
   * A hash table
   * Usually the hashtable is generated by the create_hash_table script, from a .table file.
   *
   * The implementation uses an array of entries, "size" is the total size of that array.
   * The entries between 0 and hashSize-1 are the entry points
   * for each hash value, and the entries between hashSize and size-1
   * are the overflow entries for the hash values that need one.
   * The "next" pointer of the entry links entry points to overflow entries,
   * and links overflow entries between them.
   */
  struct HashTable {
    /**
     * type is a version number. Currently always 2
     */
    int type;
    /**
     * size is the total number of entries in the hashtable, including the null entries,
     * i.e. the size of the "entries" array.
     * Used to iterate over all entries in the table
     */
    int size;
    /**
     * pointer to the array of entries
     * Mind that some entries in the array are null (0,0,0,0).
     */
    const HashEntry *const entries;
    /**
     * the maximum value for the hash. Always smaller than size.
     */
    int hashSize;

    /**
     * pointer to the string table.
     */
    const char* const sbase;
  };

  /**
   * @short Fast keyword lookup.
   */
  class KJS_EXPORT Lookup {
  public:
    /**
     * Find an entry in the table, and return its value (i.e. the value field of HashEntry)
     */
    static int find(const struct HashTable *table, const Identifier &s);
    static int find(const struct HashTable *table,
		    const UChar *c, unsigned int len);

    /**
     * Find an entry in the table, and return the entry
     * This variant gives access to the other attributes of the entry,
     * especially the attr field.
     */
    static const HashEntry* findEntry(const struct HashTable *table,
                                      const Identifier &s);
    static const HashEntry* findEntry(const struct HashTable *table,
                                      const UChar *c, unsigned int len);

    /**
     * Calculate the hash value for a given key
     */
    static unsigned int hash(const Identifier &key);
    static unsigned int hash(const UChar *c, unsigned int len);
    static unsigned int hash(const char *s);
  };

  class ExecState;
  class UString;
  /**
   * @internal
   * Helper for lookupFunction and lookupValueOrFunction
   */
  template <class FuncImp>
  inline Value lookupOrCreateFunction(ExecState *exec, const Identifier &propertyName,
                                      const ObjectImp *thisObj, int token, int params, int attr)
  {
      // Look for cached value in dynamic map of properties (in ObjectImp)
      ValueImp * cachedVal = thisObj->ObjectImp::getDirect(propertyName);
      /*if (cachedVal)
        fprintf(stderr, "lookupOrCreateFunction: Function -> looked up in ObjectImp, found type=%d\n", cachedVal->type());*/
      if (cachedVal)
        return Value(cachedVal);

      ObjectImp* func = new FuncImp( exec, token, params );
      Value val( func );
      func->setFunctionName( propertyName );
      ObjectImp *thatObj = const_cast<ObjectImp *>(thisObj);
      thatObj->ObjectImp::put(exec, propertyName, val, attr);
      return val;
  }

  /**
   * Helper method for property lookups
   *
   * This method does it all (looking in the hashtable, checking for function
   * overrides, creating the function or retrieving from cache, calling
   * getValueProperty in case of a non-function property, forwarding to parent if
   * unknown property).
   *
   * Template arguments:
   *   - @c FuncImp the class which implements this object's functions
   *   - @c ThisImp the class of "this". It must implement the 
   *        getValueProperty(exec,token) method, for non-function properties.
   *   - @c ParentImp the class of the parent, to propagate the lookup.
   *
   * Method arguments:
   * @param exec execution state, as usual
   * @param propertyName the property we're looking for
   * @param table the static hashtable for this class
   * @param thisObj "this"
   */
  template <class FuncImp, class ThisImp, class ParentImp>
  inline Value lookupGet(ExecState *exec, const Identifier &propertyName,
                         const HashTable* table, const ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found, forward to parent
      return thisObj->ParentImp::get(exec, propertyName);

    //fprintf(stderr, "lookupGet: found value=%d attr=%d\n", entry->value, entry->attr);
    if (entry->attr & Function)
      return lookupOrCreateFunction<FuncImp>(exec, propertyName, thisObj, entry->value, entry->params, entry->attr);
    return thisObj->getValueProperty(exec, entry->value);
  }

  /**
   * Simplified version of lookupGet in case there are only functions.
   * Using this instead of lookupGet prevents 'this' from implementing a dummy getValueProperty.
   */
  template <class FuncImp, class ParentImp>
  inline Value lookupGetFunction(ExecState *exec, const Identifier &propertyName,
                         const HashTable* table, const ObjectImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found, forward to parent
      return static_cast<const ParentImp *>(thisObj)->ParentImp::get(exec, propertyName);

    if (entry->attr & Function)
      return lookupOrCreateFunction<FuncImp>(exec, propertyName, thisObj, entry->value, entry->params, entry->attr);

    fprintf(stderr, "Function bit not set! Shouldn't happen in lookupGetFunction!\n" );
    return Undefined();
  }

  /**
   * Simplified version of lookupGet in case there are no functions, only "values".
   * Using this instead of lookupGet removes the need for a FuncImp class.
   */
  template <class ThisImp, class ParentImp>
  inline Value lookupGetValue(ExecState *exec, const Identifier &propertyName,
                           const HashTable* table, const ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found, forward to parent
      return thisObj->ParentImp::get(exec, propertyName);

    if (entry->attr & Function)
      fprintf(stderr, "Function bit set! Shouldn't happen in lookupGetValue! propertyName was %s\n", propertyName.ascii() );
    return thisObj->getValueProperty(exec, entry->value);
  }

  /**
   * This one is for "put".
   * Lookup hash entry for property to be set, and set the value.
   */
  template <class ThisImp, class ParentImp>
  inline void lookupPut(ExecState *exec, const Identifier &propertyName,
                        const Value& value, int attr,
                        const HashTable* table, ThisImp* thisObj)
  {
    const HashEntry* entry = Lookup::findEntry(table, propertyName);

    if (!entry) // not found: forward to parent
      thisObj->ParentImp::put(exec, propertyName, value, attr);
    else if (entry->attr & Function) // function: put as override property
      thisObj->ObjectImp::put(exec, propertyName, value, attr);
    else if (entry->attr & ReadOnly) // readonly! Can't put!
#ifdef KJS_VERBOSE
      fprintf(stderr,"WARNING: Attempt to change value of readonly property '%s'\n",propertyName.ascii());
#else
      ; // do nothing
#endif
    else
      thisObj->putValueProperty(exec, entry->value, value, attr);
  }


  /**
   * This template method retrieves or create an object that is unique
   * (for a given interpreter) The first time this is called (for a given
   * property name), the Object will be constructed, and set as a property
   * of the interpreter's global object. Later calls will simply retrieve
   * that cached object. Note that the object constructor must take 1 argument, exec.
   */
  template <class ClassCtor>
  inline KJS::Object cacheGlobalObject(ExecState *exec, const Identifier &propertyName)
  {
    ValueImp *obj = static_cast<KJS::ObjectImp*>(exec->interpreter()->globalObject().imp())->getDirect(propertyName);
    if (obj)
      return KJS::Object::dynamicCast(Value(obj));
    else
    {
      KJS::Object newObject(new ClassCtor(exec));
      exec->interpreter()->globalObject().put(exec, propertyName, newObject, Internal);
      return newObject;
    }
  }


  /**
   * Helpers to define prototype objects (each of which simply implements
   * the functions for a type of objects).
   * Sorry for this not being very readable, but it actually saves much copy-n-paste.
   * ParentProto is not our base class, it's the object we use as fallback.
   * The reason for this is that there should only be ONE DOMNode.hasAttributes (e.g.),
   * not one in each derived class. So we link the (unique) prototypes between them.
   *
   * Using those macros is very simple: define the hashtable (e.g. "DOMNodeProtoTable"), then
   * DEFINE_PROTOTYPE("DOMNode",DOMNodeProto)
   * IMPLEMENT_PROTOFUNC(DOMNodeProtoFunc)
   * IMPLEMENT_PROTOTYPE(DOMNodeProto,DOMNodeProtoFunc)
   * and use DOMNodeProto::self(exec) as prototype in the DOMNode constructor.
   * If the prototype has a "parent prototype", e.g. DOMElementProto falls back on DOMNodeProto,
   * then the last line will use IMPLEMENT_PROTOTYPE_WITH_PARENT, with DOMNodeProto as last argument.
   * PUBLIC_DEFINE_PROTOTYPE and PUBLIC_IMPLEMENT_PROTOTYPE are versions with support for separate compilation
   */

#define PUBLIC_DEFINE_PROTOTYPE(ClassName,ClassProto) \
  namespace KJS { \
  class ClassProto : public KJS::ObjectImp { \
    friend KJS::Object cacheGlobalObject<ClassProto>(KJS::ExecState *exec, const KJS::Identifier &propertyName); \
  public: \
    static KJS::Object self(KJS::ExecState *exec) \
    { \
      return cacheGlobalObject<ClassProto>( exec, "[[" ClassName ".prototype]]" ); \
    } \
  protected: \
    ClassProto( KJS::ExecState *exec ) \
      : KJS::ObjectImp( exec->interpreter()->builtinObjectPrototype() ) {} \
    \
  public: \
    virtual const KJS::ClassInfo *classInfo() const { return &info; } \
    static const KJS::ClassInfo info; \
    KJS::Value get(KJS::ExecState *exec, const KJS::Identifier &propertyName) const; \
    bool hasProperty(KJS::ExecState *exec, const KJS::Identifier &propertyName) const; \
  }; \
  }

#define IMPLEMENT_CLASSINFO(ClassName,ClassProto) \
  namespace KJS {\
  const KJS::ClassInfo ClassProto::info = { ClassName, 0, &ClassProto##Table, 0 }; \
  }

#define DEFINE_PROTOTYPE(ClassName,ClassProto) \
  PUBLIC_DEFINE_PROTOTYPE(ClassName,ClassProto) \
  IMPLEMENT_CLASSINFO(ClassName,ClassProto)

#define IMPLEMENT_PROTOTYPE(ClassProto,ClassFunc) \
    KJS::Value KJS::ClassProto::get(KJS::ExecState *exec, const KJS::Identifier &propertyName) const \
    { \
      /*fprintf( stderr, "%sProto::get(%s) [in macro, no parent]\n", info.className, propertyName.ascii());*/ \
      return lookupGetFunction<ClassFunc,KJS::ObjectImp>(exec, propertyName, &ClassProto##Table, this ); \
    } \
    bool KJS::ClassProto::hasProperty(KJS::ExecState *exec, const KJS::Identifier &propertyName) const \
    { /*stupid but we need this to have a common macro for the declaration*/ \
      return KJS::ObjectImp::hasProperty(exec, propertyName); \
    }

#define PUBLIC_IMPLEMENT_PROTOTYPE(ClassProto,ClassName,ClassFunc) \
    IMPLEMENT_PROTOTYPE(ClassProto,ClassFunc)\
    IMPLEMENT_CLASSINFO(ClassName,ClassProto)

#define IMPLEMENT_PROTOTYPE_WITH_PARENT(ClassProto,ClassFunc,ParentProto)  \
    KJS::Value KJS::ClassProto::get(KJS::ExecState *exec, const KJS::Identifier &propertyName) const \
    { \
      /*fprintf( stderr, "%sProto::get(%s) [in macro]\n", info.className, propertyName.ascii());*/ \
      KJS::Value val = lookupGetFunction<ClassFunc,KJS::ObjectImp>(exec, propertyName, &ClassProto##Table, this ); \
      if ( val.type() != UndefinedType ) return val; \
      /* Not found -> forward request to "parent" prototype */ \
      return ParentProto::self(exec).get( exec, propertyName ); \
    } \
    bool KJS::ClassProto::hasProperty(KJS::ExecState *exec, const KJS::Identifier &propertyName) const \
    { \
      if (KJS::ObjectImp::hasProperty(exec, propertyName)) \
        return true; \
      return ParentProto::self(exec).hasProperty(exec, propertyName); \
    }
    
#define PUBLIC_IMPLEMENT_PROTOTYPE_WITH_PARENT(ClassProto,ClassName,ClassFunc,ParentProto)  \
    IMPLEMENT_PROTOTYPE_WITH_PARENT(ClassProto,ClassFunc,ParentProto) \
    IMPLEMENT_CLASSINFO(ClassName,ClassProto)

#define IMPLEMENT_PROTOFUNC(ClassFunc) \
  namespace KJS { \
  class ClassFunc : public ObjectImp { \
  public: \
    ClassFunc(KJS::ExecState *exec, int i, int len) \
       : ObjectImp( /*proto? */ ), id(i) { \
       KJS::Value protect(this); \
       put(exec,lengthPropertyName,Number(len),DontDelete|ReadOnly|DontEnum); \
    } \
    virtual bool implementsCall() const { return true; } \
    /** You need to implement that one */ \
    virtual KJS::Value call(KJS::ExecState *exec, KJS::Object &thisObj, const KJS::List &args); \
  private: \
    int id; \
  }; \
  }

  // To be used in all call() implementations, before casting the type of thisObj
#define KJS_CHECK_THIS( ClassName, theObj ) \
  if (!theObj.isValid() || !theObj.inherits(&ClassName::info)) { \
    KJS::UString errMsg = "Attempt at calling a function that expects a "; \
    errMsg += ClassName::info.className; \
    errMsg += " on a "; \
    errMsg += thisObj.className(); \
    KJS::Object err = KJS::Error::create(exec, KJS::TypeError, errMsg.ascii()); \
    exec->setException(err); \
    return err; \
  }

  /*
   * List of things to do when porting an objectimp to the 'static hashtable' mechanism:
   * - write the hashtable source, between @begin and @end
   * - add a rule to build the .lut.h
   * - include the .lut.h
   * - mention the table in the classinfo (add a classinfo if necessary)
   * - write/update the class enum (for the tokens)
   * - turn get() into getValueProperty(), put() into putValueProperty(), using a switch and removing funcs
   * - write get() and/or put() using a template method
   * - cleanup old stuff (e.g. hasProperty)
   * - compile, test, commit ;)
   */
} // namespace

#endif
