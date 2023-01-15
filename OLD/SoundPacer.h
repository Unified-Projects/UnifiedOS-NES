#include <AL/al.h>
#include <AL/alc.h>

#include <istream>
#include <cstring>
#include <climits>
#include <condition_variable>
#include <algorithm>
#include <queue>
#include <thread>
#include <functional>
#include <atomic>
#include <list>
#undef min
#undef max

namespace UnifiedEmulation
{
    namespace NES{
        #pragma pack(push, 1)
        typedef struct {
            uint16_t wFormatTag;
            uint16_t nChannels;
            uint32_t nSamplesPerSec;
            uint32_t nAvgBytesPerSec;
            uint16_t nBlockAlign;
            uint16_t wBitsPerSample;
            uint16_t cbSize;
        } OLC_WAVEFORMATEX;
        #pragma pack(pop)

        class EmulationSound{
        public:
            class AudioSample
            {
            public:
                AudioSample();

            public:
                OLC_WAVEFORMATEX wavHeader;
                float *fSample = nullptr;
                long nSamples = 0;
                int nChannels = 0;
                bool bSampleValid = false;
            };

            struct sCurrentlyPlayingSample
            {
                int nAudioSampleID = 0;
                long nSamplePosition = 0;
                bool bFinished = false;
                bool bLoop = false;
                bool bFlagForStop = false;
            };

            static std::list<sCurrentlyPlayingSample> listActiveSamples;

        public:
            static bool InitialiseAudio(unsigned int nSampleRate = 44100, unsigned int nChannels = 1, unsigned int nBlocks = 8, unsigned int nBlockSamples = 512);
            static bool DestroyAudio();
            static void SetUserSynthFunction(std::function<float(int, float, float)> func);
            static void SetUserFilterFunction(std::function<float(int, float, float)> func);

        public:
            //static int LoadAudioSample(std::string sWavFile, olc::ResourcePack *pack = nullptr);
            static void PlaySample(int id, bool bLoop = false);
            static void StopSample(int id);
            static void StopAll();
            static float GetMixerOutput(int nChannel, float fGlobalTime, float fTimeStep);

        public:
            static std::queue<ALuint> m_qAvailableBuffers;
            static ALuint *m_pBuffers;
            static ALuint m_nSource;
            static ALCdevice *m_pDevice;
            static ALCcontext *m_pContext;
            static unsigned int m_nSampleRate;
            static unsigned int m_nChannels;
            static unsigned int m_nBlockCount;
            static unsigned int m_nBlockSamples;
            static short* m_pBlockMemory;

            static void AudioThread();
            static std::thread m_AudioThread;
            static std::atomic<bool> m_bAudioThreadActive;
            static std::atomic<float> m_fGlobalTime;
            static std::function<float(int, float, float)> funcUserSynth;
            static std::function<float(int, float, float)> funcUserFilter;
        };

        EmulationSound::AudioSample::AudioSample()
        {	}

        // This vector holds all loaded sound samples in memory
        std::vector<EmulationSound::AudioSample> vecAudioSamples;

        void EmulationSound::SetUserSynthFunction(std::function<float(int, float, float)> func)
        {
            funcUserSynth = func;
        }

        void EmulationSound::SetUserFilterFunction(std::function<float(int, float, float)> func)
        {
            funcUserFilter = func;
        }
        
        // Add sample 'id' to the mixers sounds to play list
        void EmulationSound::PlaySample(int id, bool bLoop)
        {
            EmulationSound::sCurrentlyPlayingSample a;
            a.nAudioSampleID = id;
            a.nSamplePosition = 0;
            a.bFinished = false;
            a.bFlagForStop = false;
            a.bLoop = bLoop;
            EmulationSound::listActiveSamples.push_back(a);
        }

        void EmulationSound::StopSample(int id)
        {
            // Find first occurence of sample id
            auto s = std::find_if(listActiveSamples.begin(), listActiveSamples.end(), [&](const EmulationSound::sCurrentlyPlayingSample &s) { return s.nAudioSampleID == id; });
            if (s != listActiveSamples.end())
                s->bFlagForStop = true;
        }

        void EmulationSound::StopAll()
        {
            for (auto &s : listActiveSamples)
            {
                s.bFlagForStop = true;
            }
        }

        float EmulationSound::GetMixerOutput(int nChannel, float fGlobalTime, float fTimeStep)
        {
            // Accumulate sample for this channel
            float fMixerSample = 0.0f;

            for (auto &s : listActiveSamples)
            {
                if (m_bAudioThreadActive)
                {
                    if (s.bFlagForStop)
                    {
                        s.bLoop = false;
                        s.bFinished = true;
                    }
                    else
                    {
                        // Calculate sample position
                        s.nSamplePosition += roundf((float)vecAudioSamples[s.nAudioSampleID - 1].wavHeader.nSamplesPerSec * fTimeStep);

                        // If sample position is valid add to the mix
                        if (s.nSamplePosition < vecAudioSamples[s.nAudioSampleID - 1].nSamples)
                            fMixerSample += vecAudioSamples[s.nAudioSampleID - 1].fSample[(s.nSamplePosition * vecAudioSamples[s.nAudioSampleID - 1].nChannels) + nChannel];
                        else
                        {
                            if (s.bLoop)
                            {
                                s.nSamplePosition = 0;
                            }
                            else
                                s.bFinished = true; // Else sound has completed
                        }
                    }
                }
                else
                    return 0.0f;
            }

            // If sounds have completed then remove them
            listActiveSamples.remove_if([](const sCurrentlyPlayingSample &s) {return s.bFinished; });

            // The users application might be generating sound, so grab that if it exists
            if (funcUserSynth != nullptr)
                fMixerSample += funcUserSynth(nChannel, fGlobalTime, fTimeStep);

            // Return the sample via an optional user override to filter the sound
            if (funcUserFilter != nullptr)
                return funcUserFilter(nChannel, fGlobalTime, fMixerSample);
            else
                return fMixerSample;
        }

        std::thread EmulationSound::m_AudioThread;
        std::atomic<bool> EmulationSound::m_bAudioThreadActive{ false };
        std::atomic<float> EmulationSound::m_fGlobalTime{ 0.0f };
        std::list<EmulationSound::sCurrentlyPlayingSample> EmulationSound::listActiveSamples;
        std::function<float(int, float, float)> EmulationSound::funcUserSynth = nullptr;
        std::function<float(int, float, float)> EmulationSound::funcUserFilter = nullptr;

        bool EmulationSound::InitialiseAudio(unsigned int nSampleRate, unsigned int nChannels, unsigned int nBlocks, unsigned int nBlockSamples)
        {
            // Initialise Sound Engine
            m_bAudioThreadActive = false;
            m_nSampleRate = nSampleRate;
            m_nChannels = nChannels;
            m_nBlockCount = nBlocks;
            m_nBlockSamples = nBlockSamples;
            m_pBlockMemory = nullptr;

            // Open the device and create the context
            m_pDevice = alcOpenDevice(NULL);
            if (m_pDevice)
            {
                m_pContext = alcCreateContext(m_pDevice, NULL);
                alcMakeContextCurrent(m_pContext);
            }
            else
                return DestroyAudio();

            // Allocate memory for sound data
            alGetError();
            m_pBuffers = new ALuint[m_nBlockCount];
            alGenBuffers(m_nBlockCount, m_pBuffers);
            alGenSources(1, &m_nSource);

            for (unsigned int i = 0; i < m_nBlockCount; i++)
                m_qAvailableBuffers.push(m_pBuffers[i]);

            listActiveSamples.clear();

            // Allocate Wave|Block Memory
            m_pBlockMemory = new short[m_nBlockSamples];
            if (m_pBlockMemory == nullptr)
                return DestroyAudio();
            std::fill(m_pBlockMemory, m_pBlockMemory + m_nBlockSamples, 0);

            m_bAudioThreadActive = true;
            m_AudioThread = std::thread(&EmulationSound::AudioThread);
            return true;
        }

        // Stop and clean up audio system
        bool EmulationSound::DestroyAudio()
        {
            m_bAudioThreadActive = false;
            if(m_AudioThread.joinable())
                m_AudioThread.join();

            alDeleteBuffers(m_nBlockCount, m_pBuffers);
            delete[] m_pBuffers;
            alDeleteSources(1, &m_nSource);

            alcMakeContextCurrent(NULL);
            alcDestroyContext(m_pContext);
            alcCloseDevice(m_pDevice);
            return false;
        }

        void EmulationSound::AudioThread()
        {
            m_fGlobalTime = 0.0f;
            static float fTimeStep = 1.0f / (float)m_nSampleRate;

            // Goofy hack to get maximum integer for a type at run-time
            short nMaxSample = (short)pow(2, (sizeof(short) * 8) - 1) - 1;
            float fMaxSample = (float)nMaxSample;
            short nPreviousSample = 0;

            std::vector<ALuint> vProcessed;

            while (m_bAudioThreadActive)
            {
                ALint nState, nProcessed;
                alGetSourcei(m_nSource, AL_SOURCE_STATE, &nState);
                alGetSourcei(m_nSource, AL_BUFFERS_PROCESSED, &nProcessed);

                // Add processed buffers to our queue
                vProcessed.resize(nProcessed);
                alSourceUnqueueBuffers(m_nSource, nProcessed, vProcessed.data());
                for (ALint nBuf : vProcessed) m_qAvailableBuffers.push(nBuf);

                // Wait until there is a free buffer (ewww)
                if (m_qAvailableBuffers.empty()) continue;

                short nNewSample = 0;

                auto clip = [](float fSample, float fMax)
                {
                    if (fSample >= 0.0)
                        return fmin(fSample, fMax);
                    else
                        return fmax(fSample, -fMax);
                };

                for (unsigned int n = 0; n < m_nBlockSamples; n += m_nChannels)
                {
                    // User Process
                    for (unsigned int c = 0; c < m_nChannels; c++)
                    {
                        nNewSample = (short)(clip(GetMixerOutput(c, m_fGlobalTime, fTimeStep), 1.0) * fMaxSample);
                        m_pBlockMemory[n + c] = nNewSample;
                        nPreviousSample = nNewSample;
                    }

                    m_fGlobalTime = m_fGlobalTime + fTimeStep;
                }

                // Fill OpenAL data buffer
                alBufferData(
                    m_qAvailableBuffers.front(),
                    m_nChannels == 1 ? AL_FORMAT_MONO16 : AL_FORMAT_STEREO16,
                    m_pBlockMemory,
                    2 * m_nBlockSamples,
                    m_nSampleRate
                );
                // Add it to the OpenAL queue
                alSourceQueueBuffers(m_nSource, 1, &m_qAvailableBuffers.front());
                // Remove it from ours
                m_qAvailableBuffers.pop();

                // If it's not playing for some reason, change that
                if (nState != AL_PLAYING)
                    alSourcePlay(m_nSource);
            }
        }

        std::queue<ALuint> EmulationSound::m_qAvailableBuffers;
        ALuint *EmulationSound::m_pBuffers = nullptr;
        ALuint EmulationSound::m_nSource = 0;
        ALCdevice *EmulationSound::m_pDevice = nullptr;
        ALCcontext *EmulationSound::m_pContext = nullptr;
        unsigned int EmulationSound::m_nSampleRate = 0;
        unsigned int EmulationSound::m_nChannels = 0;
        unsigned int EmulationSound::m_nBlockCount = 0;
        unsigned int EmulationSound::m_nBlockSamples = 0;
        short* EmulationSound::m_pBlockMemory = nullptr;
    }
}
