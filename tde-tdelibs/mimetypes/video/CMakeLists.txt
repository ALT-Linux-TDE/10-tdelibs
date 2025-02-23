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
    mpeg.desktop quicktime.desktop x-flic.desktop x-flv.desktop
    x-msvideo.desktop x-mng.desktop x-ms-asf.desktop x-ms-wmv.desktop
    x-theora.desktop mp4.desktop x-ogm.desktop x-matroska.desktop
    vnd.rn-realvideo.desktop avi.desktop
  DESTINATION ${MIME_INSTALL_DIR}/video
  PO_DIR mimetypes
)
