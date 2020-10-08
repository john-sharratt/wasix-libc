// Copyright (c) 2015-2016 Nuxi, https://nuxi.nl/
//
// SPDX-License-Identifier: BSD-2-Clause

#include <common/errno.h>

#include <wasi/api.h>
#include <dirent.h>
#include <errno.h>
#include <stdlib.h>

#include "dirent_impl.h"

DIR *fdopendir(int fd) {
  // Allocate new directory object and read buffer.
  DIR *dirp = malloc(sizeof(*dirp));
  if (dirp == NULL)
    return NULL;
  dirp->buffer = malloc(DIRENT_DEFAULT_BUFFER_SIZE);
  if (dirp->buffer == NULL) {
    free(dirp);
    return NULL;
  }

  // Ensure that this is really a directory by already loading the first
  // chunk of data.
  __wasi_errno_t error =
#ifdef __wasilibc_unmodified_upstream
      __wasi_file_readdir(fd, dirp->buffer, DIRENT_DEFAULT_BUFFER_SIZE,
#else
      // TODO: Remove the cast on `dirp->buffer` once the witx is updated with char8 support.
      __wasi_fd_readdir(fd, (uint8_t *)dirp->buffer, DIRENT_DEFAULT_BUFFER_SIZE,
#endif
                                __WASI_DIRCOOKIE_START, &dirp->buffer_used);
  if (error != 0) {
    free(dirp->buffer);
    free(dirp);
    errno = errno_fixup_directory(fd, error);
    return NULL;
  }

  // Initialize other members.
  dirp->fd = fd;
  dirp->cookie = __WASI_DIRCOOKIE_START;
  dirp->buffer_processed = 0;
  dirp->buffer_size = DIRENT_DEFAULT_BUFFER_SIZE;
  dirp->dirent = NULL;
  dirp->dirent_size = 1;
#ifdef __wasm32__
  dirp->offsets = NULL;
  dirp->offsets_len = 0;
  dirp->offsets_allocated = 0;
#endif
  return dirp;
}
