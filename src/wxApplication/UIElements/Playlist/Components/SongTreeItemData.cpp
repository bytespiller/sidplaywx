/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021 Jasmin Rutic (bytespiller@gmail.com)
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

#include "SongTreeItemData.h"

namespace UIElements
{
	namespace Playlist
	{
		SongTreeItemData::SongTreeItemData(const wxString& title, const wxString& filepath, int defaultSubsong, uint_least32_t duration, SongItemType songItemType, ItemStatus playability, int iconIndex) :
			_title(title),
			_filepath(filepath),
			_defaultSubsong(defaultSubsong),
			_duration(duration),
			_type(songItemType),
			_canonicalIconIndex(iconIndex),
			_status(playability)
		{
		}

		const wxString& SongTreeItemData::GetFilePath() const
		{
			return _filepath;
		}

		const wxString& SongTreeItemData::GetTitle() const
		{
			return _title;
		}

		int SongTreeItemData::GetDefaultSubsong() const
		{
			return _defaultSubsong;
		}

		uint_least32_t SongTreeItemData::GetDuration() const
		{
			return _duration;
		}

		SongTreeItemData::ItemStatus SongTreeItemData::GetStatus() const
		{
			return _status;
		}

		bool SongTreeItemData::IsSubsong() const
		{
			return _type == SongItemType::Subsong;
		}

		SongTreeItemData::SongItemType SongTreeItemData::GetType() const
		{
			return _type;
		}

		int SongTreeItemData::GetCanonicalIconIndex() const
		{
			return _canonicalIconIndex;
		}

		void SongTreeItemData::ChangeStatus(ItemStatus status)
		{
			_status = status;
		}
	}
}
