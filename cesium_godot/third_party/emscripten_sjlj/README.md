# emscripten_sjlj

Vendored copies of two files from emscripten's compiler-rt (source: emsdk 3.1.56,
`upstream/emscripten/system/lib/compiler-rt/`):

- `emscripten_setjmp.c` — defines `saveSetjmp`, `testSetjmp`, `__wasm_longjmp`.
- `emscripten_tempret.s` — defines `setTempRet0` / `getTempRet0` used by `saveSetjmp`.

## Why vendored

emsdk 3.1.56's emcc has a bug where `-sSUPPORT_LONGJMP=wasm` is accepted on the
command line but not honored during compile-time setjmp/longjmp lowering for
relocatable objects (SIDE_MODULE builds). The `.o` files ship with undefined
references to `saveSetjmp` / `testSetjmp` anyway. In a Godot web export our
extension is a SIDE_MODULE; those undefined symbols try to resolve dynamically
against Godot's main wasm at runtime, which doesn't provide them, and the
extension aborts the first time any image decoder's format-probe path runs.

By compiling these two files into the extension's own wasm, `saveSetjmp`,
`testSetjmp`, and `__wasm_longjmp` become *defined* symbols in the SIDE_MODULE.
libturbojpeg's undefined references resolve locally, no runtime lookup happens,
and the normal longjmp-based "not this format, try another" unwind works.

## Build flags

Compile `emscripten_setjmp.c` with `-D__USING_WASM_SJLJ__` to select the
wasm-EH branch (defines `__wasm_longjmp`, matches what libturbojpeg expects
via the `__c_longjmp` tag). Without that flag, the file defines
`emscripten_longjmp` instead and pulls in `setThrew` / `_emscripten_throw_longjmp` /
`emscripten_internal.h` which we don't have.

`cesium_godot/SCsub` wires this up for web-only builds.

## Proper fix

Upgrading emsdk past ~3.1.60 resolves the emcc compile-time bug. Blocked in
this repo by the KTX vcpkg port, which breaks on newer emsdk. When both can
move together, delete this directory and the SCsub wiring.

## License

MIT / University of Illinois/NCSA dual license. See the emsdk distribution for
the full LICENSE text (`upstream/emscripten/system/lib/compiler-rt/LICENSE.TXT`).
