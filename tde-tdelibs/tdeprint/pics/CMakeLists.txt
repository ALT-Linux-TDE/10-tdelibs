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

install( FILES
    tdeprint_collate.png tdeprint_collate_reverse.png
    tdeprint_color.png tdeprint_duplex_none.png
    tdeprint_duplex_long.png tdeprint_duplex_short.png
    tdeprint_grayscale.png tdeprint_landscape.png
    tdeprint_nup1.png tdeprint_nup2.png tdeprint_nup4.png
    tdeprint_portrait.png tdeprint_revlandscape.png
    tdeprint_revportrait.png tdeprint_uncollate.png
    tdeprint_uncollate_reverse.png tdeprint_nupother.png
    tdeprint_prettyprint.png
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/pics )

tde_install_icons(
    tdeprint_job tdeprint_job_error tdeprint_job_process
    tdeprint_job_stopped tdeprint_printer tdeprint_printer_class
    tdeprint_printer_class_stopped tdeprint_printer_defect
    tdeprint_printer_remote tdeprint_printer_remote_stopped
    tdeprint_printer_stopped tdeprint_printer_remote_process
    tdeprint_printer_process tdeprint_printer_class_process
    tdeprint_computer tdeprint_printer_infos history
    tdeprint_printer_users filterstop tdeprint_job_completed
    tdeprint_job_cancelled package_utilities tdeprint_enableprinter
    tdeprint_stopprinter tdeprint_addprinter tdeprint_addpseudo
    tdeprint_configmgr tdeprint_configsrv tdeprint_defaulthard
    tdeprint_defaultsoft tdeprint_report tdeprint_testprinter
    tdeprint_uploadsmb tdeprint_restartsrv tdeprint_inklevel
    tdeprint_printstate tdeprint_queuestate
  DESTINATION ${DATA_INSTALL_DIR}/tdeprint/icons )
