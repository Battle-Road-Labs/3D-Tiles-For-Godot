#ifndef CESIUM_GD_TEXTURE_LOADER_H
#define CESIUM_GD_TEXTURE_LOADER_H

#include "CesiumGltf/ImageAsset.h"
#if defined(CESIUM_GD_EXT)
#include <godot_cpp/classes/image_texture.hpp>
using namespace godot;
#elif defined(CESIUM_GD_MODULE)
#include "scene/resources/image_texture.h"
#endif


#include "CesiumGltf/Image.h"

class CesiumGDTextureLoader {

public:
	static Ref<ImageTexture> load_image_texture(const CesiumGltf::ImageAsset& image, bool generateMipMaps, bool imageHasMipMaps);
private:
	static Error try_get_image_format(int32_t channelCount, int32_t bytesPerChannel, Image::Format* outFormat);

};

#endif // !CESIUM_GD_TEXTURE_LOADER_H
