# Vendored verbatim from emsdk 3.1.56:
#   <emsdk>/upstream/emscripten/system/lib/compiler-rt/emscripten_tempret.s
#
# Provides setTempRet0 / getTempRet0 (and their __set_temp_ret / __get_temp_ret
# aliases). saveSetjmp in emscripten_setjmp.c calls setTempRet0, so we need
# this symbol defined inside our SIDE_MODULE. See the header comment in
# emscripten_setjmp.c and memory/project_emsdk_wasm_longjmp_bug.md for full
# back-story. License: see emsdk LICENSE file (MIT / NCSA dual).

.section .globals,"",@

.globaltype tempRet0, i32
tempRet0:

.section .text,"",@

.globl setTempRet0
setTempRet0:
  .functype setTempRet0 (i32) -> ()
  local.get 0
  global.set tempRet0
  end_function

.globl getTempRet0
getTempRet0:
  .functype getTempRet0 () -> (i32)
  global.get tempRet0
  end_function

# These aliases exist solely for LegalizeJSInterface pass in binaryen
# They get exported by emcc and the exports are then removed by the
# binaryen pass
.globl __get_temp_ret
.type __get_temp_ret, @function
__get_temp_ret = getTempRet0

.globl __set_temp_ret
.type __set_temp_ret, @function
__set_temp_ret = setTempRet0
