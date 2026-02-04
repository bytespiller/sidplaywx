/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include "MultiImageButton.h"
#include "../../Util/SimpleSignal/SimpleSignalProvider.h"
#include <map>
#include <optional>
#include <vector>

namespace UIElements
{
	enum class SignalsRepeatModeButton
	{
		SIGNAL_EXTRA_OPTION_SELECTED
	};

	class RepeatModeButton : public MultiImageButton, public SimpleSignalProvider<SignalsRepeatModeButton>
	{
	public:
		enum class RepeatMode : int // Reminder: don't change the order nilly-willy, the settings may be already saved with current values.
		{
			Undefined = 0,
			Normal,
			RepeatOne,
			RepeatAll,
			InfiniteDuration,
			PlayOnce
		};

		struct ButtonState
		{
			ButtonState() = delete;
			ButtonState(const ThemeData::ThemeImage& aImage, const wxString& aText, bool aEnabled = true) :
				themeImage(aImage),
				text(aText),
				enabled(aEnabled)
			{
			}

			const ThemeData::ThemeImage& themeImage;
			wxString text;
			bool enabled;
		};

		using ButtonStates = std::map<RepeatMode, ButtonState>;

	public:
		struct ExtraOptionsHandler
		{
			enum class ExtraOptionId : int // Must be greater (different) than the RepeatMode enum.
			{
				// Toggles
				Undefined = -1,
				DefaultSubsong = 100,
				IncludeSubsongs = 101,
				PreRenderEnabled = 102,

				// Actions
				ActionShufflePlaylist = 200
			};

			enum class Type
			{
				Toggle,
				Action
			};

			struct ExtraOption
			{
				ExtraOption() = delete;
				ExtraOption(const wxString& aText, bool aEnabled, bool aSeparator = false, ExtraOptionsHandler::Type aType = ExtraOptionsHandler::Type::Toggle, bool aBottom = true) :
					text(aText),
					enabled(aEnabled),
					separator(aSeparator),
					type(aType),
					bottom(aBottom)
				{
				}

				wxString text;
				bool enabled;
				bool separator;
				ExtraOptionsHandler::Type type;
				bool bottom;
			};

			using ExtraOptions = std::map<ExtraOptionId, ExtraOption>;
			using ExtraOptionsMenuOpenUpdateCallback = std::function<void(RepeatModeButton&)>;

			ExtraOptionsHandler() = delete;
			ExtraOptionsHandler(const ExtraOptions& aExtraOptions, ExtraOptionsMenuOpenUpdateCallback aMenuOpenUpdateCallback) :
				extraOptions(aExtraOptions),
				menuOpenUpdateCallback(aMenuOpenUpdateCallback)
			{
			}

			ExtraOptions extraOptions;
			ExtraOptionsMenuOpenUpdateCallback menuOpenUpdateCallback;
		};

	private:
		using ImageList = RepeatModeButton::MultiImageButton::ImageList;

	public:
		RepeatModeButton() = delete;
		RepeatModeButton(const ButtonStates& buttonStates, const wxSize& size, wxPanel& panel, const std::optional<ExtraOptionsHandler>& extraOptionsHandler);

	public:
		void SetRepeatMode(RepeatMode forceMode);
		RepeatMode GetRepeatMode() const;
		RepeatMode Cycle();

		void SetRepeatModeOptionEnabled(RepeatMode mode, bool enabled);
		void SetExtraOptionEnabled(ExtraOptionsHandler::ExtraOptionId optionId, bool enabled = true);

	private:
		void UpdateTooltip();
		bool AddExtraOptions(wxMenu* menu, bool bottom);

		void OnContextMenuOpen(wxContextMenuEvent& evt);
		void OnContextMenuItemSelected(wxCommandEvent& evt);

	private:
		static ImageList AsImageList(const ButtonStates& buttonStates);

	private:
		ButtonStates _buttonStates;

	private:
		std::optional<ExtraOptionsHandler> _extraOptionsHandler;
	};
}
