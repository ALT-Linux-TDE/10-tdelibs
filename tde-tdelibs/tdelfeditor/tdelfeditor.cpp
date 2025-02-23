/*
 *
 *  Copyright (c) 2008-2011 Erich E. Hoover
 *  TDE version (c) 2011 Timothy Pearson
 *
 *  elfres - Adds resources into ELF files
 * 
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 * 
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 * 
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * To provide feedback, report bugs, or otherwise contact me:
 * ehoover at mines dot edu
 *
 */

/* Obtaining command-line options */
#include <getopt.h>
#include <cstdlib>

/* Internationalization support */
extern "C" {
	#include <libr-i18n.h>
}

#include "tdelfeditor.h"

#include <tqstring.h>
#include <tqstringlist.h>
#include <tqfileinfo.h>

#include <tdeglobal.h>
#include <tdeaboutdata.h>
#include <tdecmdlineargs.h>
#include <kiconloader.h>
#include <tdeapplication.h>
#include <kstandarddirs.h>

/* return application name */
extern const char *__progname;
#define progname() __progname

typedef enum {
	PARAM_PROGNAME      = 0,
	PARAM_OPTIONS       = 1,
	PARAM_ELF_FILE      = 2,
	PARAM_UUID          = 3,
	PARAM_RESOURCE_NAME = 3,
	PARAM_ICON_NAME     = 3,
	PARAM_ICON_SIZE     = 3,
	PARAM_RESOURCE_FILE = 4,
	PARAM_ICON_FILE     = 4,

	PARAM_EXECUTABLE_NAME       = 3,
	PARAM_DESCRIPTION           = 4,
	PARAM_LICENSE               = 5,
	PARAM_COPYRIGHT             = 6,
	PARAM_AUTHORS               = 7,
	PARAM_PRODUCT               = 8,
	PARAM_ORGANIZATION          = 9,
	PARAM_VERSION               = 10,
	PARAM_DATETIME              = 11,
	PARAM_SYSICON               = 12,
	PARAM_SCMMODULE             = 13,
	PARAM_SCMREVISION           = 14,
	PARAM_NOTES                 = 15,
} eParams;

typedef struct {
    char short_options[ELFICON_OPTIONS+1];
    struct option long_options[ELFICON_OPTIONS+1];
    char descriptions[ELFICON_OPTIONS][100];
} IconOptions;

IconOptions elficon_options = {
	"acefgmrstv",
	{
		{"add-icon", 0, 0, 'a'},
		{"clear-icons", 0, 0, 'c'},
		{"set-empty-uuid", 0, 0, 'e'},
		{"find-icon", 0, 0, 'f'},
		{"get-uuid", 0, 0, 'g'},
		{"write-metadata", 0, 0, 'm'},
		{"retrieve-icon", 0, 0, 'r'},
		{"set-uuid", 0, 0, 's'},
		{"tde-autoadd-icon", 0, 0, 't'},
		{"version", 0, 0, 'v'},
		{NULL, 0, 0, 0}
    },
    {
        N_("add an icon to the ELF file"),
        N_("clear the file's ELF icon"),
        N_("set an empty icon UUID for the ELF file"),
        N_("find an ELF icon in the file by closest size"),
        N_("get the icon UUID for the file"),
        N_("write metadata information to the ELF file"),
        N_("retrieve an icon from the ELF file"),
        N_("set the icon UUID for the ELF file"),
        N_("automatically add the appropriate TDE icon to the ELF file"),
        N_("display the current application revision"),
    }
};

typedef struct {
    char short_options[ELFRES_OPTIONS+1];
    struct option long_options[ELFRES_OPTIONS+1];
    char descriptions[ELFRES_OPTIONS][100];
} RcOptions;

RcOptions elfres_options = {
	"aclrv",
	{
		{"add", 0, 0, 'a'},
		{"clear", 0, 0, 'c'},
		{"list", 0, 0, 'l'},
		{"retrieve", 0, 0, 'r'},
		{"version", 0, 0, 'v'},
		{NULL, 0, 0, 0}
    },
    {
        N_("add a resource to the ELF file"),
        N_("clear an ELF file resource"),
        N_("list libr-compatible resources"),
        N_("retrieve a resource from the ELF file"),
        N_("display the current application revision"),
    }
};

typedef enum {
	MODE_ADD_RESOURCE,
	MODE_CLEAR_RESOURCE,
	MODE_RETRIEVE_RESOURCE,
	MODE_LIST_RESOURCES,
	MODE_ADD_ICON,
	MODE_TDE_AUTOADD_ICON,
	MODE_RETRIEVE_ICON,
	MODE_CLEAR_ICON,
	MODE_SET_UUID,
	MODE_SET_EMPTY_UUID,
	MODE_GET_UUID,
	MODE_FIND_ICON,
	MODE_SET_METADATA,
	MODE_LAUNCH_GUI
} eMode;

/*
 * Return the size of the file represented by the file handle
 */
off_t file_size(FILE *file)
{
	struct stat file_stat;
	
	if(fstat(fileno(file), &file_stat) == ERROR)
		return ERROR;
	return file_stat.st_size;
}

/*
 * Output the application version
 */
void output_version(char *progname)
{
	printf(_("%s: Version %s\n"), progname, VERSION);
}

/*
 * Use command-line arguments to set the appropriate data mode
 */
int handle_arguments(int argc, char **argv, eMode *mode)
{
	int required_params = 0;
	int i, c, index = 0;
	
	opterr = 0;  /* Prevent automatic getopt() error message */

	while ((c = getopt_long((argc>3)?3:argc, argv, elficon_options.short_options, elficon_options.long_options, &index)) != EOF)
	{
		switch(c)
		{
			case 'a':
				*mode = MODE_ADD_ICON;
				required_params = 5;
				break;
			case 'c':
				*mode = MODE_CLEAR_ICON;
				required_params = 3;
				break;
			case 'e':
				*mode = MODE_SET_EMPTY_UUID;
				required_params = 3;
				break;
			case 'f':
				*mode = MODE_FIND_ICON;
				required_params = 5;
				break;
			case 'g':
				*mode = MODE_GET_UUID;
				required_params = 3;
				break;
			case 'm':
				*mode = MODE_SET_METADATA;
				required_params = 16;
				break;
			case 'r':
				*mode = MODE_RETRIEVE_ICON;
				required_params = 5;
				break;
			case 's':
				*mode = MODE_SET_UUID;
				required_params = 4;
				break;
			case 't':
				*mode = MODE_TDE_AUTOADD_ICON;
				required_params = 4;
				break;
			case 'v':
				output_version(argv[PARAM_PROGNAME]);
				return FALSE;
			default:
				goto print_icon_usage;
		}
		index++;
		if(argc != required_params) {
			fprintf(stderr, _("%s: was given %d arguments, needed %d\n"), argv[PARAM_PROGNAME], argc, required_params);
			goto print_icon_usage;
		}
	}
	return TRUE;
print_icon_usage:
	fprintf(stderr, _("usage: %s [-a|-r] elf-file-name icon-name svg-file-name\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("usage: %s [-t] elf-file-name icon-name\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("usage: %s [-c|-e|-g] elf-file-name\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("usage: %s [-s] elf-file-name uuid\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("usage: %s [-s] elf-file-name uuid\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("usage: %s [-m] elf-file-name \"executable name\" \"description\" \"license\" \"copyright\" \"authors\" \"product\" \"organization\" \"version\" \"datetime\" \"sysicon\" \"scmmodule\" \"scmrevision\" \"notes\"\n"), argv[PARAM_PROGNAME]);
	fprintf(stderr, _("If -t is set the TDEDIRS environment variable must include your TDE installation prefix\n"));
	fprintf(stderr, _("for example: TDEDIRS=/opt/trinity ./tdelfeditor -t ./konqueror konqueror\n"));
	for(i=0;i<ELFICON_OPTIONS;i++)
	{
		fprintf(stderr, "\t-%c, --%s\t= %s\n", elficon_options.short_options[i],
			elficon_options.long_options[i].name, gettext(elficon_options.descriptions[i]));
	}
	return FALSE;
}

/*
 * Add a resource to an ELF file
 */
int add_resource(libr_file *libr_handle, char *resource_name, char *input_file)
{
	char *buffer = NULL;
	FILE *handle = NULL;
	off_t size, len;
	int ret = FALSE;
	
	if((handle = fopen(input_file, "r")) == NULL)
	{
		errorf(_("open \"%s\" failed: %m"), input_file);
		return FALSE;
	}
	/* Get the size of the file */
	if((size = file_size(handle)) == ERROR)
	{
		errorf(_("failed to obtain file size: %m"));
		goto done_handle;
	}
	/* Allocate buffers for the uncompressed and compressed data */
	buffer = (char *) malloc(size);
	if(buffer == NULL)
	{
		errorf(_("failed to create buffer: %m"));
		goto done_handle;
	}
	/* Read the uncompressed data from the disk */
	if((len = fread(buffer, 1, size, handle)) <= 0)
	{
		errorf(_("failed to read input file: %m"));
		goto done_buffer;
	}
	if(len != size)
	{
		errorf(_("failed to read entire file."));
		goto done_buffer;
	}
	/* Compress the data */
	if(!libr_write(libr_handle, resource_name, buffer, size, LIBR_COMPRESSED, LIBR_OVERWRITE))
	{
		errorf(_("failed to write ELF resource: %s"), libr_errmsg());
		goto done_buffer;
	}
	
	ret = TRUE;
	/* Close remaining resources */
done_buffer:
	free(buffer);
done_handle:
	fclose(handle);
	return ret;
}

/*
 * Add a resource string to an ELF file
 */
int add_resource_string(libr_file *libr_handle, char *resource_name, char *input_string)
{
	off_t size, len;
	int ret = FALSE;
	
	size = strlen(input_string);
	/* Allocate buffers for the uncompressed and compressed data */
	/* Compress the data */
	if(!libr_write(libr_handle, resource_name, input_string, size, LIBR_COMPRESSED, LIBR_OVERWRITE))
	{
		errorf(_("failed to write ELF resource: %s"), libr_errmsg());
		goto done_buffer;
	}
	
	ret = TRUE;
done_buffer:
	return ret;
}

/*
 * Get a resource stored in an ELF file
 */
void get_resource(libr_file *handle, char *section_name, char *output_file)
{
	size_t buffer_size = 0;
	char *buffer = NULL;
	FILE *file = NULL;
	off_t len;
	
	/* Get the resource from the ELF binary */
	if(!libr_size(handle, section_name, &buffer_size))
	{
		errorf(_("failed to obtain ELF resource size: %s"), libr_errmsg());
		return;
	}
	/* Get the resource from the ELF file */
	buffer = (char *) malloc(buffer_size);
	if(!libr_read(handle, section_name, buffer))
	{
		errorf(_("failed to obtain ELF resource: %s"), libr_errmsg());
		goto fail;
	}
	/* Store the resource to the disk */
	if((file = fopen(output_file, "w")) == NULL)
	{
		errorf(_("open \"%s\" failed"), output_file);
		goto fail;
	}
	if((len = fwrite(buffer, 1, buffer_size, file)) <= 0)
	{
		errorf(_("failed to write output file: %m"));
		goto fail;
	}
	
fail:
	/* Close remaining resources */
	if(file != NULL)
		fclose(file);
	free(buffer);
}

/*
 * Clear the icon stored in an ELF file
 */
void clear_resource(libr_file *handle, char *resource_name)
{
	if(!libr_clear(handle, resource_name))
		errorf(_("failed to remove resource: %s"), libr_errmsg());
}

/* 
 * List all libr-compatible resources
 */
void list_resources(libr_file *handle)
{
	int i, res = libr_resources(handle);
	
	if(res == 0)
	{
		printf(_("The file contains no libr-compatible resources.\n"));
		return;
	}
	printf(_("%d resource(s):\n"), res);
	for(i=0;i<res;i++)
	{
		char *name = libr_list(handle, i);
		
		if(name == NULL)
		{
			printf(_("error!\n"));
			continue;
		}
		printf(_("resource %d: %s\n"), i, name);
		free(name);
	}
}

/*
 * Started from console
 */
int main_console(int argc, char **argv)
{
	char *section = NULL, *input_file = NULL, *output_file = NULL;
	libr_access_t access = LIBR_READ;
	libr_file *handle = NULL;
	eMode mode;
	
	/* Process command-line arguments */
	if(!handle_arguments(argc, argv, &mode))
		return EX_USAGE;
	
	/* Open the ELF file to be edited */
	if(mode == MODE_CLEAR_ICON || mode == MODE_CLEAR_RESOURCE
	 || mode == MODE_ADD_ICON || mode == MODE_ADD_RESOURCE
	 || mode == MODE_SET_UUID || mode == MODE_TDE_AUTOADD_ICON
	 || mode == MODE_SET_EMPTY_UUID || mode == MODE_SET_METADATA)
	{
		access = LIBR_READ_WRITE;
	}
	printf("opening executable file %s...\n", argv[PARAM_ELF_FILE]); fflush(stdout);
	if((handle = libr_open(argv[PARAM_ELF_FILE], access)) == NULL)
	{
		errorf(_("failed to open file \"%s\": %s"), argv[PARAM_ELF_FILE], libr_errmsg());
		return EX_SOFTWARE;
	}
	
	/* Perform the requested user operation */
	switch(mode)
	{
		case MODE_FIND_ICON:
		{
			unsigned int icon_size;
			libr_icon *icon = NULL;
			
			sscanf(argv[PARAM_ICON_SIZE], "%d", &icon_size);
			icon = libr_icon_geticon_bysize(handle, icon_size);
			if(icon == NULL)
			{
				errorf(_("failed to obtain ELF icon: %s"), libr_errmsg());
				goto fail;
			}
			if(!libr_icon_save(icon, argv[PARAM_ICON_FILE]))
			{
				libr_icon_close(icon);
				errorf(_("failed to save the icon to a file: %s"), libr_errmsg());
				goto fail;
			}
			libr_icon_close(icon);
		}	break;
		case MODE_GET_UUID:
		{
			char uuid[UUIDSTR_LENGTH];

			if(!libr_icon_getuuid(handle, uuid))
			{
				errorf(_("Failed to get UUID: %s\n"), libr_errmsg());
				goto fail;
			}
			printf("%s\n", uuid);
		}	break;
		case MODE_SET_UUID:
			if(!libr_icon_setuuid(handle, argv[PARAM_UUID]))
			{
				errorf(_("Failed to set UUID: %s\n"), libr_errmsg());
				goto fail;
			}
			break;
		case MODE_SET_METADATA:
		{
			// There are 13 of these
			// The metadata sequence is:
			// "executable name" "description" "license" "copyright" "authors" "product" "organization" "version" "datetime" "sysicon" "scmmodule" "scmrevision" "notes"
			if (strlen(argv[PARAM_EXECUTABLE_NAME]) > 0) add_resource_string(handle, ".metadata_name", argv[PARAM_EXECUTABLE_NAME]);
			if (strlen(argv[PARAM_DESCRIPTION]) > 0) add_resource_string(handle, ".metadata_description", argv[PARAM_DESCRIPTION]);
			if (strlen(argv[PARAM_LICENSE]) > 0) add_resource_string(handle, ".metadata_license", argv[PARAM_LICENSE]);
			if (strlen(argv[PARAM_COPYRIGHT]) > 0) add_resource_string(handle, ".metadata_copyright", argv[PARAM_COPYRIGHT]);
			if (strlen(argv[PARAM_AUTHORS]) > 0) add_resource_string(handle, ".metadata_authors", argv[PARAM_AUTHORS]);
			if (strlen(argv[PARAM_PRODUCT]) > 0) add_resource_string(handle, ".metadata_product", argv[PARAM_PRODUCT]);
			if (strlen(argv[PARAM_ORGANIZATION]) > 0) add_resource_string(handle, ".metadata_organization", argv[PARAM_ORGANIZATION]);
			if (strlen(argv[PARAM_VERSION]) > 0) add_resource_string(handle, ".metadata_version", argv[PARAM_VERSION]);
			if (strlen(argv[PARAM_DATETIME]) > 0) add_resource_string(handle, ".metadata_datetime", argv[PARAM_DATETIME]);
			if (strlen(argv[PARAM_SYSICON]) > 0) add_resource_string(handle, ".metadata_sysicon", argv[PARAM_SYSICON]);
			if (strlen(argv[PARAM_SCMMODULE]) > 0) add_resource_string(handle, ".metadata_scmmodule", argv[PARAM_SCMMODULE]);
			if (strlen(argv[PARAM_SCMREVISION]) > 0) add_resource_string(handle, ".metadata_scmrevision", argv[PARAM_SCMREVISION]);
			if (strlen(argv[PARAM_NOTES]) > 0) add_resource_string(handle, ".metadata_notes", argv[PARAM_NOTES]);
		}	break;
		case MODE_SET_EMPTY_UUID:
			section = ICON_SECTION;
			clear_resource(handle, section);

			if(!libr_icon_setuuid(handle, "00000000-0000-0000-0000-000000000000"))
			{
				errorf(_("Failed to set UUID: %s\n"), libr_errmsg());
				goto fail;
			}
			break;
		case MODE_LIST_RESOURCES:
			list_resources(handle);
			break;
		case MODE_CLEAR_ICON:
			section = ICON_SECTION;
			/* intentional fall-through */
		case MODE_CLEAR_RESOURCE:
			if(section == NULL)
				section = argv[PARAM_RESOURCE_NAME];
			clear_resource(handle, section);
			break;
		case MODE_ADD_ICON:
		{
			libr_icon *icon = NULL;

			icon = libr_icon_newicon_byfile(LIBR_PNG, 0, argv[PARAM_ICON_FILE]);
			if(icon == NULL)
			{
				errorf(_("failed to open icon file \"%s\": %s"), argv[PARAM_ICON_FILE], libr_errmsg());
				goto fail;
			}
			if(!libr_icon_write(handle, icon, argv[PARAM_ICON_NAME], LIBR_OVERWRITE))
			{
				libr_icon_close(icon);
				errorf(_("failed to add icon to ELF file: %s"), libr_errmsg());
				goto fail;
			}
			libr_icon_close(icon);
		}	break;
		case MODE_TDE_AUTOADD_ICON:
		{
			printf("Searching for standard icon for name %s in the following directories:\n", argv[PARAM_ICON_NAME]);
			TDEApplication::disableAutoDcopRegistration();
			TDEAboutData aboutd("tdelfeditor", "tdelfeditor", "0.0.1");
			TDECmdLineArgs::init(&aboutd);
			TDEApplication app(false, false);

			TQStringList rds = TDEGlobal::dirs()->resourceDirs("icon");
			for ( TQStringList::Iterator it = rds.begin(); it != rds.end(); ++it ) {
				printf(" * %s\n", (*it).ascii()); fflush(stdout);
			}
			TQString systemIcon = TDEGlobal::iconLoader()->iconPath(argv[PARAM_ICON_NAME], 0, true);
			if (systemIcon.isNull()) {
				systemIcon = TDEGlobal::iconLoader()->iconPath(argv[PARAM_ICON_NAME], 0, false);
				printf("NOT FOUND, refusing to add unknown icon (this message is harmless)\n");
				section = ICON_SECTION;
				clear_resource(handle, section);
				goto fail;
			}
			else {
				printf("found %s\n", systemIcon.ascii());
			}

			libr_icon *icon = NULL;

			icon = libr_icon_newicon_byfile(LIBR_PNG, 0, const_cast<char*>(systemIcon.ascii()));
			if(icon == NULL)
			{
				errorf(_("failed to open icon file \"%s\": %s"), systemIcon.ascii(), libr_errmsg());
				goto fail;
			}
			TQFileInfo ifi(systemIcon);
			TQString iconBaseName = ifi.baseName();
			printf("using %s as icon name\n", iconBaseName.ascii());
			if(!libr_icon_write(handle, icon, const_cast<char*>(iconBaseName.ascii()), LIBR_OVERWRITE))
			{
				libr_icon_close(icon);
				errorf(_("failed to add icon to ELF file: %s"), libr_errmsg());
				goto fail;
			}
			libr_icon_close(icon);
		}	break;
		case MODE_ADD_RESOURCE:
			if(section == NULL)
			{
				section = argv[PARAM_RESOURCE_NAME];
				input_file = argv[PARAM_RESOURCE_FILE];
			}
			add_resource(handle, section, input_file);
			break;
		case MODE_RETRIEVE_ICON:
		{
			libr_icon *icon = NULL;

			icon = libr_icon_geticon_byname(handle, argv[PARAM_ICON_NAME]);
			if(icon == NULL)
			{
				errorf(_("failed to get icon named \"%s\": %s"), argv[PARAM_ICON_NAME], libr_errmsg());
				goto fail;
			}
			if(!libr_icon_save(icon, argv[PARAM_ICON_FILE]))
			{
				libr_icon_close(icon);
				errorf(_("failed to save the icon to a file: %s"), libr_errmsg());
				goto fail;
			}
			libr_icon_close(icon);
		}	break;
		case MODE_RETRIEVE_RESOURCE:
			if(section == NULL)
			{
				section = argv[PARAM_RESOURCE_NAME];
				output_file = argv[PARAM_RESOURCE_FILE];
			}
			get_resource(handle, section, output_file);
			break;
		default:
			errorf(_("Unhandled operation code."));
			goto fail;
	}
	
fail:
	/* Close file handle and exit */
	libr_close(handle);
	return EX_OK;
}

/*
 * Main execution routine
 */
int main(int argc, char **argv)
{
	libr_i18n_autoload(PACKAGE);
	return main_console(argc, argv);
}
