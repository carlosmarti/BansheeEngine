//********************************** Banshee Engine (www.banshee3d.com) **************************************************//
//**************** Copyright (c) 2016 Marko Pintera (marko.pintera@gmail.com). All rights reserved. **********************//
#pragma once

#include "BsCorePrerequisites.h"

namespace BansheeEngine 
{
	/** @addtogroup RenderAPI-Internal
	 *  @{
	 */

	/**
	 * Abstract class defining common features of hardware buffers. Hardware buffers usually represent areas of memory the 
	 * GPU or the driver can access directly.
	 *
	 * @note	Core thread only.
	 * @note	Be aware that reading from non-system memory hardware buffers is usually slow and should be avoided.
	 */
	class BS_CORE_EXPORT HardwareBuffer
    {
    public:
        virtual ~HardwareBuffer() {}

		/**
		 * Locks a portion of the buffer and returns pointer to the locked area. You must call unlock() when done.
		 *
		 * @param[in]	offset		Offset in bytes from which to lock the buffer.
		 * @param[in]	length		Length of the area you want to lock, in bytes.
		 * @param[in]	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do
		 *							anything he hasn't requested (for example don't try to read from the buffer unless you
		 *							requested it here).
		 * @param[in]	syncMask	Mask that determines how are read or write operations synchronized with other operations
		 *							on this resource. This corresponds to the sync mask property on a CommandBuffer object.
		 *							If reading, the system will wait on all commands buffers with the shared mask bits
		 *							before continuing.
		 *							If writing, Setting a mask that's different from used command buffers allows the writes
		 *							to be queued independantly from normal rendering commands, allowing for async execution.
		 *
		 *							It's up to the caller to ensure the usage is valid (e.g. not reading something that is
		 *							currently being written to with a different sync mask). If not sure leave at default.
		 */
		virtual void* lock(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask = 0x00000001)
        {
            assert(!isLocked() && "Cannot lock this buffer, it is already locked!");
            void* ret = map(offset, length, options, syncMask);
            mIsLocked = true;

			mLockStart = offset;
			mLockSize = length;
            return ret;
        }

		/**
		 * Locks the entire buffer and returns pointer to the locked area. You must call unlock() when done.
		 *
		 * @param[in]	options		Signifies what you want to do with the returned pointer. Caller must ensure not to do 
		 *							anything he hasn't requested (for example don't try to read from the buffer unless you
		 *							requested it here).
		 * @param[in]	syncMask	Mask that determines how are read or write operations synchronized with other operations
		 *							on this resource. This corresponds to the sync mask property on a CommandBuffer object.
		 *							If reading, the system will wait on all commands buffers with the shared mask bits
		 *							before continuing. 
		 *							If writing, Setting a mask that's different from used command buffers allows the writes
		 *							to be queued independantly from normal rendering commands, allowing for async execution.
		 *							
		 *							It's up to the caller to ensure the usage is valid (e.g. not reading something that is
		 *							currently being written to with a different sync mask). If not sure leave at default.
		 */
        void* lock(GpuLockOptions options, UINT32 syncMask = 0x00000001)
        {
            return this->lock(0, mSizeInBytes, options, syncMask);
        }

		/**	Releases the lock on this buffer. */
		virtual void unlock()
        {
            assert(isLocked() && "Cannot unlock this buffer, it is not locked!");

            unmap();
            mIsLocked = false;
        }

		/**
		 * Reads data from a portion of the buffer and copies it to the destination buffer. Caller must ensure destination 
		 * buffer is large enough.
		 *
		 * @param[in]	offset		Offset in bytes from which to copy the data.
		 * @param[in]	length		Length of the area you want to copy, in bytes.
		 * @param[in]	dest		Destination buffer large enough to store the read data.
		 * @param[in]	syncMask	Mask that determines how is the read operation synchronized with other operations on 
		 *							this resource. This corresponds to the sync mask property on a CommandBuffer object.
		 *							The system will wait on all commands buffers with the shared mask bits before continuing
		 *							with the read. 
		 *							
		 *							It's up to the caller to ensure the usage is valid (e.g. not reading something that is
		 *							currently being written to with a different sync mask). If not sure leave at default.
		 */
        virtual void readData(UINT32 offset, UINT32 length, void* dest, UINT32 syncMask = 0x00000001) = 0;

		/**
		 * Writes data into a portion of the buffer from the source memory. 
		 *
		 * @param[in]	offset		Offset in bytes from which to copy the data.
		 * @param[in]	length		Length of the area you want to copy, in bytes.
		 * @param[in]	source		Source buffer containing the data to write.
		 * @param[in]	writeFlags	Optional write flags that may affect performance.
		 * @param[in]	syncMask	Mask that determines how is the write operation synchronized with other operations
		 *							on this resource. This corresponds to the sync mask property on a CommandBuffer object.
		 *							Setting a mask that's different from used command buffers allows the writes to be queued
		 *							independantly from normal rendering commands, allowing for async execution.
		 *							
		 *							It's up to the caller to ensure the usage is valid (e.g. not writing to something that
		 *							is currently being used by the GPU). If not sure leave at default.
		 */
        virtual void writeData(UINT32 offset, UINT32 length, const void* source,
				BufferWriteType writeFlags = BWT_NORMAL, UINT32 syncMask = 0x00000001) = 0;

		/**
		 * Copies data from a specific portion of the source buffer into a specific portion of this buffer.
		 *
		 * @param[in]	srcBuffer			Buffer to copy from.
		 * @param[in]	srcOffset			Offset into the source buffer to start copying from, in bytes.
		 * @param[in]	dstOffset			Offset into this buffer to start copying to, in bytes.
		 * @param[in]	length				Size of the data to copy, in bytes.
		 * @param[in]	discardWholeBuffer	Specify true if the data in the current buffer can be entirely discarded. This
		 *									may improve performance.
		 * @param[in]	syncMask			Mask that determines how is the copy operation synchronized with other 
		 *									operations on this resource. This corresponds to the sync mask property on a 
		 *									CommandBuffer object. Setting a mask that's different from used command buffers
		 *									allows the copy to be queued independantly from normal rendering commands, 
		 *									allowing for async execution.
		 *									
		 *									It's up to the caller to ensure the usage is valid (e.g. not reading from
		 *									something that is currently being written to, or writing to something that is
		 *									currently being read).		
		 */
		virtual void copyData(HardwareBuffer& srcBuffer, UINT32 srcOffset, 
			UINT32 dstOffset, UINT32 length, bool discardWholeBuffer = false, UINT32 syncMask = 0x00000001)
		{
			const void *srcData = srcBuffer.lock(
				srcOffset, length, GBL_READ_ONLY, syncMask);
			this->writeData(dstOffset, length, srcData, discardWholeBuffer ? BWT_DISCARD : BWT_NORMAL, syncMask);
			srcBuffer.unlock();
		}

		/**
		 * Copy data from the provided buffer into this buffer. If buffers are not the same size, smaller size will be used.
		 * 
		* @param[in]	syncMask			Mask that determines how is the copy operation synchronized with other 
		 *									operations on this resource. This corresponds to the sync mask property on a 
		 *									CommandBuffer object. Setting a mask that's different from used command buffers
		 *									allows the copy to be queued independantly from normal rendering commands, 
		 *									allowing for async execution.
		 *									
		 *									It's up to the caller to ensure the usage is valid (e.g. not reading from
		 *									something that is currently being written to, or writing to something that is
		 *									currently being read).		
		 */
		virtual void copyData(HardwareBuffer& srcBuffer, UINT32 syncMask = 0x00000001)
		{
			UINT32 sz = std::min(getSizeInBytes(), srcBuffer.getSizeInBytes()); 
			copyData(srcBuffer, 0, 0, sz, true, syncMask);
		}
			
		/** Returns the size of this buffer in bytes. */
        UINT32 getSizeInBytes() const { return mSizeInBytes; }

		/**	Returns the Usage flags with which this buffer was created. */
        GpuBufferUsage getUsage() const { return mUsage; }

		/**	Returns whether or not this buffer is currently locked. */
        bool isLocked() const { return mIsLocked; }	

	protected:
		friend class HardwareBufferManager;

		/**
		 * Constructs a new buffer.
		 *
		 * @param[in]	usage			Determines most common usage of the buffer. Usually has effect on what type of 
		 *								memory will be buffer allocated in but that depends on render API. Specify dynamic 
		 *								if you plan on modifying it often, static otherwise.
		 */
		HardwareBuffer(GpuBufferUsage usage)
			: mUsage(usage), mIsLocked(false)
		{  }

		/** @copydoc lock */
		virtual void* map(UINT32 offset, UINT32 length, GpuLockOptions options, UINT32 syncMask) = 0;

		/** @copydoc unlock */
		virtual void unmap() = 0;

	protected:
		UINT32 mSizeInBytes;
		GpuBufferUsage mUsage;
		bool mIsLocked;
		UINT32 mLockStart;
		UINT32 mLockSize;
    };

	/** @} */
}