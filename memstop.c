/*
memstop - A LD_PRELOAD shared object that delays execution memory is low.
Copyright (C) 2025 Dr. Sebastian Urban

This program is free software: you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct MemoryInfo {
  long total_kb;
  long available_kb;
};

static int get_memory_info(struct MemoryInfo *mem_info) {
  mem_info->total_kb = -1;
  mem_info->available_kb = -1;

  FILE *fp = fopen("/proc/meminfo", "r");
  if (fp == NULL)
    return -1;

  char line[256];
  while (fgets(line, sizeof(line), fp)) {
    if (strncmp(line, "MemTotal:", 9) == 0)
      sscanf(line, "MemTotal: %ld kB", &mem_info->total_kb);
    else if (strncmp(line, "MemAvailable:", 13) == 0)
      sscanf(line, "MemAvailable: %ld kB", &mem_info->available_kb);

    if (mem_info->total_kb != -1 && mem_info->available_kb != -1)
      break;
  }

  fclose(fp);

  if (mem_info->total_kb == -1 || mem_info->available_kb == -1)
    return -1;

  return 0;
}

void __attribute__((constructor)) memstop(void) {
  struct MemoryInfo memory;
  long required_kb;
  long percent = 10;
  char *env;
  bool verbose = false;
  bool stats = false;
  bool informed = false;

  env = getenv("MEMSTOP_PERCENT");
  if (env) {
    percent = atol(env);
    if (percent < 0)
      percent = 0;
    if (percent > 100)
      percent = 100;
  }

  env = getenv("MEMSTOP_VERBOSE");
  if (env)
    verbose = true;

  while (true) {
    if (get_memory_info(&memory) != 0)
      return;

    if (verbose && !stats) {
      fprintf(stderr,
              "==== memstop: %ld%% required, %ld%% available (%ld MB / %ld MB) "
              "====\n",
              percent, memory.available_kb * 100 / memory.total_kb,
              memory.available_kb / 1024, memory.total_kb / 1024);
      stats = true;
    }

    required_kb = memory.total_kb * percent / 100;
    if (memory.available_kb >= required_kb)
      break;

    if (verbose && !informed) {
      fprintf(stderr, "==== memstop hold ====\n");
      informed = true;
    }

    usleep(10000);
  }

  if (verbose && informed)
    fprintf(stderr, "==== memstop release ====\n");
}
