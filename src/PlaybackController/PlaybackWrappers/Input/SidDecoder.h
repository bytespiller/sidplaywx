/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2024 Jasmin Rutic (bytespiller@gmail.com)
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see https://www.gnu.org/licenses/gpl-3.0.html
 */

#pragma once

#include "../IBufferWriter.h"
#include "../../Util/RomUtil.h"

#include <sidplayfp/SidInfo.h>
#include <sidplayfp/sidplayfp.h>
#include <sidplayfp/SidTune.h>
#include <sidplayfp/builders/residfp.h>

#include <memory>
#include <vector>

class SidDecoder : public IBufferWriter
{
public:
    enum class RomRequirement
    {
        None,
        BasicRom,
        R64
    };

    enum class SongInfoCategory
    {
        Title = 0,
        Author,
        Released // a.k.a. Copyright
    };

    struct FilterConfig
    {
        FilterConfig() = delete;
        FilterConfig(double aFilter6581Curve, double aFilter8580Curve) :
            filter6581Curve(aFilter6581Curve),
            filter8580Curve(aFilter8580Curve)
        {
        }

        const double filter6581Curve;
        const double filter8580Curve;
    };

    using SidVoicesEnabledStatus = std::vector< std::vector<bool> >; // Not using the std::bitset, the code becomes messy, not worth it!
    using SidFiltersEnabledStatus = std::vector<bool>;

public:
    SidDecoder();
    SidDecoder(SidDecoder&) = delete;

public:
    bool TryFillBuffer(void* buffer, unsigned long framesPerBuffer) override;

public:
    // Needed for playback. Can be skipped if intending to just read tunes' info.
    bool TryInitEmulation(const SidConfig& sidConfig, const FilterConfig& filterConfig);

    RomUtil::RomStatus TrySetRoms(const std::wstring& pathKernal, const std::wstring& pathBasic, const std::wstring& pathChargen);

    // Unicode paths not supported for filepath variant, rather use the oneFileFormatSidtune variant and do custom file loading.
    bool TryLoadSong(const char* filepath, unsigned int subsong = 0);
    bool TryLoadSong(const uint_least8_t* oneFileFormatSidtune, uint_least32_t sidtuneLength, unsigned int subsong = 0);

    bool TrySetSubsong(unsigned int subsong);
    void Stop();

    uint_least32_t GetTime() const;
    int GetCurrentSubsong() const;
    int GetDefaultSubsong() const;
    int GetTotalSubsongs() const;

    std::string GetCurrentTuneInfoString(SongInfoCategory category) const;
    const SidTuneInfo& GetCurrentSongInfo() const;
    RomRequirement GetCurrentSongRomRequirement() const;
    int GetCurrentTuneSidChipsRequired() const;

    // Calculates the MD5 hash of the current tune. Returns 0 if no tune is loaded.
    const char* CalcCurrentTuneMd5() const;

    const SidInfo& GetEngineInfo() const;
    const SidVoicesEnabledStatus& GetSidVoicesEnabledStatus() const;
    const SidFiltersEnabledStatus& GetSidFiltersEnabledStatus() const;
    const SidConfig& GetSidConfig() const;
    const FilterConfig& GetFilterConfig() const;

    void SeekTo(uint_least32_t timeMs, const SeekStatusCallback& callback);
    void ToggleVoice(unsigned int sidNum, unsigned int voice, bool enable);
    void ToggleFilter(unsigned int sidNum, bool enable);

    void UnloadActiveTune();

private:
    void PrepareLoadSong();

    // Applies the SidVoicesEnabledStatus and SidFiltersEnabledStatus to SIDs.
    void ApplyCanonicalVoiceAndFilterStates();

private:
    bool _seeking = false;
    SidConfig _sidConfigCache;
    std::unique_ptr<FilterConfig> _filterConfigCache;
    SidVoicesEnabledStatus _sidVoicesEnabledStatus;
    SidFiltersEnabledStatus _sidFiltersEnabledStatus;
    sidplayfp _sidEngine;
    std::unique_ptr<SidTune> _tune;
    ReSIDfpBuilder _rs;
};
