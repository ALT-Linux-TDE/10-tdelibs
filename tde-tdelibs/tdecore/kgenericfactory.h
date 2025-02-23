/* This file is part of the KDE project
 * Copyright (C) 2001 Simon Hausmann <hausmann@kde.org>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Library General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Library General Public License for more details.
 *
 * You should have received a copy of the GNU Library General Public License
 * along with this library; see the file COPYING.LIB.  If not, write to
 * the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 * Boston, MA 02110-1301, USA.
 */
#ifndef __kgenericfactory_h__
#define __kgenericfactory_h__

#include <klibloader.h>
#include <ktypelist.h>
#include <kinstance.h>
#include <kgenericfactory.tcc>
#include <tdeglobal.h>
#include <tdelocale.h>
#include <kdebug.h>

/* @internal */
template <class T>
class KGenericFactoryBase
{
public:
    KGenericFactoryBase( const char *instanceName )
        : m_instanceName( instanceName )
    {
        m_aboutData=0L;
        s_self = this;
        m_catalogueInitialized = false;
    }
    KGenericFactoryBase( const TDEAboutData *data )
        : m_aboutData(data)
    {
        s_self = this;
        m_catalogueInitialized = false;
    }

    virtual ~KGenericFactoryBase()
    {
        if ( s_instance )
            TDEGlobal::locale()->removeCatalogue( TQString::fromAscii( s_instance->instanceName() ) );
        delete s_instance;
        s_instance = 0;
        s_self = 0;
    }

    static TDEInstance *instance();

protected:
    virtual TDEInstance *createInstance()
    {
        if ( m_aboutData )
            return new TDEInstance( m_aboutData );
        if ( m_instanceName.isEmpty() ) {
            kdWarning() << "KGenericFactory: instance requested but no instance name or about data passed to the constructor!" << endl;
            return 0;
        }
        return new TDEInstance( m_instanceName );
    }

    virtual void setupTranslations( void )
    {
        if ( instance() )
            TDEGlobal::locale()->insertCatalogue( TQString::fromAscii( instance()->instanceName() ) );
    }

    void initializeMessageCatalogue()
    {
        if ( !m_catalogueInitialized )
        {
            m_catalogueInitialized = true;
            setupTranslations();
        }
    }

private:
    TQCString m_instanceName;
    const TDEAboutData *m_aboutData;
    bool m_catalogueInitialized;

    static TDEInstance *s_instance;
    static KGenericFactoryBase<T> *s_self;
};

/* @internal */
template <class T>
TDEInstance *KGenericFactoryBase<T>::s_instance = 0;

/* @internal */
template <class T>
KGenericFactoryBase<T> *KGenericFactoryBase<T>::s_self = 0;

/* @internal */
template <class T>
TDEInstance *KGenericFactoryBase<T>::instance()
{
    if ( !s_instance && s_self )
        s_instance = s_self->createInstance();
    return s_instance;
}

/**
 * This template provides a generic implementation of a KLibFactory ,
 * for use with shared library components. It implements the pure virtual
 * createObject method of KLibFactory and instantiates objects of the
 * specified class (template argument) when the class name argument of
 * createObject matches a class name in the given hierarchy.
 *
 * In case you are developing a KParts component, skip this file and
 * go directly to KParts::GenericFactory .
 *
 * Note that the class specified as template argument needs to provide
 * a certain constructor:
 * <ul>
 *     <li>If the class is derived from TQObject then it needs to have
 *         a constructor like:
 *         <code>MyClass( TQObject *parent, const char *name,
 *                        const TQStringList &args );</code>
 *     <li>If the class is derived from TQWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( TQWidget *parent, const char *name,
 *                         const TQStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( TQWidget *parentWidget, const char *widgetName,
 *                       TQObject *parent, const char *name,
 *                       const TQStringList &args );</code>
 * </ul>
 * The args TQStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * TDEInstance object for your component, accessible through the
 * static instance() method. The instanceName argument of the
 * KGenericFactory constructor is passed to the TDEInstance object.
 *
 * The creation of the TDEInstance object can be customized by inheriting
 * from this template class and re-implementing the virtual createInstance
 * method. For example it could look like this:
 * \code
 *     TDEInstance *MyFactory::createInstance()
 *     {
 *         return new TDEInstance( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( TQObject *parent, const char *name,
 *                   const TQStringList &args );
 *         ...
 *     };
 *
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory&lt;MyPlugin&gt; )
 * \endcode
 */
template <class Product, class ParentType = TQObject>
class KGenericFactory : public KLibFactory, public KGenericFactoryBase<Product>
{
public:
    KGenericFactory( const char *instanceName = 0 )
        : KGenericFactoryBase<Product>( instanceName )
    {}

    /**
     * @since 3.3
	*/
    KGenericFactory( const TDEAboutData *data )
        : KGenericFactoryBase<Product>( data )
    {}


protected:
    virtual TQObject *createObject( TQObject *parent, const char *name,
                                  const char *className, const TQStringList &args )
    {
        KGenericFactoryBase<Product>::initializeMessageCatalogue();
        return (KDEPrivate::ConcreteFactory<Product, ParentType>
            ::create( 0, 0, parent, name, className, args ));
    }
};

/**
 * This template provides a generic implementation of a KLibFactory ,
 * for use with shared library components. It implements the pure virtual
 * createObject method of KLibFactory and instantiates objects of the
 * specified classes in the given typelist template argument when the class
 * name argument of createObject matches a class names in the given hierarchy
 * of classes.
 *
 * Note that each class in the specified in the typelist template argument
 * needs to provide a certain constructor:
 * <ul>
 *     <li>If the class is derived from TQObject then it needs to have
 *         a constructor like:
 *         <code>MyClass( TQObject *parent, const char *name,
 *                        const TQStringList &args );</code>
 *     <li>If the class is derived from TQWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( TQWidget *parent, const char *name,
 *                         const TQStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( TQWidget *parentWidget, const char *widgetName,
 *                       TQObject *parent, const char *name,
 *                       const TQStringList &args );</code>
 * </ul>
 * The args TQStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * TDEInstance object for your component, accessible through the
 * static instance() method. The instanceName argument of the
 * KGenericFactory constructor is passed to the TDEInstance object.
 *
 * The creation of the TDEInstance object can be customized by inheriting
 * from this template class and re-implementing the virtual createInstance
 * method. For example it could look like this:
 * \code
 *     TDEInstance *MyFactory::createInstance()
 *     {
 *         return new TDEInstance( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( TQObject *parent, const char *name,
 *                   const TQStringList &args );
 *         ...
 *     };
 *
 *     class MyDialogComponent : public KDialogBase
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyDialogComponent( TQWidget *parentWidget, const char *name,
 *                            const TQStringList &args );
 *         ...
 *     };
 *
 *     typedef K_TYPELIST_2( MyPlugin, MyDialogComponent ) Products;
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory&lt;Products&gt; )
 * \endcode
 */
template <class Product, class ProductListTail>
class KGenericFactory< KTypeList<Product, ProductListTail>, TQObject >
    : public KLibFactory,
      public KGenericFactoryBase< KTypeList<Product, ProductListTail> >
{
public:
    KGenericFactory( const char *instanceName  = 0 )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( instanceName )
    {}

    /**
     * @since 3.3
	*/
    KGenericFactory( const TDEAboutData *data )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( data )
    {}


protected:
    virtual TQObject *createObject( TQObject *parent, const char *name,
                                   const char *className, const TQStringList &args )
    {
        this->initializeMessageCatalogue();
        return KDEPrivate::MultiFactory< KTypeList< Product, ProductListTail > >
            ::create( 0, 0, parent, name, className, args );
    }
};

/**
 * This template provides a generic implementation of a KLibFactory ,
 * for use with shared library components. It implements the pure virtual
 * createObject method of KLibFactory and instantiates objects of the
 * specified classes in the given typelist template argument when the class
 * name argument of createObject matches a class names in the given hierarchy
 * of classes.
 *
 * Note that each class in the specified in the typelist template argument
 * needs to provide a certain constructor:
 * <ul>
 *     <li>If the class is derived from TQObject then it needs to have
 *         a constructor like:
 *         <code>MyClass( TQObject *parent, const char *name,
 *                        const TQStringList &args );</code>
 *     <li>If the class is derived from TQWidget then it needs to have
 *         a constructor like:
 *         <code>MyWidget( TQWidget *parent, const char *name,
 *                         const TQStringList &args);</code>
 *     <li>If the class is derived from KParts::Part then it needs to have
 *         a constructor like:
 *         <code>MyPart( TQWidget *parentWidget, const char *widgetName,
 *                       TQObject *parent, const char *name,
 *                       const TQStringList &args );</code>
 * </ul>
 * The args TQStringList passed to the constructor is the args string list
 * that the caller passed to KLibFactory's create method.
 *
 * In addition upon instantiation this template provides a central
 * TDEInstance object for your component, accessible through the
 * static instance() method. The instanceName argument of the
 * KGenericFactory constructor is passed to the TDEInstance object.
 *
 * The creation of the TDEInstance object can be customized by inheriting
 * from this template class and re-implementing the virtual createInstance
 * method. For example it could look like this:
 * \code
 *     TDEInstance *MyFactory::createInstance()
 *     {
 *         return new TDEInstance( myAboutData );
 *     }
 * \endcode
 *
 * Example of usage of the whole template:
 * \code
 *     class MyPlugin : public KParts::Plugin
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyPlugin( TQObject *parent, const char *name,
 *                   const TQStringList &args );
 *         ...
 *     };
 *
 *     class MyDialogComponent : public KDialogBase
 *     {
 *         Q_ OBJECT
 *     public:
 *         MyDialogComponent( TQWidget *parentWidget, const char *name,
 *                            const TQStringList &args );
 *         ...
 *     };
 *
 *     typedef K_TYPELIST_2( MyPlugin, MyDialogComponent ) Products;
 *     K_EXPORT_COMPONENT_FACTORY( libmyplugin, KGenericFactory&lt;Products&gt; )
 * \endcode
 */
template <class Product, class ProductListTail,
          class ParentType, class ParentTypeListTail>
class KGenericFactory< KTypeList<Product, ProductListTail>,
                       KTypeList<ParentType, ParentTypeListTail> >
    : public KLibFactory,
      public KGenericFactoryBase< KTypeList<Product, ProductListTail> >
{
public:
    KGenericFactory( const char *instanceName  = 0 )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( instanceName )
    {}
	/**
	* @since 3.3
	*/
    KGenericFactory( const TDEAboutData *data )
        : KGenericFactoryBase< KTypeList<Product, ProductListTail> >( data )
    {}


protected:
    virtual TQObject *createObject( TQObject *parent, const char *name,
                                   const char *className, const TQStringList &args )
    {
        this->initializeMessageCatalogue();
        return KDEPrivate::MultiFactory< KTypeList< Product, ProductListTail >,
                                         KTypeList< ParentType, ParentTypeListTail > >
                                       ::create( 0, 0, parent, name,
                                                 className, args );
    }
};

#endif
