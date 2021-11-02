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

#include "SettingsBase.h"
#include "../Config/UIStrings.h"
#include <wx/xml/xml.h>
#include <map>
#include <memory>
#include <stdexcept>

namespace Settings
{
	namespace detail
	{
		constexpr const char* const NODE_SETTINGS = "Settings";
		constexpr const char* const ATTR_TYPE = "type";
		constexpr const char* const ATTR_VALUE = "value";

		inline const std::map<Option::TypeSerialized, const std::string> typeToStringMap
		{
			{Option::TypeSerialized::Int, "Int"},
			{Option::TypeSerialized::Double, "Double"},
			{Option::TypeSerialized::String, "String"}
		};

		inline const std::string& TypeAsString(Option::TypeSerialized type)
		{
			return detail::typeToStringMap.at(type);
		}

		inline Option::TypeSerialized StringAsType(const std::string& type)
		{
			const auto it = std::find_if(detail::typeToStringMap.begin(), detail::typeToStringMap.end(), [&type](const auto& pair)
			{
				return pair.second == type;
			});

			return (*it).first;
		}
	}

	// Main class -------------------------------------------------

	SettingsBase::SettingsBase(const char* const filename) :
		_filename(filename)
	{
	}

	bool SettingsBase::TryLoad(const std::vector<Option>& defaults)
	{
		ResetTo(defaults);

		std::unique_ptr<wxXmlDocument> doc = std::make_unique<wxXmlDocument>();
		if (!wxFileExists(_filename) || !doc->Load(_filename))
		{
			return false;
		}

		wxXmlNode* child = doc->GetRoot()->GetChildren();
		while (child)
		{
			const std::string& name = child->GetName().ToStdString();
			const std::string& strType = child->GetAttribute(detail::ATTR_TYPE).ToStdString();
			const wxString& strValue = child->GetAttribute(detail::ATTR_VALUE).ToStdWstring();

			const Option::TypeSerialized type = detail::StringAsType(strType);
			switch (type)
			{
				case Option::TypeSerialized::Double:
					TryUpdateOption(Option(name, stod(strValue.ToStdString())));
					break;

				case Option::TypeSerialized::Int:
					TryUpdateOption(Option(name, stoi(strValue.ToStdString())));
					break;

				case Option::TypeSerialized::String:
					TryUpdateOption(Option(name, strValue));
					break;

				default:
					throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
			}

			child = child->GetNext();
		}

		return true;
	}

	bool SettingsBase::TrySave()
	{
		std::unique_ptr<wxXmlDocument> doc = std::make_unique<wxXmlDocument>();
		wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, detail::NODE_SETTINGS);

		for (const Option& option : _options)
		{
			if (!option.ShouldSerialize() || !option.HasValue())
			{
				continue;
			}

			wxXmlNode* settingNode = new wxXmlNode(wxXML_ELEMENT_NODE, option.name);
			wxXmlAttribute* attrValue = new wxXmlAttribute(detail::ATTR_VALUE, option.GetValueAsString());
			settingNode->SetAttributes(new wxXmlAttribute(detail::ATTR_TYPE, detail::TypeAsString(option.GetValueType()), attrValue));

			root->AddChild(settingNode);
		}

		doc->SetRoot(root);
		return doc->Save(_filename);
	}

	Option* SettingsBase::GetOption(const std::string& name)
	{
		auto it = std::find_if(_options.begin(), _options.end(), [&name](const Option& cOption)
		{
			return cOption.name == name;
		});

		if (it != _options.end())
		{
			return &(*it);
		}

		return nullptr;
	}

	void SettingsBase::AddOption(const Option& option)
	{
		if (GetOption(option.name) != nullptr)
		{
			throw std::runtime_error("Option with the same name already exists!");
		}

		_options.emplace_back(option);
	}

	void SettingsBase::ResetTo(const std::vector<Option>& newOptions)
	{
		_options.clear();
		_options.reserve(newOptions.size());
		for (const Option& dOpt : newOptions)
		{
			AddOption(dOpt);
		}
	}

	bool SettingsBase::TryUpdateOption(const Option& option)
	{
		Option* existingOpt = GetOption(option.name);
		if (existingOpt == nullptr)
		{
			return false;
		}

		existingOpt->UpdateValue(option);

		return true;
	}
}
