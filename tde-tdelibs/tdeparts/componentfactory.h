#ifndef __tdeparts_componentfactory_h__
#define __tdeparts_componentfactory_h__

#include <tdeparts/factory.h>
#include <tdeparts/part.h>
#include <ktrader.h>
#include <tqmetaobject.h>

namespace KParts
{

    // this is a namespace and not a class because stupid egcs 1.1.2 doesn't grok
    // static template methods in classes. !@%@#$!
    /**
     * Namespace for KParts components
     */
    namespace ComponentFactory
    {
        /**
         * This enum type defines the possible error cases that can happen
         * when loading a component.
         *
         * <ul>
         *  <li><code>ErrNoServiceFound</code> - no service implementing the
         *      given mimetype and fullfilling the given constraint expression
         *      can be found.</li>
         *  <li><code>ErrServiceProvidesNoLibrary</code> - the specified service
         *      provides no shared library</li>
         *  <li><code>ErrNoLibrary</code> - the specified library could not be
         *      loaded. Use KLibLoader::lastErrorMessage for details.</li>
         *  <li><code>ErrNoFactory</code> - the library does not export a factory
         *      for creating components</li>
         *  <li><code>ErrNoComponent</code> - the factory does not support creating
         *      components of the specified type</li>
         * </ul>
         */
        enum ComponentLoadingError { ErrNoServiceFound = 1,
                                     ErrServiceProvidesNoLibrary,
                                     ErrNoLibrary,
                                     ErrNoFactory,
                                     ErrNoComponent };

        /**
         * This template function allows to ask the given factory to create an
         * instance of the given template type.
         *
         * Example of usage:
         * \code
         *     MyPlugin *plugin = KParts::ComponentFactory::createInstanceFromFactory<MyPlugin>( factory, parent );
         * \endcode
         *
         * @param factory The factory to ask for the creation of the component
         * @param parent The parent object (see TQObject constructor)
         * @param name The name of the object to create (see TQObject constructor)
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
        template <class T>
        static T *createInstanceFromFactory( KLibFactory *factory, TQObject *parent = 0,
                                             const char *name = 0,
                                             const TQStringList &args = TQStringList() )
        {
            TQObject *object = factory->create( parent, name,
                                               T::staticMetaObject()->className(),
                                               args );

            T *result = dynamic_cast<T *>( object );
            if ( !result )
                    delete object;
            return result;
        }

        /**
         * This template function allows to ask the given tdeparts factory to create an
         * instance of the given template type.
         *
         * Example of usage:
         * \code
         *     KViewPart *doc = KParts::ComponentFactory::createPartInstanceFromFactory<KViewPart>( factory, parent );
         * \endcode
         *
         * @param factory The factory to ask for the creation of the component
         * @param parentWidget the parent widget for the part
         * @param widgetName the name of the part's widget
         * @param parent The parent object (see TQObject constructor)
         * @param name The name of the object to create (see TQObject constructor)
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
        template <class T>
        static T *createPartInstanceFromFactory( KParts::Factory *factory,
                                                 TQWidget *parentWidget = 0,
                                                 const char *widgetName = 0,
                                                 TQObject *parent = 0,
                                                 const char *name = 0,
                                                 const TQStringList &args = TQStringList() )
        {
            KParts::Part *object = factory->createPart( parentWidget, widgetName,
                                                        parent, name,
                                                        T::staticMetaObject()->className(),
                                                        args );

            T *result = dynamic_cast<T *>( object );
            if ( !result )
                delete object;
            return result;
        }

        /**
         * This template allows to load the specified library and ask the
         * factory to create an instance of the given template type.
         *
         * @param libraryName The library to open
         * @param parent The parent object (see TQObject constructor)
         * @param name The name of the object to create (see TQObject constructor)
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @param error
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
        template <class T>
        static T *createInstanceFromLibrary( const char *libraryName, TQObject *parent = 0,
                                             const char *name = 0,
                                             const TQStringList &args = TQStringList(),
                                             int *error = 0 )
        {
            KLibrary *library = KLibLoader::self()->library( libraryName );
            if ( !library )
            {
                if ( error )
                    *error = ErrNoLibrary;
                return 0;
            }
            KLibFactory *factory = library->factory();
            if ( !factory )
            {
                library->unload();
                if ( error )
                    *error = ErrNoFactory;
                return 0;
            }
            T *res = createInstanceFromFactory<T>( factory, parent, name, args );
            if ( !res )
            {
                library->unload();
                if ( error )
                    *error = ErrNoComponent;
            }
            return res;
        }

        template <class T>
        static T *createPartInstanceFromLibrary( const char *libraryName,
                                                 TQWidget *parentWidget = 0,
                                                 const char *widgetName = 0,
                                                 TQObject *parent = 0,
                                                 const char *name = 0,
                                                 const TQStringList &args = TQStringList(),
                                                 int *error = 0 )
        {
            KLibrary *library = KLibLoader::self()->library( libraryName );
            if ( !library )
            {
                if ( error )
                    *error = ErrNoLibrary;
                return 0;
            }
            KLibFactory *factory = library->factory();
            if ( !factory )
            {
                library->unload();
                if ( error )
                    *error = ErrNoFactory;
                return 0;
            }
            KParts::Factory *partFactory = dynamic_cast<KParts::Factory *>( factory );
            if ( !partFactory )
            {
                library->unload();
                if ( error )
                    *error = ErrNoFactory;
                return 0;
            }
            T *res = createPartInstanceFromFactory<T>( partFactory, parentWidget,
                                                       widgetName, parent, name, args );
            if ( !res )
            {
                library->unload();
                if ( error )
                    *error = ErrNoComponent;
            }
            return res;
        }

        template <class T>
        static T *createInstanceFromService( const KService::Ptr &service,
                                             TQObject *parent = 0,
                                             const char *name = 0,
                                             const TQStringList &args = TQStringList(),
                                             int *error = 0 )
        {
            TQString library = service->library();
            if ( library.isEmpty() )
            {
                if ( error )
                    *error = ErrServiceProvidesNoLibrary;
                return 0;
            }

            return createInstanceFromLibrary<T>( library.local8Bit().data(), parent,
	    					 name, args, error );
        }

        template <class T>
        static T *createPartInstanceFromService( const KService::Ptr &service,
                                                 TQWidget *parentWidget = 0,
                                                 const char *widgetName = 0,
                                                 TQObject *parent = 0,
                                                 const char *name = 0,
                                                 const TQStringList &args = TQStringList(),
                                                 int *error = 0 )
        {
            TQString library = service->library();
            if ( library.isEmpty() )
            {
                if ( error )
                    *error = ErrServiceProvidesNoLibrary;
                return 0;
            }

            return createPartInstanceFromLibrary<T>( library.local8Bit().data(), parentWidget,
                                                     widgetName, parent, name, args, error );
        }

        template <class T, class ServiceIterator>
        static T *createInstanceFromServices( ServiceIterator begin, ServiceIterator end,
                                              TQObject *parent = 0,
                                              const char *name = 0,
                                              const TQStringList &args = TQStringList(),
                                              int *error = 0 )
        {
            for (; begin != end; ++begin )
            {
                KService::Ptr service = *begin;

                if ( error )
                    *error = 0;

                T *component = createInstanceFromService<T>( service, parent, name,
                                                             args, error );
                if ( component )
                    return component;
            }

            if ( error )
                *error = ErrNoServiceFound;

            return 0;

        }

        template <class T, class ServiceIterator>
        static T *createPartInstanceFromServices( ServiceIterator begin,
                                                  ServiceIterator end,
                                                  TQWidget *parentWidget = 0,
                                                  const char *widgetName = 0,
                                                  TQObject *parent = 0,
                                                  const char *name = 0,
                                                  const TQStringList &args = TQStringList(),
                                                  int *error = 0 )
         {
            for (; begin != end; ++begin )
            {
                KService::Ptr service = *begin;

                if ( error )
                    *error = 0;

                T *component = createPartInstanceFromService<T>( service, parentWidget,
                                                                 widgetName, parent,
                                                                 name, args, error );
                if ( component )
                    return component;
            }

            if ( error )
                *error = ErrNoServiceFound;

            return 0;

        }

        /**
         * This method creates and returns a plugin, from the trader query for a given serviceType.
         *
         * Example:
         * \code
         * KMyAppPlugin* plugin = KParts::ComponentFactory::createInstanceFromQuery<KMyAppPlugin>( serviceType, TQString::null, parentObject );
         * if ( plugin ) {
         *     ....
         * }
         * \endcode
         *
         * @param serviceType the type of service for which to find a plugin
         * @param constraint an optional constraint to pass to the trader (see TDEIO::TDETrader)
         * @param parent the parent object for the part itself
         * @param name the name that will be given to the part
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @param error The int passed here will receive an error code in case of errors.
         *              (See enum #ComponentLoadingError)
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
        template <class T>
        static T *createInstanceFromQuery( const TQString &serviceType,
                                           const TQString &constraint = TQString::null,
                                           TQObject *parent = 0,
                                           const char *name = 0,
                                           const TQStringList &args = TQStringList(),
                                           int *error = 0 )
        {
            TDETrader::OfferList offers = TDETrader::self()->query( serviceType, constraint );
            if ( offers.isEmpty() )
            {
                if ( error )
                    *error = ErrNoServiceFound;
                return 0;
            }

            return createInstanceFromServices<T>( offers.begin(),
                                                  offers.end(),
                                                  parent, name, args, error );
        }

        /**
         * This method creates and returns a KParts part from a serviceType (e.g. a mimetype).
         *
         * You can use this method to create a generic viewer - that can display any
         * kind of file, provided that there is a ReadOnlyPart installed for it - in 5 lines:
         * \code
         * // Given the following: KURL url, TQWidget* parentWidget and TQObject* parentObject.
         * TQString mimetype = KMimeType::findByURL( url )->name();
         * KParts::ReadOnlyPart* part = KParts::ComponentFactory::createPartInstanceFromQuery<KParts::ReadOnlyPart>( mimetype, TQString::null, parentWidget, 0, parentObject, 0 );
         * if ( part ) {
         *     part->openURL( url );
         *     part->widget()->show();  // also insert the widget into a layout, or simply use a TQVBox as parentWidget
         * }
         * \endcode
         *
         * @param serviceType the type of service for which to find a part, e.g. a mimetype
         * @param constraint an optional constraint to pass to the trader (see TDETrader)
         * @param parentWidget the parent widget, will be set as the parent of the part's widget
         * @param widgetName the name that will be given to the part's widget
         * @param parent the parent object for the part itself
         * @param name the name that will be given to the part
         * @param args A list of string arguments, passed to the factory and possibly
         *             to the component (see KLibFactory)
         * @param error The int passed here will receive an error code in case of errors.
         *              (See enum #ComponentLoadingError)
         * @return A pointer to the newly created object or a null pointer if the
         *         factory was unable to create an object of the given type.
         */
        template <class T>
        static T *createPartInstanceFromQuery( const TQString &serviceType,
                                               const TQString &constraint,
                                               TQWidget *parentWidget = 0,
                                               const char *widgetName = 0,
                                               TQObject *parent = 0,
                                               const char *name = 0,
                                               const TQStringList &args = TQStringList(),
                                               int *error = 0 )
        {
            TDETrader::OfferList offers = TDETrader::self()->query( serviceType, TQString::fromLatin1("KParts/ReadOnlyPart"), constraint, TQString::null );
            if ( offers.isEmpty() )
            {
                if ( error )
                    *error = ErrNoServiceFound;
                return 0;
            }

            return createPartInstanceFromServices<T>( offers.begin(), offers.end(),
                                                      parentWidget, widgetName,
                                                      parent, name, args, error );
        }

    }

}

#endif
