#include <stdlib.h>
#include <stdio.h>
#include <bpf/bpf.h>

#define sizeof_array(ARRAY) (sizeof(ARRAY)/sizeof(ARRAY[0]))

struct bpf_insn bpf_program[] = {
  { 0x28, 0, 0, (__s32)0x0000000c },
  { 0x15, 0, 1, (__s32)0x00000001 },
  { 0x6, 0, 0, (__s32)0xffffffff },
  { 0x95, 0, 0, (__s32)0x00000000 },
};

int
main()
{
  char log_buf[1024];
  printf("size of program = %ld\n", sizeof_array(bpf_program));
  int prog_fd = bpf_load_program(BPF_PROG_TYPE_SOCKET_FILTER, bpf_program, sizeof_array(bpf_program),
                             "GPL", 0, log_buf, sizeof_array(log_buf));
  if (prog_fd < 0)
  {
    printf("failed to load program\n");
    printf("%s\n", log_buf);
    goto cleanup;
  }

cleanup:
  return 0;
}
