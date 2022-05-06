/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2022 Jasmin Rutic (bytespiller@gmail.com)
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

#include "../Settings/SettingsBase.h"
#include "../UIElements/CompositeSeekbar.h"
#include "../UIElements/RepeatModeButton.h"

namespace Settings
{
	class AppSettings : public SettingsBase
	{
	public:
		struct ID
		{
			// Prefs
			static constexpr const char* const AudioOutputDevice = "AudioOutputDevice";
			static constexpr const char* const LowLatency = "LowLatency";
			static constexpr const char* const ForceMono = "ForceMono";

			static constexpr const char* const AutoPlay = "AutoPlay";
			static constexpr const char* const SongFallbackDuration = "SongFallbackDuration";
			static constexpr const char* const SkipShorter = "SkipShorter";
			static constexpr const char* const PopSilencer = "PopSilencer";
			static constexpr const char* const DragDropMode = "DragDropMode";

			static constexpr const char* const RepeatMode = "RepeatMode";
			static constexpr const char* const RepeatModeIncludeSubsongs = "RepeatModeIncludeSubsongs";
			static constexpr const char* const RepeatModeDefaultSubsong = "RepeatModeDefaultSubsong";

			static constexpr const char* const SelectionFollowsPlayback = "SelectionFollowsPlayback";
			static constexpr const char* const AutoExpandSubsongs = "AutoExpandSubsongs";
			static constexpr const char* const TaskbarProgress = "TaskbarProgress";

			static constexpr const char* const SonglengthsPath = "SonglengthsPath";
			static constexpr const char* const SonglengthsTrim = "SonglengthsTrim";

			static constexpr const char* const DefaultC64Model = "DefaultC64Model";
			static constexpr const char* const DefaultSidModel = "DefaultSidModel";

			static constexpr const char* const FilterEnabled = "FilterEnabled";
			static constexpr const char* const FilterCurve6581 = "FilterCurve6581";
			static constexpr const char* const FilterCurve8580 = "FilterCurve8580";

			static constexpr const char* const DigiBoost = "DigiBoost";

			static constexpr const char* const RomKernalPath = "RomKernalPath";
			static constexpr const char* const RomBasicPath = "RomBasicPath";
			static constexpr const char* const RomChargenPath = "RomChargenPath";

			static constexpr const char* const MediaKeys = "MediaKeys";
			static constexpr const char* const SingleInstance = "SingleInstance";

			// Internal
			static constexpr const char* const Volume = "Volume";
			static constexpr const char* const VolumeControlEnabled = "VolumeControlEnabled";
			static constexpr const char* const RestoreDefaults = "RestoreDefaults";
		};

		enum class DragDropMode
		{
			Dual = 0,
			Replace,
			Enqueue,
			Disabled
		};

		enum class DefaultC64Model
		{
			Prefer_PAL = 0,
			Prefer_NTSC,
			Prefer_Old_NTSC,
			Prefer_Drean,
			Prefer_PAL_M,
			Force_PAL,
			Force_NTSC,
			Force_Old_NTSC,
			Force_Drean,
			Force_PAL_M
		};

		enum class DefaultSidModel
		{
			Prefer_MOS6581 = 0,
			Prefer_MOS8580,
			Force_MOS6581,
			Force_MOS8580
		};

	public:
		AppSettings() :
			SettingsBase("settings.xml")
		{
		}

	public:
		std::vector<Option> GetDefaultSettings() const override
		{
			return
			{
				// Prefs
				Option(ID::AudioOutputDevice, ""),
				Option(ID::LowLatency, true),
				Option(ID::ForceMono, false),

				Option(ID::AutoPlay, true),
				Option(ID::RepeatMode, static_cast<int>(UIElements::RepeatModeButton::RepeatMode::Normal)),
				Option(ID::RepeatModeIncludeSubsongs, false),
				Option(ID::RepeatModeDefaultSubsong, true),
				Option(ID::SongFallbackDuration, 180),
				Option(ID::SkipShorter, 0),
				Option(ID::PopSilencer, 100),
				Option(ID::DragDropMode, static_cast<int>(DragDropMode::Dual)),

				Option(ID::SelectionFollowsPlayback, true),
				Option(ID::AutoExpandSubsongs, true),
				Option(ID::TaskbarProgress, static_cast<int>(UIElements::CompositeSeekBar::TaskbarProgressOption::Enabled)),

				Option(ID::SonglengthsPath, ""),
				Option(ID::SonglengthsTrim, 0),

				Option(ID::DefaultC64Model, static_cast<int>(DefaultC64Model::Prefer_PAL)),
				Option(ID::DefaultSidModel, static_cast<int>(DefaultSidModel::Prefer_MOS6581)),

				Option(ID::FilterEnabled, true),
				Option(ID::FilterCurve6581, 0.5),
				Option(ID::FilterCurve8580, 0.5),

				Option(ID::DigiBoost, false),

				Option(ID::RomKernalPath, ""),
				Option(ID::RomBasicPath, ""),
				Option(ID::RomChargenPath, ""),

				Option(ID::MediaKeys, true),
				Option(ID::SingleInstance, 1),
				RuntimeOption(ID::RestoreDefaults, false),

				// Internal
				Option(ID::Volume, 100),
				Option(ID::VolumeControlEnabled, true),
			};
		}
	};
}
