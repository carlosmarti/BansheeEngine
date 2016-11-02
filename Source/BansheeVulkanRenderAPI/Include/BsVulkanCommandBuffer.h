//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsVulkanPrerequisites.h"
#include "BsCommandBuffer.h"
#include "BsVulkanRenderAPI.h"
#include "BsVulkanResource.h"

namespace BansheeEngine
{
	/** @addtogroup Vulkan
	 *  @{
	 */

	class VulkanCmdBuffer;

#define BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE 32

	/** Pool that allocates and distributes Vulkan command buffers. */
	class VulkanCmdBufferPool
	{
	public:
		VulkanCmdBufferPool(VulkanDevice& device);
		~VulkanCmdBufferPool();

		/** Attempts to find a free command buffer, or creates a new one if not found. */
		VulkanCmdBuffer* getBuffer(GpuQueueType type, UINT32 queueIdx, bool secondary);

	private:
		/** Command buffer pool and related information. */
		struct PoolInfo
		{
			VkCommandPool pool = VK_NULL_HANDLE;
			UINT32 queueFamily = -1;
		};

		/** Creates a new command buffer. */
		VulkanCmdBuffer* createBuffer(GpuQueueType type, bool secondary);

		/** Returns a Vulkan command pool for the specified queue type. */
		const PoolInfo& getPool(GpuQueueType type);

		VulkanDevice& mDevice;
		PoolInfo mPools[GQT_COUNT];

		VulkanCmdBuffer* mBuffers[GQT_COUNT][BS_MAX_QUEUES_PER_TYPE][BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE];
		UINT32 mNextId;
	};

	/** 
	 * Represents a direct wrapper over an internal Vulkan command buffer. This is unlike VulkanCommandBuffer which is a
	 * higher level class, and it allows for re-use by internally using multiple low-level command buffers.
	 */
	class VulkanCmdBuffer
	{
		/** Possible states a command buffer can be in. */
		enum class State
		{
			/** Buffer is ready to be re-used. */
			Ready,
			/** Buffer is currently recording commands, but isn't recording a render pass. */
			Recording,
			/** Buffer is currently recording render pass commands. */
			RecordingRenderPass,
			/** Buffer is done recording but hasn't been submitted. */
			RecordingDone,
			/** Buffer is done recording and is currently submitted on a queue. */
			Submitted
		};

	public:
		VulkanCmdBuffer(VulkanDevice& device, UINT32 id, VkCommandPool pool, UINT32 queueFamily, bool secondary);
		~VulkanCmdBuffer();

		/** Returns an unique identifier of this command buffer. */
		UINT32 getId() const { return mId; }

		/** Returns the index of the queue family this command buffer is executing on. */
		UINT32 getQueueFamily() const { return mQueueFamily; }

		/** Makes the command buffer ready to start recording commands. */
		void begin();

		/** Ends command buffer command recording (as started with begin()). */
		void end();

		/** Begins render pass recording. Must be called within begin()/end() calls. */
		void beginRenderPass();

		/** Ends render pass recording (as started with beginRenderPass(). */
		void endRenderPass();

		/** Returns the handle to the internal Vulkan command buffer wrapped by this object. */
		VkCommandBuffer getHandle() const { return mCmdBuffer; }

		/** Returns a fence that can be used for tracking when the command buffer is done executing. */
		VkFence getFence() const { return mFence; }

		/** 
		 * Returns a semaphore that may be used for synchronizing execution between command buffers executing on different 
		 * queues. 
		 */
		VkSemaphore getSemaphore() const { return mSemaphore; }

		/** Returns true if the command buffer is currently being processed by the device. */
		bool isSubmitted() const { return mState == State::Submitted; }

		/** Returns true if the command buffer is ready to be submitted to a queue. */
		bool isReadyForSubmit() const { return mState == State::RecordingDone; }

		/** Returns a counter that gets incremented whenever the command buffer is done executing. */
		UINT32 getFenceCounter() const { return mFenceCounter; }

		/** Checks the internal fence and changes command buffer state if done executing. */
		void refreshFenceStatus();

		/** 
		 * Lets the command buffer know that the provided resource has been queued on it, and will be used by the
		 * device when the command buffer is submitted.
		 */
		void registerResource(VulkanResource* res, VulkanUseFlags flags);

	private:
		friend class VulkanCmdBufferPool;
		friend class VulkanCommandBuffer;

		/** Information about a resource currently queued for use on the command buffer. */
		struct ResourceInfo
		{
			VulkanUseFlags flags;
		};

		/** Called after the buffer has been submitted to the queue. */
		void notifySubmit();

		UINT32 mId;
		UINT32 mQueueFamily;
		State mState;
		VulkanDevice& mDevice;
		VkCommandPool mPool;
		VkCommandBuffer mCmdBuffer;
		VkFence mFence;
		VkSemaphore mSemaphore;
		UINT32 mFenceCounter;

		UnorderedMap<VulkanResource*, ResourceInfo> mResources;
	};

	/** CommandBuffer implementation for Vulkan. */
	class VulkanCommandBuffer : public CommandBuffer
	{
	public:
		/** 
		 * Submits the command buffer for execution. 
		 * 
		 * @param[in]	syncMask	Mask that controls which other command buffers does this command buffer depend upon
		 *							(if any). See description of @p syncMask parameter in RenderAPICore::executeCommands().
		 */
		void submit(UINT32 syncMask);

		/** Checks if the submitted buffer finished executing, and updates state if it has. */
		void refreshSubmitStatus();

	private:
		friend class VulkanCommandBufferManager;

		VulkanCommandBuffer(VulkanDevice& device, UINT32 id, GpuQueueType type, UINT32 deviceIdx, UINT32 queueIdx,
			bool secondary);

		/** 
		 * Tasks the command buffer to find a new internal command buffer. Call this after the command buffer has been
		 * submitted to a queue (it's not allowed to be used until the queue is done with it).
		 */
		void acquireNewBuffer();

		VulkanCmdBuffer* mBuffer;
		VulkanCmdBuffer* mSubmittedBuffer;
		VulkanDevice& mDevice;
		VulkanQueue* mQueue;
		UINT32 mIdMask;

		VkSemaphore mSemaphoresTemp[BS_MAX_COMMAND_BUFFERS];
	};

	/** @} */
}
