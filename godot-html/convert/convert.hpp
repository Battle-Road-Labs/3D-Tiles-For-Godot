#ifndef CONVERT_H
#define CONVERT_H

#include <godot_cpp/variant/variant.hpp>

#include <Ultralight/Ultralight.h>
#include <AppCore/JSHelpers.h>

using namespace ultralight;
using namespace godot;
using namespace std;

class Convert {
public:
    static Variant ToVariant(JSContextRef context, JSValueRef value);
    static JSValueRef ToJSValue(JSContextRef context, Variant variant);
    static JSValueRef CallableCallback(JSContextRef ctx, JSObjectRef function, JSObjectRef thisObject, size_t argumentCount, const JSValueRef arguments[], JSValueRef* exception);
    static void FinalizeCallable(JSObjectRef object);
};

#endif