//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "SpriteSheet.hpp"
#include "Strawberry/Graphics/Vulkan/CommandBuffer.hpp"
#include "Strawberry/Graphics/Vulkan/Device.hpp"
#include "Strawberry/Graphics/Vulkan/Buffer.hpp"
// Strawberry Core
#include <Strawberry/Core/IO/DynamicByteBuffer.hpp>


//======================================================================================================================
//  Class Definitions
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Graphics
{
	Core::Optional<SpriteSheet>
	SpriteSheet::FromFile(  Vulkan::Queue& queue,
						  Core::Math::Vec2u spriteCount,
						  const std::filesystem::path& filepath)
	{
		// Read file
		auto fileContents = Core::IO::DynamicByteBuffer::FromImage(filepath);
		if (!fileContents) return Core::NullOpt;


		// Unpack file data
		auto [size, channels, bytes] = fileContents.Unwrap();
		Vulkan::Image image(*queue.GetDevice(), size, VK_FORMAT_R8G8B8A8_SRGB, VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
		Vulkan::Buffer imageBuffer(*queue.GetDevice(), bytes.Size(), VK_BUFFER_USAGE_TRANSFER_SRC_BIT);
		imageBuffer.SetData(bytes);


		// Create command buffer
		auto commandBuffer = queue.Create<Vulkan::CommandBuffer>();


		// Copy image data
		commandBuffer.Begin(true);
		commandBuffer.CopyBufferToImage(imageBuffer, image);
		commandBuffer.End();
		queue.Submit(std::move(commandBuffer));
		queue.WaitUntilIdle();


		// Return sprite sheet
		return SpriteSheet(queue, std::move(image), spriteCount);
	}


	SpriteSheet::SpriteSheet(const Vulkan::Queue& queue, Vulkan::Image image, Core::Math::Vec2u spriteCount)
		 : mDevice(queue.GetDevice())
		 , mQueue(queue)
		 , mCommandPool(mQueue->Create<Vulkan::CommandPool>(false))
		 , mImage(std::move(image))
		 , mImageView(mImage.Create<Vulkan::ImageView::Builder>()
		     .WithFormat(VK_FORMAT_R8G8B8A8_SRGB)
			 .WithType(VK_IMAGE_VIEW_TYPE_2D)
		     .Build())
		 , mSpriteCount(spriteCount)

	{}


	Core::Math::Vec2u SpriteSheet::GetSize() const
	{
		return mImage.GetSize().AsType<unsigned int>().AsSize<2>();
	}


	Core::Math::Vec2u SpriteSheet::GetSpriteCount() const
	{
		return mSpriteCount;
	}


	Core::Math::Vec2u SpriteSheet::GetSpriteSize() const
	{
		return {mImage.GetSize()[0] / mSpriteCount[0], mImage.GetSize()[1] / mSpriteCount[1]};
	}


	const Vulkan::Image& SpriteSheet::GetImage() const
	{
		return mImage;
	}
}
