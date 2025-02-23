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

##### other data ################################

tde_create_translated_desktop(
  SOURCE
    enscript.desktop psnup.desktop
    imagetops.desktop pdfwrite.desktop
    psbook1.desktop psbook2.desktop
    psbook.desktop psselect.desktop
    ps2pdf.desktop pdf2ps.desktop
    poster.desktop
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/filters
  PO_DIR tdeprint-desktops
)

install(
  FILES
    enscript.xml psnup.xml
    imagetops.xml pdfwrite.xml
    psbook1.xml psbook2.xml
    psbook.xml psselect.xml
    ps2pdf.xml pdf2ps.xml
    poster.xml
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/filters
)

if( WITH_IMAGETOPS_BINARY )
  install( PROGRAMS imagetops DESTINATION ${BIN_INSTALL_DIR} )
endif()
