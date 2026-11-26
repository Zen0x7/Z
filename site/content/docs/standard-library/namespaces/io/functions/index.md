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
| [`read`](/docs/standard-library/namespaces/io/functions/read) | `read(source, destination, offset, count)` | `zlang_std_io_read`, `zlang_std_io_read_stream` | `ZLANG_STD_IO_READ` |
| [`read_line`](/docs/standard-library/namespaces/io/functions/read_line) | `read_line(source, destination, offset, count)` | `zlang_std_io_read_line`, `zlang_std_io_read_line_stream` | `ZLANG_STD_IO_READ_LINE` |
| [`write`](/docs/standard-library/namespaces/io/functions/write) | `write(destination, source, offset, count)` | `zlang_std_io_write_*` | `ZLANG_STD_IO_WRITE_*` |

Convenience overloads keep implicit stdin for `read` and stdout for `write`. The explicit stream overloads are canonical: a socket, pipe, descriptor, memory stream or hardware backend can implement the same native operations without changing Zlang signatures.

## Result contract

`value` is the number of bytes transferred. `code` is `SUCCESS`, `END_OF_INPUT` or `IO_ERROR`. Invalid offsets, null storage and capacity overflow trap.

## Pages at this level

- [Overview](/docs/standard-library/namespaces/io/functions) — this catalog.
- [read](/docs/standard-library/namespaces/io/functions/read)
- [read_line](/docs/standard-library/namespaces/io/functions/read_line)
- [write](/docs/standard-library/namespaces/io/functions/write)
