// adad-rt : Adad runtime library
// Copyright (C) Université Paris-Saclay, UVSQ
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#include <linux/perf_event.h>

// include the e9patch replacement stdlib
#include "stdlib.c"

#include <sys/syscall.h>

#define MAX_CALLS 4096
#define NUM_RAPL_DOMAINS 1
#define TOTAL_PACKAGES 1
#define DOMAIN_PATH "/sys/bus/event_source/devices/power/events/"
#define POWER_TYPE_PATH "/sys/bus/event_source/devices/power/type"

static int package_map[TOTAL_PACKAGES];

char rapl_domain_name[256] = "energy-pkg";

static int type, config, calls = 0;
static long long unsigned samples[MAX_CALLS] = {0.0};

static int fd[TOTAL_PACKAGES];
static struct perf_event_attr attr;

#define is_main_thread() \
  (getpid() == gettid()) // check if the current thread is the main thread

static int perf_event_open(struct perf_event_attr *hw_event_uptr, pid_t pid,
                           int cpu, int group_fd, unsigned long flags)
{
  return syscall(__NR_perf_event_open, hw_event_uptr, pid, cpu, group_fd,
                 flags);
}

// Helper function: open a file and exit with an error message if it fails.
static FILE *open_file_or_exit(const char *path, const char *mode, const char *errmsg)
{
  FILE *f = fopen(path, mode);
  if (!f)
  {
    fprintf(stderr, "error: %s (%s)\n", errmsg, path);
    exit(1);
  }
  return f;
}

// rapl_init() initializes the selected RAPL domain and creates the
// rapl_samples.bin file with a header containing the domain name and scale
void rapl_init(void)
{
  FILE *f;
  char filename[BUFSIZ];
  char scale[BUFSIZ];

  // Optionally set the RAPL domain from environment variable.
  char *env_domain = getenv("ADAD_RAPL_DOMAIN");
  if (env_domain != NULL)
  {
    snprintf(rapl_domain_name, sizeof(rapl_domain_name), "%s", env_domain);
  }

  // Read the 'type' value.
  f = open_file_or_exit(POWER_TYPE_PATH, "r", "No perf_event rapl support found");
  fscanf(f, "%d", &type);
  fclose(f);

  // Read the event configuration.
  snprintf(filename, sizeof(filename), "%s%s", DOMAIN_PATH, rapl_domain_name);
  f = open_file_or_exit(filename, "r", "Cannot retrieve event for RAPL domain");
  fscanf(f, "event=%x", &config);
  fclose(f);

  // Open the output file for writing.
  FILE *fs = open_file_or_exit("rapl_samples.bin", "w", "Cannot open rapl_samples.bin for writing");

  // Write the domain name to the output file.
  fprintf(fs, "%s\n", rapl_domain_name);

  // Retrieve and write the scale value.
  snprintf(filename, sizeof(filename), "%s%s.scale", DOMAIN_PATH, rapl_domain_name);
  f = open_file_or_exit(filename, "r", "Cannot open scale file");
  fscanf(f, "%s", scale);
  fclose(f);
  fprintf(fs, "%s\n", scale);
  fclose(fs);
}

// rapl_dump writes the samples to the rapl_samples.bin file and resets the call counter
void rapl_dump(void)
{
  FILE *fs = open_file_or_exit("rapl_samples.bin", "a", "Cannot open rapl_samples.bin for writing");
  int n = fwrite(&samples, sizeof(long long unsigned), calls, fs);
  if (n != calls)
  {
    fprintf(stderr, "error: Cannot write all samples to rapl_samples.txt\n");
    exit(1);
  }
  fclose(fs);
  calls = 0;
}

// rapl_open opens the RAPL domain for the specified package
void rapl_open()
{
  for (int j = 0; j < TOTAL_PACKAGES; j++)
  {
    fd[j] = -1;
    memset(&attr, 0x0, sizeof(attr));
    attr.type = type;
    attr.config = config;
    if (config == 0)
      continue;
    fd[j] = perf_event_open(&attr, -1, package_map[j], -1, 0);
    if (fd[j] < 0)
    {
      fprintf(stderr, "error: Cannot open core %d config %d: %s\n\n", package_map[j],
              config, strerror(errno));
      exit(1);
    }
  }
}

// rapl_close reads the RAPL value from the file descriptor and stores it in the samples array
// It also increments the call counter and dumps the samples if the maximum number of calls is reached
void rapl_close()
{
  long long unsigned value;
  read(fd[0], &value, 8);
  samples[calls] = value;
  calls += 1;
  if (calls >= MAX_CALLS)
  {
    rapl_dump();
  }
  close(fd[0]);
}

// Initialization hook
void init(int argc, char **argv, char **envp)
{
  if (!is_main_thread())
    return;
  environ = envp;
  rapl_init();
}

// Finalization hook
void fini(void)
{
  if (!is_main_thread())
    return;
  rapl_dump();
  printf("\e[32mRAPL samples written to rapl_samples.bin\e[0m\n");
}

// Function entry hook
void hook_entry()
{
  if (!is_main_thread())
    return;
  rapl_open();
}

// Function exit hook
void hook_exit()
{
  if (!is_main_thread())
    return;
  rapl_close(0);
}