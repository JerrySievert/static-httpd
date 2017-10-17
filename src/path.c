#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <string.h>
#include <stdbool.h>
#include <unistd.h>
#include <sys/stat.h>
#ifdef __linux__
#define __USE_XOPEN
#define _GNU_SOURCE
#endif
#include <time.h>
#include <sys/time.h>

#include "config.h"
#include "paths.h"
#include "mime.h"
#include "http_request.h"
#include "http_response.h"

const char *search_paths[3] = {
  "index.html",
  "index.htm",
  NULL
};

static char *real_path (char *path) {
#ifdef __APPLE__
  static char buffer[MAX_PATH_LENGTH + MAX_ROOT_LENGTH + 32];
  realpath(path, buffer);
#else
  char *buffer = realpath(path, NULL);
#endif

  return buffer;
}

int open_file_maybe (char *path, char *real) {
  // don't want to allocate here if i can avoid it, should give some headway
  // and still allow for long paths and the filling of index.html etc
  char buf[MAX_PATH_LENGTH + MAX_ROOT_LENGTH + 32];
  char path_copy[MAX_PATH_LENGTH];
  int ret;

  strcpy(path_copy, path);

  buf[0] = '\0';

  strcat(buf, config_root);
  strcat(buf, "/");
  strcat(buf, path_copy);

  // if the last character is not a slash, try the file itself
  if (path_copy[strlen(path_copy) - 1] != '/') {
    char *cret = real_path(buf);
    struct stat statbuf = { 0 };

    // check to see if this is a directory, if it is, add a '/' and continue on
    stat(cret, &statbuf);

    // add a slash if a directory, and drop to searching paths
    if ((statbuf.st_mode & S_IFDIR) == S_IFDIR) {
      // check to make sure there is enough room to add a '/'
      int len = strlen(path_copy);

      // if there is not, return that we cannot open it
      if (len >= MAX_PATH_LENGTH) {
        return -1;
      } else {
        path_copy[len] = '/';
        path_copy[len + 1] = '\0';
      }
    } else {
      if (cret != NULL && real != NULL) {
        strcpy(real, cret);
      }

      if (cret != NULL && cret[0] != '\0' && strncmp(cret, config_root, config_root_length) == 0) {
        ret = open(cret, O_RDONLY);

  #ifdef __linux__
        free(cret);
  #endif

        return ret;
      }

  #ifdef __linux__
      if (cret) {
        free(cret);
      }
  #endif

      return -1;
    }
  }

  for (uint8_t i = 0; search_paths[i] != NULL; i++) {
    buf[0] = '\0';
    strcat(buf, config_root);
    strcat(buf, "/");
    strcat(buf, path_copy);
    strcat(buf, "/");
    strcat(buf, search_paths[i]);
    char *cret = real_path(buf);

    if (cret != NULL &&real != NULL) {
      strcpy(real, cret);
    }

    if (cret != NULL && cret[0] != '\0' && strncmp(cret, config_root, config_root_length) == 0) {
      ret = open(cret, O_RDONLY);

#ifdef __linux__
      free(cret);
#endif

      if (ret != -1) {
        return ret;
      }
    }

#ifdef __linux__
    if (cret) {
      free(cret);
    }
#endif
  }

  return -1;
}

bool serve_file (Request *request, Response *response) {
  char real[MAX_PATH_LENGTH + MAX_ROOT_LENGTH + 32] = { 0 };
  struct stat statbuf = { 0 };
  char date_hdr[256], mod_hdr[256], age_hdr[256], len_hdr[256];

  int fd = open_file_maybe((char *) request->path, real);
  if (fd == -1) {
    return false;
  }

  // there is a file, so we are going to serve it
  response->code = 200;

  // set up the header information
  char *mime_type = mime_type_from_filename(real);
  response_set_content_type(response, (uint8_t *) mime_type);

  // content length
  fstat(fd, &statbuf);

  // modificaton date
#ifdef __APPLE__
  time_t modtime = (time_t) statbuf.st_mtimespec.tv_sec;
#else
  time_t modtime = (time_t) statbuf.st_mtim.tv_sec;
#endif
  struct tm *timeinfo = localtime(&modtime);
  strftime(mod_hdr, 256, "Last-Modified: %a, %d %b %Y %T %Z", timeinfo);
  response_set_header(response, (uint8_t *) mod_hdr);

  // current date
  time_t curtime;

  curtime = time(NULL);
  timeinfo = localtime(&curtime);

  strftime(date_hdr, 256, "Date: %a, %d %b %Y %T %Z", timeinfo);
  response_set_header(response, (uint8_t *) date_hdr);

  // age
  sprintf(age_hdr, "Age: %ld", curtime - modtime);
  response_set_header(response, (uint8_t *) age_hdr);

  // check the if-modified-since header
  for (uint8_t i = 0; request->headers != NULL && request->headers[i] != NULL; i++) {
    char *found, *str;
    struct tm timedata = { 0 };

    str = strdup((char *) request->headers[i]);
    if ((found = strsep(&str, ": ")) != NULL) {
      if (strcmp(found, "If-Modified-Since") == 0) {
        strptime(str, " %a, %d %b %Y %H:%M:%S %Z", &timedata);
        time_t m = mktime(&timedata);

        if (modtime <= m) {
          response->code = 304;
          response_write(response, (uint8_t *) "");
          close(fd);
          free(found);

          return true;
        }
      }
    }

    free(found);
  }

#ifdef __APPLE__
  sprintf(len_hdr, "Content-Length: %lld", statbuf.st_size);
#else
  sprintf(len_hdr, "Content-Length: %ld", statbuf.st_size);
#endif
  response_set_header(response, (uint8_t *) len_hdr);

  response_sendfile(response, fd);

  close(fd);

  return true;
}
