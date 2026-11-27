# native_interop

Shows `z.yaml native` declarations with a project header, include directory and C source. The Zlang
source calls the C function through `extern function ... from @native/my_math`; zc emits a quoted
include and links the configured implementation.

The program prints `5`.
