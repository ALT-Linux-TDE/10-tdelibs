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
    x-wav.desktop x-mod.desktop x-midi.desktop
    x-mp3.desktop x-mp2.desktop mp4.desktop mpeg.desktop
    basic.desktop x-adpcm.desktop x-aiff.desktop
    x-pn-realaudio.desktop x-pn-realaudio-plugin.desktop vnd.rn-realaudio.desktop
    x-mpegurl.desktop mpegurl.desktop x-scpls.desktop
    vorbis.desktop x-vorbis.desktop
    x-flac.desktop x-oggflac.desktop x-speex.desktop
    x-matroska.desktop x-musepack.desktop
    ac3.desktop aac.desktop x-ms-wma.desktop prs.sid.desktop
  DESTINATION ${MIME_INSTALL_DIR}/audio
  PO_DIR mimetypes
)
