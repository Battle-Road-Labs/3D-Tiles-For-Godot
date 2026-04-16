#ifdef __EMSCRIPTEN__

#include "WebFetchClient.h"

#include <cstring>
#include <emscripten.h>

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

	var invokeCb = function(status, dataPtr, dataLen) {
		try {
			var fn = resolveFn(callback_fn_ptr);
			fn(request_id, status, dataPtr, dataLen);
		} catch (e) {
			console.error('[cesium] dispatch to C++ fetch-complete failed:', e);
		}
	};

	try {
		var url = UTF8ToString(url_ptr);
		var method = UTF8ToString(method_ptr);
		var headers = new Headers();
		if (headers_kv_ptr) {
			// Unsigned shift to keep the index non-negative for memory > 2 GiB builds.
			var idx = headers_kv_ptr >>> 2;
			while (true) {
				var keyPtr = HEAPU32[idx];
				if (keyPtr === 0) break;
				var valPtr = HEAPU32[idx + 1];
				try {
					headers.append(UTF8ToString(keyPtr), UTF8ToString(valPtr));
				} catch (e) {
					// Invalid header name per browser validation — skip.
				}
				idx += 2;
			}
		}
		var init = { method: method, headers: headers };
		if (body_len > 0 && body_ptr) {
			init.body = HEAPU8.slice(body_ptr, body_ptr + body_len);
		}

		fetch(url, init).then(function(resp) {
			return resp.arrayBuffer().then(function(buf) {
				var bytes = new Uint8Array(buf);
				var dataPtr = 0;
				if (bytes.length > 0) {
					dataPtr = _malloc(bytes.length);
					HEAPU8.set(bytes, dataPtr);
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

namespace cesium_web_fetch_detail {

void start_fetch(int32_t request_id,
				 const char* method,
				 const char* url,
				 const char* const* headers_kv,
				 const uint8_t* body,
				 int32_t body_len) {
	// On wasm32, function pointers are 32-bit table indices.
	int32_t cb_ptr = static_cast<int32_t>(
		reinterpret_cast<uintptr_t>(&cesium_web_fetch_complete));
	cesium_web_fetch_js_start(request_id, cb_ptr, method, url, headers_kv, body, body_len);
}

} // namespace cesium_web_fetch_detail

#endif // __EMSCRIPTEN__
