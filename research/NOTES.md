## Random

 - Should we support the n in bit<n>?
      Yes. For example, bit<12> may be used as the type of the VID field in the VLAN header.
 - A P4 table T can be considered as having a 'type', which defines the properties of all instances of T.
 - We need a mechanism to allocate temporary memory such that it can be reset and reused repeatedly.

## XDP

```
ip link set dev eth0 xdp obj xdp-example.o
ip link set dev eth0 xdp obj xdp-example.o sec my_prog
ip link set dev eth0 xdp off

ip link add dev veth0 type veth peer name veth1
for i in {0..3}; do sudo ip link set dev veth$i xdp off; done
```

Debugging
```
sudo echo 1 > /sys/kernel/debug/tracing/tracing_on
sudo cat /sys/kernel/debug/tracing/trace_pipe
```

The kernel part of XDP source should be compiled with -02, or else the BPF loader could reject it:
```
llvm-objdump -S -no-show-raw-insn xdp-example.o
readelf -a xde-example.o
```

'gnu/stubs-32.h' file not found:
```
sudo apt-get install libc6-dev-i386
```

```
pkt=Ether(src="fc:75:16:8d:86:17",dst="1c:6f:65:2c:b5:3c")/IP(src="1.1.1.1",dst="2.2.2.2")/TCP(dport=1222,sport=1223)
pktv6=Ether(src="fc:75:16:8d:86:17",dst="1c:6f:65:2c:b5:3c")/IPv6(src="ff02::1",dst="ff02::2")/TCP(dport=1222,sport=1223)

pkt/=Raw("W"*(100-len(pkt)))

./xdp_redirect_user -S 61 10
```

## Static linking of libc

GNU libc is ~not~ designed to be statically linked. Important functions, e.g. gethostbyname and iconv,
will malfunction or not work at all in a static binary. Arguably even worse, under some conditions a
static binary will attempt to dynamically open and use libc.so.6, even though the whole point of
static linkage is to avoid such dependencies.

## Address-space Randomization

Disable address-space randomization:
```
sudo sysctl -w kernel.randomize_va_space=0
sudo sysctl -p
```

Enable address-space randomization:
```
sudo sysctl -w kernel.randomize_va_space=2
sudo sysctl -p
```

## C99

Stricter checking of function usage, which leads to better error reporting by the compiler.
For example, a warning is reported if a function is used without declaration:

  `warning| implicit declaration of function ‘hashmap_init’`

## Core Dump

Enable coredump:
```
$ ulimit -c unlimited
$ echo "/.coredump" > /proc/sys/kernel/core_pattern
```
