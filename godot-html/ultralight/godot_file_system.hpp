#ifndef GFILESYSTEM_H
#define GFILESYSTEM_H

#include <godot_cpp/classes/file_access.hpp>

#include "MimeTypes.h"

namespace ultralight
{
    class GodotFileSystem : public FileSystem
    {
    public:
        GodotFileSystem() {}
        ~GodotFileSystem() override {}

        String SetupPath(const String& file_path)
        {
            return ("res://"+file_path);
        }

        bool FileExists(const String& file_path) override
        {
            return godot::FileAccess::file_exists(SetupPath(file_path).utf8().data());
        }

        String GetFileCharset(const String& file_path) override
        {
            return "utf-8";
        }

        String GetFileMimeType(const String& file_path) override
        {
            return MimeTypes::getType(SetupPath(file_path).utf8().data());
        }

        RefPtr<Buffer> OpenFile(const String& file_path) override
        {
            godot::PackedByteArray data = godot::FileAccess::get_file_as_bytes(SetupPath(file_path).utf8().data());
            return Buffer::CreateFromCopy(data.ptr(), data.size());
        }

    };
};

#endif
