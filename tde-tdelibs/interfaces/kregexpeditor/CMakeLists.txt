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

##### headers ###################################

install( FILES
    kregexpeditorinterface.h
  DESTINATION ${INCLUDE_INSTALL_DIR} )

##### other data ################################

tde_create_translated_desktop(
  SOURCE kregexpeditor.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR interfaces-desktops
)
