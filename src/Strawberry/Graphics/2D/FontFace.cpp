//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "FontFace.hpp"
#include "Strawberry/Graphics/Vulkan/Buffer.hpp"
#include "Strawberry/Graphics/Vulkan/Device.hpp"
#include "Strawberry/Graphics/Vulkan/CommandBuffer.hpp"
// Strawberry Core
#include "Strawberry/Core/Assert.hpp"
// Standard Library
#include <memory>


//======================================================================================================================
//  Class Definitions
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Graphics
{
	static FT_Library sFreetypeLibrary{};


	void FreeType::Initialise()
	{
		Core::AssertEQ(FT_Init_FreeType(&sFreetypeLibrary),
					   0);
	}


	void FreeType::Terminate()
	{
		Core::AssertEQ(FT_Done_FreeType(sFreetypeLibrary),
					   0);
		sFreetypeLibrary = nullptr;
	}


	Core::Optional<FontFace> FontFace::FromFile(const std::filesystem::path& file)
	{
		FontFace face;

		Core::Assert(exists(file));

		if (auto error = FT_New_Face(sFreetypeLibrary, file.string().c_str(), 0, &face.mFace); error != 0)
		{
			return Core::NullOpt;
		}

		return face;
	}


	FontFace::FontFace(Strawberry::Graphics::FontFace&& rhs) noexcept
		: mFace(std::exchange(rhs.mFace, {}))
	{
		SetPixelSize(GetPixelSize());
	}


	FontFace& FontFace::operator=(Strawberry::Graphics::FontFace&& rhs) noexcept
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	FontFace::~FontFace()
	{
		if (sFreetypeLibrary && mFace && mFace->internal)
		{
			Core::AssertEQ(FT_Done_Face(mFace),
						   0);
		}
	}


	Core::Math::Vec2f FontFace::GetGlyphBoundingBox(char32_t c) const
	{
		auto index = FT_Get_Char_Index(mFace, c);
		Core::AssertEQ(FT_Load_Glyph(mFace, index, FT_LOAD_DEFAULT), 0);
		return Core::Math::Vec2f(static_cast<float>(mFace->glyph->metrics.width) , static_cast<float>(mFace->glyph->metrics.height)) / 64.0;
	}


	Core::Math::Vec2f FontFace::GetGlyphHorizontalBearing(char32_t c) const
	{
		auto index = FT_Get_Char_Index(mFace, c);
		Core::AssertEQ(FT_Load_Glyph(mFace, index, FT_LOAD_DEFAULT), 0);
		return Core::Math::Vec2f(static_cast<float>(mFace->glyph->metrics.horiBearingX) / 64.0f , static_cast<float>(mFace->glyph->metrics.horiBearingY) / 64.0f);
	}


	Core::Math::Vec2f FontFace::GetGlyphAdvance(char32_t c) const
	{
		auto index = FT_Get_Char_Index(mFace, c);
		Core::AssertEQ(FT_Load_Glyph(mFace, index, FT_LOAD_DEFAULT), 0);
		return {static_cast<float>(mFace->glyph->advance.x) / 64.0f, static_cast<float>(mFace->glyph->advance.y) / 64.0f};
	}


	Core::Optional<Vulkan::Image*> FontFace::GetGlyphBitmap(Vulkan::Queue& queue, char32_t c)
	{
		FT_Glyph* glyph;

		if (mGlyphCache.contains(c))
		{
			return &mGlyphCache.at(c);
		}


		auto index = FT_Get_Char_Index(mFace, c);
		Core::AssertEQ(FT_Load_Glyph(mFace, index, FT_LOAD_DEFAULT), 0);
		Core::AssertEQ(FT_Render_Glyph(mFace->glyph, FT_RENDER_MODE_NORMAL), 0);

		const uint32_t pixelCount = mFace->glyph->bitmap.width * mFace->glyph->bitmap.rows;
		if (pixelCount == 0) return Core::NullOpt;

		const uint32_t bitmapLength = std::abs(mFace->glyph->bitmap.pitch) * mFace->glyph->bitmap.rows;
		Vulkan::Buffer buffer(*queue.GetDevice(), 4 * pixelCount, VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		Core::IO::DynamicByteBuffer glyphBytes = Core::IO::DynamicByteBuffer::WithCapacity(4 * pixelCount);
		for (int i = 0; i < bitmapLength; i++)
		{
			auto byte = mFace->glyph->bitmap.buffer[i];
			glyphBytes.Push<uint8_t>(byte);
			glyphBytes.Push<uint8_t>(byte);
			glyphBytes.Push<uint8_t>(byte);
			glyphBytes.Push<uint8_t>(byte);
		}
		buffer.SetData(glyphBytes);

		Vulkan::Image image(*queue.GetDevice(), Core::Math::Vec2u(mFace->glyph->bitmap.width, mFace->glyph->bitmap.rows), VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);

		auto commandBuffer = queue.Create<Vulkan::CommandBuffer>();
		commandBuffer.Begin(true);
		commandBuffer.CopyBufferToImage(buffer, image);
		commandBuffer.ImageMemoryBarrier(image, VK_IMAGE_ASPECT_COLOR_BIT, VK_IMAGE_LAYOUT_GENERAL);
		commandBuffer.End();
		queue.Submit(commandBuffer);

		mGlyphCache.emplace(c, std::move(image));

		return &mGlyphCache.at(c);
	}


	void FontFace::SetPixelSize(uint32_t pixelSize)
	{
		SetPixelSize({pixelSize, 0});
	}


	void FontFace::SetPixelSize(Core::Math::Vec2u pixelSize)
	{
		Core::AssertEQ(FT_Set_Pixel_Sizes(mFace, pixelSize[0], pixelSize[1]),
					   0);
		mPixelSize = pixelSize;
	}


	Core::Math::Vec2u FontFace::GetPixelSize() const
	{
		return mPixelSize;
	}
}
