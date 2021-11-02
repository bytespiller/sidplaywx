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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/treectrl.h>

namespace UIElements
{
	namespace Playlist
	{
		class SongTreeItemData : public wxTreeItemData
		{
		public:
			enum class ItemStatus
			{
				Normal,
				Unplayable,
				IgnoredPlayable
			};

			enum class SongItemType
			{
				Song,
				Subsong
			};

		public:
			SongTreeItemData() = delete;
			SongTreeItemData(const wxString& title, const wxString& filepath, int defaultSubsong, uint_least32_t duration, SongItemType songItemType, ItemStatus playability, int iconIndex);

		public:
			const wxString& GetFilePath() const;
			const wxString& GetTitle() const;
			int GetDefaultSubsong() const; // Returns for; Multi-tune: default subsong. Subsong: its own index. Single-tune: 1.
			uint_least32_t GetDuration() const;
			ItemStatus GetStatus() const;
			bool IsSubsong() const;
			SongItemType GetType() const;
			int GetCanonicalIconIndex() const;

			void ChangeStatus(ItemStatus status);

		private:
			const wxString _title;
			const wxString _filepath;
			const int _defaultSubsong;
			const uint_least32_t _duration;
			const SongItemType _type;
			const int _canonicalIconIndex;
			ItemStatus _status;
		};
	}
}
