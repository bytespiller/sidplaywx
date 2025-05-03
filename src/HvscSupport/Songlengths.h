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

#pragma once

// This is used instead of the libsidplayfp's SidDatabase which unfortunately segfaults when rapidly reading songlengths of huge amount of tunes from the Zip files (data from buffer).

#include <sidplayfp/SidTune.h>

#include <cstdint>
#include <filesystem>
#include <memory>
#include <string>
#include <unordered_map>

class Songlengths
{
public:
    struct HvscInfo
    {
        // Default constructor for dummy/empty response.
        HvscInfo() = default;

        HvscInfo(uint_least32_t aDuration, const std::string& aHvscPath, const char* aMd5) :
            duration(aDuration),
            hvscPath(aHvscPath),
            md5(aMd5)
        {
        }

        const uint_least32_t duration = 0;
        const std::string hvscPath;
        const char* const md5 = 0;
    };

private:
    struct HvscInfoRaw
    {
        HvscInfoRaw() = delete;
        HvscInfoRaw(const std::string& aDurations, const std::string& aHvscPath) :
            durations(aDurations),
            hvscPath(aHvscPath)
        {
        }

        const std::string durations;
        const std::string hvscPath;
    };

public:
    Songlengths() = default;

public:
    bool TryLoad(const std::filesystem::path& songlengthsMd5Filepath);
    void Unload();

    bool IsLoaded() const;

    static uint_least32_t GetDurationMs(const std::string& preformattedDuration);
    HvscInfo GetHvscInfo(const char* tuneMd5, int subsong = 1) const;

private:
    using MD5 = std::string;
    std::unordered_map<MD5, std::unique_ptr<HvscInfoRaw>> _database;
};
