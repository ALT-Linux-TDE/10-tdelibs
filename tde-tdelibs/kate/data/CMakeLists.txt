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
  SOURCE katepart.desktop
  DESTINATION ${SERVICES_INSTALL_DIR}
  PO_DIR kate-desktops
)

install( FILES
    katepartui.rc katepartreadonlyui.rc
  DESTINATION ${DATA_INSTALL_DIR}/katepart )

install( FILES
    katesyntaxhighlightingrc katefiletyperc
  DESTINATION ${CONFIG_INSTALL_DIR} )

install( FILES
    language.dtd syntax.template
    4dos.xml abap.xml abc.xml actionscript.xml ada.xml ahdl.xml ahk.xml alert_indent.xml
    alert.xml ample.xml ansic89.xml ansys.xml apache.xml asm6502.xml asm-avr.xml
    asm-dsp56k.xml asm-m68k.xml asn1.xml asp.xml asterisk.xml awk.xml bash.xml bitbake.xml
    bibtex.xml bmethod.xml carto-css.xml ccss.xml cgis.xml cg.xml changelog.xml chicken.xml
    cisco.xml clipper.xml clojure.xml cmake.xml coffee.xml coldfusion.xml
    commonlisp.xml component-pascal.xml context.xml cpp.xml crk.xml css.xml
    cs.xml cue.xml curry.xml c.xml ddoc.xml debianchangelog.xml debiancontrol.xml
    desktop.xml diff.xml djangotemplate.xml dockerfile.xml dosbat.xml dot.xml
    doxygenlua.xml doxygen.xml dtd.xml d.xml eiffel.xml email.xml erlang.xml
    euphoria.xml e.xml fasm.xml ferite.xml fgl-4gl.xml fgl-per.xml fortran.xml freebasic.xml
    fsharp.xml fstab.xml ftl.xml gap.xml gcc.xml gcode.xml gdb.xml gdl.xml gettext.xml
    git-ignore.xml git-rebase.xml gitolite.xml glosstex.xml glsl.xml gnuassembler.xml gnuplot.xml
    go.xml grammar.xml groovy.xml hamlet.xml haml.xml haskell.xml haxe.xml html.xml
    hunspell-aff.xml hunspell-dat.xml hunspell-dic.xml hunspell-idx.xml
    idconsole.xml idl.xml ilerpg.xml inform.xml
    ini.xml isocpp.xml jam.xml javadoc.xml javascript.xml java.xml json.xml
    jsp.xml julia.xml j.xml kbasic.xml latex.xml ldif.xml ld.xml less.xml lex.xml lilypond.xml
    literate-curry.xml literate-haskell.xml logtalk.xml lpc.xml lsl.xml lua.xml m3u.xml
    m4.xml mab.xml magma.xml makefile.xml mako.xml mandoc.xml markdown.xml mason.xml mathematica.xml
    matlab.xml maxima.xml mediawiki.xml mel.xml mergetagtext.xml meson.xml metafont.xml mips.xml
    modelica.xml modelines.xml modula-2-iso-only.xml modula-2-pim-only.xml modula-2-r10-only.xml
    modula-2.xml monobasic.xml mup.xml nagios.xml nasm.xml nemerle.xml nesc.xml
    noweb.xml objectivecpp.xml objectivec.xml ocaml.xml ocamllex.xml ocamlyacc.xml
    octave.xml oors.xml opal.xml opencl.xml
    pango.xml pascal.xml perl.xml pgn.xml php.xml picsrc.xml pig.xml pike.xml
    postscript.xml povray.xml ppd.xml praat.xml progress.xml prolog.xml protobuf.xml puppet.xml
    purebasic.xml python.xml qmake.xml qml.xml qt4.xml q.xml rapidq.xml relaxngcompact.xml
    relaxng.xml replicode.xml restructuredtext.xml rest.xml rexx.xml rhtml.xml rib.xml
    roff.xml rpmspec.xml rsiidl.xml rtf.xml ruby.xml rust.xml r.xml sather.xml scala.xml scheme.xml
    sci.xml scss.xml sed.xml sgml.xml sieve.xml sisu.xml sml.xml spice.xml sql-mysql.xml
    sql-oracle.xml sql-postgresql.xml sql.xml stata.xml systemc.xml systemverilog.xml tads3.xml
    taskjuggler.xml tcl.xml tcsh.xml template-toolkit.xml texinfo.xml textile.xml tibasic.xml
    txt2tags.xml update-files.xml uscript.xml valgrind-suppression.xml
    varnishtest.xml varnish.xml vcard.xml velocity.xml vera.xml verilog.xml vhdl.xml
    vrml.xml winehq.xml wml.xml xharbour.xml xmldebug.xml xml.xml xonotic-console.xml
    xorg.xml xslt.xml xul.xml yacas.xml yacc.xml yaml.xml zonnon.xml zsh.xml
    ${CMAKE_CURRENT_BINARY_DIR}/html-php.xml
    ${CMAKE_CURRENT_BINARY_DIR}/css-php.xml
    ${CMAKE_CURRENT_BINARY_DIR}/javascript-php.xml
  DESTINATION ${DATA_INSTALL_DIR}/katepart/syntax )


##### generate php headers ######################

add_custom_command( OUTPUT html-php.xml
  COMMAND perl ARGS generate-php.pl < html.xml > ${CMAKE_CURRENT_BINARY_DIR}/html-php.xml
  DEPENDS html.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_custom_command( OUTPUT css-php.xml
  COMMAND perl ARGS generate-php.pl < css.xml > ${CMAKE_CURRENT_BINARY_DIR}/css-php.xml
  DEPENDS css.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_custom_command( OUTPUT javascript-php.xml
  COMMAND perl ARGS generate-php.pl < javascript.xml > ${CMAKE_CURRENT_BINARY_DIR}/javascript-php.xml
  DEPENDS javascript.xml
  WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} )

add_custom_target( php-headers ALL
  DEPENDS html-php.xml css-php.xml javascript-php.xml )
