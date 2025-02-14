#ifndef JSCALLABLE_H
#define JSCALLABLE_H

#include "godot_cpp/variant/callable.hpp"

#include <AppCore/JSHelpers.h>

using namespace ultralight;

namespace godot {
    class JSCallable : public Callable
    {
    private:
        JSContextRef context;
        JSValue js_value;
    public:
        JSCallable(JSContextRef p_context, JSValue p_js_value)
            : context(p_context), js_value(p_js_value) {}
    };
}

#endif
