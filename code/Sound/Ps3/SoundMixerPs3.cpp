#include <cmath>
#include "Core/Log/Log.h"
#include "Core/Memory/Alloc.h"
#include "Core/Misc/Align.h"
#include "Core/Misc/SafeDestroy.h"
#include "Core/Thread/Ps3/Spurs/SpursJobQueue.h"
#include "Core/Thread/Ps3/Spurs/SpursManager.h"
#include "Sound/SoundMixer.h"
#include "Sound/Ps3/SoundMixerPs3.h"
#include "Sound/Ps3/Spu/JobMC.h"

#if !defined(_DEBUG)

extern char _binary_jqjob_Traktor_Sound_JobMulConst_bin_start[];
extern char _binary_jqjob_Traktor_Sound_JobMulConst_bin_size[];
extern char _binary_jqjob_Traktor_Sound_JobAddMulConst_bin_start[];
extern char _binary_jqjob_Traktor_Sound_JobAddMulConst_bin_size[];
extern char _binary_jqjob_Traktor_Sound_JobStretch_bin_start[];
extern char _binary_jqjob_Traktor_Sound_JobStretch_bin_size[];

static char* job_mc_start = _binary_jqjob_Traktor_Sound_JobMulConst_bin_start;
static char* job_mc_size = _binary_jqjob_Traktor_Sound_JobMulConst_bin_size;
static char* job_amc_start = _binary_jqjob_Traktor_Sound_JobAddMulConst_bin_start;
static char* job_amc_size = _binary_jqjob_Traktor_Sound_JobAddMulConst_bin_size;
static char* job_stretch_start = _binary_jqjob_Traktor_Sound_JobStretch_bin_start;
static char* job_stretch_size = _binary_jqjob_Traktor_Sound_JobStretch_bin_size;

#else

extern char _binary_jqjob_Traktor_Sound_JobMulConst_d_bin_start[];
extern char _binary_jqjob_Traktor_Sound_JobMulConst_d_bin_size[];
extern char _binary_jqjob_Traktor_Sound_JobAddMulConst_d_bin_start[];
extern char _binary_jqjob_Traktor_Sound_JobAddMulConst_d_bin_size[];
extern char _binary_jqjob_Traktor_Sound_JobStretch_d_bin_start[];
extern char _binary_jqjob_Traktor_Sound_JobStretch_d_bin_size[];

static char* job_mc_start = _binary_jqjob_Traktor_Sound_JobMulConst_d_bin_start;
static char* job_mc_size = _binary_jqjob_Traktor_Sound_JobMulConst_d_bin_size;
static char* job_amc_start = _binary_jqjob_Traktor_Sound_JobAddMulConst_d_bin_start;
static char* job_amc_size = _binary_jqjob_Traktor_Sound_JobAddMulConst_d_bin_size;
static char* job_stretch_start = _binary_jqjob_Traktor_Sound_JobStretch_d_bin_start;
static char* job_stretch_size = _binary_jqjob_Traktor_Sound_JobStretch_d_bin_size;

#endif

#define T_USE_PPU_MIXER	0
#define T_SPU_SYNCHRONIZED 0

namespace traktor
{
	namespace sound
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.sound.SoundMixerPs3", SoundMixerPs3, ISoundMixer)

bool SoundMixerPs3::create()
{
	T_FATAL_ASSERT_M(sizeof(JobMC) == 128, L"Incorrect size of job descriptor; must be 128 bytes");
	m_jobQueue = SpursManager::getInstance().createJobQueue(sizeof(JobMC), 256);
	m_mixer = new SoundMixer();
	return true;
}

void SoundMixerPs3::destroy()
{
	safeDestroy(m_jobQueue);
}

void SoundMixerPs3::mulConst(float* sb, uint32_t count, float factor) const
{
#if !T_USE_PPU_MIXER

	JobMC job;

	__builtin_memset(&job, 0, sizeof(JobMC));
	job.header.eaBinary = (uintptr_t)job_mc_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_mc_size);
	job.mixer.lsbEA = (uintptr_t)sb;
	job.mixer.rsbEA = (uintptr_t)sb;
	job.mixer.count = count;
	job.mixer.factor = factor;
	if (!m_jobQueue->push(&job))
	{
		log::debug << L"Mixer failed; unable to queue SPU job" << Endl;
		return;
	}

#	if T_SPU_SYNCHRONIZED
	m_jobQueue->wait();
#	endif
#else
	m_mixer->mulConst(sb, count, factor);
#endif
}

void SoundMixerPs3::mulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
#if !T_USE_PPU_MIXER

	JobMC job;

	__builtin_memset(&job, 0, sizeof(JobMC));
	job.header.eaBinary = (uintptr_t)job_mc_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_mc_size);
	job.mixer.lsbEA = (uintptr_t)lsb;
	job.mixer.rsbEA = (uintptr_t)rsb;
	job.mixer.count = count;
	job.mixer.factor = factor;
	if (!m_jobQueue->push(&job))
	{
		log::debug << L"Mixer failed; unable to queue SPU job" << Endl;
		return;
	}

#	if T_SPU_SYNCHRONIZED
	m_jobQueue->wait();
#	endif
#else
	m_mixer->mulConst(lsb, rsb, count, factor);
#endif
}

void SoundMixerPs3::addMulConst(float* lsb, const float* rsb, uint32_t count, float factor) const
{
#if !T_USE_PPU_MIXER

	JobMC job;

	__builtin_memset(&job, 0, sizeof(JobMC));
	job.header.eaBinary = (uintptr_t)job_amc_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_amc_size);
	job.mixer.lsbEA = (uintptr_t)lsb;
	job.mixer.rsbEA = (uintptr_t)rsb;
	job.mixer.count = count;
	job.mixer.factor = factor;
	if (!m_jobQueue->push(&job))
	{
		log::debug << L"Mixer failed; unable to queue SPU job" << Endl;
		return;
	}

#	if T_SPU_SYNCHRONIZED
	m_jobQueue->wait();
#	endif
#else
	m_mixer->addMulConst(lsb, rsb, count, factor);
#endif
}

void SoundMixerPs3::stretch(float* lsb, uint32_t lcount, const float* rsb, uint32_t rcount, float factor) const
{
#if !T_USE_PPU_MIXER

	JobMC job;

	__builtin_memset(&job, 0, sizeof(JobMC));
	job.header.eaBinary = (uintptr_t)job_stretch_start;
	job.header.sizeBinary = CELL_SPURS_GET_SIZE_BINARY(job_stretch_size);

	job.mixer.lsbEA = (uintptr_t)lsb;
	job.mixer.rsbEA = (uintptr_t)rsb;
	job.mixer.count = lcount;
	job.mixer.rcount = rcount;
	job.mixer.factor = factor;
	if (!m_jobQueue->push(&job))
	{
		log::debug << L"Mixer failed; unable to queue SPU job" << Endl;
		return;
	}

#	if T_SPU_SYNCHRONIZED
	m_jobQueue->wait();
#	endif
#else
	m_mixer->stretch(lsb, lcount, rsb, rcount, factor);
#endif
}

void SoundMixerPs3::mute(float* sb, uint32_t count) const
{
	m_mixer->mute(sb, count);
}

void SoundMixerPs3::synchronize() const
{
#if !T_USE_PPU_MIXER
#	if !T_SPU_SYNCHRONIZED
	m_jobQueue->wait();
#	endif
#else
	m_mixer->synchronize();
#endif
}

	}
}
