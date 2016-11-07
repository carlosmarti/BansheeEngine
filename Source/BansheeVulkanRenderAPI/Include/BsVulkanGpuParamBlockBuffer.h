//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsGpuParamBlockBuffer.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	/**	Vulkan implementation of a parameter block buffer (uniform buffer in Vulkan lingo). */
	class VulkanGpuParamBlockBufferCore : public GpuParamBlockBufferCore
	{
	public:
		VulkanGpuParamBlockBufferCore(UINT32 size, GpuParamBlockUsage usage, GpuDeviceFlags deviceMask);
		~VulkanGpuParamBlockBufferCore();

		/** @copydoc GpuParamBlockBufferCore::writeToGPU */
		void writeToGPU(const UINT8* data) override;

		/** @copydoc GpuParamBlockBufferCore::readFromGPU */
		void readFromGPU(UINT8* data) const override;

		/** 
		 * Gets the resource wrapping the buffer object, on the specified device. If GPU param block buffer's device mask
		 * doesn't include the provided device, null is returned. 
		 */
		VulkanBuffer* getResource(UINT32 deviceIdx) const;
	protected:
		/** @copydoc GpuParamBlockBufferCore::initialize */
		void initialize() override;

	private:
		VulkanHardwareBuffer* mBuffer;
		GpuDeviceFlags mDeviceMask;
	};

	/** @} */
}