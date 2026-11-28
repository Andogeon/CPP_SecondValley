#include "pch.h"
#include "Sound.h"
#include <mmsystem.h>
#include <cmath>
#include <cstring>
#include <fstream>
#include <mutex>
#include <vector>

#pragma comment(lib, "winmm.lib")

struct Sound::Implementation
{
    mutable std::mutex __Mutex;
    std::vector<char> __Data;
    WAVEFORMATEX __Format = {};
    HWAVEOUT __Device = nullptr;
    WAVEHDR __Header = {};
    float __Volume = 1.0f;

    void Stop()
    {
        if (!__Device) return;
        waveOutReset(__Device);
        if (__Header.dwFlags & WHDR_PREPARED)
            waveOutUnprepareHeader(__Device, &__Header, sizeof(__Header));
        waveOutClose(__Device);
        __Device = nullptr;
        __Header = {};
    }

    ~Implementation() { Stop(); }
};

Sound::Sound() : __Implementation(std::make_unique<Implementation>()) {}
Sound::~Sound() = default;

bool Sound::LoadResource(const std::filesystem::path& _FilePath, FString& _Error)
{
    _Error = TEXT("Expected a valid PCM WAV file (8/16 bit, mono/stereo, at most 64 MiB).");
    std::ifstream Input(_FilePath, std::ios::binary | std::ios::ate);
    if (!Input) return false;
    const auto Size = Input.tellg();
    if (Size < 12 || Size > 64 * 1024 * 1024) return false;
    std::vector<unsigned char> File(static_cast<size_t>(Size));
    Input.seekg(0);
    if (!Input.read(reinterpret_cast<char*>(File.data()), static_cast<std::streamsize>(File.size()))) return false;
    const auto Read16 = [&](size_t _Offset) -> unsigned int
    {
        return File[_Offset] | (static_cast<unsigned int>(File[_Offset + 1]) << 8);
    };
    const auto Read32 = [&](size_t _Offset) -> unsigned int
    {
        return Read16(_Offset) | (Read16(_Offset + 2) << 16);
    };
    if (std::memcmp(File.data(), "RIFF", 4) || std::memcmp(File.data() + 8, "WAVE", 4)) return false;
    const size_t End = static_cast<size_t>(Read32(4)) + 8;
    if (End != File.size()) return false;
    WAVEFORMATEX Format = {};
    std::vector<char> Data;
    bool HasFormat = false;
    bool HasData = false;
    size_t Offset = 12;
    while (Offset < End)
    {
        if (End - Offset < 8) return false;
        const size_t ChunkSize = Read32(Offset + 4);
        const size_t Start = Offset + 8;
        if (ChunkSize > End - Start) return false;
        if (!std::memcmp(File.data() + Offset, "fmt ", 4))
        {
            if (HasFormat || ChunkSize < 16) return false;
            HasFormat = true;
            Format.wFormatTag = static_cast<WORD>(Read16(Start));
            Format.nChannels = static_cast<WORD>(Read16(Start + 2));
            Format.nSamplesPerSec = Read32(Start + 4);
            Format.nAvgBytesPerSec = Read32(Start + 8);
            Format.nBlockAlign = static_cast<WORD>(Read16(Start + 12));
            Format.wBitsPerSample = static_cast<WORD>(Read16(Start + 14));
        }
        else if (!std::memcmp(File.data() + Offset, "data", 4))
        {
            if (HasData || !ChunkSize) return false;
            HasData = true;
            Data.assign(File.begin() + Start, File.begin() + Start + ChunkSize);
        }
        Offset = Start + ChunkSize + (ChunkSize & 1);
        if (Offset > End) return false;
    }
    if (!HasFormat || !HasData || Format.wFormatTag != WAVE_FORMAT_PCM ||
        (Format.nChannels != 1 && Format.nChannels != 2) ||
        (Format.wBitsPerSample != 8 && Format.wBitsPerSample != 16) ||
        Format.nSamplesPerSec < 8000 || Format.nSamplesPerSec > 192000 ||
        Format.nBlockAlign != Format.nChannels * Format.wBitsPerSample / 8 ||
        Format.nAvgBytesPerSec != Format.nSamplesPerSec * Format.nBlockAlign ||
        Data.size() % Format.nBlockAlign != 0) return false;
    std::lock_guard<std::mutex> Lock(__Implementation->__Mutex);
    __Implementation->Stop();
    __Implementation->__Data = std::move(Data);
    __Implementation->__Format = Format;
    _Error.Empty();
    return true;
}

bool Sound::Play(bool _Loop)
{
    std::lock_guard<std::mutex> Lock(__Implementation->__Mutex);
    auto& State = *__Implementation;
    if (State.__Data.empty()) return false;
    State.Stop();
    if (waveOutOpen(&State.__Device, WAVE_MAPPER, &State.__Format, 0, 0, CALLBACK_NULL) != MMSYSERR_NOERROR)
    {
        State.__Device = nullptr;
        return false;
    }
    const DWORD Volume = static_cast<DWORD>(State.__Volume * 65535.0f);
    waveOutSetVolume(State.__Device, Volume | (Volume << 16));
    State.__Header.lpData = State.__Data.data();
    State.__Header.dwBufferLength = static_cast<DWORD>(State.__Data.size());
    if (_Loop)
    {
        State.__Header.dwFlags = WHDR_BEGINLOOP | WHDR_ENDLOOP;
        State.__Header.dwLoops = MAXDWORD;
    }
    if (waveOutPrepareHeader(State.__Device, &State.__Header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR ||
        waveOutWrite(State.__Device, &State.__Header, sizeof(WAVEHDR)) != MMSYSERR_NOERROR)
    {
        State.Stop();
        return false;
    }
    return true;
}

void Sound::Stop()
{
    std::lock_guard<std::mutex> Lock(__Implementation->__Mutex);
    __Implementation->Stop();
}

bool Sound::IsPlaying() const
{
    std::lock_guard<std::mutex> Lock(__Implementation->__Mutex);
    return __Implementation->__Device && (__Implementation->__Header.dwFlags & WHDR_INQUEUE) != 0;
}

bool Sound::SetVolume(float _Volume)
{
    if (!std::isfinite(_Volume) || _Volume < 0.0f || _Volume > 1.0f) return false;
    std::lock_guard<std::mutex> Lock(__Implementation->__Mutex);
    const DWORD Volume = static_cast<DWORD>(_Volume * 65535.0f);
    if (__Implementation->__Device &&
        waveOutSetVolume(__Implementation->__Device, Volume | (Volume << 16)) != MMSYSERR_NOERROR) return false;
    __Implementation->__Volume = _Volume;
    return true;
}

unsigned int Sound::GetSampleRate() const { return __Implementation->__Format.nSamplesPerSec; }
unsigned int Sound::GetChannelCount() const { return __Implementation->__Format.nChannels; }
