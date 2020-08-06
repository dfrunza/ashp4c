#include <linux/bpf.h>
#include <linux/if_link.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>
#include <net/if.h>

#include "bpf_load.h"
#include "bpf_util.h"
#include "libbpf.h"

#define MAP_RX_CNT 0

static int ifindex = 4; // enp5s0
static __u32 xdp_flags = XDP_FLAGS_SKB_MODE;

static void interrupt_exit(int sig)
{
  printf("delete xdp program\n");
  set_link_xdp_fd(ifindex, -1, xdp_flags);
  exit(0);
}

static void poll_stats(int interval)
{
  unsigned int nr_cpus = bpf_num_possible_cpus();
  __u64 values[1][nr_cpus], prev[1][nr_cpus];

#if 1
  memset(prev[0], 0, sizeof(prev[0]));
  memset(prev[1], 0, sizeof(prev[1]));
#endif

  __u64 sum = 0;
  __u32 key = 0;
  while (1)
  {
    int i;
    sleep(interval);
    assert(bpf_map_lookup_elem(map_fd[MAP_RX_CNT], &key, values[key]) == 0);
    for (i = 0; i < nr_cpus; i++)
      sum += (values[key][i] - prev[key][i]);
    printf("%10llu pkt\n", sum);
    memcpy(prev[key], values[key], sizeof(values[key]));
    printf("--------------------\n");
  }
}

int main(int argc, char **argv)
{
  char filename[256];

  printf("load xdp program\n");
  snprintf(filename, sizeof(filename), "%s_kern.o", argv[0]);
  if (load_bpf_file(filename)) {
    printf("%s", bpf_log_buf);
    return 1;
  }

  if (!prog_fd[0]) {
    printf("ERROR: load_bpf_file: %s\n", strerror(errno));
    return 1;
  }

  if (set_link_xdp_fd(ifindex, prog_fd[0], xdp_flags) < 0) {
    printf("ERROR: link set xdp fd failed on %d\n", ifindex);
    return 1;
  }

  signal(SIGINT, interrupt_exit);
  signal(SIGTERM, interrupt_exit);

  poll_stats(1);
  return 0;
}
