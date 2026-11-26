---
title: Streams
description: The stable native IO contract shared by every stream backend.
navigation:
  title: Streams
---

# Streams

A stream is a borrowed, trivially copyable handle. It does not own storage, allocate memory or expose
the underlying backend. Zlang code sees one native `stream` type; stdio, memory, sockets, pipes and
hardware backends implement the same contract.

## Operations

The canonical source-level shapes are:

```zlang
read(source, destination, offset, count)
write(destination, source, offset, count)
```

`source` and `destination` are roles, not overload families. Convenience overloads may keep implicit
stdin for `read` and stdout for `write`, but explicit stream overloads are canonical.

`offset` is the byte position in the destination for reads and in the source for writes. `count` is
the maximum number of bytes transferred in that call.

## Native contract

```c
typedef struct zlang_stream_t {
    const zlang_stream_ops_t *ops_;
    void *state_;
} zlang_stream_t;
```

`ops_` is a constant table with read and write operations. `state_` is caller-owned backend state.
Backends include stdio streams, caller-owned memory buffers, POSIX descriptors, Windows handles,
sockets, pipes and device adapters.

The generic helpers are `zlang_stream_read` and `zlang_stream_write`. They validate arguments,
dispatch through the constant operations table and report status through `zlang_std_status_t`.

## Results

IO results are canonical `result_type<int32, status_code>` values. `value` is the number of bytes transferred.
`code` is `SUCCESS`, `END_OF_INPUT` or `IO_ERROR`. Invalid offsets, null storage and capacity failures
trap.

## Extension rule

A new IO domain must not introduce `read_socket`, `read_serial` or `read_hardware`. It exposes a
constructor that produces a `stream`; the existing `read` and `write` functions do the rest.
