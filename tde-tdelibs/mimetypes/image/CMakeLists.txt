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
    jpeg.desktop jpg.desktop tiff.desktop x-xpm.desktop x-xbm.desktop
    svg+xml.desktop svg-xml.desktop png.desktop gif.desktop fax-g3.desktop cgm.desktop
    x-bmp.desktop x-photo-cd.desktop x-eps.desktop x-ico.desktop
    x-jng.desktop x-xcf-gimp.desktop x-wmf.desktop x-msod.desktop
    x-xfig.desktop x-portable-bitmap.desktop x-portable-greymap.desktop
    x-portable-pixmap.desktop x-pcx.desktop x-targa.desktop jp2.desktop
    x-vnd.trolltech.qpicture.desktop x-vnd.adobe.photoshop.desktop
    x-xcursor.desktop pjpeg.desktop x-exr.desktop
    x-rgb.desktop x-dds.desktop x-djvu.desktop x-djvu-2.desktop x-hdr.desktop
    fits.desktop x-raw.desktop webp.desktop
  DESTINATION ${MIME_INSTALL_DIR}/image
  PO_DIR mimetypes
)

install( FILES uninstall.desktop DESTINATION ${MIME_INSTALL_DIR}/image RENAME wmf.desktop )
