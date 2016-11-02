//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#include "BsVulkanCommandBuffer.h"
#include "BsVulkanCommandBufferManager.h"
#include "BsVulkanUtility.h"
#include "BsVulkanDevice.h"
#include "BsVulkanQueue.h"

namespace BansheeEngine
{
	VulkanCmdBufferPool::VulkanCmdBufferPool(VulkanDevice& device)
		:mDevice(device), mPools{}, mBuffers {}, mNextId(1)
	{
		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			UINT32 familyIdx = device.getQueueFamily((GpuQueueType)i);

			if (familyIdx == (UINT32)-1)
				continue;

			VkCommandPoolCreateInfo poolInfo;
			poolInfo.sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
			poolInfo.pNext = nullptr;
			poolInfo.flags = VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
			poolInfo.queueFamilyIndex = familyIdx;

			mPools[i].queueFamily = familyIdx;
			vkCreateCommandPool(device.getLogical(), &poolInfo, gVulkanAllocator, &mPools[i].pool);
		}
	}

	VulkanCmdBufferPool::~VulkanCmdBufferPool()
	{
		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			for(UINT32 j = 0; j < BS_MAX_QUEUES_PER_TYPE; j++)
			{
				VulkanCmdBuffer** buffers = mBuffers[i][j];
				for(UINT32 k = 0; k < BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE; k++)
				{
					if (buffers[k] == nullptr)
						break;

					bs_delete(buffers[k]);
				}
			}
		}

		for (UINT32 i = 0; i < GQT_COUNT; i++)
		{
			if (mPools[i].pool == VK_NULL_HANDLE)
				continue;

			vkDestroyCommandPool(mDevice.getLogical(), mPools[i].pool, gVulkanAllocator);
		}
	}

	VulkanCmdBuffer* VulkanCmdBufferPool::getBuffer(GpuQueueType type, UINT32 queueIdx, bool secondary)
	{
		assert(queueIdx < BS_MAX_QUEUES_PER_TYPE);

		VulkanCmdBuffer** buffers = mBuffers[type][queueIdx];

		UINT32 i = 0;
		for(; i < BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE; i++)
		{
			if (buffers[i] == nullptr)
				break;

			if(buffers[i]->mState == VulkanCmdBuffer::State::Ready)
			{
				buffers[i]->begin();
				return buffers[i];
			}
		}

		assert(i < BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE && 
			"Too many command buffers allocated. Increment BS_MAX_VULKAN_COMMAND_BUFFERS_PER_QUEUE to a higher value. ");

		buffers[i] = createBuffer(type, secondary);
		buffers[i]->begin();

		return buffers[i];
	}

	VulkanCmdBuffer* VulkanCmdBufferPool::createBuffer(GpuQueueType type, bool secondary)
	{
		const PoolInfo& poolInfo = getPool(type);

		return bs_new<VulkanCmdBuffer>(mDevice, mNextId++, poolInfo.pool, poolInfo.queueFamily, secondary);
	}

	const VulkanCmdBufferPool::PoolInfo& VulkanCmdBufferPool::getPool(GpuQueueType type)
	{
		const PoolInfo* poolInfo = &mPools[type];
		if (poolInfo->pool == VK_NULL_HANDLE)
			poolInfo = &mPools[GQT_GRAPHICS]; // Graphics queue is guaranteed to exist

		return *poolInfo;
	}

	VulkanCmdBuffer::VulkanCmdBuffer(VulkanDevice& device, UINT32 id, VkCommandPool pool, UINT32 queueFamily, bool secondary)
		:mId(id), mQueueFamily(queueFamily), mState(State::Ready), mDevice(device), mPool(pool), mFenceCounter(0)
	{
		VkCommandBufferAllocateInfo cmdBufferAllocInfo;
		cmdBufferAllocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
		cmdBufferAllocInfo.pNext = nullptr;
		cmdBufferAllocInfo.commandPool = pool;
		cmdBufferAllocInfo.level = secondary ? VK_COMMAND_BUFFER_LEVEL_SECONDARY : VK_COMMAND_BUFFER_LEVEL_PRIMARY;
		cmdBufferAllocInfo.commandBufferCount = 1;

		VkResult result = vkAllocateCommandBuffers(mDevice.getLogical(), &cmdBufferAllocInfo, &mCmdBuffer);
		assert(result == VK_SUCCESS);

		VkFenceCreateInfo fenceCI;
		fenceCI.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
		fenceCI.pNext = nullptr;
		fenceCI.flags = 0;

		result = vkCreateFence(mDevice.getLogical(), &fenceCI, gVulkanAllocator, &mFence);
		assert(result == VK_SUCCESS);

		VkSemaphoreCreateInfo semaphoreCI;
		semaphoreCI.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;
		semaphoreCI.pNext = nullptr;
		semaphoreCI.flags = 0;

		result = vkCreateSemaphore(mDevice.getLogical(), &semaphoreCI, gVulkanAllocator, &mSemaphore);
		assert(result == VK_SUCCESS);
	}

	VulkanCmdBuffer::~VulkanCmdBuffer()
	{
		VkDevice device = mDevice.getLogical();

		if(mState == State::Submitted)
		{
			// Wait 1s
			UINT64 waitTime = 1000 * 1000 * 1000;
			VkResult result = vkWaitForFences(device, 1, &mFence, true, waitTime);
			assert(result == VK_SUCCESS || result == VK_TIMEOUT);

			if (result == VK_TIMEOUT)
				LOGWRN("Freeing a command buffer before done executing because fence wait expired!");
		}
		
		vkDestroyFence(device, mFence, gVulkanAllocator);
		vkDestroySemaphore(device, mSemaphore, gVulkanAllocator);
		vkFreeCommandBuffers(device, mPool, 1, &mCmdBuffer);
	}

	void VulkanCmdBuffer::begin()
	{
		assert(mState == State::Ready);

		VkCommandBufferBeginInfo beginInfo;
		beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
		beginInfo.pNext = nullptr;
		beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
		beginInfo.pInheritanceInfo = nullptr;

		VkResult result = vkBeginCommandBuffer(mCmdBuffer, &beginInfo);
		assert(result == VK_SUCCESS);

		mState = State::Recording;
	}

	void VulkanCmdBuffer::end()
	{
		assert(mState == State::Recording);

		VkResult result = vkEndCommandBuffer(mCmdBuffer);
		assert(result == VK_SUCCESS);

		mState = State::RecordingDone;
	}

	void VulkanCmdBuffer::beginRenderPass()
	{
		assert(mState == State::Recording);

		// TODO
		BS_EXCEPT(NotImplementedException, "Not implemented");

		mState = State::RecordingRenderPass;
	}

	void VulkanCmdBuffer::endRenderPass()
	{
		assert(mState == State::RecordingRenderPass);

		vkCmdEndRenderPass(mCmdBuffer);

		mState = State::Recording;
	}

	void VulkanCmdBuffer::refreshFenceStatus()
	{
		VkResult result = vkGetFenceStatus(mDevice.getLogical(), mFence);
		assert(result == VK_SUCCESS || result == VK_NOT_READY);

		bool signaled = result == VK_SUCCESS;

		if (mState == State::Submitted)
		{
			if(signaled)
			{
				mState = State::Ready;
				vkResetCommandBuffer(mCmdBuffer, VK_COMMAND_BUFFER_RESET_RELEASE_RESOURCES_BIT); // Note: Maybe better not to release resources?

				result = vkResetFences(mDevice.getLogical(), 1, &mFence);
				assert(result == VK_SUCCESS);

				mFenceCounter++;

				for (auto& entry : mResources)
					entry.first->notifyDone(this);

				mResources.clear();
			}
		}
		else
			assert(!signaled); // We reset the fence along with mState so this shouldn't be possible
	}

	void VulkanCmdBuffer::registerResource(VulkanResource* res, VulkanUseFlags flags)
	{
		mResources[res].flags |= flags;
	}

	void VulkanCmdBuffer::notifySubmit()
	{
		// TODO - Issue pipeline barrier for resources transitioning to a new queue family

		for (auto& entry : mResources)
			entry.first->notifyUsed(this, entry.second.flags);
	}

	VulkanCommandBuffer::VulkanCommandBuffer(VulkanDevice& device, UINT32 id, GpuQueueType type, UINT32 deviceIdx,
		UINT32 queueIdx, bool secondary)
		: CommandBuffer(id, type, deviceIdx, queueIdx, secondary), mBuffer(nullptr), mSubmittedBuffer(nullptr)
		, mDevice(device), mQueue(nullptr), mIdMask(0)
	{
		GpuQueueType queueType = mType;

		UINT32 numQueues = device.getNumQueues(queueType);
		if (numQueues == 0) // Fallback to graphics queue
		{
			queueType = GQT_GRAPHICS;
			numQueues = device.getNumQueues(GQT_GRAPHICS);
		}

		mQueue = device.getQueue(queueType, mQueueIdx % numQueues);

		// If multiple command buffer IDs map to the same queue, mark them in the mask
		UINT32 curIdx = mQueueIdx;
		while (curIdx < BS_MAX_QUEUES_PER_TYPE)
		{
			mIdMask |= CommandSyncMask::getGlobalQueueIdx(queueType, curIdx);
			curIdx += numQueues;
		}

		acquireNewBuffer();
	}

	void VulkanCommandBuffer::refreshSubmitStatus()
	{
		if (mSubmittedBuffer == nullptr) // Nothing was submitted
			return;

		mSubmittedBuffer->refreshFenceStatus();
		if (!mSubmittedBuffer->isSubmitted())
			mSubmittedBuffer = nullptr;
	}

	void VulkanCommandBuffer::acquireNewBuffer()
	{
		VulkanCmdBufferPool& pool = mDevice.getCmdBufferPool();

		if (mBuffer != nullptr)
			assert(mBuffer->isSubmitted());

		mSubmittedBuffer = mBuffer;
		mBuffer = pool.getBuffer(mType, mQueueIdx, mIsSecondary);
	}

	void VulkanCommandBuffer::submit(UINT32 syncMask)
	{
		assert(mBuffer != nullptr && mBuffer->isReadyForSubmit());

		VkCommandBuffer cmdBuffer = mBuffer->getHandle();
		VkSemaphore signalSemaphore = mBuffer->getSemaphore();

		VkSubmitInfo submitInfo;
		submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;
		submitInfo.pNext = nullptr;
		submitInfo.pWaitDstStageMask = 0;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &cmdBuffer;
		submitInfo.signalSemaphoreCount = 1;
		submitInfo.pSignalSemaphores = &signalSemaphore;

		// Ignore myself
		syncMask &= ~mIdMask;

		VulkanCommandBufferManager& cmdBufManager = static_cast<VulkanCommandBufferManager&>(CommandBufferManager::instance());
		cmdBufManager.getSyncSemaphores(mDeviceIdx, syncMask, mSemaphoresTemp, submitInfo.waitSemaphoreCount);

		if (submitInfo.waitSemaphoreCount > 0)
			submitInfo.pWaitSemaphores = mSemaphoresTemp;
		else
			submitInfo.pWaitSemaphores = nullptr;

		VkQueue queue = mQueue->getHandle();
		VkFence fence = mBuffer->getFence();
		vkQueueSubmit(queue, 1, &submitInfo, fence);

		cmdBufManager.refreshStates(mDeviceIdx);

		mBuffer->notifySubmit();
		mQueue->notifySubmit(*this, mBuffer->getFenceCounter());

		// Note: Uncommented for debugging only, prevents any device concurrency issues.
		// vkQueueWaitIdle(mQueue);

		mBuffer->mState = VulkanCmdBuffer::State::Submitted;
		acquireNewBuffer();
	}
}