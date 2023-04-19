'''
Author: zzzzztw
Date: 2023-04-17 22:38:30
LastEditors: Do not edit
LastEditTime: 2023-04-17 23:15:18
FilePath: /cpptest/ebpf/tracefile/tracefile.py
'''

#!/usr/bin/env python3
# 1) import bcc library
#from bcc import BPF

# 2) load BPF program
#b = BPF(src_file="hello.c")
# 3) attach kprobe
#b.attach_kprobe(event="do_sys_openat2", fn_name="hello_world")
# 4) read and print /sys/kernel/debug/tracing/trace_pipe
#b.trace_print()



from bcc import BPF

# 1) load BPF program
b = BPF(src_file="trace-open.c")
b.attach_kprobe(event="do_sys_openat2", fn_name="trace_file")

# 2) print header
print("%-18s %-16s %-6s %-16s" % ("TIME(s)", "COMM", "PID", "FILE"))

# 3) define the callback for perf event
start = 0
def print_event(cpu, data, size):
    global start
    event = b["events"].event(data)
    if start == 0:
            start = event.ts
    time_s = (float(event.ts - start)) / 1000000000
    print("%-18.9f %-16s %-6d %-16s" % (time_s, event.comm, event.pid, event.fname))

# 4) loop with callback to print_event
b["events"].open_perf_buffer(print_event)
while 1:
    try:
        b.perf_buffer_poll()
    except KeyboardInterrupt:
        exit()