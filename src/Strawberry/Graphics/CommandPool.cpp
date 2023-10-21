//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
#include "CommandPool.hpp"
#include "Device.hpp"
// Strawberry Core
#include <Strawberry/Core/Assert.hpp>
// Standard Library
#include <memory>


//======================================================================================================================
//  Class Definitions
//----------------------------------------------------------------------------------------------------------------------
namespace Strawberry::Graphics
{
	CommandPool::CommandPool(const Device& device, bool resetBit)
		: mDevice(device.mDevice)
		, mQueueFamilyIndex(device.mQueueFamilyIndex)
	{
		VkCommandPoolCreateInfo createInfo{
			.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
			.pNext = nullptr,
			.flags = resetBit ? VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT : VkCommandPoolCreateFlags(0),
			.queueFamilyIndex = device.mQueueFamilyIndex
		};

		Core::Assert(vkCreateCommandPool(device.mDevice, &createInfo, nullptr, &mCommandPool) == VK_SUCCESS);
	}


	CommandPool::CommandPool(CommandPool&& rhs) noexcept
		: mCommandPool(std::exchange(rhs.mCommandPool, nullptr))
		  , mDevice(std::exchange(rhs.mDevice, nullptr))
		  , mQueueFamilyIndex(std::exchange(rhs.mQueueFamilyIndex, 0))
	  {

	  }


	CommandPool& CommandPool::operator=(CommandPool&& rhs)
	{
		if (this != &rhs)
		{
			std::destroy_at(this);
			std::construct_at(this, std::move(rhs));
		}

		return *this;
	}


	CommandPool::~CommandPool()
	{
		if (mCommandPool)
		{
			vkDestroyCommandPool(mDevice, mCommandPool, nullptr);
		}
	}
}
