//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsGpuBuffer.h"

namespace BansheeEngine 
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	DirectX 11 implementation of a generic GPU buffer. */
	class VulkanGpuBufferCore : public GpuBufferCore
    {
    public:
		~VulkanGpuBufferCore();

		/** @copydoc GpuBufferCore::lock */
		void* lock(UINT32 offset, UINT32 length, GpuLockOptions options) override;

		/** @copydoc GpuBufferCore::unlock */
		void unlock() override;

		/** @copydoc GpuBufferCore::readData */
		void readData(UINT32 offset, UINT32 length, void* pDest) override;

		/** @copydoc GpuBufferCore::writeData */
        void writeData(UINT32 offset, UINT32 length, const void* pSource,
			BufferWriteType writeFlags = BWT_NORMAL) override;

		/** @copydoc GpuBufferCore::copyData */
		void copyData(GpuBufferCore& srcBuffer, UINT32 srcOffset, 
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false) override;
		
		/** 
		 * Gets the resource wrapping the buffer object, on the specified device. If GPU param block buffer's device mask
		 * doesn't include the provided device, null is returned. 
		 */
		VulkanBuffer* getResource(UINT32 deviceIdx) const;
	protected:
		friend class VulkanHardwareBufferCoreManager;

		VulkanGpuBufferCore(const GPU_BUFFER_DESC& desc, GpuDeviceFlags deviceMask);

		/** @copydoc GpuBufferCore::initialize */
		void initialize() override;
	private:
		VulkanHardwareBuffer* mBuffer;
		GpuDeviceFlags mDeviceMask;
    };

	/** @} */
}
