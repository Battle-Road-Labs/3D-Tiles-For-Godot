#ifdef __EMSCRIPTEN__

#include "WebFetchClient.h"

#include <cstring>
#include <emscripten.h>
#include <emscripten/threading.h>

namespace cesium_web_fetch_detail {

std::mutex& registry_mutex() {
	static std::mutex m;
	return m;
}

std::unordered_map<int32_t, std::unique_ptr<PendingRequest>>& registry() {
	static std::unordered_map<int32_t, std::unique_ptr<PendingRequest>> r;
	return r;
}

std::atomic<int32_t>& next_id() {
	static std::atomic<int32_t> id{1};
	return id;
}

} // namespace cesium_web_fetch_detail

// JS -> C trampoline. Invoked from the EM_JS block below via wasmTable lookup
// of this function's pointer. extern "C" + EMSCRIPTEN_KEEPALIVE guarantee it
// survives DCE and has a stable table index.
extern "C" EMSCRIPTEN_KEEPALIVE
void cesium_web_fetch_complete(int32_t request_id, int32_t status, uint8_t* data, int32_t size) {
	std::unique_ptr<cesium_web_fetch_detail::PendingRequest> ctx;
	{
		std::lock_guard<std::mutex> lock(cesium_web_fetch_detail::registry_mutex());
		auto& map = cesium_web_fetch_detail::registry();
		auto it = map.find(request_id);
		if (it == map.end()) {
			return;
		}
		ctx = std::move(it->second);
		map.erase(it);
	}

	PackedByteArray body;
	if (size > 0 && data != nullptr) {
		body.resize(static_cast<int64_t>(size));
		std::memcpy(body.ptrw(), data, static_cast<size_t>(size));
	}
	ctx->callback(status, body);
}

// Launches a browser fetch(). The callback function pointer is the wasm table
// index of cesium_web_fetch_complete; JS invokes it via wasmTable.get when the
// fetch resolves.
//
// MUST run on the main browser thread. If called from an emscripten pthread
// (Web Worker), the fetch() Promise's .then() never fires because the worker's
// JS event loop stays blocked in Atomics.wait while the worker is idle. The
// initiator (start_fetch) proxies to main thread before reaching this.
EM_JS(void, cesium_web_fetch_js_start, (
		int32_t request_id,
		int32_t callback_fn_ptr,
		const char* method_ptr,
		const char* url_ptr,
		const char* const* headers_kv_ptr,
		const uint8_t* body_ptr,
		int32_t body_len), {
	var resolveFn = function(ptr) {
		if (typeof wasmTable !== 'undefined' && wasmTable && typeof wasmTable.get === 'function') {
			return wasmTable.get(ptr);
		}
		if (typeof Module !== 'undefined' && Module['wasmTable'] && typeof Module['wasmTable'].get === 'function') {
			return Module['wasmTable'].get(ptr);
		}
		if (typeof getWasmTableEntry === 'function') {
			return getWasmTableEntry(ptr);
		}
		throw new Error('cesium: cannot resolve wasm function pointer');
	};

	// Under MEMORY64, EM_JS pointer args arrive as BigInt; HEAPU64 exists.
	// Under wasm32 pointers are Number and HEAPU64 is undefined. The Number()
	// coercion is a no-op on Number and lossless on BigInt below 2^53.
	var wasm64 = (typeof HEAPU64 !== 'undefined');

	var invokeCb = function(status, dataPtr, dataLen) {
		try {
			var fn = resolveFn(callback_fn_ptr);
			// cesium_web_fetch_complete signature on the C side is
			//   (int32, int32, uint8_t*, int32)
			// under MEMORY64 the uint8_t* arg is i64, so dataPtr must be a
			// BigInt at the wasm boundary. Number 0 is the no-data sentinel
			// passed by the error paths, so wrap whatever we got.
			var dataArg = wasm64 ? BigInt(dataPtr || 0) : dataPtr;
			fn(request_id, status, dataArg, dataLen);
		} catch (e) {
			console.error('[cesium] dispatch to C++ fetch-complete failed:', e);
		}
	};
	var refreshViews = function() {
		// Refresh HEAP* typed-array views after a potential memory.grow inside
		// _malloc. Without this, HEAPU8 etc. may point at a detached buffer,
		// so HEAPU8.set silently writes nothing and the wasm-side pointer
		// reads uninitialized garbage. Symptom: cesium parses tile responses
		// as JSON/glTF and hits a cascade of Invalid UTF-8 errors.
		if (typeof growMemViews === 'function') growMemViews();
	};

	try {
		var url = UTF8ToString(Number(url_ptr));
		var method = UTF8ToString(Number(method_ptr));
		var headers = new Headers();
		if (headers_kv_ptr) {
			// char* const* is an array of pointers — 4 bytes wide on wasm32,
			// 8 bytes on wasm64. Use the matching heap view and stride.
			var stride = wasm64 ? 8 : 4;
			var heap = wasm64 ? HEAPU64 : HEAPU32;
			var idx = Number(headers_kv_ptr) / stride;
			while (true) {
				var keyPtr = heap[idx];
				if (keyPtr === 0 || keyPtr === 0n) break;
				var valPtr = heap[idx + 1];
				var name = UTF8ToString(Number(keyPtr));
				// Strip x-cesium-* telemetry headers — third-party hosts (Bing,
				// Mapbox, etc.) don't whitelist them in CORS preflight, which
				// causes the preflight to fail and the real request to be blocked.
				// Browsers auto-ignore User-Agent but dropping here avoids any
				// preflight contribution.
				var lower = name.toLowerCase();
				if (lower.indexOf('x-cesium-') === 0 || lower === 'user-agent') {
					idx += 2;
					continue;
				}
				try {
					headers.append(name, UTF8ToString(Number(valPtr)));
				} catch (e) { /* invalid header name — skip */ }
				idx += 2;
			}
		}
		var init = { method: method, headers: headers };
		if (body_len > 0 && body_ptr) {
			var bodyStart = Number(body_ptr);
			init.body = HEAPU8.slice(bodyStart, bodyStart + body_len);
		}

		fetch(url, init).then(function(resp) {
			return resp.arrayBuffer().then(function(buf) {
				var bytes = new Uint8Array(buf);
				var dataPtr = 0;
				if (bytes.length > 0) {
					dataPtr = _malloc(bytes.length);
					// Refresh HEAPU8 in case _malloc grew memory. Without
					// this, .set() writes to a detached buffer and the
					// wasm-side dataPtr ends up holding garbage — which
					// cesium then mis-parses as UTF-8.
					refreshViews();
					HEAPU8.set(bytes, Number(dataPtr));
				}
				invokeCb(resp.status | 0, dataPtr, bytes.length | 0);
				if (dataPtr) _free(dataPtr);
			});
		}).catch(function(err) {
			console.error('[cesium] fetch failed for', url, err);
			invokeCb(0, 0, 0);
		});
	} catch (e) {
		console.error('[cesium] fetch setup failed:', e);
		invokeCb(0, 0, 0);
	}
});

// Owned context for proxying a fetch kickoff from a worker thread to main.
// The original char* storage belonged to the caller's stack/std::string and can
// vanish as soon as start_fetch returns, so we deep-copy into this struct.
struct CesiumFetchDispatch {
	int32_t request_id;
	std::string method;
	std::string url;
	std::vector<std::pair<std::string, std::string>> headers;
	std::vector<uint8_t> body;
};

// Runs on the main browser thread (either directly, or after the async proxy).
// Rebuilds the flat header-pointer array, calls the EM_JS, and deletes itself.
extern "C" EMSCRIPTEN_KEEPALIVE
void cesium_web_fetch_invoke_on_main(CesiumFetchDispatch* dispatch) {
	std::vector<const char*> kv;
	kv.reserve(dispatch->headers.size() * 2 + 1);
	for (const auto& h : dispatch->headers) {
		kv.push_back(h.first.c_str());
		kv.push_back(h.second.c_str());
	}
	kv.push_back(nullptr);

	int32_t cb_ptr = static_cast<int32_t>(
		reinterpret_cast<uintptr_t>(&cesium_web_fetch_complete));

	cesium_web_fetch_js_start(
		dispatch->request_id, cb_ptr,
		dispatch->method.c_str(), dispatch->url.c_str(),
		kv.data(),
		dispatch->body.empty() ? nullptr : dispatch->body.data(),
		static_cast<int32_t>(dispatch->body.size()));

	delete dispatch;
}

namespace cesium_web_fetch_detail {

void start_fetch(int32_t request_id,
				 const char* method,
				 const char* url,
				 const char* const* headers_kv,
				 const uint8_t* body,
				 int32_t body_len) {
	auto* dispatch = new CesiumFetchDispatch();
	dispatch->request_id = request_id;
	dispatch->method = method;
	dispatch->url = url;
	if (headers_kv) {
		for (size_t i = 0; headers_kv[i] != nullptr; i += 2) {
			const char* k = headers_kv[i];
			const char* v = headers_kv[i + 1] ? headers_kv[i + 1] : "";
			dispatch->headers.emplace_back(std::string(k), std::string(v));
		}
	}
	if (body != nullptr && body_len > 0) {
		dispatch->body.assign(body, body + body_len);
	}

	if (emscripten_is_main_runtime_thread()) {
		cesium_web_fetch_invoke_on_main(dispatch);
	} else {
		// Worker thread's JS event loop is blocked in Atomics.wait while the
		// pthread idles, so a fetch() launched from here would register but its
		// Promise .then would never fire. Main thread's event loop always runs.
		//
		// Use a void(pointer) signature, not void(int): under MEMORY64 the
		// function takes a 64-bit pointer, so the wasm signature is void(i64).
		// Declaring SIG_VI here makes the task-queue executor do a
		// call_indirect with type void(i32) against a wasm function whose real
		// type is void(i64) — the engine traps with "function signature
		// mismatch" the first time a worker thread (e.g. cesium's async tile
		// loader) hands a fetch off to main. emsdk 4.0.11 doesn't ship an
		// EM_FUNC_SIG_VP convenience macro, but EM_FUNC_SIG_PARAM_P resolves
		// to PARAM_J (i64) under MEMORY64 and PARAM_I (i32) under wasm32, so
		// constructing the sig manually works on both arches.
		constexpr EM_FUNC_SIGNATURE sig_vp =
			EM_FUNC_SIG_RETURN_VALUE_V
			| EM_FUNC_SIG_WITH_N_PARAMETERS(1)
			| EM_FUNC_SIG_SET_PARAM(0, EM_FUNC_SIG_PARAM_P);
		emscripten_async_run_in_main_runtime_thread(
			sig_vp,
			(void*)&cesium_web_fetch_invoke_on_main,
			dispatch);
	}
}

} // namespace cesium_web_fetch_detail

#endif // __EMSCRIPTEN__
