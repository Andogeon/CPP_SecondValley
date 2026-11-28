#pragma once

#include "AssetResource.h"
#include <memory>

class ENGINE_API Sound final : public AssetResource
{
public:
    Sound();
    ~Sound() override;
    bool Play(bool _Loop = false);
    void Stop();
    bool IsPlaying() const;
    bool SetVolume(float _Volume);
    unsigned int GetSampleRate() const;
    unsigned int GetChannelCount() const;

protected:
    bool LoadResource(const std::filesystem::path& _FilePath, FString& _Error) override;

private:
    struct Implementation;
    std::unique_ptr<Implementation> __Implementation;
};
