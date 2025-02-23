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
    block.desktop chardevice.desktop directory-locked.desktop
    directory.desktop fifo.desktop socket.desktop
  DESTINATION ${MIME_INSTALL_DIR}/inode
  PO_DIR mimetypes
)
