#include <tdelibs_export.h>

#include <stdint.h>
#include <cstdlib>

#include <tqdict.h>
#include <tqvalidator.h>
#include <tqcstring.h>
#include <tqfile.h>
#include <tqdatetime.h>

extern "C" {
  #include <libr-icons.h>

  // BEGIN HACK
  // libr does not export these structures and defines,
  // but we need access to them to make the UI behave sanely
  // Keep them in sync with libr and all should be OK

  // Valid for libr version 0.6.0
  // See libr detection code in ConfigureChecks.cmake

  typedef uint32_t ID8;
  typedef uint16_t ID4;
  typedef struct {uint64_t p:48;} __attribute__((__packed__)) ID12;

  typedef struct {
    ID8  g1;
    ID4  g2;
    ID4  g3;
    ID4  g4;
    ID12 g5;
  } __attribute__((__packed__)) UUID;

  typedef struct {
    char *name;
    size_t offset;
    size_t entry_size;
    libr_icontype_t type;
    unsigned int icon_size;
  } iconentry;

  typedef struct{
    size_t size;
    char *buffer;
    iconentry entry;
  } iconlist;

  #define ICON_SECTION     ".icon"
  // END HACK
}

TDEIO_EXPORT int get_iconlist(libr_file *file_handle, iconlist *icons);
TDEIO_EXPORT iconentry *get_nexticon(iconlist *icons, iconentry *last_entry);
TDEIO_EXPORT TQString elf_get_resource(libr_file *handle, char *section_name);
