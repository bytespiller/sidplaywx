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

#include "SettingsBase.h"
#include "../Config/UIStrings.h"
#include "../Helpers/HelpersWx.h"

#include <wx/filename.h>
#include <wx/xml/xml.h>

#include <map>
#include <memory>
#include <stdexcept>

namespace Settings
{
	namespace detail
	{
		constexpr const char* const XML_NODE_SETTINGS = "Settings";
		constexpr const char* const XML_ATTR_TYPE = "type";
		constexpr const char* const XML_ATTR_VALUE = "value";

		const std::map<Option::TypeSerialized, const wxString> typeToStringMap
		{
			{Option::TypeSerialized::Int, "Int"},
			{Option::TypeSerialized::Double, "Double"},
			{Option::TypeSerialized::String, "String"}
		};

		inline const wxString& TypeAsString(Option::TypeSerialized type)
		{
			return detail::typeToStringMap.at(type);
		}

		inline Option::TypeSerialized StringAsType(const wxString& type)
		{
			const auto it = std::find_if(detail::typeToStringMap.cbegin(), detail::typeToStringMap.cend(), [&type](const auto& pair)
			{
				return pair.second == type;
			});

			return (*it).first;
		}
	}

	// Main class -------------------------------------------------

	SettingsBase::SettingsBase(const char* const filename) :
		_filepath(Helpers::Wx::Files::GetConfigFilePath(filename))
	{
	}

	bool SettingsBase::TryLoad(const std::vector<Option>& defaults)
	{
		ResetTo(defaults); // Defaults are a base on which the values from the config file are applied. Any Options in the config file which don't exist in the defaults (e.g., deprecated Options in the config file) will be ignored.

		std::unique_ptr<wxXmlDocument> doc = std::make_unique<wxXmlDocument>();
		if (!wxFileExists(_filepath) || !doc->Load(_filepath))
		{
			return false;
		}

		wxXmlNode* child = doc->GetRoot()->GetChildren();
		while (child)
		{
			const wxString& name = child->GetName();
			const wxString& strType = child->GetAttribute(detail::XML_ATTR_TYPE);
			const wxString& strValue = child->GetAttribute(detail::XML_ATTR_VALUE);

			const Option::TypeSerialized type = detail::StringAsType(strType);
			switch (type)
			{
				case Option::TypeSerialized::Int:
					TryUpdateOption(Option(name, stoi(strValue.ToStdString())));
					break;

				case Option::TypeSerialized::Double:
					TryUpdateOption(Option(name, stod(strValue.ToStdString())));
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
		wxXmlNode* root = new wxXmlNode(wxXML_ELEMENT_NODE, detail::XML_NODE_SETTINGS);

		for (const Option& option : _options)
		{
			if (!option.ShouldSerialize() || option.IsUnchangedDefault() || !option.HasValue())
			{
				continue;
			}

			wxXmlAttribute* attrValue = nullptr;
			switch (option.GetValueType())
			{
				case Option::TypeSerialized::Int:
					attrValue = new wxXmlAttribute(detail::XML_ATTR_VALUE, std::to_string(option.GetValueAsInt()));
					break;
				case Option::TypeSerialized::Double:
					attrValue = new wxXmlAttribute(detail::XML_ATTR_VALUE, std::to_string(option.GetValueAsDouble()));
					break;
				case Option::TypeSerialized::String:
					attrValue = new wxXmlAttribute(detail::XML_ATTR_VALUE, option.GetValueAsString());
					break;
				default:
					throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
			}

			wxXmlNode* settingNode = new wxXmlNode(wxXML_ELEMENT_NODE, option.name);
			settingNode->SetAttributes(new wxXmlAttribute(detail::XML_ATTR_TYPE, detail::TypeAsString(option.GetValueType()), attrValue));

			root->AddChild(settingNode);
		}

		doc->SetRoot(root);

#ifndef WIN32
		if (!wxDirExists(_filepath))
		{
			wxFileName::Mkdir(wxFileName(_filepath).GetPath(), wxS_DIR_DEFAULT, wxPATH_MKDIR_FULL);
		}
#endif

		return doc->Save(_filepath);
	}

	Option* SettingsBase::GetOption(const wxString& name)
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

	const wxString& SettingsBase::GetSettingsFilePath() const
	{
		return _filepath;
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
