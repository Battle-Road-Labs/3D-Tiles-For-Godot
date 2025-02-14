#ifndef GFONTLOADER_H
#define GFONTLOADER_H

#include <Ultralight/Ultralight.h>
#include <AppCore/Platform.h>
#include <JavaScriptCore/JavaScript.h>

#include <godot_cpp/classes/file_access.hpp>

namespace ultralight
{
    class GodotFontLoader : public FontLoader
    {
    public:
        GodotFontLoader() {}
        ~GodotFontLoader() override {}

        String fallback_font() const override
        {

        }

        String 	fallback_font_for_characters(const String &characters, int weight, bool italic) const override
        {
            
        }

        RefPtr<FontFile> Load(const String &family, int weight, bool italic) override
        {
            
        } 

    };
};

#endif
