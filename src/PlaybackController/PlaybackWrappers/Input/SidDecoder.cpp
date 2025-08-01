/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "SidDecoder.h"

#include <sidplayfp/SidTuneInfo.h>
#include <cmath>
#include <fstream>
#include <iostream>

namespace
{
    // Load ROM dump from file. Allocate the buffer if file exists, otherwise return 0.
    char* loadRom(const std::filesystem::path& path, size_t romSize)
    {
        char* buffer = 0;
        std::ifstream is(path, std::ios::binary);
        if (is.good())
        {
            buffer = new char[romSize];
            is.read(buffer, romSize);
        }
        is.close();
        return buffer;
    }
}

SidDecoder::SidDecoder() :
    _rs(ReSIDfpBuilder(""))
{
    // Create SID emulators
    _rs.create(_sidEngine.info().maxsids());
}

bool SidDecoder::TryFillBuffer(void* buffer, unsigned long framesPerBuffer)
{
    const uint_least32_t length = (framesPerBuffer * _sidConfigCache.playback);
    short* const out = static_cast<short*>(buffer);
    const uint_least32_t ret = _sidEngine.play(out, length);

    if (ret < length)
    {
        std::cerr << "SID engine error!" << std::endl;
        return false;
    }

    return true;
}

bool SidDecoder::TryInitEmulation(const SidConfig& sidConfig, const FilterConfig& filterConfig)
{
    // Check if builder is ok
    if (!_rs.getStatus())
    {
        std::cerr << _rs.error() << std::endl;
        return false;
    }

    // Configure the engine
    _sidConfigCache = sidConfig;
    _sidConfigCache.sidEmulation = &_rs;

    if (!_sidEngine.config(_sidConfigCache))
    {
        std::cerr << _sidEngine.error() << std::endl;
        return false;
    }

    _filterConfigCache = std::make_unique<FilterConfig>(filterConfig);

    _rs.filter6581Curve(_filterConfigCache->filter6581Curve);
    _rs.filter8580Curve(_filterConfigCache->filter8580Curve);

    // Reset the voices enabled status (fourth "voice" is digi samples, added in libsidplayfp v2.10.0)
    _sidVoicesEnabledStatus =
    {
        {true, true, true, true},
        {true, true, true, true},
        {true, true, true, true}
    };

    // Reset the filter enabled status
    _sidFiltersEnabledStatus = {true, true, true};

    ApplyCanonicalVoiceAndFilterStates();

    return true;
}

RomUtil::RomStatus SidDecoder::TrySetRoms(const std::filesystem::path& pathKernal, const std::filesystem::path& pathBasic, const std::filesystem::path& pathChargen)
{
    char* kernal = loadRom(pathKernal, RomUtil::ROM_SIZE_KERNAL);
    char* basic = loadRom(pathBasic, RomUtil::ROM_SIZE_BASIC);
    char* chargen = loadRom(pathChargen, RomUtil::ROM_SIZE_CHARGEN);

    RomUtil::RomStatus status;
    status.Mark(RomUtil::RomType::Kernal, kernal != 0);
    status.Mark(RomUtil::RomType::Basic, basic != 0);
    status.Mark(RomUtil::RomType::Chargen, chargen != 0);

    _sidEngine.setRoms(
        reinterpret_cast<const uint8_t*>(kernal),
        reinterpret_cast<const uint8_t*>(basic),
        reinterpret_cast<const uint8_t*>(chargen)
    );

    delete[] kernal;
    delete[] basic;
    delete[] chargen;

    return status;
}

void SidDecoder::PrepareLoadSong()
{
    UnloadActiveTune();
}

bool SidDecoder::TryLoadSong(const std::filesystem::path& filepath, unsigned int subsong)
{
    PrepareLoadSong();

    // Load new tune from file
    _tune = std::make_unique<SidTune>(filepath.generic_string().c_str());

    // Do rest
    return TrySetSubsong(subsong);
}

bool SidDecoder::TryLoadSong(const uint_least8_t* oneFileFormatSidtune, uint_least32_t sidtuneLength, unsigned int subsong)
{
    PrepareLoadSong();

    // Load new tune from buffer
    _tune = std::make_unique<SidTune>(oneFileFormatSidtune, sidtuneLength);

    // Do rest
    return TrySetSubsong(subsong);
}

bool SidDecoder::TrySetSubsong(unsigned int subsong)
{
    // Check if the tune is valid
    if (!_tune->getStatus())
    {
        std::cerr << _tune->statusString() << std::endl;
        return false;
    }

    // Select song
    _tune->selectSong(subsong);

    // Load tune into engine
    if (!_sidEngine.load(_tune.get()))
    {
        std::cerr << _sidEngine.error() << std::endl;
        return false;
    }

    return true;
}

void SidDecoder::Stop()
{
    _sidEngine.stop();
}

uint_least32_t SidDecoder::GetTime() const
{
    return _sidEngine.timeMs();
}

int SidDecoder::GetCurrentSubsong() const
{
    return _tune->getInfo()->currentSong();
}

int SidDecoder::GetDefaultSubsong() const
{
    return _tune->getInfo()->startSong();
}

int SidDecoder::GetTotalSubsongs() const
{
    return _tune->getInfo()->songs();
}

static void trimString(std::string& str)
{
    str.erase(str.find_last_not_of(' ') + 1); // ltrim
    str.erase(0, str.find_first_not_of(' ')); // rtrim
}

std::string SidDecoder::GetCurrentTuneInfoString(SongInfoCategory category) const
{
    const SidTuneInfo& info = *_tune->getInfo();
    const unsigned int index = static_cast<unsigned int>(category);

    std::string retStr(info.infoString(index));
    trimString(retStr);

    // Try get similar MUS fields in case this is a MUS file
    if (category != SongInfoCategory::Title && retStr.empty()) [[unlikely]]
    {
        retStr = info.commentString(index); // Any index overflow is handled by the lib already (returns an empty string).
        trimString(retStr);
    }

    return retStr;
}

std::string SidDecoder::GetCurrentTuneMusComments() const
{
    static constexpr char SEPARATOR[] = "   ***   ";

    std::string retStr;

    const SidTuneInfo& info = *_tune->getInfo();
    const unsigned int count = info.numberOfCommentStrings();

    for (unsigned int i = 0; i < count; ++i)
    {
        std::string curr(info.commentString(i));
        trimString(curr);

        if (!curr.empty())
        {
            retStr.append(curr + ((i + 1 < count) ? SEPARATOR : ""));
        }
    }

    return retStr;
}

const SidTuneInfo& SidDecoder::GetCurrentSongInfo() const
{
    return *_tune->getInfo();
}

SidDecoder::RomRequirement SidDecoder::GetCurrentSongRomRequirement() const
{
    switch (_tune->getInfo()->compatibility())
    {
        case SidTuneInfo::COMPATIBILITY_BASIC:
            return RomRequirement::BasicRom;

        case SidTuneInfo::COMPATIBILITY_R64:
            return RomRequirement::R64;

        default:
            return RomRequirement::None;
    }
}

int SidDecoder::GetCurrentTuneSidChipsRequired() const
{
    if (_tune != nullptr)
    {
        return _tune->getInfo()->sidChips();
    }

    return 0;
}

const char* SidDecoder::CalcCurrentTuneMd5() const
{
    if (_tune == nullptr)
    {
        return 0;
    }

    return _tune->createMD5New();
}

const SidInfo& SidDecoder::GetEngineInfo() const
{
    return _sidEngine.info();
}

const SidDecoder::SidVoicesEnabledStatus& SidDecoder::GetSidVoicesEnabledStatus() const
{
    return _sidVoicesEnabledStatus;
}

const SidDecoder::SidFiltersEnabledStatus& SidDecoder::GetSidFiltersEnabledStatus() const
{
    return _sidFiltersEnabledStatus;
}

const SidConfig& SidDecoder::GetSidConfig() const
{
    return _sidEngine.config();
}

const SidDecoder::FilterConfig& SidDecoder::GetFilterConfig() const
{
    return *_filterConfigCache;
}

void SidDecoder::SeekTo(uint_least32_t timeMs, const SeekStatusCallback& callback)
{
    _seeking = true;

    uint_least32_t cTimeMs = _sidEngine.timeMs();
    if (cTimeMs >= timeMs)
    {
        _sidEngine.stop();
        cTimeMs = 0;
    }

    // Disable voices and filters of all SIDs -- yields additional ~4x speedup when seeking
    const unsigned int maxSids = _sidEngine.info().maxsids();
    for (unsigned int sid = 0; sid < maxSids; ++sid)
    {
        _sidEngine.mute(sid, 0, true); // Voice 1
        _sidEngine.mute(sid, 1, true); // Voice 2
        _sidEngine.mute(sid, 2, true); // Voice 3
        _sidEngine.mute(sid, 3, true); // Digi
        _sidEngine.filter(sid, false);
    }

    // Seeking: decode until target timeMs
    bool aborted = false;
    while (cTimeMs < timeMs)
    {
        _sidEngine.play(nullptr, 0);

        if (callback(cTimeMs, false))
        {
            aborted = true;
            break;
        }

        cTimeMs = _sidEngine.timeMs();
    }

    // Restore explicitly disabled voices back to their canonical state
    ApplyCanonicalVoiceAndFilterStates();

    // Seeking done
    _seeking = false;
    if (!aborted)
    {
        callback(cTimeMs, true);
    }
}

void SidDecoder::ToggleVoice(unsigned int sidNum, unsigned int voice, bool enable)
{
    // Remember as canonical state
    _sidVoicesEnabledStatus.at(sidNum).at(voice) = enable;

    // Apply immediately, unless seeking (the seek operation will do it when finished)
    if (!_seeking)
    {
        _sidEngine.mute(sidNum, voice, !enable); // Reminder: inverted, makes sense due to a "mute" verb.
    }
}

void SidDecoder::ToggleFilter(unsigned int sidNum, bool enable)
{
    // Remember as canonical state
    _sidFiltersEnabledStatus.at(sidNum) = enable;

    // Apply immediately, unless seeking (the seek operation will do it when finished)
    if (!_seeking)
    {
        _sidEngine.filter(sidNum, enable);
    }
}

void SidDecoder::UnloadActiveTune()
{
    if (_tune != nullptr)
    {
        _sidEngine.stop();
        _sidEngine.load(0);
        _tune = nullptr;
    }
}

void SidDecoder::ApplyCanonicalVoiceAndFilterStates()
{
    const unsigned int maxSids = _sidEngine.info().maxsids();
    for (unsigned int sid = 0; sid < maxSids; ++sid)
    {
        _sidEngine.mute(sid, 0, !_sidVoicesEnabledStatus.at(sid).at(0)); // Voice 1
        _sidEngine.mute(sid, 1, !_sidVoicesEnabledStatus.at(sid).at(1)); // Voice 2
        _sidEngine.mute(sid, 2, !_sidVoicesEnabledStatus.at(sid).at(2)); // Voice 3
        _sidEngine.mute(sid, 3, !_sidVoicesEnabledStatus.at(sid).at(3)); // Digi
        _sidEngine.filter(sid, _sidFiltersEnabledStatus.at(sid));
    }
}
