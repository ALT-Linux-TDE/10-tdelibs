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
  ${CMAKE_SOURCE_DIR}/tdecore
  ${CMAKE_SOURCE_DIR}/tdeui
  ${CMAKE_SOURCE_DIR}/tdefx
  ${CMAKE_SOURCE_DIR}/dcop
)

link_directories(
  ${TQT_LIBRARY_DIRS}
)

set( check_PROGS
  kaboutdialogtest kblendtest kbuttonboxtest kcharselecttest
  kcolortest kcolordlgtest kcomboboxtest kcompletiontest
  kdatepicktest kdatewidgettest kdialogbasetest
  kdockwidgetdemo kdockwidgettest
  kdualcolortest keditlistboxtest tdefontdialogtest
  kgradienttest khashtest kdesattest
  kledtest klineedittest klineeditdlgtest
  kmessageboxtest knuminputtest
  kpalettetest
  tdepopuptest kprogresstest krulertest kseparatortest
  kstatusbartest ktabctltest
  kunbalancedgrdtest twindowtest kwizardtest
  itemcontainertest kpanelmenutest kedittest
  kxmlguitest tdeaccelgentest ksystemtraytest tdelistviewtest
  ktextedittest tdemainwindowtest tdemainwindowrestoretest
  tdeactiontest kdatetimewidgettest ktimewidgettest
  tdespelltest kinputdialogtest ktabwidgettest kwordwraptest
  kdocktest qxembedtest ksqueezedtextlabeltest
  kpixmapregionselectordialogtest kjanuswidgettest tdetoolbarlabelactiontest
)

foreach( PROG ${test_PROGS} )
  tde_add_check_executable( ${PROG} AUTOMOC LINK tdeui-shared )
endforeach( PROG ${test_PROGS} )
