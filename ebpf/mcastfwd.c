#include <linux/bpf.h>
#include <linux/if_link.h>
#include <assert.h>
#include <errno.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <libgen.h>

#include "bpf_load.h"
#include "bpf_util.h"
#include "libbpf.h"

#define MAP_TX_PORT 0
#define MAP_RX_CNT 1

static int ifindex_in;
static int ifindex_out;

static __u32 xdp_flags;

static void interrupt_exit(int sig)
{
  printf("delete xdp programs\n");
  set_link_xdp_fd(ifindex_in, -1, xdp_flags);
  set_link_xdp_fd(ifindex_out, -1, xdp_flags);
  exit(0);
}

static void poll_stats(int interval)
{
  unsigned int nr_cpus = bpf_num_possible_cpus();
  __u64 values[2][nr_cpus], prev[2][nr_cpus];

#if 1
  memset(prev[0], 0, sizeof(prev[0]));
  memset(prev[1], 0, sizeof(prev[1]));
#endif

  int ifindex;
  while (1) {
    int i;

    sleep(interval);
    __u64 sum = 0;
    __u32 key = 0;
    while (key <= 1)
    {
      sum = 0;
      assert(bpf_map_lookup_elem(map_fd[MAP_TX_PORT], &key, &ifindex) == 0);
      assert(bpf_map_lookup_elem(map_fd[MAP_RX_CNT], &key, values[key]) == 0);
      for (i = 0; i < nr_cpus; i++) {
        sum += (values[key][i] - prev[key][i]);
      }
#if 0
      if (sum) {
        printf("ifindex %i: %10llu pkt/s\n", ifindex, sum / interval);
      }
#endif
      memcpy(prev[key], values[key], sizeof(values[key]));
      key++;
    }
#if 0
    printf("--------------------\n");
#endif
  }
}

static void usage(const char *prog)
{
  fprintf(stderr,
    "usage: %s [options] <ifindex-in> <ifindex-out>\n\n"
    "options:\n"
    "    -S    use skb-mode\n"
    "    -N    enforce native mode\n",
    prog);
}


int main(int argc, char **argv)
{
  const char *optstr = "SN";
  char filename[256];
  int ret, opt;

  while ((opt = getopt(argc, argv, optstr)) != -1) {
    switch (opt) {
    case 'S':
      xdp_flags |= XDP_FLAGS_SKB_MODE;
      break;
    case 'N':
      xdp_flags |= XDP_FLAGS_DRV_MODE;
      break;
    default:
      usage(basename(argv[0]));
      return 1;
    }
  }

  if (optind == argc) {
    printf("usage: %s <ifindex-in> <ifindex-out>\n", argv[0]);
    return 1;
  }

  ifindex_in = strtoul(argv[optind], NULL, 0);
  ifindex_out = strtoul(argv[optind + 1], NULL, 0);
  printf("input: %d output: %d\n", ifindex_in, ifindex_out);

  snprintf(filename, sizeof(filename), "%s_k.o", argv[0]);

  if (load_bpf_file(filename)) {
    printf("%s", bpf_log_buf);
    return 1;
  }

  if (!prog_fd[0]) {
    printf("load_bpf_file: %s\n", strerror(errno));
    return 1;
  }

  if (set_link_xdp_fd(ifindex_in, prog_fd[0], xdp_flags) < 0) {
    printf("ERROR: link set xdp fd failed on %d\n", ifindex_in);
    return 1;
  }

  if (set_link_xdp_fd(ifindex_out, prog_fd[1], xdp_flags) < 0) {
    printf("ERROR: link set xdp fd failed on %d\n", ifindex_out);
    return 1;
  }

  signal(SIGINT, interrupt_exit);
  signal(SIGTERM, interrupt_exit);

  /* bpf redirect port 0 -> ifindex_out */
  int key = 0;
  ret = bpf_map_update_elem(map_fd[MAP_TX_PORT], &key, &ifindex_out, 0);
  if (ret) {
    perror("bpf_update_elem");
    goto out;
  }

  /* bpf redirect port 1 -> ifindex_in */
  key = 1;
  ret = bpf_map_update_elem(map_fd[MAP_TX_PORT], &key, &ifindex_in, 0);
  if (ret) {
    perror("bpf_update_elem");
    goto out;
  }

  poll_stats(1);

out:
  return 0;
}

