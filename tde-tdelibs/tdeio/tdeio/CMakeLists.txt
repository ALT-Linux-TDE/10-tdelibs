#################################################
#
#  (C) 2010 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################

include_directories(
  ${TQT_INCLUDE_DIRS}
  ${CMAKE_CURRENT_BINARY_DIR}
  ${CMAKE_BINARY_DIR}/tdeio/kssl
  ${CMAKE_BINARY_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_BINARY_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}
  ${CMAKE_SOURCE_DIR}/dcop
  ${CMAKE_SOURCE_DIR}/libltdl
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdecore/network
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdeio
  ${CMAKE_SOURCE_DIR}/tdeio/kssl
  ${CMAKE_SOURCE_DIR}/interfaces
  ${LIBR_INCLUDEDIR}
  ${GAMIN_INCLUDEDIR}
)

link_directories(
  ${GAMIN_LIBDIR}
)

##### headers ###################################

install( FILES
    kservicetype.h kmimetype.h kmimemagic.h kservice.h
    krun.h kdirwatch.h kautomount.h kuserprofile.h
    kshred.h kar.h ktar.h kzip.h ktrader.h kurifilter.h
    kurlcompletion.h kshellcompletion.h tdefileitem.h
    tdefileshare.h ksambashare.h knfsshare.h kdirlister.h
    kservicegroup.h kimageio.h kdirnotify.h kdirnotify_stub.h
    kurlpixmapprovider.h kprotocolinfo.h tdeprotocolmanager.h
    kfilterbase.h kfilterdev.h tdeemailsettings.h kscan.h
    kdatatool.h karchive.h tdefilefilter.h tdefilemetainfo.h
    renamedlgplugin.h kmimetyperesolver.h kdcopservicestarter.h
    kremoteencoding.h kmimetypechooser.h kacl.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )

install( FILES
    connection.h slaveinterface.h slave.h slaveconfig.h
    sessiondata.h global.h passdlg.h netaccess.h job.h
    scheduler.h jobclasses.h paste.h slavebase.h
    progressbase.h defaultprogress.h statusbarprogress.h
    tcpslavebase.h forwardingslavebase.h observer.h
    chmodjob.h kmdbase.h authinfo.h ioslave_defaults.h
    http_slave_defaults.h previewjob.h thumbcreator.h
    metainfojob.h davjob.h renamedlg.h skipdlg.h tdelficon.h
    ${CMAKE_CURRENT_BINARY_DIR}/uiserver_stub.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/tdeio )


##### tdeiocore ###################################

set( target tdeiocore )

set( ${target}_SRCS
  authinfo.cpp kshred.cpp tdeprotocolmanager.cpp slave.cpp
  slaveinterface.cpp observer.stub sessiondata.cpp
  scheduler.cpp connection.cpp job.cpp global.cpp
  slaveconfig.cpp kurlpixmapprovider.cpp netaccess.cpp
  paste.cpp pastedialog.cpp kmimemagic.cpp tcpslavebase.cpp
  slavebase.cpp passdlg.cpp forwardingslavebase.cpp
  progressbase.cpp defaultprogress.cpp statusbarprogress.cpp
  kdirnotify.cpp kdirnotify.skel kdirnotify_stub.cpp
  observer.cpp ../misc/uiserver.stub observer.skel tdeemailsettings.cpp
  kprotocolinfo.cpp renamedlg.cpp skipdlg.cpp kremoteencoding.cpp
  kmimetypechooser.cpp
)

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
)


##### tdesycoca ###################################

set( target tdesycoca )

set( ${target}_SRCS
  kdirwatch.cpp tdefileshare.cpp ksambashare.cpp
  knfsshare.cpp ktrader.cpp ktraderparse.cpp
  ktraderparsetree.cpp kservicetypefactory.cpp
  kservicetype.cpp kmimetype.cpp kservicegroup.cpp
  kservice.cpp kservicefactory.cpp kuserprofile.cpp
  kservicegroupfactory.cpp kurifilter.cpp kfilterbase.cpp
  kfilterdev.cpp kshellcompletion.cpp kurlcompletion.cpp
  kautomount.cpp krun.cpp tdefileitem.cpp kdirlister.cpp
  kimageio.cpp yacc.c lex.c chmodjob.cpp kscan.cpp
  kar.cpp ktar.cpp kzip.cpp previewjob.cpp metainfojob.cpp
  davjob.cpp kdatatool.cpp karchive.cpp tdefilefilter.cpp
  tdefilemetainfo.cpp kdcopservicestarter.cpp dataslave.cpp
  dataprotocol.cpp kacl.cpp
)

if( USE_POSIX_ACL )
  set( ${target}_SRCS ${${target}_SRCS} posixacladdons.cpp )
endif( USE_POSIX_ACL )

tde_add_library( ${target} STATIC_PIC AUTOMOC
  SOURCES ${${target}_SRCS}
  LINK ${MAGIC_LIBRARIES} ${GAMIN_LIBRARIES}
)


##### tdelficon ###################################

if( HAVE_ELFICON )

  set( target tdelficon )

  set( ${target}_SRCS
    tdelficon.cpp
  )

  tde_add_library( ${target} STATIC_PIC AUTOMOC
    SOURCES ${${target}_SRCS}
  )

endif( HAVE_ELFICON )
