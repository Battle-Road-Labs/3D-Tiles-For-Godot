#include "CesiumGDTextureLoader.h"
#include "CesiumGltf/ImageAsset.h"
#include "godot_cpp/core/error_macros.hpp"
#include "error_names.hpp"

constexpr int32_t RGBA_CHANNEL_COUNT = 4;
constexpr int32_t RGB_CHANNEL_COUNT = 3;

Ref<ImageTexture> CesiumGDTextureLoader::load_image_texture(const CesiumGltf::ImageAsset& image, bool generateMipMaps, bool imageHasMipMaps)
{
	//TODO: we can probably optimize this
	PackedByteArray rawImageData;
	size_t pixelSize = image.pixelData.size();

	for (size_t i = 0; i < pixelSize; ++i)
	{
		const std::byte& element = image.pixelData.at(i);
		rawImageData.push_back(static_cast<uint8_t>(element));
	}

	Image::Format cesiumFormat;
	Error err = try_get_image_format(image.channels, image.bytesPerChannel, &cesiumFormat);
	ERR_FAIL_COND_V_MSG(err != Error::OK, Ref<ImageTexture>(), "Image format not recognized!");

	Ref<Image> godotImage = Image::create_from_data(
		image.width,
		image.height,
		imageHasMipMaps,
		cesiumFormat,
		rawImageData
	);

	if (generateMipMaps) {
		err = godotImage->generate_mipmaps();

		if (err != Error::OK) {
			ERR_PRINT(String("Mipmaps were not generated! Error: ") + REFLECT_ERR_NAME(err));
		}

	}
	Ref<ImageTexture> textureToUse = ImageTexture::create_from_image(godotImage);
  return textureToUse;
}

Error CesiumGDTextureLoader::try_get_image_format(int32_t channelCount, int32_t bytesPerChannel, Image::Format* outFormat)
{
	bool isRGB = channelCount == RGB_CHANNEL_COUNT;
	bool isRGBA = channelCount == RGBA_CHANNEL_COUNT;
	ERR_FAIL_COND_V_MSG(
		!isRGB && !isRGBA,
		Error::ERR_FILE_CORRUPT,
		"Cesium image does not conform to RGB or RGBA color space"
	);

	constexpr int32_t SINGLE_BYTE_PER_CHANNEL = 1;
	constexpr int32_t FLOATING_POINT_BYTES_PER_CHANNEL = 4;

	switch (bytesPerChannel) {
	case SINGLE_BYTE_PER_CHANNEL:
		*outFormat = isRGBA ? Image::FORMAT_RGBA8 : Image::FORMAT_RGB8;
		break;
	case FLOATING_POINT_BYTES_PER_CHANNEL:
		*outFormat = isRGBA ? Image::FORMAT_RGBAF : Image::FORMAT_RGBF;
		break;
	}
	return Error::OK;
}
