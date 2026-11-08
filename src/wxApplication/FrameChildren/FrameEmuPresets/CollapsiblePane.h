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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
    #include <wx/wx.h>
#endif

#include <wx/collpane.h>
#include <memory>

#include "Widgets/FilterWidgetBase.h"

namespace FrameElements
{
	class CollapsiblePane
	{
	public:
		CollapsiblePane() = delete;
		CollapsiblePane(const wxString& caption, wxSizer* sizer, wxWindow* parent);

	public:
		/// @brief Attaches and takes ownership of widgets.
		void AdoptWidgets(std::vector< std::unique_ptr<FilterWidgetBase> >& widgets);

		/// @return Widget with specified uid or nullptr.
		FilterWidgetBase* TryGetWidget(const char* uid);

		/// @return The control proxy. You probably don't need this.
		wxCollapsiblePane* GetProxy() const;

		/// @brief Alias for GetProxy()->GetPane()
		wxWindow* GetPane() const;

		/// @return The panel on which the widgets are attached to.
		wxPanel* GetContentPanel() const;

		/// @return The collapsible pane's initial caption text (from its creation).
		const wxString& GetCanonicalCaption() const;

		/// @brief Alias for GetProxy()->GetLabelText()
		/// @return The collapsible pane's current caption text.
		wxString GetCurrentCaption() const;

		/// @brief Sets collapsible pane's caption.
		void SetCaption(const wxString& caption) const;

	private:
		std::vector< std::unique_ptr<FilterWidgetBase> > _widgets;
		wxString _caption;
		wxCollapsiblePane* _collpane;
		wxPanel* _contentPanel;
	};
}
