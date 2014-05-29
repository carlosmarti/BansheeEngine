#pragma once

#include "CmPrerequisites.h"
#include "CmModule.h"
#include "CmCPUProfiler.h"

namespace BansheeEngine
{
/**
 * @brief	Shortcut for profiling a single function call.
 */
#define PROFILE_CALL(call, name)						\
	BansheeEngine::gProfiler().beginSample(##name##);	\
	call;												\
	BansheeEngine::gProfiler().endSample(##name##);

	/**
	 * @brief	Contains data about a profiling session.
	 */
	struct ProfilerReport
	{
		CPUProfilerReport cpuReport;
	};

	/**
	 * @brief	Type of thread used by the profiler.
	 */
	enum class ProfiledThread
	{
		Sim,
		Core
	};

	/**
	 * @brief	Provides performance measuring methods for the CPU.
	 */
	class BS_CORE_EXPORT Profiler : public Module<Profiler>
	{
	public:
		Profiler();
		~Profiler();

		/**
		 * @copydoc CPUProfiler::beginThread
		 */
		void beginThread(const ProfilerString& name) 
		{ 
#if BS_PROFILING_ENABLED
			mCPUProfiler->beginThread(name); 
#endif
		}

		/**
		 * @copydoc CPUProfiler::endThread
		 */
		void endThread() 
		{ 
#if BS_PROFILING_ENABLED
			mCPUProfiler->endThread(); 
#endif
		}

		/**
		 * @copydoc CPUProfiler::beginSample
		 */
		void beginSample(const ProfilerString& name) 
		{ 
#if BS_PROFILING_ENABLED
			mCPUProfiler->beginSample(name); 
#endif
		}

		/**
		 * @copydoc CPUProfiler::endSample
		 */
		void endSample(const ProfilerString& name) 
		{ 
#if BS_PROFILING_ENABLED
			mCPUProfiler->endSample(name); 
#endif
		}

		/**
		 * @copydoc CPUProfiler::beginSamplePrecise
		 */
		void beginSamplePrecise(const ProfilerString& name) 
		{ 
#if BS_PROFILING_ENABLED
			mCPUProfiler->beginSamplePrecise(name); 
#endif
		}

		/**
		 * @copydoc CPUProfiler::endSamplePrecise
		 */
		void endSamplePrecise(const ProfilerString& name) 
		{ 
#if BS_PROFILING_ENABLED
			mCPUProfiler->endSamplePrecise(name); 
#endif
		}

		/**
		 * @brief	Called every frame.
		 *
		 * @note	Internal method.
		 */
		void _update();

		/**
		 * @brief	Called every frame from the core thread.
		 * 			
		 * @note	Internal method. Only call from core thread.
		 */
		void _updateCore();

		/**
		 * @brief	Returns a profiler report for the specified frame, for the specified thread.
		 *
		 * @param	Profiler report index, ranging [0, NUM_SAVED_FRAMES]. 0 always returns the latest
		 * 					 report. Increasing indexes return reports for older and older frames. Out of range
		 * 					 indexes will be clamped.
		 *
		 * @note	Profiler reports get updated every frame. Oldest reports that no longer fit in the saved reports buffer
		 * 			are discarded.
		 */
		const ProfilerReport& getReport(ProfiledThread thread, UINT32 idx = 0) const;

	private:
		static const UINT32 NUM_SAVED_FRAMES;
		ProfilerReport* mSavedSimReports;
		UINT32 mNextSimReportIdx;

		ProfilerReport* mSavedCoreReports;
		UINT32 mNextCoreReportIdx;

		CPUProfiler* mCPUProfiler;

		BS_MUTEX(mSync);
	};

	/**
	 * @brief	Quick way to access the profiler.
	 */
	BS_CORE_EXPORT Profiler& gProfiler();
}