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
			ButtonState(const ThemeData::ThemeImage& aImage, const wxString& aText) :
				themeImage(aImage),
				text(aText)
			{
			}

			const ThemeData::ThemeImage& themeImage;
			wxString text;
		};

		using ButtonStates = std::map<RepeatMode, ButtonState>;

	public:
		struct ExtraOptionsHandler
		{
			enum class ExtraOptionId : int
			{
				Undefined = 0,
				DefaultSubsong = 1,
				IncludeSubsongs = 2,
				PreRenderEnabled = 3
			};

			struct ExtraOption
			{
				ExtraOption() = delete;
				ExtraOption(const wxString& aText, bool aEnabled) :
					text(aText),
					enabled(aEnabled)
				{
				}

				wxString text;
				bool enabled;
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
		void SetRepeatModeImage(RepeatMode forceMode);
		RepeatMode GetRepeatMode() const;
		RepeatMode Cycle();

		void SetModeTexts(const std::vector<wxString>& tooltips); // TODO: improve after API change
		void SetExtraOptionEnabled(ExtraOptionsHandler::ExtraOptionId optionId, bool enabled);

	private:
		void UpdateTooltip();

	private:
		void OnContextMenuOpen(wxContextMenuEvent& evt);
		void OnContextMenuItemSelected(wxCommandEvent& evt);

	private:
		static ImageList AsImageList(const ButtonStates& buttonStates);

	private:
		int _modeImagesCount = 0;
		std::vector<wxString> _modeTexts;

	private:
		std::optional<ExtraOptionsHandler> _extraOptionsHandler;
	};
}
