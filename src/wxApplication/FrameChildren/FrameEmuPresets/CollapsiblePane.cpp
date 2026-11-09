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

#include "CollapsiblePane.h"
#include <numeric>

namespace FrameElements
{
	static constexpr int LABEL_OFFSET = 25; // 25 on MSW, have to check others

	CollapsiblePane::CollapsiblePane(const wxString& caption, wxSizer* sizer, wxWindow* parent) :
		_caption(caption)
	{
		_collpane = new wxCollapsiblePane(parent, wxID_ANY, _caption);
		sizer->Add(_collpane, 0, wxEXPAND | wxALL);

		_collpane->Bind(wxEVT_COLLAPSIBLEPANE_CHANGED, [parent](wxCollapsiblePaneEvent &) { parent->Layout(); });
		_collpane->GetPane()->SetSizer(new wxBoxSizer(wxVERTICAL));

		// Add internal padding
		_contentPanel = new wxPanel(_collpane->GetPane(), wxID_ANY);
		wxBoxSizer* panelSizer = new wxBoxSizer(wxVERTICAL);
		_contentPanel->SetSizer(panelSizer);
		_collpane->GetPane()->GetSizer()->Add(_contentPanel, 1, wxEXPAND | wxLEFT, LABEL_OFFSET);

		_collpane->Expand();
	}

	void CollapsiblePane::AdoptWidgets(std::vector< std::unique_ptr<FilterWidgetBase> >& widgets)
	{
		_widgets.reserve(_widgets.size() + widgets.size());

		for (auto& widget : widgets)
		{
			_widgets.emplace_back(std::move(widget));
			_widgets.back()->Bind(wxEVT_FILTER_WIDGET_TOGGLED, [this](wxCommandEvent& /*event*/)
			{
				const int sum = GetEnabledWidgetsCount();
				if (sum == 0)
				{
					SetCaption(GetCanonicalCaption());
				}
				else
				{
					SetCaption(wxString::Format("%s (%i)", GetCanonicalCaption(), sum));
				}
			});
		}

		widgets.clear();
	}

	FilterWidgetBase* CollapsiblePane::TryGetWidget(const char* uid)
	{
		const auto it = std::find_if(_widgets.cbegin(), _widgets.cend(), [uid](const std::unique_ptr<FilterWidgetBase>& cWidget)
		{
			return cWidget->GetUid() == uid;
		});

		return (it != _widgets.cend()) ? it->get() : nullptr;
	}

	wxWindow* CollapsiblePane::GetPane() const
	{
		return GetProxy()->GetPane();
	}

	wxPanel* CollapsiblePane::GetContentPanel() const
	{
		return _contentPanel;
	}

	const wxString& CollapsiblePane::GetCanonicalCaption() const
	{
		return _caption;
	}

	wxString CollapsiblePane::GetCurrentCaption() const
	{
		return GetProxy()->GetLabelText();
	}

	void CollapsiblePane::SetCaption(const wxString& caption) const
	{
		GetProxy()->SetLabelText(caption);
		GetProxy()->Refresh();
	}

	void CollapsiblePane::AutoExpandOrCollapse()
	{
		if (GetEnabledWidgetsCount() == 0)
		{
			GetProxy()->Collapse();
		}
		else
		{
			GetProxy()->Expand();
		}
	}

	wxCollapsiblePane* CollapsiblePane::GetProxy() const
	{
		return _collpane;
	}

	int CollapsiblePane::GetEnabledWidgetsCount() const
	{
		return std::accumulate(_widgets.begin(), _widgets.end(), 0, [](int cSum, const std::unique_ptr<FilterWidgetBase>& widget)
		{
			return cSum + static_cast<int>(widget->IsEnabled());
		});
	}
}
