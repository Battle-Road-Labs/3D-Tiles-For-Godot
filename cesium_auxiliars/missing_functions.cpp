#include "missing_functions.hpp"

// On web, curl is not available — networking uses Godot's HTTPRequest via
// NetworkAssetAccessor. Provide no-op stubs for curl functions that
// cesium-native's CesiumCurl library references so they resolve at runtime.
#ifdef __EMSCRIPTEN__
extern "C" {
    int curl_global_init(long) { return 0; }
    void curl_global_cleanup(void) {}
    void *curl_easy_init(void) { return nullptr; }
    void curl_easy_cleanup(void *) {}
    int curl_easy_setopt(void *, int, ...) { return 0; }
    int curl_easy_perform(void *) { return 0; }
    int curl_easy_getinfo(void *, int, ...) { return 0; }
    void *curl_multi_init(void) { return nullptr; }
    int curl_multi_cleanup(void *) { return 0; }
    int curl_multi_add_handle(void *, void *) { return 0; }
    int curl_multi_remove_handle(void *, void *) { return 0; }
    int curl_multi_perform(void *, int *) { return 0; }
    int curl_multi_poll(void *, void *, unsigned int, int, int *) { return 0; }
    struct curl_slist *curl_slist_append(struct curl_slist *list, const char *) { return list; }
    void curl_slist_free_all(struct curl_slist *) {}
    const char *curl_easy_strerror(int) { return "curl not available on web"; }
}
#endif

#if defined(CESIUM_GD_EXT)
#include "godot_cpp/classes/file_access.hpp"
#include "godot_cpp/classes/engine.hpp"
using namespace godot;


Ref<FileAccess> open_file_access_with_err(const String &p_path, FileAccess::ModeFlags p_flags, Error* err) {
  Ref<FileAccess> result = FileAccess::open(p_path, p_flags);
  *err = FileAccess::get_open_error();
  return result;
}


bool is_editor_mode() {
  return Engine::get_singleton()->is_editor_hint();
}

#elif defined(CESIUM_GD_MODULE)

Ref<FileAccess> open_file_access_with_err(const String &p_path, FileAccess::ModeFlags p_flags, Error* err) {
  return FileAccess::open(p_path, p_flags, err);
}

bool is_editor_mode() {
    return Engine::get_singleton()->is_editor_hint();
}

#endif
