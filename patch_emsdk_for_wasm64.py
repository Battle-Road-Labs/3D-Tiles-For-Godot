#!/usr/bin/env python3
"""Patch emsdk's libpthread.js for MEMORY64 + SIDE_MODULE + pthreads dlopen.

emsdk 4.0.x ships a bug where `_emscripten_dlsync_threads` (and `_async`
variant) passes pthread_ptr to a wasm export without BigInt-wrapping it. Under
MEMORY64 the wasm side expects i64 and V8 refuses the Number→BigInt coercion
at the boundary:

    TypeError: Cannot convert <addr> to a BigInt
        at __emscripten_proxy_dlsync (or _async)
        at _emscripten_dlsync_threads
        at dlsync → load_library_done → _dlopen → dlopen

That dlopen path runs for every GDExtension load, so this single bug blocks
Godot's wasm64 web templates from loading any GDExtension. Fix is to wrap the
arg with emscripten's `{{{ to64('pthread_ptr') }}}` macro at both call sites.
The macro expands to `BigInt(pthread_ptr)` under MEMORY64 and plain
`pthread_ptr` under wasm32 — safe for both build modes.

Usage:
    python patch_emsdk_for_wasm64.py <path-to-emsdk>
    python patch_emsdk_for_wasm64.py                  # uses $EMSDK env var

Idempotent — running multiple times is a no-op once patched. Re-run after
`emsdk install <ver>` since that re-extracts the source files and wipes
the patch.

Recommended integration for Godot's scons_web_build_release_and_debug.bat:
add a line right after `call "%EMSDK_DIR%\\emsdk_env.bat"`:
    python <path-to-this-script> "%EMSDK_DIR%"

Submitted upstream as an emscripten bug; this script is obsolete once emsdk
ships a fix natively (likely a 4.0.x point release).
"""

import os
import subprocess
import sys


def patch(emsdk_dir):
    libpthread_js = os.path.join(
        emsdk_dir, "upstream", "emscripten", "src", "lib", "libpthread.js"
    )
    if not os.path.exists(libpthread_js):
        print(f"[emsdk-patch] Not found: {libpthread_js}", file=sys.stderr)
        return 1
    with open(libpthread_js, "r", encoding="utf-8") as f:
        content = f.read()
    if "to64('pthread_ptr')" in content:
        print(f"[emsdk-patch] Already patched: {libpthread_js}")
        return 0
    replacements = [
        (
            "__emscripten_proxy_dlsync(pthread_ptr);",
            "__emscripten_proxy_dlsync({{{ to64('pthread_ptr') }}});",
        ),
        (
            "__emscripten_proxy_dlsync_async(pthread_ptr, info.id);",
            "__emscripten_proxy_dlsync_async({{{ to64('pthread_ptr') }}}, info.id);",
        ),
    ]
    patched = content
    applied = 0
    for old, new in replacements:
        if old in patched:
            patched = patched.replace(old, new)
            applied += 1
    if applied == 0:
        print(
            f"[emsdk-patch] Neither call site found in {libpthread_js} — "
            "unfamiliar emsdk version. Inspect the file manually.",
            file=sys.stderr,
        )
        return 1
    with open(libpthread_js, "w", encoding="utf-8") as f:
        f.write(patched)
    print(
        f"[emsdk-patch] Patched {libpthread_js}: "
        f"{applied}/2 dlsync_threads call site(s) BigInt-wrapped."
    )
    # Clear emcc cache so the next link re-reads the patched library JS.
    emcc = os.path.join(
        emsdk_dir,
        "upstream",
        "emscripten",
        "emcc.bat" if os.name == "nt" else "emcc",
    )
    if os.path.exists(emcc):
        try:
            subprocess.run([emcc, "--clear-cache"], cwd=emsdk_dir, check=False)
        except Exception as e:
            print(
                f"[emsdk-patch] Warning: could not clear emcc cache: {e}",
                file=sys.stderr,
            )
    return 0


def main():
    emsdk_dir = sys.argv[1] if len(sys.argv) > 1 else os.environ.get("EMSDK", "")
    if not emsdk_dir:
        print(__doc__)
        return 1
    return patch(emsdk_dir)


if __name__ == "__main__":
    sys.exit(main())
