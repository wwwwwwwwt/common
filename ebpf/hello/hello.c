/*
 * @Author: zzzzztw
 * @Date: 2023-04-17 23:16:19
 * @LastEditors: Do not edit
 * @LastEditTime: 2023-04-17 23:16:42
 * @FilePath: /cpptest/ebpf/hello/hello.c
 */

int hello_world(void *ctx)
{
    bpf_trace_printk("Hello, World!");
    return 0;
}