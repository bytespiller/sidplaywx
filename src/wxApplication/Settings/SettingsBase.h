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

#include "Option.h"
#include <wx/string.h>
#include <vector>

namespace Settings
{
	class SettingsBase
	{
	public:
		explicit SettingsBase(const char* const filename);

	public:
		virtual std::vector<Option> GetDefaultSettings() const = 0;

	public:
		bool TryLoad(const std::vector<Option>& defaults);
		bool TrySave();

		Option* GetOption(const wxString& name);
		void AddOption(const Option& option);

		void ResetTo(const std::vector<Option>& newOptions);

		/// @brief Returns the full path to the settings file.
		const wxString& GetSettingsFilePath() const;

	private:
		bool TryUpdateOption(const Option& option);

	private:
		const wxString _filepath;
		std::vector<Option> _options;
	};
}
