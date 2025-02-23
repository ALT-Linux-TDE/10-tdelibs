/*
 *  This file is part of the KDE libraries
 *  Copyright (c) 2001 Michael Goffioul <tdeprint@swing.be>
 *  Copyright (c) 2014 Timothy Pearson <kb9vqf@pearsoncomputing.net>
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Library General Public
 *  License version 2 as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Library General Public License for more details.
 *
 *  You should have received a copy of the GNU Library General Public License
 *  along with this library; see the file COPYING.LIB.  If not, write to
 *  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
 *  Boston, MA 02110-1301, USA.
 **/

#include <config.h>
#ifndef _GNU_SOURCE
#define _GNU_SOURCE   /* Needed for getline */
#endif
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <unistd.h>
#include <ctype.h>
#include <zlib.h>

#include <tqstringlist.h>
#include <tqlocale.h>

extern "C" {
	#include "driverparse.h"
}

#define PROCESS_PPD_FILE_CONTENTS												\
		memset(value,0,256);												\
		c1 = strchr(line,':');												\
		if (c1)														\
		{														\
			c2 = strchr(c1,'"');											\
			if (c2)													\
			{													\
				c2++;												\
				c1 = strchr(c2,'"');										\
				if (c1) strlcpy(value,c2,c1-c2+1);								\
			}													\
			else													\
			{													\
				c1++;												\
				while (*c1 && isspace(*c1))									\
					c1++;											\
				if (!*c1)											\
					continue;										\
				c2 = line+strlen(line)-1;	/* point to \n */						\
				while (*c2 && isspace(*c2))									\
					c2--;											\
				strlcpy(value,c1,c2-c1+2);									\
			}													\
		}														\
		count++;													\
		if (strncmp(line,"*Manufacturer:",14) == 0) fprintf(output_file,"MANUFACTURER=%s\n",value);			\
		else if (strncmp(line,"*ShortNickName:",15) == 0) fprintf(output_file,"MODEL=%s\n",value);			\
		else if (strncmp(line,"*ModelName:",11) == 0) fprintf(output_file,"MODELNAME=%s\n",value);			\
		else if (strncmp(line,"*NickName:",10) == 0) strncat(desc,value,255-strlen(desc));				\
		else if (strncmp(line,"*pnpManufacturer:",17) == 0) fprintf(output_file,"PNPMANUFACTURER=%s\n",value);		\
		else if (strncmp(line,"*pnpModel:",10) == 0) fprintf(output_file,"PNPMODEL=%s\n",value);			\
		else if (strncmp(line,"*LanguageVersion:",17) == 0) strncat(langver,value,63-strlen(langver));			\
		else count--;													\
		/* Either we got everything we needed, or we encountered an "OpenUI" directive					\
		 * and it's reasonable to assume that there's no needed info further in the file,				\
		 * just stop here */												\
		if (count >= 7 || strncmp(line, "*OpenUI", 7) == 0)								\
		{														\
			if (strlen(langver) > 0)										\
			{													\
				strncat(desc, " [", 255-strlen(desc));								\
				strncat(desc, langver, 255-strlen(desc));							\
				strncat(desc, "]", 255-strlen(desc));								\
			}													\
			if (strlen(desc) > 0)											\
				fprintf(output_file, "DESCRIPTION=%s\n", desc);							\
			break;													\
		}

void initPpd(const char *dirname)
{
	struct stat stat_res;
	if (stat(dirname, &stat_res) == -1) {
		fprintf(stderr, "Can't open drivers directory : %s\n", dirname);
		return;
	}

	if (S_ISDIR(stat_res.st_mode)) {
		DIR	*dir = opendir(dirname);
		struct dirent	*entry;
		char		buffer[4096] = {0};
		char	drFile[256];
		int		len = strlen(dirname);
	
		if (dir == NULL)
		{
			fprintf(stderr, "Can't open drivers directory : %s\n", dirname);
			return;
		}
		while ((entry=readdir(dir)) != NULL)
		{
			if (strcmp(entry->d_name,".") == 0 || strcmp(entry->d_name,"..") == 0)
			{
				continue;
			}
			if (len+strlen(entry->d_name)+1 < 4096)
			{
				struct stat	st;
	
				strcpy(buffer,dirname);
				strcat(buffer,"/");
				strcat(buffer,entry->d_name);
				if (stat(buffer,&st) == 0)
				{
					if (S_ISDIR(st.st_mode))
					{
						initPpd(buffer);
					}
					else if (S_ISREG(st.st_mode))
					{
						char	*c = strrchr(buffer,'.');
						snprintf(drFile, 255, "ppd:%s", buffer);
						if (c && strncmp(c,".ppd",4) == 0)
						{
							addFile(drFile, "", "");
						}
						else if (c && strncmp(c, ".gz", 3) == 0)
						{ /* keep also compressed driver files */
							while (c != buffer)
							{
								if (*(--c) == '.') break;
							}
							if (*c == '.' && strncmp(c, ".ppd",4) == 0)
							{
								addFile(drFile, "", "");
							}
						}
					}
				}
			}
		}
		closedir(dir);
	}
	else if (access(dirname, X_OK) != -1) {
		char *filename;
		int n = strlen(dirname)+strlen(" list");
		filename = (char*)malloc(n*sizeof(char)+1);
		memset(filename,0,n);
		strcat(filename, dirname);
		strcat(filename, " list");

		FILE* file = popen(filename, "r");
		if (file) {
			char * line = NULL;
			size_t len = 0;
			ssize_t read;
			while ((read = getline(&line, &len, file)) != -1) {
				char * pos1 = strstr(line, "\"");
				if (pos1 != NULL) {
					char * pos2 = strstr(pos1 + 1, "\"");
					if (pos2 != NULL) {
						*pos2 = 0;
						char * pos3 = strstr(pos1 + 1, ":");
						if (pos3 != NULL) {
							char *ppduri;
							int n2 = strlen("compressed-ppd:")+strlen(pos3+1);
							ppduri = (char*)malloc(n2*sizeof(char)+1);
							memset(ppduri,0,n2);
							strcat(ppduri, "compressed-ppd:");
							strcat(ppduri, pos3+1);
							addFile(ppduri, dirname, pos2+1);
							free(ppduri);
							ppduri = NULL;
						}
					}
				}
			}
			if (line) {
				free(line);
			}

			pclose(file);
		}
		else {
			fprintf(stderr, "Can't execute compressed driver handler : %s\n", dirname);
		}

		free(filename);
		filename = NULL;
	}
	else {
		fprintf(stderr, "Can't open drivers directory : %s\n", dirname);
		return;
	}
}

void initCompressedPpd(const char *dirname)
{
	// HACK
	// The initPpd function actually handles the compressed PPDs as well, so do nothing here
	// If we were to rerun initPpd here then all drivers would be duplicated!
}

int parsePpdFile(const char *filename, const char * /*origin*/, const char * /*metadata*/, FILE *output_file)
{
	gzFile	ppd_file;
	char	line[4096], value[256], langver[64] = {0}, desc[256] = {0};
	char	*c1, *c2;
	int	count = 0;

	ppd_file = gzopen(filename,"r");
	if (ppd_file == NULL)
	{
		fprintf(stderr, "Can't open driver file : %s\n", filename);
		return 0;
	}
	fprintf(output_file,"FILE=ppd:%s\n",filename);

	while (gzgets(ppd_file,line,4095) != Z_NULL)
	{
		PROCESS_PPD_FILE_CONTENTS
	}
	fprintf(output_file,"\n");

	gzclose(ppd_file);
	return 1;
}

int parseCompressedPpdFile(const char *ppdfilename, const char *origin, const char *metadata, FILE *output_file)
{
	char	value[256], langver[64] = {0}, desc[256] = {0};
	char	*c1, *c2;
	int	count = 0;

	bool useFallbackExtractionMethod = false;

	if (strlen(metadata) > 0) {
		TQString metadataProcessed(metadata);
		metadataProcessed = metadataProcessed.stripWhiteSpace();
		TQStringList metadataList = TQStringList::split(" ", metadataProcessed, TRUE);
		TQLocale ppdLanguage(metadataList[0]);
		TQString languageVersion = TQLocale::languageToString(ppdLanguage.language());
		metadataList = TQStringList::split("\" \"", metadataProcessed, TRUE);
		TQString description = metadataList[1];

		int pos = metadataProcessed.find("MFG:");
		if (pos < 0) {
			pos = metadataProcessed.find("MANUFACTURER:");
		}
		if (pos >= 0) {
			TQString manufacturer;
			TQString model;
			TQString modelName;
			TQString pnpManufacturer;
			TQString pnpModel;
			TQString driver;
			TQStringList metadataList = TQStringList::split(";", metadataProcessed.mid(pos), TRUE);
			for (TQStringList::Iterator it = metadataList.begin(); it != metadataList.end(); ++it) {
				TQStringList kvPair = TQStringList::split(":", *it, TRUE);
				if ((kvPair[0].upper() == "MFG") || (kvPair[0].upper() == "MANUFACTURER")) {
					manufacturer = kvPair[1];
				}
				else if ((kvPair[0].upper() == "MDL") ||(kvPair[0].upper() == "MODEL")) {
					modelName = kvPair[1];
				}
// 				else if (kvPair[0].upper() == "PNPMANUFACTURER") {
// 					pnpManufacturer = kvPair[1];
// 				}
// 				else if (kvPair[0].upper() == "PNPMODEL") {
// 					pnpModel = kvPair[1];
// 				}
				else if ((kvPair[0].upper() == "DRV") || (kvPair[0].upper() == "DRIVER")) {
					driver = kvPair[1];
				}
			}

			manufacturer = manufacturer.stripWhiteSpace();
			modelName = modelName.stripWhiteSpace();
			driver = driver.stripWhiteSpace();

			TQStringList driverList = TQStringList::split(",", driver, TRUE);
			driver = driverList[0];
			if (driver.startsWith("D")) {
				driver = driver.mid(1);
				driver = driver.stripWhiteSpace();
			}
			model = manufacturer + " " + modelName + " " + driver;
			description = description + " [" + languageVersion + "]";

			fprintf(output_file,"FILE=compressed-ppd:%s:%s\n", origin, ppdfilename);

			fprintf(output_file,"MANUFACTURER=%s\n",manufacturer.ascii());
			fprintf(output_file,"MODELNAME=%s\n",modelName.ascii());
			fprintf(output_file,"MODEL=%s\n",model.ascii());
			if (pnpManufacturer.length() > 0) {
				fprintf(output_file,"PNPMANUFACTURER=%s\n",pnpManufacturer.ascii());
			}
			if (pnpModel.length() > 0) {
				fprintf(output_file,"PNPMODEL=%s\n",pnpModel.ascii());
			}
			if (description.length() > 0) {
				fprintf(output_file,"DESCRIPTION=%s\n",description.ascii());
			}
		}
		else {
			useFallbackExtractionMethod = true;
		}
	}

	if (useFallbackExtractionMethod) {
		char *filename;
		int n = strlen(origin)+strlen(" cat ")+strlen(ppdfilename);
		filename = (char*)malloc(n*sizeof(char)+1);
		memset(filename,0,n);
		strcat(filename, origin);
		strcat(filename, " cat ");
		strcat(filename, ppdfilename);
	
		FILE* file = popen(filename, "r");
		if (file) {
			char * line = NULL;
			size_t len = 0;
			ssize_t read;
	
			fprintf(output_file,"FILE=compressed-ppd:%s:%s\n", origin, ppdfilename);
	
			while ((read = getline(&line, &len, file)) != -1) {
				PROCESS_PPD_FILE_CONTENTS
			}
			if (line) {
				free(line);
			}
	
			pclose(file);
		}
		else {
			fprintf(stderr, "Can't open driver file : %s\n", ppdfilename);
			return 0;
		}
	
		free(filename);
		filename = NULL;
	}

	return 1;
}

int main(int argc, char *argv[])
{
	registerHandler("ppd:", initPpd, parsePpdFile);
	registerHandler("compressed-ppd:", initCompressedPpd, parseCompressedPpdFile);
	initFoomatic();
	return execute(argc, argv);
}
