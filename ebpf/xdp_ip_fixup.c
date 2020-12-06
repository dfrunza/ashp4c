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

static int ifindex;
static __u32 xdp_flags;

static void interrupt_exit(int sig)
{
  printf("delete xdp programs\n");
  set_link_xdp_fd(ifindex, -1, xdp_flags);
  exit(0);
}

static void usage(const char *prog)
{
  fprintf(stderr,
    "usage: %s [OPTS] IFINDEX\n\n"
    "OPTS:\n"
    "    -S    use skb-mode\n"
    "    -N    enforce native mode\n",
    prog);
}

int main(int argc, char **argv)
{
  const char *optstr = "SN";
  char filename[256];
  int opt;

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
    printf("usage: %s IFINDEX\n", argv[0]);
    return 1;
  }

  ifindex = strtoul(argv[optind], NULL, 0);
  snprintf(filename, sizeof(filename), "%s_kern.o", argv[0]);

  if (load_bpf_file(filename)) {
    printf("%s", bpf_log_buf);
    return 1;
  }

  if (set_link_xdp_fd(ifindex, prog_fd[0], xdp_flags) < 0) {
    printf("ERROR: link set xdp fd failed on %d\n", ifindex);
    return 1;
  }

  signal(SIGINT, interrupt_exit);
  signal(SIGTERM, interrupt_exit);

  while (1)
  {
    sleep(1);
  }

  return 0;
}
