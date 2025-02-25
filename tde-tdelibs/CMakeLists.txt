#################################################
#
#  (C) 2010-2011 Serghei Amelian
#  serghei (DOT) amelian (AT) gmail.com
#
#  Improvements and feedback are welcome
#
#  This file is released under GPL >= 2
#
#################################################


##### set project version ########################

include( TDEVersion )
cmake_minimum_required( VERSION ${TDE_CMAKE_MINIMUM_VERSION} )
tde_set_project_version( )


##### general package setup #####################

project( tdelibs )

set( PACKAGE tdelibs )


##### include essential cmake modules ###########

include( FindPkgConfig )
include( CheckIncludeFile )
include( CheckIncludeFiles )
include( CheckSymbolExists )
include( CheckFunctionExists )
include( CheckCSourceCompiles )
include( CheckCXXSourceCompiles )
include( CheckTypeSize )
include( CheckCSourceRuns )
include( CheckLibraryExists )
include( CheckStructHasMember )


##### include our cmake modules #################

include( TDEMacros )
include( TDESetupPaths )


##### setup and find essential stuff ############

tde_setup_paths( )
tde_setup_architecture_flags( )
enable_testing( )
find_package( TQt )

list( APPEND TDECORE_LIBRARY_DIRS ${TQT_LIBRARY_DIRS} )


##### add apidox targets ############

add_custom_target( apidox
  COMMAND ${CMAKE_COMMAND} -E env
    "TQTDOCDIR=/usr/share/tqt3/doc/html"
    ${CMAKE_SOURCE_DIR}/admin/doxygen.sh
    --no-modulename
    ${CMAKE_SOURCE_DIR}
  COMMAND
    find ${CMAKE_BINARY_DIR}/apidocs -name '*.html' |
    xargs -r sed -i 's|${CMAKE_BINARY_DIR}/apidocs|${HTML_INSTALL_DIR}/en/${PROJECT_NAME}-apidocs|g'
  COMMAND
    find ${CMAKE_BINARY_DIR}/apidocs -name '*.tag' |
    xargs -r sed -i 's|${CMAKE_SOURCE_DIR}/|${PROJECT_NAME}/|g'
  WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
)

add_custom_target( install-apidox
  COMMAND ${CMAKE_COMMAND} -E echo Apidox installed in ${HTML_INSTALL_DIR}/en/${PROJECT_NAME}-apidocs
)

install( CODE "
  if( EXISTS ${CMAKE_BINARY_DIR}/apidocs )
    file(
      INSTALL ${CMAKE_BINARY_DIR}/apidocs/
      DESTINATION ${HTML_INSTALL_DIR}/en/${PROJECT_NAME}-apidocs
      PATTERN doxygen_sqlite3.db EXCLUDE
      PATTERN installdox-slow EXCLUDE
    )
  endif()"
)


##### user requested OPTIONs ####################

option( WITH_ALL_OPTIONS "Enable all OPTIONal support" OFF )

option( TDE_MALLOC "Use own malloc implementation" OFF )
option( TDE_MALLOC_DEBUG "Enable debugging in fast malloc" OFF )
option( TDE_MALLOC_FULL "Make alloc as fast as possible" OFF )

option( WITH_ARTS "Build with aRts" ON )
option( WITH_ALSA "Enable ALSA support" ON )
option( WITH_LIBART "Enable libart support (for SVG icons)" ${WITH_ALL_OPTIONS} )
option( WITH_LIBIDN "Enable support for libidn" ${WITH_ALL_OPTIONS} )
option( WITH_SSL "Enable support for SSL" ON )
option( WITH_CUPS "Enable CUPS support" ON )
option( WITH_IMAGETOPS_BINARY "Enable installation of imagetops binary" ${WITH_ALL_OPTIONS} )
option( WITH_LUA "Enable LUA support" ${WITH_ALL_OPTIONS} )
option( WITH_TIFF "Enable tiff support" ${WITH_ALL_OPTIONS} )
option( WITH_JASPER "Enable jasper (jpeg2k) support" ${WITH_ALL_OPTIONS} )
option( WITH_WEBP "Enable WebP support" ${WITH_ALL_OPTIONS} )
option( WITH_OPENEXR "Enable openexr support" ${WITH_ALL_OPTIONS} )
option( WITH_UTEMPTER "Use utempter for utmp management" ${WITH_ALL_OPTIONS} )
option( WITH_AVAHI "Enable AVAHI support" ${WITH_ALL_OPTIONS} )
option( WITH_ELFICON "Enable ELF embedded icon support" ${WITH_ALL_OPTIONS} )
option( WITH_PCRE2 "Enable pcre2 regex support for kjs" ON )
option( WITH_GCC_VISIBILITY "Enable fvisibility and fvisibility-inlines-hidden" ${WITH_ALL_OPTIONS} )
option( WITH_INOTIFY "Enable inotify support for tdeio" ON )
option( WITH_GAMIN "Enable FAM/GAMIN support" ${WITH_ALL_OPTIONS} )
option( WITH_TDEHWLIB "Enable TDE hwlib globally" ON )
option( WITH_TDEHWLIB_DAEMONS "Enable daemons for TDE hwlib" ${WITH_TDEHWLIB} )
option( WITH_DEVKITPOWER "Enable DeviceKit Power support" OFF )
option( WITH_LOGINDPOWER "Enable Logind/Systemd Power support" OFF )
option( WITH_UPOWER "Enable uPower support" ${WITH_ALL_OPTIONS} )
option( WITH_UDISKS "Enable uDisks support" ${WITH_ALL_OPTIONS} )
option( WITH_UDISKS2 "Enable uDisks2 support" ${WITH_ALL_OPTIONS} )
option( WITH_UDEVIL "Enable udevil support" ${WITH_ALL_OPTIONS} )
option( WITH_CONSOLEKIT "Enable ConsoleKit support" ${WITH_ALL_OPTIONS} )
option( WITH_NETWORK_MANAGER_BACKEND "Enable network-manager support" OFF )
option( WITH_SUDO_TDESU_BACKEND "Use sudo as backend for tdesu (default is su)" OFF )
option( WITH_OLD_XDG_STD "Use the pre R14.0.0 XDG standard where both TDE and KDE are recognized in desktop files" OFF )
option( WITH_PCSC "Enable PC/SC SmartCard support" ${WITH_ALL_OPTIONS} )
option( WITH_PKCS "Enable PKCS support" ${WITH_ALL_OPTIONS} )
option( WITH_CRYPTSETUP "Enable cryptsetup support" ${WITH_ALL_OPTIONS} )
option( WITH_LZMA "Enable support for LZMA/XZ" ${WITH_ALL_OPTIONS} )
option( WITH_LIBBFD "Enable pretty backtraces with libbfd from GNU binutils" OFF )
option( WITH_XRANDR "Build the tderandr library" ON )
option( WITH_XCOMPOSITE "Enable xcomposite support" ON )
option( WITH_MITSHM "Use MIT-SHM for pixmap loading/saving" ${WITH_ALL_OPTIONS} )
option( WITH_KDE4_MENU_SUFFIX "Add [KDE4] tag to KDE4 menu items" OFF )

option( WITH_ISPELL "Enable ispell support" ON )
option( WITH_ASPELL "Enable aspell support" ${WITH_ALL_OPTIONS} )
option( WITH_HSPELL "Enable hspell support" ${WITH_ALL_OPTIONS} )

option( WITH_TDEICONLOADER_DEBUG "Enable debugging in TDEIconLoader class" OFF )


##### set PKG_CONFIG_PATH #######################

set( ENV{PKG_CONFIG_PATH} "${PKG_CONFIG_PATH}:${PKGCONFIG_INSTALL_DIR}:${LIB_INSTALL_DIR}/pkgconfig:$ENV{PKG_CONFIG_PATH}" )


##### check for CPU architecture ################

### FIXME fast malloc is also available on x86_64 architecture?
if( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i?86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
  set( TDE_MALLOC_X86 1 )
  message( STATUS "Found x86 architecture" )
else( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i?86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )
  if( TDE_MALLOC )
    message( FATAL_ERROR "\nKDE fast malloc is available only on x86 architecture" )
  endif( TDE_MALLOC )
endif( ${CMAKE_SYSTEM_PROCESSOR} MATCHES "i?86" OR ${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64" )


##### check for operating system ################

if( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )
  set( TDEINIT_SETUID 1 )
  set( TDEINIT_OOM_PROTECT 1 )
endif( ${CMAKE_SYSTEM_NAME} STREQUAL "Linux" )


##### get the system's default path for libraries

tde_save_and_set( CMAKE_INSTALL_PREFIX "/usr" )
include( GNUInstallDirs OPTIONAL )
if( CMAKE_INSTALL_LIBDIR )
  set( SYSTEM_LIBDIR "${CMAKE_INSTALL_LIBDIR}" )
else( )
  set( SYSTEM_LIBDIR "lib${LIB_SUFFIX}" )
endif( )
tde_restore( CMAKE_INSTALL_PREFIX )


##### check for compiler capabilities ###########

include( CheckCXXCompilerFlag )

check_cxx_compiler_flag( -Woverloaded-virtual WOVERLOADED_VIRTUAL )
if( WOVERLOADED_VIRTUAL )
  set( WOVERLOADED_VIRTUAL -Woverloaded-virtual )
endif( WOVERLOADED_VIRTUAL )

check_cxx_compiler_flag( -fno-builtin NO_BUILTIN )
if( NO_BUILTIN )

endif( NO_BUILTIN )


##### system checks #############################

include(TestBigEndian)
test_big_endian(WORDS_BIGENDIAN)

tde_setup_largefiles( )

tde_save_and_set( CMAKE_REQUIRED_INCLUDES ${CMAKE_SOURCE_DIR}/libltdl )
check_include_file( "ltdl.h" HAVE_LTDL_H )
if( NOT HAVE_LTDL_H )
  tde_message_fatal( "ltdl.h is required, but not found on your system" )
endif( )
tde_restore( CMAKE_REQUIRED_INCLUDES )

check_include_file( "alloca.h" HAVE_ALLOCA_H )
check_include_file( "arpa/nameser8_compat.h" HAVE_ARPA_NAMESER8_COMPAT_H )
check_include_file( "awe_voice.h" HAVE_AWE_VOICE_H )
check_include_file( "Carbon/Carbon.h" HAVE_CARBON_CARBON_H )
check_include_file( "crt_externs.h" HAVE_CRT_EXTERNS_H )
check_include_file( "ctype.h" HAVE_CTYPE_H )
check_include_file( "dirent.h" HAVE_DIRENT_H )
check_include_file( "dld.h" HAVE_DLD_H )
check_include_file( "dlfcn.h" HAVE_DLFCN_H )
check_include_file( "dl.h" HAVE_DL_H )
check_include_files( "sys/types.h;sys/dl.h" HAVE_SYS_DL_H )
check_include_file( "float.h" HAVE_FLOAT_H )
check_include_file( "fstab.h" HAVE_FSTAB_H )
check_include_file( "ieeefp.h" HAVE_IEEEFP_H )
check_include_file( "inttypes.h" HAVE_INTTYPES_H )
check_include_file( "libutil.h" HAVE_LIBUTIL_H )
check_include_file( "limits.h" HAVE_LIMITS_H )
check_include_file( "linux/awe_voice.h" HAVE_LINUX_AWE_VOICE_H )
check_include_file( "locale.h" HAVE_LOCALE_H )
check_include_file( "machine/soundcard.h" HAVE_MACHINE_SOUNDCARD_H )
check_include_file( "malloc.h" HAVE_MALLOC_H )
check_include_file( "memory.h" HAVE_MEMORY_H )
check_include_file( "mntent.h" HAVE_MNTENT_H )
check_include_file( "sys/mntent.h" HAVE_SYS_MNTENT_H )
check_include_file( "ndir.h" HAVE_NDIR_H )
check_include_file( "netinet/in.h" HAVE_NETINET_IN_H )
check_include_file( "net/if.h" HAVE_NET_IF_H )
if( NOT HAVE_NET_IF_H )
  find_path( NET_IF_PATH "net/if.h" )
  if( NET_IF_PATH )
    set( HAVE_NET_IF_H "1" )
  endif( )
endif( )
check_include_file( "paths.h" HAVE_PATHS_H )
check_include_file( "pty.h" HAVE_PTY_H )
check_include_file( "stdint.h" HAVE_STDINT_H )
check_include_file( "stdio.h" HAVE_STDIO_H )
check_include_file( "stdlib.h" HAVE_STDLIB_H )
check_include_file( "strings.h" HAVE_STRINGS_H )
check_include_file( "string.h" HAVE_STRING_H )
check_include_file( "sysent.h" HAVE_SYSENT_H )
check_include_file( "sys/acl.h" HAVE_SYS_ACL_H )
check_include_file( "sys/bitypes.h" HAVE_SYS_BITYPES_H )
check_include_file( "sys/dir.h" HAVE_SYS_DIR_H )
check_include_file( "sys/filio.h" HAVE_SYS_FILIO_H )
check_include_file( "sys/mman.h" HAVE_SYS_MMAN_H )
check_include_file( "sys/mntent.h" HAVE_SYS_MNTENT_H )
check_include_files( "stdio.h;sys/mnttab.h" HAVE_SYS_MNTTAB_H )
check_include_file( "sys/mount.h" HAVE_SYS_MOUNT_H )
if( NOT HAVE_SYS_MOUNT_H )
  find_path( SYS_MOUNT_PATH "sys/mount.h" )
  if( SYS_MOUNT_PATH )
    set( HAVE_SYS_MOUNT_H "1" )
  endif( )
endif( )
check_include_file( "sys/ndir.h" HAVE_SYS_NDIR_H )
check_include_file( "sys/param.h" HAVE_SYS_PARAM_H )
check_include_file( "sys/prctl.h" HAVE_SYS_PRCTL_H )
check_include_file( "sys/select.h" HAVE_SYS_SELECT_H )
check_include_file( "sys/soundcard.h" HAVE_SYS_SOUNDCARD_H )
check_include_file( "sys/stat.h" HAVE_SYS_STAT_H )
check_include_file( "sys/stropts.h" HAVE_SYS_STROPTS_H )
check_include_file( "sys/types.h" HAVE_SYS_TYPES_H )
check_include_file( "sys/ucred.h" HAVE_SYS_UCRED_H )
if( NOT HAVE_SYS_UCRED_H )
  find_path( SYS_UCRED_PATH "sys/ucred.h" )
  if( SYS_UCRED_PATH )
    set( HAVE_SYS_UCRED_H 1 )
  endif( )
endif( )
check_include_file( "ucred.h" HAVE_UCRED_H )
if( NOT HAVE_UCRED_H )
  find_path( UCRED_PATH "ucred.h" )
  if( UCRED_PATH )
    set( HAVE_UCRED_H 1 )
  endif()
endif()
check_include_file( "sys/xattr.h" HAVE_SYS_XATTR_H )
check_include_file( "termios.h" HAVE_TERMIOS_H )
check_include_file( "termio.h" HAVE_TERMIO_H )
check_include_file( "unistd.h" HAVE_UNISTD_H )
check_include_file( "util.h" HAVE_UTIL_H )
check_include_file( "values.h" HAVE_VALUES_H )

# FIXME I'm not sure if test TIME_WITH_SYS_TIME are correct
check_include_file( "sys/time.h" HAVE_SYS_TIME_H )
check_include_file( "time.h" HAVE_TIME_H )
if( HAVE_SYS_TIME_H AND HAVE_TIME_H )
  set( TIME_WITH_SYS_TIME 1 )
endif( HAVE_SYS_TIME_H AND HAVE_TIME_H )

if( HAVE_SYS_ACL_H )
  check_include_file( "acl/libacl.h" HAVE_ACL_LIBACL_H )
  if( HAVE_ACL_LIBACL_H )
    check_library_exists( acl acl_init "" HAVE_ACL_INIT )
    if( HAVE_ACL_INIT AND HAVE_SYS_XATTR_H )
      set( USE_POSIX_ACL 1 )
      set( HAVE_NON_POSIX_ACL_EXTENSIONS 1 )
      set( ACL_LIBRARIES acl attr )
    endif( HAVE_ACL_INIT AND HAVE_SYS_XATTR_H )
  else( HAVE_ACL_LIBACL_H )
    check_library_exists( c acl_init "" HAVE_ACL_INIT )
    if( HAVE_ACL_INIT )
      set( USE_POSIX_ACL 1 )
      set( ACL_LIBRARIES c )
    endif( HAVE_ACL_INIT )
  endif( HAVE_ACL_LIBACL_H )
endif( HAVE_SYS_ACL_H )

check_include_file( "valgrind/memcheck.h" HAVE_VALGRIND_MEMCHECK_H )
check_include_file( "values.h" HAVE_VALUES_H )
check_include_file( "X11/extensions/shape.h" HAVE_X11_EXTENSIONS_SHAPE_H )
check_include_file( "/usr/src/sys/gnu/i386/isa/sound/awe_voice.h" HAVE__USR_SRC_SYS_GNU_I386_ISA_SOUND_AWE_VOICE_H )
check_include_file( "/usr/src/sys/i386/isa/sound/awe_voice.h" HAVE__USR_SRC_SYS_I386_ISA_SOUND_AWE_VOICE_H )

check_function_exists( random HAVE_RANDOM )
check_symbol_exists( random "stdlib.h" HAVE_RANDOM_PROTO )

check_function_exists( srandom HAVE_SRANDOM )
check_symbol_exists( srandom "stdlib.h" HAVE_SRANDOM_PROTO )

check_function_exists( seteuid HAVE_SETEUID )
check_function_exists( setegid HAVE_SETEGID )

check_function_exists( gethostname HAVE_GETHOSTNAME )
check_symbol_exists( gethostname "unistd.h" HAVE_GETHOSTNAME_PROTO )

check_function_exists( vsnprintf HAVE_VSNPRINTF )
check_function_exists( snprintf HAVE_SNPRINTF )

check_function_exists( setenv HAVE_SETENV )
check_symbol_exists( setenv "stdlib.h" HAVE_SETENV_PROTO )

check_function_exists( unsetenv HAVE_UNSETENV )
check_symbol_exists( unsetenv "stdlib.h" HAVE_UNSETENV_PROTO )

check_function_exists( mkdtemp HAVE_MKDTEMP )
check_symbol_exists( mkdtemp "stdlib.h" HAVE_MKDTEMP_PROTO )

check_function_exists( mkstemp HAVE_MKSTEMP )
check_symbol_exists( mkstemp "stdlib.h" HAVE_MKSTEMP_PROTO )

check_function_exists( mkstemps HAVE_MKSTEMPS )
check_symbol_exists( mkstemps "stdlib.h;unistd.h" HAVE_MKSTEMPS_PROTO )

check_function_exists( initgroups HAVE_INITGROUPS )
check_symbol_exists( initgroups "grp.h;unistd.h" HAVE_INITGROUPS_PROTO )

check_function_exists( strlcat HAVE_STRLCAT )
check_symbol_exists( strlcat "string.h" HAVE_STRLCAT_PROTO )

check_function_exists( strlcpy HAVE_STRLCPY )
check_symbol_exists( strlcpy "string.h" HAVE_STRLCPY_PROTO )

check_symbol_exists( S_ISSOCK "sys/stat.h" HAVE_S_ISSOCK )

check_symbol_exists( gethostbyname2 "netdb.h" HAVE_GETHOSTBYNAME2 )
check_symbol_exists( gethostbyname2_r "netdb.h" HAVE_GETHOSTBYNAME2_R )
check_symbol_exists( gethostbyname_r "netdb.h" HAVE_GETHOSTBYNAME_R )
check_symbol_exists( gai_strerror "sys/types.h;sys/socket.h;netdb.h" HAVE_GAI_STRERROR )
check_symbol_exists( getaddrinfo "sys/types.h;sys/socket.h;netdb.h" HAVE_GETADDRINFO )
if ( CMAKE_MAJOR_VERSION LESS "3" )
  check_symbol_exists( backtrace "execinfo.h" HAVE_BACKTRACE )
  if ( HAVE_BACKTRACE )
    set ( Backtrace_LIBRARY "" )
    set ( Backtrace_HEADER "execinfo.h" )
  endif ()
else ()
  find_package( Backtrace )
  if ( Backtrace_FOUND )
    set ( HAVE_BACKTRACE 1 )
  else ()
    set ( Backtrace_LIBRARY "" )
    set ( Backtrace_HEADER "" )
  endif ()
endif ()
check_cxx_source_compiles( "#include <cxxabi.h>
    int main() { abi::__cxa_demangle(0, 0, 0, 0); return 0; }"
    HAVE_ABI_CXA_DEMANGLE )

# Some declarations are needed by demangle.h (libiberty.h) and/or bfd.h but we
# will check them uncondionally
# See WITH_LIBBFD also see binutils bugs 14243 and 14072
# NOTE: those headers use HAVE_DECL_* names but we would follow our macro
#       naming style
check_symbol_exists( basename  "libgen.h" HAVE_BASENAME_PROTO )
check_symbol_exists( ffs       "string.h" HAVE_FFS_PROTO )
check_symbol_exists( asprintf  "stdio.h"  HAVE_ASPRINTF_PROTO )
check_symbol_exists( vasprintf "stdio.h"  HAVE_VASPRINTF_PROTO )
check_symbol_exists( snprintf  "stdio.h"  HAVE_SNPRINTF_PROTO )
check_symbol_exists( vsnprintf "stdarg.h;stdio.h" HAVE_VSNPRINTF_PROTO )
check_symbol_exists( strvercmp "string.h" HAVE_STRVERCMP_PROTO )


check_function_exists( usleep HAVE_USLEEP )
check_symbol_exists( usleep "unistd.h" HAVE_USLEEP_PROTO )

if (HAVE_ALLOCA_H )
  check_symbol_exists( alloca "alloca.h" HAVE_ALLOCA )
else (HAVE_ALLOCA_H )
  check_symbol_exists( alloca "stdlib.h" HAVE_ALLOCA )
endif (HAVE_ALLOCA_H )
if( ${CMAKE_SYSTEM_NAME} STREQUAL "OpenBSD" )
  set( HAVE_ALLOCA 1 )
endif( )

check_function_exists( getmntinfo HAVE_GETMNTINFO )
if( HAVE_GETMNTINFO )
  check_cxx_source_compiles("
    #include <sys/types.h>
    #include <sys/statvfs.h>
    int main(){
      struct statvfs *mntbufp;
      int flags;
      return getmntinfo(&mntbufp, flags);
    }"
    GETMNTINFO_USES_STATVFS )
endif( HAVE_GETMNTINFO )
check_function_exists( getnameinfo HAVE_GETNAMEINFO )
check_function_exists( getpagesize HAVE_GETPAGESIZE )
check_function_exists( getpeereid HAVE_GETPEEREID )
check_function_exists( getpeerucred HAVE_GETPEERUCRED )
check_function_exists( getpeername HAVE_GETPEERNAME )
check_function_exists( getprotobyname_r HAVE_GETPROTOBYNAME_R )
check_function_exists( getpt HAVE_GETPT )
check_function_exists( getservbyname_r HAVE_GETSERVBYNAME_R )
check_symbol_exists( getservbyname_r "netdb.h" HAVE_DECL_GETSERVBYNAME_R )
check_function_exists( getservbyport_r HAVE_GETSERVBYPORT_R )
check_function_exists( getsockname HAVE_GETSOCKNAME )
check_function_exists( getsockopt HAVE_GETSOCKOPT )
check_function_exists( gettimeofday HAVE_GETTIMEOFDAY )
check_function_exists( grantpt HAVE_GRANTPT )
check_function_exists( if_nametoindex HAVE_IF_NAMETOINDEX )
check_function_exists( index HAVE_INDEX)
check_function_exists( inet_ntop HAVE_INET_NTOP )
check_function_exists( inet_pton HAVE_INET_PTON )
check_function_exists( setfsent HAVE_SETFSENT )
check_function_exists( setgroups HAVE_SETGROUPS )
check_function_exists( setlocale HAVE_SETLOCALE )
check_function_exists( setmntent HAVE_SETMNTENT )
check_function_exists( setpriority HAVE_SETPRIORITY )
check_function_exists( freeaddrinfo HAVE_FREEADDRINFO )
check_function_exists( strtoll HAVE_STRTOLL )
check_function_exists( socket HAVE_SOCKET )
check_function_exists( strfmon HAVE_STRFMON )
check_function_exists( stpcpy HAVE_STPCPY )
check_function_exists( readdir_r HAVE_READDIR_R )
check_function_exists( tcgetattr HAVE_TCGETATTR )
check_function_exists( tcsetattr HAVE_TCSETATTR )
check_function_exists( strcasecmp HAVE_STRCASECMP )
check_function_exists( strchr HAVE_STRCHR )
check_function_exists( strcmp HAVE_STRCMP )
check_function_exists( strrchr HAVE_STRRCHR )
check_function_exists( posix_openpt HAVE_POSIX_OPENPT )
check_function_exists( ptsname HAVE_PTSNAME )
check_function_exists( unlockpt HAVE_UNLOCKPT )
check_function_exists( _getpty HAVE__GETPTY )
check_function_exists( __argz_count HAVE___ARGZ_COUNT )
check_function_exists( __argz_next HAVE___ARGZ_NEXT )
check_function_exists( __argz_stringify HAVE___ARGZ_STRINGIFY )
check_function_exists( sendfile HAVE_SENDFILE )
set( SENDFILE_LIBRARIES "" )
if( NOT HAVE_SENDFILE)
  check_library_exists( sendfile sendfile "" HAVE_SENDFILE_LIB )
  if( HAVE_SENDFILE_LIB )
    set( HAVE_SENDFILE 1 )
    set( SENDFILE_LIBRARIES sendfile )
  endif( HAVE_SENDFILE_LIB )
endif()
check_function_exists( rindex HAVE_RINDEX )
check_function_exists( putenv HAVE_PUTENV )
check_function_exists( poll HAVE_POLL )
check_function_exists( madvise HAVE_MADVISE )
check_function_exists( getgroups HAVE_GETGROUPS )
check_symbol_exists( posix_fadvise "fcntl.h" HAVE_FADVISE )
check_function_exists( getcwd HAVE_GETCWD )
check_function_exists( dlerror HAVE_DLERROR )
if( NOT HAVE_DLERROR )
  check_library_exists( dl dlerror "" HAVE_DLERROR_LIB )
  if( HAVE_DLERROR_LIB )
    set( HAVE_DLERROR 1 )
    set( DLERROR_LIBRARIES dl )
  endif( HAVE_DLERROR_LIB )
endif( NOT HAVE_DLERROR )
check_function_exists( crypt HAVE_CRYPT )
if( NOT HAVE_CRYPT )
  check_library_exists( crypt crypt "" HAVE_CRYPT_LIB )
  if( HAVE_CRYPT_LIB )
    set( HAVE_CRYPT 1 )
    set( CRYPT_LIBRARIES crypt )
  endif( HAVE_CRYPT_LIB )
endif( NOT HAVE_CRYPT )
if( NOT HAVE_CRYPT )
  check_library_exists( c crypt "" HAVE_CRYPT_LIBC )
  if( HAVE_CRYPT_LIBC )
    set( HAVE_CRYPT 1 )
    set( CRYPT_LIBRARIES c )
  endif( HAVE_CRYPT_LIBC )
endif( NOT HAVE_CRYPT )
check_function_exists( mmap HAVE_MMAP )
check_function_exists( munmap HAVE_MUNMAP )

check_type_size( "char*" SIZEOF_CHAR_P )
check_type_size( "int" SIZEOF_INT )
check_type_size( "long" SIZEOF_LONG )
check_type_size( "short" SIZEOF_SHORT )
check_type_size( "size_t" SIZEOF_SIZE_T )
check_type_size( "unsigned long" SIZEOF_UNSIGNED_LONG )

set( CMAKE_EXTRA_INCLUDE_FILES "netdb.h" )
check_type_size( "struct addrinfo" STRUCT_ADDRINFO )
set( CMAKE_EXTRA_INCLUDE_FILES "netinet/in.h" )
check_type_size( "struct sockaddr_in6" STRUCT_SOCKADDR_IN6 )
check_struct_has_member( "struct sockaddr_in6" sin6_scope_id "sys/types.h;sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN6_SIN6_SCOPE_ID )
check_struct_has_member( "struct sockaddr" sa_len "sys/types.h;sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_SA_LEN )
check_struct_has_member( "struct sockaddr_un" sun_len "sys/types.h;sys/un.h" HAVE_STRUCT_SOCKADDR_UN_SUN_LEN )
set( CMAKE_EXTRA_INCLUDE_FILES "sys/socket.h" )
check_type_size( "struct ucred" STRUCT_UCRED )
check_struct_has_member( "struct sockaddr_in" sin_len "sys/types.h;sys/socket.h;netinet/in.h" HAVE_STRUCT_SOCKADDR_IN_SIN_LEN )
unset( CMAKE_EXTRA_INCLUDE_FILES )

check_c_source_compiles("
  #include <features.h>
  #ifndef __GNU_LIBRARY__
  #error
  #endif
  int main(int argc, char *argv[]) { return 0; } "
  _GNU_SOURCE )

check_c_source_compiles("
  #include <unistd.h>
  int main(int argc, char *argv[]) { revoke(\"/dev/tty\"); return 0; } "
  HAVE_REVOKE )

check_c_source_runs("
  #include <sys/types.h>
  #include <sys/socket.h>
  #include <netdb.h>
  #include <string.h>
  int main()
  {
    struct addrinfo hint, *res;
    int err;
    memset(&hint, 0, sizeof(hint));
    hint.ai_family = AF_INET;
    hint.ai_protocol = 0;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_flags = AI_PASSIVE;
    err = getaddrinfo(0, \"18300\", &hint, &res); /* kxmlrpc tries this */
    if (err != 0 || res == 0 || res->ai_family != AF_INET)
      return 1;
    return 0;
  }"
  HAVE_GOOD_GETADDRINFO
)

if( NOT HAVE_GOOD_GETADDRINFO )
  set( HAVE_BROKEN_GETADDRINFO 1 )
endif( NOT HAVE_GOOD_GETADDRINFO )


# save CMAKE_REQUIRED_LIBRARIES
set( bak_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} )

check_symbol_exists( res_init "sys/types.h;netinet/in.h;arpa/nameser.h;resolv.h" HAVE_RES_INIT_PROTO )

set( CMAKE_REQUIRED_LIBRARIES resolv )
check_c_source_compiles("
  #include <sys/types.h>
  #include <netinet/in.h>
  #include <arpa/nameser.h>
  #include <resolv.h>
  int main(int argc, char *argv[]) { res_init(); return 0; } "
  HAVE_RES_INIT )

if( HAVE_RES_INIT )
  set( RESOLV_LIBRARIES resolv )
endif( HAVE_RES_INIT)

set( CMAKE_REQUIRED_LIBRARIES util )
if( HAVE_PTY_H )
  set( USE_OPENPTY_H pty.h )
elseif( HAVE_UTIL_H )
  set( USE_OPENPTY_H util.h )
elseif( HAVE_LIBUTIL_H )
  set( USE_OPENPTY_H libutil.h )
endif( )
if( USE_OPENPTY_H )
  check_c_source_runs("
      #include <${USE_OPENPTY_H}>
      int main(int argc, char* argv[]) {
        int master_fd, slave_fd;
        int result;
        result = openpty(&master_fd, &slave_fd, 0, 0, 0);
        return 0;
    }"
    HAVE_OPENPTY
  )
endif( )
if( HAVE_OPENPTY )
  set( LIB_UTIL util )
endif( )

# restore CMAKE_REQUIRED_LIBRARIES
set( CMAKE_REQUIRED_LIBRARIES ${bak_CMAKE_REQUIRED_LIBRARIES} )
unset( bak_CMAKE_REQUIRED_LIBRARIES )


#### set tdesu backend ##########################

if( WITH_SUDO_TDESU_BACKEND )
  set( DEFAULT_SUPER_USER_COMMAND sudo CACHE INTERNAL "" FORCE )
else()
  set( DEFAULT_SUPER_USER_COMMAND su CACHE INTERNAL "" FORCE )
endif()
find_file( TDESU_BACKEND ${DEFAULT_SUPER_USER_COMMAND} PATHS ENV PATH )
if( TDESU_BACKEND-NOTFOUND )
  tde_message_fatal( "${DEFAULT_SUPER_USER_COMMAND} was chosen as tdesu backend, but was not found in path." )
endif( TDESU_BACKEND-NOTFOUND )


#### set some constants #########################

if( NOT "${TDEDIR}" )
  if( DEFINED ENV{TDEDIR} )
    set( TDEDIR "$ENV{TDEDIR}" )
  else( )
    set( TDEDIR "${CMAKE_INSTALL_PREFIX}" )
  endif( )
endif( )
set( KDE_COMPILER_VERSION "${CMAKE_CXX_COMPILER_ID} ${CMAKE_CXX_COMPILER_VERSION}" )
set( KDE_COMPILING_OS "${CMAKE_SYSTEM_NAME}" )
set( LTDL_OBJDIR \".\" )
set( KDELIBSUFF "${LIB_SUFFIX}" )
set( kde_socklen_t socklen_t )

if( WITH_TDEHWLIB )
  set( __TDE_HAVE_TDEHWLIB 1 )
endif( WITH_TDEHWLIB )


##### check for libdl ###########################

set( DL_LIBRARIES dl )
check_library_exists( ${DL_LIBRARIES} dlopen /lib HAVE_LIBDL )
if( NOT HAVE_LIBDL )
  unset( DL_LIBRARIES )
  check_function_exists( dlopen HAVE_DLOPEN )
  if( HAVE_DLOPEN )
    set( HAVE_LIBDL 1 )
  endif( HAVE_DLOPEN )
endif( NOT HAVE_LIBDL )
check_library_exists( dld, dld_link "" HAVE_DLD )


##### check for magic ###########################

set( MAGIC_LIBRARIES magic )
check_library_exists( ${MAGIC_LIBRARIES} magic_load "" HAVE_LIBMAGIC )
if( HAVE_LIBMAGIC )
  check_library_exists( ${MAGIC_LIBRARIES} magic_getpath "" HAVE_LIBMAGIC_GETPATH )
  if( NOT HAVE_LIBMAGIC_GETPATH )
    execute_process(
      COMMAND file --version
      OUTPUT_VARIABLE LIBMAGIC_VERSION
      ERROR_VARIABLE LIBMAGIC_VERSION
    )
    string(REGEX REPLACE ".*magic file from ([^ \n]*).*" "\\1" LIBMAGIC_PATH ${LIBMAGIC_VERSION} )
    message( STATUS "libmagick default path ${LIBMAGIC_PATH}" )
  endif( NOT HAVE_LIBMAGIC_GETPATH )
else( HAVE_LIBMAGIC )
  tde_message_fatal( "libmagic is required, but not found on your system" )
endif( HAVE_LIBMAGIC )


##### check for utempter ########################

if( WITH_UTEMPTER )
  check_include_file( utempter.h HAVE_UTEMPTER_H )
  check_library_exists( utempter addToUtmp "" HAVE_ADDTOUTEMP )
  if( NOT DEFINED UTEMPTER_HELPER )
    message( STATUS "Looking for utempter helper" )
    find_program( UTEMPTER_HELPER utempter
      PATHS
        /usr/lib/${CMAKE_LIBRARY_ARCHITECTURE}/utempter
        /usr/lib${LIB_SUFFIX}/utempter
        /usr/libexec/utempter
        /usr/libexec
        /usr/sbin
        /usr/local/lib/${CMAKE_LIBRARY_ARCHITECTURE}/utempter
        /usr/local/lib${LIB_SUFFIX}/utempter
        /usr/local/libexec/utempter
        /usr/local/libexec
        /usr/local/sbin
    )
    if( UTEMPTER_HELPER )
      message( STATUS "Looking for utempter helper - found" )
    else( )
      message( STATUS "Looking for utempter helper - not found" )
    endif( )
  endif( )
  if( HAVE_UTEMPTER_H AND HAVE_ADDTOUTEMP AND UTEMPTER_HELPER )
    set( HAVE_UTEMPTER 1 )
    set( UTEMPTER_LIBRARY utempter )
  else( )
    message(FATAL_ERROR "\nutempter are requested, but not found on your system" )
  endif( )
endif( )


##### check for zlib ############################

find_package( ZLIB )
if( NOT ZLIB_FOUND )
    message(FATAL_ERROR "\nzlib are required, but not found on your system" )
endif( NOT ZLIB_FOUND )

set( HAVE_LIBZ 1 )


#### check for bzip2 ############################

find_package( BZip2 )
if( NOT BZIP2_FOUND )
  # FIXME I'm not sure if bzip2 are required; maybe is OPTIONal?
  message(FATAL_ERROR "\nbzip2 are required, but not found on your system" )
endif( NOT BZIP2_FOUND )

set( HAVE_BZIP2_SUPPORT 1 )

if( BZIP2_NEED_PREFIX )
  set( NEED_BZ2_PREFIX 1 )
endif( BZIP2_NEED_PREFIX )


##### check for lzma/xz #########################

if( WITH_LZMA )
  pkg_search_module( LZMA liblzma )
  if( NOT LZMA_FOUND )
    find_path( LZMA_INCLUDE_DIR lzma.h )
    find_library( LZMA_LIBRARY lzma )
    if( LZMA_LIBRARY AND LZMA_INCLUDE_DIR )
      message( STATUS "Found LZMA: ${LZMA_LIBRARY}" )
      set( LZMA_FOUND 1 )
      set( LZMA_INCLUDE_DIRS ${LZMA_INCLUDE_DIR} )
      set( LZMA_LIBRARIES lzma )
    endif( LZMA_LIBRARY AND LZMA_INCLUDE_DIR )
  endif( NOT LZMA_FOUND )
  check_library_exists( lzma lzma_code "" LZMA_CODE_FOUND )
  if( NOT LZMA_FOUND OR NOT LZMA_CODE_FOUND )
    tde_message_fatal( "LZMA is requested, but not found on your system" )
  endif( NOT LZMA_FOUND OR NOT LZMA_CODE_FOUND )
  set( HAVE_XZ_SUPPORT 1 )
endif( WITH_LZMA )


##### check for pcsc ############################

if( WITH_TDEHWLIB AND WITH_PCSC )
  pkg_search_module( PCSCLITE libpcsclite )
  if( NOT PCSCLITE_FOUND )
    find_path( PCSCLITE_INCLUDE_DIR pcsclite.h
      HINTS /usr/include/PCSC )
    find_library( PCSCLITE_LIBRARY NAMES pcsclite libpcsclite PCSC )
    if( PCSCLITE_LIBRARY AND PCSCLITE_INCLUDE_DIR )
      message( STATUS "Found PCSCLITE: ${PCSCLITE_LIBRARY}" )
      set( PCSCLITE_FOUND 1 )
      set( PCSCLITE_INCLUDE_DIRS ${PCSCLITE_INCLUDE_DIR} )
      set( PCSCLITE_LIBRARIES ${PCSCLITE_LIBRARY} )
    endif( PCSCLITE_LIBRARY AND PCSCLITE_INCLUDE_DIR )
  endif( NOT PCSCLITE_FOUND )
  if( NOT PCSCLITE_FOUND )
    tde_message_fatal( "PCSCLITE is requested, but not found on your system" )
  endif( NOT PCSCLITE_FOUND )
  set( HAVE_PCSC_SUPPORT 1 )
endif( WITH_TDEHWLIB AND WITH_PCSC )


##### check for pkcs ############################

if( WITH_TDEHWLIB AND WITH_PKCS )
  pkg_search_module( LIBPKCS11-HELPER libpkcs11-helper-1 )
  if( NOT LIBPKCS11-HELPER_FOUND )
      message(FATAL_ERROR "\nPKCS support was requested, but libpkcs11-helper-1 was not found on your system" )
  endif( NOT LIBPKCS11-HELPER_FOUND )
  find_library( OPENSC_PKCS11_PROVIDER_LIBRARY NAMES opensc-pkcs11.so )
  if( OPENSC_PKCS11_PROVIDER_LIBRARY )
    set( OPENSC_PKCS11_PROVIDER_FOUND 1 )
    message( STATUS "Found OpenSC PKCS11 provider: ${OPENSC_PKCS11_PROVIDER_LIBRARY}" )
  endif( OPENSC_PKCS11_PROVIDER_LIBRARY )
  if( NOT OPENSC_PKCS11_PROVIDER_FOUND )
    tde_message_fatal( "PKCS is requested, but no OpenSC PKCS11 provider was found on your system" )
  endif( NOT OPENSC_PKCS11_PROVIDER_FOUND )
  pkg_search_module( SSL openssl )
  if( SSL_FOUND )
    set( HAVE_PKCS 1 )
  endif( )
endif( )


##### check for pkcs ############################

if( WITH_TDEHWLIB AND WITH_CRYPTSETUP )
  pkg_search_module( LIBCRYPTSETUP libcryptsetup )
  if( NOT LIBCRYPTSETUP_FOUND )
    find_library( LIBCRYPTSETUP_LIBRARY cryptsetup )
    if( NOT LIBCRYPTSETUP_LIBRARY )
      message(FATAL_ERROR "\ncryptsetup support was requested, but libcryptsetup was not found on your system" )
    endif ( NOT LIBCRYPTSETUP_LIBRARY )
    set( LIBCRYPTSETUP_LIBRARIES ${LIBCRYPTSETUP_LIBRARY} )
  endif( NOT LIBCRYPTSETUP_FOUND )
  set( HAVE_CRYPTSETUP 1 )
  if ( HAVE_CRYPTSETUP )
    # save CMAKE_REQUIRED_LIBRARIES
    set( bak_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} )
    set( CMAKE_REQUIRED_LIBRARIES ${LIBCRYPTSETUP_LIBRARIES} )
    check_c_source_compiles("
      #include <stddef.h>
      #include <libcryptsetup.h>
      int main(int argc, char *argv[]) { crypt_keyslot_info status = CRYPT_SLOT_ACTIVE; return 0; } "
      HAVE_NEW_CRYPTSETUP )
    check_c_source_compiles("
      #include <stddef.h>
      #include <libcryptsetup.h>
      int main(int argc, char *argv[]) { crypt_get_type(NULL); return 0; } "
      HAVE_CRYPTSETUP_GET_TYPE )
    # restore CMAKE_REQUIRED_LIBRARIES
    set( CMAKE_REQUIRED_LIBRARIES ${bak_CMAKE_REQUIRED_LIBRARIES} )
    unset( bak_CMAKE_REQUIRED_LIBRARIES )
  endif( )
  if ( NOT HAVE_NEW_CRYPTSETUP )
    set( CRYPTSETUP_OLD_API 1 )
  endif( )
endif( )


##### check for jpeg ############################

find_package( JPEG )
if( NOT JPEG_FOUND )
  # FIXME I think jpeg support must be required
  message(FATAL_ERROR "\njpeg are required, but not found on your system" )
endif( NOT JPEG_FOUND )

set( HAVE_LIBJPEG 1 )


##### check for png #############################

find_package( PNG )
if( NOT PNG_FOUND )
  # FIXME I think png support must be required
  message(FATAL_ERROR "\npng are required, but not found on your system" )
endif( NOT PNG_FOUND )

set( HAVE_LIBPNG 1 )


##### check for tiff ############################

if( WITH_TIFF )
  find_package( TIFF )
  if( NOT TIFF_FOUND )
    message(FATAL_ERROR "\ntiff are requested, but not found on your system" )
  endif( NOT TIFF_FOUND )
  set( HAVE_LIBTIFF 1)
endif( WITH_TIFF )


##### check for jasper ##########################

if( WITH_JASPER )
  find_package( Jasper )
  if( NOT JASPER_FOUND )
    message(FATAL_ERROR "\njasper are requested, but not found on your system" )
  endif( NOT JASPER_FOUND )
  set( HAVE_JASPER 1 )
endif( WITH_JASPER )


##### check for webp ############################

if( WITH_WEBP )
  pkg_search_module( WEBP libwebp )
  if( WEBP_FOUND )
    set( HAVE_WEBP 1 )
  else ( NOT WEBP_FOUND )
    check_include_file( "webp/decode.h" HAVE_WEBP_DECODE_H)
    if ( HAVE_WEBP_DECODE_H )
      check_library_exists( webp WebPGetInfo        "" HAVE_WEBP__WEBPGETINFO )
      check_library_exists( webp WebPDecodeARGBInto "" HAVE_WEBP__WEBPDECODEARGBINTO )
      check_library_exists( webp WebPDecodeBGRAInto "" HAVE_WEBP__WEBPDECODEBGRAINTO )
      if( HAVE_WEBP__WEBPGETINFO AND HAVE_WEBP__WEBPDECODEARGBINTO AND HAVE_WEBP__WEBPDECODEBGRAINTO )
        set( HAVE_WEBP 1 )
      endif( )
    endif( HAVE_WEBP_DECODE_H )
  endif( WEBP_FOUND )
  if ( HAVE_WEBP )
    set( WEBP_LIBRARIES webp )
    message( STATUS "WebP support enabled" )
  else ( NOT HAVE_WEBP )
    tde_message_fatal( "WebP support requested, but not found on your system" )
  endif( HAVE_WEBP)
endif( WITH_WEBP )


##### check for openexr #########################

if( WITH_OPENEXR )
  pkg_search_module( OPENEXR OpenEXR )
  if( NOT OPENEXR_FOUND )
    message(FATAL_ERROR "\nopenexr are requested, but not found on your system" )
  endif( NOT OPENEXR_FOUND )
  set( HAVE_EXR 1 )
endif( WITH_OPENEXR )


##### check for freetype2 #######################

pkg_search_module( FREETYPE freetype2 )
if( NOT FREETYPE_FOUND )
    message(FATAL_ERROR "\nfreetype2 are required, but not found on your system" )
endif( NOT FREETYPE_FOUND )

set( TDEINIT_USE_FONTCONFIG 1 )


##### check for fontconfig ######################

pkg_search_module( FONTCONFIG fontconfig )
if( NOT FONTCONFIG_FOUND )
    message(FATAL_ERROR "\nfontconfig are required, but not found on your system" )
endif( NOT FONTCONFIG_FOUND )


##### check for Xrandr ##########################

if( WITH_XRANDR )
  pkg_search_module( XRANDR xrandr>=1.2 )
  if( XRANDR_FOUND )
    set( XRANDR_SUPPORT 1 )
  else( XRANDR_FOUND )
    tde_message_fatal( "xrandr support was required, but not found on your system" )
  endif( XRANDR_FOUND )
endif( WITH_XRANDR )


##### check for xrender #########################

pkg_search_module( XRENDER xrender )
if( XRENDER_FOUND )
  set( HAVE_XRENDER 1 )
  list( APPEND TDECORE_LIBRARY_DIRS ${XRENDER_LIBRARY_DIRS} )
else( XRENDER_FOUND)
  message(FATAL_ERROR "\nxrender support is required, but not found on your system" )
endif( XRENDER_FOUND )


##### check for xcomposite #########################

if( WITH_XCOMPOSITE )
  pkg_search_module( XCOMPOSITE xcomposite )
  if( XCOMPOSITE_FOUND )
    set( HAVE_XCOMPOSITE 1 )
  else( WITH_XCOMPOSITE )
    tde_message_fatal( "xcomposite support was requested, but xcomposite was not found on your system" )
  endif( XCOMPOSITE_FOUND )
endif( WITH_XCOMPOSITE )


##### check for mit-shm extension ##################

if( WITH_MITSHM )
  tde_save_and_set( CMAKE_REQUIRED_FLAGS "-include X11/Xlib.h" )
  check_include_file( "X11/extensions/XShm.h" HAVE_MITSHM )
  tde_restore( CMAKE_REQUIRED_FLAGS )
  if( NOT HAVE_MITSHM )
    tde_message_fatal( "XShm extension is required, but was not found on your system" )
  endif( NOT HAVE_MITSHM )
endif( WITH_MITSHM )


##### check for xext ###############################

if( HAVE_XRENDER OR WITH_XCOMPOSITE OR WITH_XRANDR OR HAVE_X11_EXTENSIONS_SHAPE_H OR WITH_MITSHM )
  # xext (tdecore, tdeui, tderandr, tdestyles/keramik)
  pkg_search_module( XEXT xext )
  if( NOT XEXT_FOUND )
    tde_message_fatal( "xext is required, but was not found on your system" )
  endif( )
endif( HAVE_XRENDER OR WITH_XCOMPOSITE OR WITH_XRANDR OR HAVE_X11_EXTENSIONS_SHAPE_H OR WITH_MITSHM )


##### check for libxml-2.0 ######################

pkg_search_module( LIBXML2 libxml-2.0 )
if( NOT LIBXML2_FOUND )
    message(FATAL_ERROR "\nlibxml-2.0 are required, but not found on your system" )
endif( NOT LIBXML2_FOUND )


##### check for libxslt ######################

pkg_search_module( LIBXSLT libxslt )
if( NOT LIBXSLT_FOUND )
    message(FATAL_ERROR "\nlibxslt are required, but not found on your system" )
endif( NOT LIBXSLT_FOUND )


##### check for glib-2.0 & friends ##############

pkg_search_module( GLIB2 glib-2.0 )
if( NOT GLIB2_FOUND )
  tde_message_fatal( "glib-2.0 are required, but not found on your system" )
endif( )

pkg_search_module( GOBJECT2 gobject-2.0 )
if( NOT GLIB2_FOUND )
  tde_message_fatal( "gobject-2.0 are required, but not found on your system" )
endif( )


##### check for ALSA ############################
# FIXME I propose to drop support for very old ALSA versions (i.e 0.5)

if( WITH_ALSA )
  find_package( ALSA )
  if( NOT ALSA_FOUND )
    message(FATAL_ERROR "\nALSA support are requested, but not found on your system" )
  endif( NOT ALSA_FOUND )
  set( HAVE_LIBASOUND2 1 )

  # FIXME I'm not sure if order is correct
  check_include_file( "alsa/asoundlib.h" HAVE_ALSA_ASOUNDLIB_H )
  if( NOT HAVE_ALSA_ASOUNDLIB_H )
    check_include_file( "sys/asoundlib.h" HAVE_SYS_ASOUNDLIB_H )
  endif( NOT HAVE_ALSA_ASOUNDLIB_H )
endif( WITH_ALSA )


##### check for aRts ############################

if( WITH_ARTS )
  pkg_search_module( ARTS arts )
  if( ARTS_FOUND )
    set( ARTS_MCOPIDL_EXECUTABLE ${ARTS_PREFIX}/bin/mcopidl )
  else( ARTS_FOUND )
    message(FATAL_ERROR "\naRts are requested, but not found on your system" )
  endif( ARTS_FOUND )
else( WITH_ARTS )
  set( WITHOUT_ARTS 1 )
endif( WITH_ARTS )


##### check for Xft #############################

string( REGEX REPLACE ".*;([^;]*qt-mt);.*" "\\1" TQT_MT ";${TQT_LIBRARIES};" )
if( TQT_MT )
  pkg_search_module( TQT_MT ${TQT_MT} )
  tde_save_and_set( CMAKE_REQUIRED_LIBRARIES "${TQT_MT_LIBRARIES}" )
  check_library_exists( tqt-mt XftInit "" TDEINIT_USE_XFT )
  tde_restore( CMAKE_REQUIRED_LIBRARIES )
endif( TQT_MT )
if( TDEINIT_USE_XFT )
  pkg_search_module( XFT xft )
  if( NOT XFT_FOUND )
    tde_message_fatal( "Xft is required, but not found on your system" )
  endif( NOT XFT_FOUND )
endif( TDEINIT_USE_XFT )


##### check for libart ##########################

if( WITH_LIBART )
  pkg_search_module( LIBART libart-2.0 )
  if( NOT LIBART_FOUND )
    message(FATAL_ERROR "\nlibart-2.0 support are requested, but not found on your system" )
  endif( NOT LIBART_FOUND )
  set( HAVE_LIBART 1 )
  list( APPEND TDECORE_LIBRARY_DIRS ${LIBART_LIBRARY_DIRS} )
endif( WITH_LIBART )


##### check for libidn ##########################

if( WITH_LIBIDN )
  pkg_search_module( LIBIDN libidn )
  if( NOT LIBIDN_FOUND )
    message(FATAL_ERROR "\nlibidn support are requested, but not found on your system" )
  endif( NOT LIBIDN_FOUND )
  set( HAVE_LIBIDN 1 )
  check_include_file( "idna.h" HAVE_IDNA_H )
  check_include_file( "punycode.h" HAVE_PUNYCODE_H )
  check_include_file( "stringprep.h" HAVE_STRINGPREP_H )
  list( APPEND TDECORE_LIBRARY_DIRS ${LIBIDN_LIBRARY_DIRS} )
endif( WITH_LIBIDN )

##### check for libbfq ##########################

if( WITH_LIBBFD )
  check_library_exists( bfd bfd_init "" HAVE_LIBBFD )
  if( NOT HAVE_LIBBFD )
    tde_message_fatal( "libbfd support is requested, but not found on your system" )
  endif( NOT HAVE_LIBBFD )
  set( LIBBFD_LIBRARIES bfd )
  check_include_file( "demangle.h" HAVE_DEMANGLE_H )
endif( WITH_LIBBFD )


##### check for openssl #########################

if( WITH_SSL )
  pkg_search_module( SSL openssl )
  if( NOT SSL_FOUND )
    check_include_file( openssl/ssl.h HAVE_OPENSSL_H )
    check_library_exists( ssl OPENSSL_init_ssl "" HAVE_LIBSSL_11 )
    if( NOT HAVE_LIBSSL_11 )
      check_library_exists( ssl SSL_library_init "" HAVE_LIBSSL )
    endif( )
    check_library_exists( crypto EVP_EncryptInit_ex "" HAVE_LIBCRYPTO )
    if( HAVE_OPENSSL_H AND (HAVE_LIBSSL_11 OR HAVE_LIBSSL) AND HAVE_LIBCRYPTO )
      set( SSL_FOUND 1 CACHE INTERNAL "" FORCE )
      find_file( OPENSSLV_H openssl/opensslv.h )
      file( STRINGS "${OPENSSLV_H}" SSL_VERSION REGEX "#[ \t]*define[ \t]*OPENSSL_VERSION_TEXT" )
      string( REGEX REPLACE "# *define[ \t]*OPENSSL_VERSION_TEXT[ \t]*\"[^0-9 ]* *([^ ]*).*" "\\1"
              SSL_VERSION "${SSL_VERSION}" )
      set( SSL_VERSION "${SSL_VERSION}" CACHE INTERNAL "" FORCE )
      message( STATUS "Found OpenSSL: version ${SSL_VERSION}" )
    endif( HAVE_OPENSSL_H AND (HAVE_LIBSSL_11 OR HAVE_LIBSSL) AND HAVE_LIBCRYPTO )
  endif( NOT SSL_FOUND )
  if( NOT SSL_FOUND )
    tde_message_fatal( "SSL support is requested, but openssl not found on your system" )
  endif( NOT SSL_FOUND )
  set( KSSL_HAVE_SSL 1 )
  set( HAVE_SSL 1 )
endif( WITH_SSL )


##### check for cups ############################
# FIXME CUPS must be at least 1.1.9

if( WITH_CUPS )
  find_package( Cups )
  if( NOT CUPS_FOUND )
    message(FATAL_ERROR "\nCUPS support are requested, but not found on your system" )
  endif( NOT CUPS_FOUND )
  set( HAVE_CUPS 1 )
  # FIXME if CUPS is at least 1.1.20, doesn't have password caching
  set( HAVE_CUPS_NO_PWD_CACHE 1 )

  # save CMAKE_REQUIRED_LIBRARIES
  set( bak_CMAKE_REQUIRED_LIBRARIES ${CMAKE_REQUIRED_LIBRARIES} )
  set( CMAKE_REQUIRED_LIBRARIES ${CUPS_LIBRARIES} )
  check_c_source_compiles("
    #include <cups/cups.h>
    #include <cups/ipp.h>
    #include <cups/language.h>
    int main(int argc, char *argv[]) { ipp_attribute_t *attr = 0; ippGetName(attr); return 0; } "
    HAVE_CUPS_1_6 )
  # restore CMAKE_REQUIRED_LIBRARIES
  set( CMAKE_REQUIRED_LIBRARIES ${bak_CMAKE_REQUIRED_LIBRARIES} )
  unset( bak_CMAKE_REQUIRED_LIBRARIES )
endif( WITH_CUPS )


##### check for LUA #############################

if( WITH_LUA )
  message(FATAL_ERROR "\nLUA support are not ready yet" )
  pkg_search_module( LUA lua )
  if( NOT LUA_FOUND )
    message(FATAL_ERROR "\nLUA support are requested, but not found on your system" )
  endif( NOT LUA_FOUND )
  set( HAVE_LUA 1 )
endif( WITH_LUA )


##### check for avahi ###########################

if( WITH_AVAHI )
    pkg_search_module( AVAHI_TQT avahi-tqt )
    pkg_search_module( AVAHI_CLIENT avahi-client )
    if( NOT AVAHI_TQT_FOUND OR NOT AVAHI_CLIENT_FOUND )
        tde_message_fatal( "avahi support is requested, but not found on your system" )
    endif( NOT AVAHI_TQT_FOUND OR NOT AVAHI_CLIENT_FOUND )
    if( NOT AVAHI_CLIENT_VERSION VERSION_LESS "0.6" )
      set( AVAHI_API_0_6 1 )
    endif( )
    set( HAVE_DNSSD 1 )
endif( )


##### check for libr ###########################
# WARNING
# before altering the LIBR_VERSION check,
# please ensure that nothing changed in the
# duplicated data definitions present in
# tdeio/tdeio/tdefileitem.cpp

if( WITH_ELFICON )
  pkg_search_module( LIBR libr )
  if( NOT LIBR_FOUND )
      message(FATAL_ERROR "\nelficon support was requested, but libr was not found on your system" )
  endif( NOT LIBR_FOUND )
  if( "${LIBR_VERSION}" VERSION_LESS "0.6" )
      message(FATAL_ERROR "\nelficon support was requested, but the libr version on your system may not be compatible with TDE" )
  endif( "${LIBR_VERSION}" VERSION_LESS "0.6" )
  set( HAVE_ELFICON 1 )
endif( )


##### check for pcre2 ###########################

if( WITH_PCRE2 )
  pkg_check_modules( LIBPCRE2 libpcre2-8 libpcre2-posix )
  if( NOT LIBPCRE2_FOUND )
    tde_message_fatal( "pcre2 support was requested, but not found on your system" )
  endif( )
  set( HAVE_PCRE2POSIX 1 )
endif( )


##### check for dbus and set files location #####

tde_setup_dbus( dbus-1-tqt )

list( APPEND TDECORE_LIBRARY_DIRS ${DBUS_TQT_LIBRARY_DIRS} )

message(STATUS "DBUS_TQT_LIBRARY_DIRS = ${DBUS_TQT_LIBRARY_DIRS}")


##### Add '[KDE4]' to KDE4 menu items #########

if( WITH_KDE4_MENU_SUFFIX )
  set( KDE4_MENU_SUFFIX 1 )
endif ( WITH_KDE4_MENU_SUFFIX )


##### check for gcc visibility support #########

if( WITH_GCC_VISIBILITY )
  tde_setup_gcc_visibility( )
endif( )


##### check for inotify ###########################

if( WITH_INOTIFY )
  check_include_file( "inotify.h" INOTIFY_FOUND )
  check_include_file( "sys/inotify.h" INOTIFY_SYS_FOUND )
  if( NOT INOTIFY_FOUND )
    if( NOT INOTIFY_SYS_FOUND )
      message(FATAL_ERROR "\ninotify support was requested, but inotify was not found on your system" )
    endif( NOT INOTIFY_SYS_FOUND )
  endif( NOT INOTIFY_FOUND )
  if( INOTIFY_FOUND )
      set( HAVE_INOTIFY 1 )
  endif( INOTIFY_FOUND )
  if( INOTIFY_SYS_FOUND )
      set( HAVE_INOTIFY 1 )
      set( HAVE_SYS_INOTIFY 1 )
  endif( INOTIFY_SYS_FOUND )
endif( )


##### check for FAM/GAMIN ##########################

if( WITH_GAMIN )
  check_include_file( "fam.h" HAVE_FAM_H )
  if( HAVE_FAM_H )
    pkg_search_module( GAMIN gamin )
    if( GAMIN_FOUND )
      set( HAVE_FAM 1 )
      list( APPEND TDECORE_LIBRARY_DIRS ${GAMIN_LIBRARY_DIRS} )
    else( GAMIN_FOUND )
      message(FATAL_ERROR "\nfam/gamin support was requested, but gamin was not found on your system" )
    endif( GAMIN_FOUND )
  else( HAVE_FAM_H )
    message(FATAL_ERROR "\nfam/gamin support was requested, but fam was not found on your system" )
  endif( HAVE_FAM_H )
endif( WITH_GAMIN )


##### check for ispell ##########################

if( WITH_ISPELL )
  if( NOT ISPELL_LIBDIR )
    find_program( ISPELL_EXECUTABLE NAMES ispell DOC "path to the ispell executable" )
    if( ISPELL_EXECUTABLE )
      execute_process (
        COMMAND ${ISPELL_EXECUTABLE} -vv
        COMMAND awk "$1 == \"LIBDIR\" && $2 == \"=\" {gsub(/\"/, \"\", $3); print $3;}"
        OUTPUT_VARIABLE ISPELL_LIBDIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
      if ( ISPELL_LIBDIR )
        set ( ISPELL_LIBDIR "${ISPELL_LIBDIR}" CACHE PATH "ispell lib directory contains dictionaries" )
      else ( ISPELL_LIBDIR )
        unset ( ISPELL_LIBDIR )
      endif ( ISPELL_LIBDIR )
    endif( ISPELL_EXECUTABLE )
  endif ( NOT ISPELL_LIBDIR )
endif( WITH_ISPELL )


##### check for aspell ##########################

if( WITH_ASPELL )
  find_package( ASPELL )
  if( NOT ASPELL_FOUND )
    message(FATAL_ERROR "\nASPELL support are requested, but not found on your system" )
  endif( NOT ASPELL_FOUND )
  if( NOT ASPELL_DATADIR )
    find_program( PSPELL_CONFIG_EXECUTABLE NAMES pspell-config DOC "path to the pspell-config executable" )
    if( PSPELL_CONFIG_EXECUTABLE )
      execute_process (
        COMMAND ${PSPELL_CONFIG_EXECUTABLE} --pkgdatadir
        OUTPUT_VARIABLE ASPELL_DATADIR
        OUTPUT_STRIP_TRAILING_WHITESPACE)
    endif( PSPELL_CONFIG_EXECUTABLE )
  endif( NOT ASPELL_DATADIR )
endif( WITH_ASPELL )
if( NOT ASPELL_DATADIR )
  set ( ASPELL_DATADIR "/usr/lib/aspell" )
endif( NOT ASPELL_DATADIR )
set ( ASPELL_DATADIR "${ASPELL_DATADIR}" CACHE PATH "aspell data directory" )


##### check for hspell ##########################

if( WITH_HSPELL )
  find_package( HSPELL )
  if( NOT HSPELL_FOUND )
    message(FATAL_ERROR "\nHSPELL support are requested, but not found on your system" )
  endif( NOT HSPELL_FOUND )
endif( WITH_HSPELL )


####" default spell checker #####################

set( DEFAULT_SPELL_CHECKER "ISPELL" CACHE STRING "default spell checker" )
if( NOT WITH_${DEFAULT_SPELL_CHECKER} )
  tde_message_fatal( "Spell checker selected as default (${DEFAULT_SPELL_CHECKER}) is not enabled to build." )
endif()


##### enable TDEIconLoader debugging ############

if ( WITH_TDEICONLOADER_DEBUG )
  set ( TDEICONLOADER_DEBUG 1 )
endif ( WITH_TDEICONLOADER_DEBUG )


##### find tools path ###########################

if( NOT WIN32 AND NOT ICEAUTH_PATH )
  find_program( ICEAUTH_PATH iceauth )
  if( ${ICEAUTH_PATH} STREQUAL "ICEAUTH_PATH-NOTFOUND" )
    tde_message_fatal( "iceauth command was not found in path." )
  endif()
  message( STATUS "Found iceauth: ${ICEAUTH_PATH}" )
endif( NOT WIN32 AND NOT ICEAUTH_PATH )


##### find the rgb.txt file

message( STATUS "Looking for rgb.txt" )
find_file( PATH_RGB rgb.txt
    PATHS
        /usr/share/X11
        /usr/X11R6/lib/X11
        /usr/openwin/lib/X11
)
if( PATH_RGB )
    message( STATUS "Found rgb.txt file: ${PATH_RGB}" )
    set( X11_RGBFILE "${PATH_RGB}" )
endif()


##### Look for xmllint

find_program( XMLLINT_EXE xmllint )
if( XMLLINT_EXE )
    set( XMLLINT "${XMLLINT_EXE}" )
    message( STATUS "Found xmllint: ${XMLLINT}" )
endif()


##### write configure files #####################

configure_file( config.h.cmake config.h )
configure_file( tdecore/tdemacros.h.cmake tdecore/tdemacros.h )
configure_file( kjs/global.h.in kjs/global.h )
configure_file( tdeio/kssl/ksslconfig.h.cmake tdeio/kssl/ksslconfig.h )
configure_file( kdoctools/checkXML.cmake kdoctools/checkXML )


##### tools #####################################

set( KDE3_DCOPIDL_EXECUTABLE dcopidl )
set( KDE3_DCOPIDLNG_EXECUTABLE
  perl -I"${CMAKE_SOURCE_DIR}/dcop/dcopidlng"
       "${CMAKE_SOURCE_DIR}/dcop/dcopidlng/kalyptus"
        --allow_k_dcop_accessors -f dcopidl
)
set( KDE3_DCOPIDL2CPP_EXECUTABLE dcopidl2cpp )
set( KDE3_MEINPROC_EXECUTABLE meinproc )
set( KDE3_KCFGC_EXECUTABLE tdeconfig_compiler )


##### global compiler settings ##################

add_definitions(
  -DHAVE_CONFIG_H
)

set( CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${TQT_CXX_FLAGS}" )
set( CMAKE_SHARED_LINKER_FLAGS "${CMAKE_SHARED_LINKER_FLAGS} -Wl,--no-undefined" )
set( CMAKE_MODULE_LINKER_FLAGS "${CMAKE_MODULE_LINKER_FLAGS} -Wl,--no-undefined" )


##### headers ###################################

install( FILES
    tde.pot
  DESTINATION ${INCLUDE_INSTALL_DIR} )


##### pkg-config #################################

execute_process(
  COMMAND awk "/^#define TDE_VERSION_MAJOR/,/^#define TDE_VERSION_RELEASE/{ v[++i]=$3 }; END { print v[1]\".\"v[2]\".\"v[3] };"
  INPUT_FILE ${CMAKE_SOURCE_DIR}/tdecore/tdeversion.h
  OUTPUT_VARIABLE TDE_RELEASE_NUM
  OUTPUT_STRIP_TRAILING_WHITESPACE )
configure_file( tdelibs.pc.cmake tdelibs.pc @ONLY )
install( FILES ${CMAKE_CURRENT_BINARY_DIR}/tdelibs.pc DESTINATION ${PKGCONFIG_INSTALL_DIR} )


##### read source metadata ######################

set( TDE_VERSION "${TDE_RELEASE_NUM}" )
tde_read_src_metadata()


##### subdirectories ############################

if( WITH_ELFICON )
  add_subdirectory( tdelfeditor )
endif( WITH_ELFICON )
add_subdirectory( dcop )
add_subdirectory( libltdl )
add_subdirectory( kglib )
add_subdirectory( tdefx )
add_subdirectory( tdecore )
add_subdirectory( tdeunittest )
add_subdirectory( tdeui )
add_subdirectory( tdesu )
add_subdirectory( kjs )
add_subdirectory( tdewallet )
add_subdirectory( tdeio )
add_subdirectory( kded )

# FIXME this directory/target doesn't exists
# add_subdirectory( kded_post )

add_subdirectory( kdoctools )
add_subdirectory( doc )
add_subdirectory( tdeparts )
add_subdirectory( tdeutils )
add_subdirectory( tdespell2 )
add_subdirectory( tdemdi )
add_subdirectory( tdeinit )
add_subdirectory( tdeprint )
add_subdirectory( kab )
add_subdirectory( tderesources )
add_subdirectory( tdeabc )
add_subdirectory( arts )
add_subdirectory( interfaces )
add_subdirectory( kate )
add_subdirectory( tdecert )
tde_conditional_add_subdirectory( WITH_XRANDR tderandr )
add_subdirectory( tdehtml )
add_subdirectory( tdemarkdown )
add_subdirectory( tdecmshell )
add_subdirectory( tdeconf_update )
add_subdirectory( tdewidgets )
add_subdirectory( kimgio )
add_subdirectory( tdeioslave )
add_subdirectory( tdestyles )
add_subdirectory( libtdemid )
add_subdirectory( libtdescreensaver )
add_subdirectory( networkstatus )
add_subdirectory( tdenewstuff )
add_subdirectory( tdersync )
add_subdirectory( licenses )
add_subdirectory( dnssd )
add_subdirectory( pics )
add_subdirectory( mimetypes )
add_subdirectory( tdefile-plugins )


##### install import cmake modules ###############

tde_install_export( )
