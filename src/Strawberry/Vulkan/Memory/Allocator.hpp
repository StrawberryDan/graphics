#pragma once
//======================================================================================================================
//  Includes
//----------------------------------------------------------------------------------------------------------------------
// Strawberry Vulkan
#include "Strawberry/Vulkan/Memory/Memory.hpp"
#include "Strawberry/Vulkan/Device.hpp"
// Strawberry Core
#include "Strawberry/Core/Types/ReflexivePointer.hpp"
#include "Strawberry/Core/Types/Variant.hpp"
#include "Strawberry/Core/Types/Result.hpp"
// Vulkan
#include <vulkan/vulkan.h>
// Standard Library
#include <concepts>
#include <unordered_map>


//======================================================================================================================
//  Class Declaration
//----------------------------------------------------------------------------------------------------------------------


namespace Strawberry::Vulkan
{
	class Allocator;
	class Allocation;


	class AllocationError
	{
	public:
		struct OutOfMemory {};

		struct MemoryTypeUnavailable {};

		struct RequestTooLarge {};


		template<typename T>
		AllocationError(T info)
			: mInfo(info) {}


		template<typename T>
		[[nodiscard]] bool IsType() const noexcept
		{
			return mInfo.IsType<T>();
		}


		template<typename T>
		[[nodiscard]] T GetInfo() const noexcept
		{
			Core::Assert(IsType<T>());
			return mInfo.Ref<T>();
		}

	private:
		using Info = Core::Variant<OutOfMemory, MemoryTypeUnavailable>;
		Info mInfo;
	};


	// Class representing a block of allocated Vulkan Memory.
	class MemoryPool final
			: public Core::EnableReflexivePointer
	{
	public:
		static Core::Result<MemoryPool, AllocationError> Allocate(Device& device, const PhysicalDevice& physicalDevice, uint32_t memoryTypeIndex, size_t size);


		MemoryPool() = default;
		MemoryPool(Device& device, const PhysicalDevice& physicalDevice, uint32_t memoryTypeIndex, VkDeviceMemory memory, size_t size);
		MemoryPool(const MemoryPool&)            = delete;
		MemoryPool& operator=(const MemoryPool&) = delete;
		MemoryPool(MemoryPool&& other) noexcept;
		MemoryPool& operator=(MemoryPool&& other) noexcept;
		~MemoryPool() override;


		Allocation AllocateView(Allocator& allocator, size_t offset, size_t size);


		Core::ReflexivePointer<Device> GetDevice() const noexcept
		{
			return mDevice;
		}


		Core::ReflexivePointer<PhysicalDevice> GetPhysicalDevice() const noexcept
		{
			return mPhysicalDevice;
		}


		VkDeviceMemory Memory() const noexcept
		{
			return mMemory;
		}


		uint32_t MemoryTypeIndex() const noexcept
		{
			return mMemoryTypeIndex;
		}


		size_t Size() const noexcept
		{
			return mSize;
		}


		VkMemoryPropertyFlags Properties() const;
		uint8_t*              GetMappedAddress() const noexcept;


		void Flush() const noexcept;
		void Overwrite(const Core::IO::DynamicByteBuffer& bytes) const noexcept;

	private:
		Core::ReflexivePointer<Device>         mDevice          = nullptr;
		Core::ReflexivePointer<PhysicalDevice> mPhysicalDevice  = nullptr;
		uint32_t                               mMemoryTypeIndex = -1;
		VkDeviceMemory                         mMemory          = VK_NULL_HANDLE;
		size_t                                 mSize            = 0;
		mutable Core::Optional<uint8_t*>       mMappedAddress   = Core::NullOpt;
	};


	using AllocationResult = Core::Result<Allocation, AllocationError>;


	struct AllocationRequest
	{
		AllocationRequest(const Device& device, size_t size, size_t alignment)
			: device(device)
			, size(size)
			, alignment(alignment) {}


		AllocationRequest(VkMemoryRequirements& requirements);

		Core::ReflexivePointer<Device> device;
		size_t                         size;
		size_t                         alignment;
		uint32_t                       memoryTypeMask = 0xFFFFFFFF;
	};


	class Allocator
			: public Core::EnableReflexivePointer
	{
	public:
		virtual AllocationResult Allocate(const AllocationRequest& allocationRequest) noexcept = 0;
		virtual void             Free(Allocation&& address) noexcept = 0;
		virtual                  ~Allocator() = default;
	};


	class Allocation
	{
	public:
		Allocation() = default;
		Allocation(const Device& device, Allocator& allocator, MemoryPool& allocation, size_t offset, size_t size);
		Allocation(const Allocation&)            = delete;
		Allocation& operator=(const Allocation&) = delete;
		Allocation(Allocation&& other) noexcept;
		Allocation& operator=(Allocation&& other) noexcept;
		~Allocation();


		explicit operator bool() const noexcept
		{
			Core::AssertImplication(!mAllocator, mRawAllocation);
			return mAllocator;
		}


		[[nodiscard]] Core::ReflexivePointer<Allocator> GetAllocator() const noexcept;
		[[nodiscard]] Address                           Address() const noexcept;
		[[nodiscard]] VkDeviceMemory                    Memory() const noexcept;
		[[nodiscard]] size_t                            Offset() const noexcept;
		[[nodiscard]] size_t                            Size() const noexcept;
		[[nodiscard]] VkMemoryPropertyFlags             Properties() const;
		[[nodiscard]] uint8_t*                          GetMappedAddress() const noexcept;


		void Flush() const noexcept;
		void Overwrite(const Core::IO::DynamicByteBuffer& bytes) const noexcept;

	private:
		VkDevice                           mDevice        = VK_NULL_HANDLE;
		Core::ReflexivePointer<Allocator>  mAllocator     = nullptr;
		Core::ReflexivePointer<MemoryPool> mRawAllocation = nullptr;
		size_t                             mOffset        = 0;
		size_t                             mSize          = 0;
	};
}
