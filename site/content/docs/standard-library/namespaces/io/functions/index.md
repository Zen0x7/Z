---
title: Overview
description: Catalog of functions exported by @std/io.
domain: io
kind: catalog
status: implemented
navigation:
  title: Overview
---

# Functions

`@std/io` exposes `read`, `read_line` and `write`. Every operation uses the stable [stream contract](/docs/specification/streams) and returns `result_type<int32, status_code>` with the transferred byte count.

## Catalog

| Name | Canonical signature | C symbol | Macro |
| --- | --- | --- | --- |
| [`read`](/docs/standard-library/namespaces/io/functions/read) | `read(destination)`; `read(stream, destination, offset, count)` | `zlang_std_io_read`, `zlang_std_io_read_stream` | `ZLANG_STD_IO_READ` |
| [`read_line`](/docs/standard-library/namespaces/io/functions/read_line) | `read_line(destination)`; `read_line(stream, destination, offset, count)` | `zlang_std_io_read_line`, `zlang_std_io_read_line_stream` | `ZLANG_STD_IO_READ_LINE` |
| [`write`](/docs/standard-library/namespaces/io/functions/write) | `write(value)`; `write(stream, value)` | `zlang_std_io_write_*` | `ZLANG_STD_IO_WRITE_*` |

These are overloads of the same function names, not separate functions. `read(destination)` and `read_line(destination)` use implicit `STDIN`, start at offset `0` and read up to the declared capacity of a caller-owned `reference string`. The explicit forms take a `clone stream` source and a bounded `offset`/`count` window in that destination. `write(value)` uses implicit `STDOUT`; `write(stream, value)` writes to a borrowed stream. Cloning a `stream` copies a borrowed descriptor, not the device. The explicit stream overloads are canonical: a socket, pipe, descriptor, memory stream or hardware backend can implement the same native operations without changing Zlang signatures.

## Result contract

`value` is the transferred byte count, `code` is a `status_code` (`STATUS_SUCCESS`, `STATUS_END_OF_INPUT` or `STATUS_IO_ERROR`), and `what` is a bounded read-only view of the fixed status message. Invalid offsets, null storage and capacity overflow trap.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/io/functions) — this catalog.
- [read](/docs/standard-library/namespaces/io/functions/read)
- [read_line](/docs/standard-library/namespaces/io/functions/read_line)
- [write](/docs/standard-library/namespaces/io/functions/write)
