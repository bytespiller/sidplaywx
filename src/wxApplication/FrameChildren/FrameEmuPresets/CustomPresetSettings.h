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

#pragma once

#include "../../Settings/SettingsBase.h"
#include "../../Config/AppSettings.h" // TODO: migrate

namespace Settings
{
	class CustomPresetSettings : public SettingsBase
	{
	public:
		CustomPresetSettings(const wxString& filename) :
			SettingsBase(filename)
		{
		}

	public:
		std::vector<Option> GetDefaultSettings() const override
		{
			return // TODO: migrate
			{
				// Common
				DefaultOption(AppSettings::ID::DefaultC64Model, static_cast<int>(AppSettings::DefaultC64Model::Prefer_PAL)),
				DefaultOption(AppSettings::ID::DefaultSidModel, static_cast<int>(AppSettings::DefaultSidModel::Prefer_MOS6581)),
				DefaultOption(AppSettings::ID::CombinedWaveformsStrength, static_cast<int>(AppSettings::CombinedWaveformsStrength::AVERAGE)),
				DefaultOption(AppSettings::ID::DacLeakage, 1.0),

				// 6581
				DefaultOption(AppSettings::ID::FilterCurve6581, 0.5),
				DefaultOption(AppSettings::ID::FilterRange6581, 0.5),
				DefaultOption(AppSettings::ID::WaveOffset6581, 0.0),
				DefaultOption(AppSettings::ID::Old6581caps, false),

				// 8580
				DefaultOption(AppSettings::ID::FilterCurve8580, 0.5),
				DefaultOption(AppSettings::ID::DigiBoost, false),
			};
		}
	};
}
