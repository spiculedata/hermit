/*
 * Copyright (c) Meta Platforms, Inc. and affiliates.
 * All rights reserved.
 *
 * This source code is licensed under the BSD-style license found in the
 * LICENSE file in the root directory of this source tree.
 */

// Directory sizes under Hermit (https://github.com/rrnewton/hermit/issues/4162).
//
// Usage: directory_size grow DIR | watch DIR N
//   grow   stat DIR, create 64 files in it, stat it again, and stat /proc;
//          print each st_size and DIR's st_blocks.
//   watch  stat DIR N times while something else may add and remove entries
//          in it; print the last st_size.
#include <fcntl.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

static struct stat must_stat(const char* path) {
  struct stat st;
  if (stat(path, &st) != 0) {
    perror(path);
    exit(1);
  }
  return st;
}

int main(int argc, char** argv) {
  if (argc >= 3 && !strcmp(argv[1], "grow")) {
    const char* dir = argv[2];
    struct stat before = must_stat(dir);
    char path[4096];
    for (int i = 0; i < 64; i++) {
      snprintf(path, sizeof(path), "%s/entry-with-a-long-name-%02d", dir, i);
      int fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, 0644);
      if (fd < 0) {
        perror(path);
        return 1;
      }
      close(fd);
    }
    struct stat after = must_stat(dir);
    struct stat proc = must_stat("/proc");
    printf("dir %lld then %lld, blocks %lld; /proc %lld\n", (long long)before.st_size,
           (long long)after.st_size, (long long)after.st_blocks, (long long)proc.st_size);
    return 0;
  }
  if (argc >= 4 && !strcmp(argv[1], "watch")) {
    int n = atoi(argv[3]);
    struct stat st = must_stat(argv[2]);
    for (int i = 1; i < n; i++) {
      st = must_stat(argv[2]);
    }
    printf("watched %d times; size %lld\n", n, (long long)st.st_size);
    return 0;
  }
  fprintf(stderr, "usage: %s grow DIR | watch DIR N\n", argv[0]);
  return 64;
}
