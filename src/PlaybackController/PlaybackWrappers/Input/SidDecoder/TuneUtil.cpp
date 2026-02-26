/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include "TuneUtil.h"
#include <sidplayfp/SidTuneInfo.h>

std::string TuneUtil::GetTuneInfoString(const SidTune& tune, SongInfoCategory category)
{
    const SidTuneInfo& info = *tune.getInfo();
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

std::string TuneUtil::GetTuneMusComments(const SidTune& tune)
{
    static constexpr char SEPARATOR[] = "   ***   ";

    std::string retStr;

    const SidTuneInfo& info = *tune.getInfo();
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

TuneUtil::RomRequirement TuneUtil::GetTuneRomRequirement(const SidTune& tune)
{
	switch (tune.getInfo()->compatibility())
    {
        case SidTuneInfo::COMPATIBILITY_BASIC:
            return RomRequirement::BasicRom;

        case SidTuneInfo::COMPATIBILITY_R64:
            return RomRequirement::R64;

        default:
            return RomRequirement::None;
    }
}
