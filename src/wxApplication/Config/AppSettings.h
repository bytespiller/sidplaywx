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

#include "../Settings/SettingsBase.h"
#include "../UIElements/CompositeSeekBar.h"
#include "../UIElements/RepeatModeButton.h"

namespace Settings
{
	static const bool DEFAULT = true;

	class AppSettings : public SettingsBase
	{
	public:
		struct ID
		{
			// Prefs
			static constexpr const char* const AudioOutputDevice = "AudioOutputDevice";
			static constexpr const char* const LowLatency = "LowLatency";
			static constexpr const char* const ForceMono = "ForceMono";

			static constexpr const char* const PreRenderEnabled = "PreRenderEnabled";
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
			static constexpr const char* const StilPath = "StilPath";

			static constexpr const char* const DefaultC64Model = "DefaultC64Model";
			static constexpr const char* const DefaultSidModel = "DefaultSidModel";

			static constexpr const char* const FilterCurve6581 = "FilterCurve6581";
			static constexpr const char* const FilterCurve8580 = "FilterCurve8580";

			static constexpr const char* const DigiBoost = "DigiBoost";

			static constexpr const char* const RomKernalPath = "RomKernalPath";
			static constexpr const char* const RomBasicPath = "RomBasicPath";
			static constexpr const char* const RomChargenPath = "RomChargenPath";

			static constexpr const char* const RememberPlaylist = "RememberPlaylist";
			static constexpr const char* const MediaKeys = "MediaKeys";
			static constexpr const char* const SingleInstance = "SingleInstance";
			static constexpr const char* const RestoreDefaults = "RestoreDefaults";

			// Menu
			static constexpr const char* const StayTopmost = "StayTopmost";
			static constexpr const char* const VisualizationEnabled = "VisualizationEnabled";
			static constexpr const char* const StilInfoEnabled = "StilInfoEnabled";

			// Internal
			static constexpr const char* const Volume = "Volume";
			static constexpr const char* const VolumeControlEnabled = "VolumeControlEnabled";

			static constexpr const char* const LastSongName = "LastSongName";
			static constexpr const char* const LastSubsongIndex = "LastSubsongIndex";

			static constexpr const char* const MainWindowPosition = "MainWindowPosition";
			static constexpr const char* const MainWindowSize = "MainWindowSize";
			static constexpr const char* const MainWindowMaximized = "MainWindowMaximized";
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
#if __WXGTK__
		static constexpr char PREFERRED_DEFAULT_AUDIO_DEVICE_NAME[] = "PulseAudio: Default Sink";
#else
		static constexpr char PREFERRED_DEFAULT_AUDIO_DEVICE_NAME[] = "";
#endif

		std::vector<Option> GetDefaultSettings() const override
		{
			return
			{
				// Prefs
				DefaultOption(ID::AudioOutputDevice, PREFERRED_DEFAULT_AUDIO_DEVICE_NAME),
				DefaultOption(ID::LowLatency, true),
				DefaultOption(ID::ForceMono, false),

				DefaultOption(ID::PreRenderEnabled, false),
				DefaultOption(ID::AutoPlay, true),
				DefaultOption(ID::RepeatMode, static_cast<int>(UIElements::RepeatModeButton::RepeatMode::Normal)),
				DefaultOption(ID::RepeatModeIncludeSubsongs, false),
				DefaultOption(ID::RepeatModeDefaultSubsong, true),
				DefaultOption(ID::SongFallbackDuration, 180),
				DefaultOption(ID::SkipShorter, 0),
				DefaultOption(ID::PopSilencer, 100),
				DefaultOption(ID::DragDropMode, static_cast<int>(DragDropMode::Dual)),

				DefaultOption(ID::SelectionFollowsPlayback, true),
				DefaultOption(ID::AutoExpandSubsongs, true),
				DefaultOption(ID::TaskbarProgress, static_cast<int>(UIElements::CompositeSeekBar::TaskbarProgressOption::Enabled)),

				DefaultOption(ID::SonglengthsPath, ""),
				DefaultOption(ID::SonglengthsTrim, 0),
				DefaultOption(ID::StilPath, ""),

				DefaultOption(ID::DefaultC64Model, static_cast<int>(DefaultC64Model::Prefer_PAL)),
				DefaultOption(ID::DefaultSidModel, static_cast<int>(DefaultSidModel::Prefer_MOS6581)),

				DefaultOption(ID::FilterCurve6581, 0.5),
				DefaultOption(ID::FilterCurve8580, 0.5),

				DefaultOption(ID::DigiBoost, false),

				DefaultOption(ID::RomKernalPath, ""),
				DefaultOption(ID::RomBasicPath, ""),
				DefaultOption(ID::RomChargenPath, ""),

				DefaultOption(ID::RememberPlaylist, true),
				DefaultOption(ID::MediaKeys, true),
				DefaultOption(ID::SingleInstance, 1),
				RuntimeOption(ID::RestoreDefaults, false, DEFAULT), // RuntimeOption

				// Menu
				DefaultOption(ID::StayTopmost, false),
				DefaultOption(ID::VisualizationEnabled, true),
				DefaultOption(ID::StilInfoEnabled, true),

				// Internal
				DefaultOption(ID::Volume, 100),
				DefaultOption(ID::VolumeControlEnabled, true),
				DefaultOption(ID::LastSongName, ""),
				DefaultOption(ID::LastSubsongIndex, 0),

				DefaultOption(ID::MainWindowPosition, ""),
				DefaultOption(ID::MainWindowSize, ""),
				DefaultOption(ID::MainWindowMaximized, false),
			};
		}
	};
}
