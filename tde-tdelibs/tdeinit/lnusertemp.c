/*
  This file is part of the KDE libraries
  Copyright (c) 2000 Waldo Bastian <bastian@kde.org>

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Library General Public
  License version 2 as published by the Free Software Foundation.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Library General Public License for more details.

  You should have received a copy of the GNU Library General Public License
  along with this library; see the file COPYING.LIB.  If not, write to
  the Free Software Foundation, Inc., 51 Franklin Street, Fifth Floor,
  Boston, MA 02110-1301, USA.
*/

#ifdef HAVE_CONFIG_H
#include "config.h"
#endif

#include <sys/types.h>
#include <sys/param.h>
#include <sys/socket.h>
#include <sys/stat.h>
#include <sys/un.h>

#include <errno.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#ifdef HAVE_LIMITS_H
#include <limits.h>
#endif

int check_tmp_dir(const char *tmp_dir);
int create_link(const char *file, const char *tmp_dir);
int build_link(const char *tmp_prefix, const char *kde_prefix, int kdehostname);

int check_tmp_dir(const char *tmp_dir)
{
  int result;
  struct stat stat_buf;
  result = lstat(tmp_dir, &stat_buf);
  if ((result == -1) && (errno == ENOENT))
  {
    result = mkdir(tmp_dir, 0700);
    if (result == -1)
    {
       fprintf(stderr, "[lnusertemp] Error: Can not create directory \"%s\".\n", tmp_dir);
       return 1;
    }
    result = stat(tmp_dir, &stat_buf);
  }
  if ((result == -1) || (!S_ISDIR(stat_buf.st_mode)))
  {
     fprintf(stderr, "[lnusertemp] Error: \"%s\" is not a directory.\n", tmp_dir);
     return 1;
  }

  if (stat_buf.st_uid != getuid())
  {
     fprintf(stderr, "[lnusertemp] Error: \"%s\" is owned by uid %d instead of uid %d.\n", tmp_dir, stat_buf.st_uid, getuid());
     return 1;
  }
  return 0;
}

int create_link(const char *file, const char *tmp_dir)
{
  int result;
  result = check_tmp_dir(tmp_dir);
  if (result) 
  {
     return result;
  }
  result = symlink(tmp_dir, file);
  if (result == -1)
  {
     fprintf(stderr, "[lnusertemp] Error: Can not create link from \"%s\" to \"%s\"\n", file, tmp_dir);
     return 1;
  }
  /*printf("[lnusertemp] Created link from \"%s\" to \"%s\"\n", file, tmp_dir);*/
  return 0;
}


#ifndef PATH_MAX
#define PATH_MAX 4096
#endif
int build_link(const char *tmp_prefix, const char *kde_prefix, int kdehostname)
{
  struct passwd *pw_ent;
  char kde_tmp_dir[PATH_MAX+1];
  char user_tmp_dir[PATH_MAX+1];
  char tmp_buf[PATH_MAX+1];
  int uid = getuid();
  const char *home_dir = getenv("HOME");
  const char *kde_home = uid ? getenv("TDEHOME") : getenv("TDEROOTHOME");
  int result;
  struct stat stat_buf;

  kde_tmp_dir[0] = 0;

  pw_ent = getpwuid(uid);
  if (!pw_ent)
  {
     fprintf(stderr, "[lnusertemp] Error: Can not find password entry for uid %d.\n", getuid());
     return 1;
  }

  strncpy(user_tmp_dir, tmp_prefix, PATH_MAX);
  user_tmp_dir[ PATH_MAX ] = '\0';
  strncat(user_tmp_dir, pw_ent->pw_name, PATH_MAX - strlen(tmp_prefix));

  if (!kde_home || !kde_home[0])
  {
     kde_home = "~/.trinity/";
  }

  if (kde_home[0] == '~')
  {
     if (!home_dir || !home_dir[0])
     {
        if ( uid == 0 ) {
           home_dir = pw_ent->pw_dir ? pw_ent->pw_dir : "/root";
        }
        else {
           fprintf(stderr, "[lnusertemp] Aborting. $HOME not set!\n");
           return 1;
        }
     }
     if (strlen(home_dir) > (PATH_MAX-100))
     {
        fprintf(stderr, "[lnusertemp] Aborting. Home directory path too long!\n");
        return 1;
     }
     kde_home++;
     strncpy(kde_tmp_dir, home_dir, PATH_MAX);
     kde_tmp_dir[ PATH_MAX ] = '\0';
  }
  strncat(kde_tmp_dir, kde_home, PATH_MAX - strlen(kde_tmp_dir));

  /** Strip trailing '/' **/
  if ( kde_tmp_dir[strlen(kde_tmp_dir)-1] == '/')
     kde_tmp_dir[strlen(kde_tmp_dir)-1] = 0;

  result = stat(kde_tmp_dir, &stat_buf);
  if ((result == -1) && (errno == ENOENT))
  {
     result = mkdir(kde_tmp_dir, 0700);
  }
  if (result == -1)
  {
     perror("[lnusertemp] mkdir failed: ");
     return 1;
  }  

  strncat(kde_tmp_dir, kde_prefix, PATH_MAX - strlen(kde_tmp_dir));

  if( kdehostname )
  {
      if( getenv("XAUTHLOCALHOSTNAME"))
          strncat(kde_tmp_dir+strlen(kde_tmp_dir), getenv("XAUTHLOCALHOSTNAME"), PATH_MAX - strlen(kde_tmp_dir) - 1);
      else
          return 0;
  }
  else
  {
     if (gethostname(kde_tmp_dir+strlen(kde_tmp_dir), PATH_MAX - strlen(kde_tmp_dir) - 1) != 0)
     {
        perror("[lnusertemp] Could not determine hostname: ");
        return 1;
     }
  }
  kde_tmp_dir[sizeof(kde_tmp_dir)-1] = '\0';

  result = lstat(kde_tmp_dir, &stat_buf);
  if ((result == 0) && (S_ISDIR(stat_buf.st_mode)))
  {
     /* $TDEHOME/tmp is a normal directory. Do nothing. */
     /*printf("[lnusertemp] Directory \"%s\" already exists.\n", kde_tmp_dir);*/
     return 0;
  }
  if ((result == -1) && (errno == ENOENT))
  {
     /*printf("[lnusertemp] Creating link %s.\n", kde_tmp_dir);*/
     result = create_link(kde_tmp_dir, user_tmp_dir);
     if (result == 0) return 0; /* Success */
     unlink(kde_tmp_dir);
     strncat(user_tmp_dir, "XXXXXX", PATH_MAX - strlen(user_tmp_dir));
     if (mkdtemp(user_tmp_dir) == NULL)
	 return 1;
     return create_link(kde_tmp_dir, user_tmp_dir);
  }
  if ((result == -1) || (!S_ISLNK(stat_buf.st_mode)))
  {
     fprintf(stderr, "[lnusertemp] Error: \"%s\" is not a link or a directory.\n", kde_tmp_dir);
     return 1;
  }
  /* kde_tmp_dir is a link. Check whether it points to a valid directory. */
  result = readlink(kde_tmp_dir, tmp_buf, PATH_MAX);
  if (result == -1)
  {
     fprintf(stderr, "[lnusertemp] Error: \"%s\" could not be read.\n", kde_tmp_dir);
     return 1;
  }
  tmp_buf[result] = '\0';  
  /*printf("[lnusertemp] Link \"%s\" points to \"%s\"\n", kde_tmp_dir, tmp_buf);*/
  if (strncmp(tmp_buf, user_tmp_dir, strlen(user_tmp_dir)) != 0)
  {
     fprintf(stderr, "[lnusertemp] Error: \"%s\" points to \"%s\" instead of \"%s\".\n", kde_tmp_dir, tmp_buf, user_tmp_dir);
     unlink(kde_tmp_dir);
     /*printf("[lnusertemp] Creating link %s.\n", kde_tmp_dir);*/
     result = create_link(kde_tmp_dir, user_tmp_dir);
     if (result == 0) return 0; /* Success */
     unlink(kde_tmp_dir);
     strncat(user_tmp_dir, "XXXXXX", PATH_MAX - strlen(user_tmp_dir));
     if (mkdtemp(user_tmp_dir) == NULL)
	 return 1;
     return create_link(kde_tmp_dir, user_tmp_dir);
  }
  result = check_tmp_dir(tmp_buf);
  if (result == 0) return 0; /* Success */
  unlink(kde_tmp_dir);
  strncat(user_tmp_dir, "XXXXXX", PATH_MAX - strlen(user_tmp_dir));
  if (mkdtemp(user_tmp_dir) == NULL)
      return 1;
  return create_link(kde_tmp_dir, user_tmp_dir);
}

int main(int argc, char **argv)
{
  const char *tmp = 0;
  char *tmp_prefix = 0;
  const char *kde_prefix = 0;
  int res = 0;

  if ((argc != 2) || 
      ((strcmp(argv[1], "tmp")!=0) && 
       (strcmp(argv[1], "socket")!=0) && 
       (strcmp(argv[1], "cache")!=0)))
  {
     fprintf(stderr, "[lnusertemp] Usage: lnusertemp tmp|socket|cache\n");
     return 1;
  }

  tmp = getenv("TDETMP");
  if (!tmp || !tmp[0])
    tmp = getenv("TMPDIR");
  if (!tmp || !tmp[0])
    tmp = getenv("TEMP");
  if (!tmp || !tmp[0])
    tmp = getenv("TMP");
  if (!tmp || !tmp[0]) 
    tmp = "/tmp";

  if (strcmp(argv[1], "tmp") == 0)
  {
    tmp_prefix = (char *)malloc(strlen(tmp)+strlen("/tde-")+1);
    strcpy(tmp_prefix, tmp);
    strcat(tmp_prefix, "/tde-"); 
    
    kde_prefix = "/tmp-"; 
  }
  else if (strcmp(argv[1], "socket") == 0)
  {
    tmp_prefix = (char *)malloc(strlen(tmp)+strlen("/tdesocket-")+1);
    strcpy(tmp_prefix, tmp );
    strcat(tmp_prefix, "/tdesocket-" );

    kde_prefix = "/socket-"; 
  }
  else if (strcmp(argv[1], "cache") == 0)
  {
    tmp = getenv("TDEVARTMP");
    if (!tmp || !tmp[0]) 
      tmp = "/var/tmp";

    tmp_prefix = (char *)malloc(strlen(tmp)+strlen("/tdecache-")+1);
    strcpy(tmp_prefix, tmp );
    strcat(tmp_prefix, "/tdecache-" );

    kde_prefix = "/cache-"; 
  }

  res = build_link(tmp_prefix, kde_prefix, 1); 
  if( build_link(tmp_prefix, kde_prefix, 0))
    res = 1;
    
  free(tmp_prefix);

  return res;
}
