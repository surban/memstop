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

  if (verbose)
    fprintf(stderr, "==== memstop loaded ====\n");

  while (true) {
    if (get_memory_info(&memory) != 0)
      return;

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
