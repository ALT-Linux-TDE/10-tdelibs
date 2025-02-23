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
    byteseditinterface.h valuecolumninterface.h
    charcolumninterface.h zoominterface.h clipboardinterface.h
  DESTINATION ${INCLUDE_INSTALL_DIR}/khexedit )


##### other data ################################

tde_create_translated_desktop(
  SOURCE kbytesedit.desktop
  DESTINATION ${SERVICETYPES_INSTALL_DIR}
  PO_DIR interfaces-desktops
)
