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

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <string>

namespace Settings
{
	class Option
	{
	public:
		enum class TypeSerialized
		{
			Undefined,
			Int,
			Double,
			String
		};

	public:
		Option() = delete;
		virtual ~Option() = default;

		// Use this for initializing with an empty string value.
		explicit Option(const std::string& aName);

		// Use this also for storing a bool value.
		Option(const std::string& aName, int value);

		Option(const std::string& aName, double value);
		Option(const std::string& aName, const wxString& value);

	public:
		void UpdateValue(int value);
		void UpdateValue(double value);
		void UpdateValue(float value);
		void UpdateValue(const wxString& value);

		void UpdateValue(const Option& other);

	public:
		bool HasValue() const;
		TypeSerialized GetValueType() const;

		bool GetValueAsBool() const;
		int GetValueAsInt() const;
		double GetValueAsDouble() const;
		float GetValueAsFloat() const;
		const wxString& GetValueAsString() const;

		bool ShouldSerialize() const;

	public:
		const std::string name;

	protected:
		bool _shouldSerialize = true;

	private:
		void InternalUpdateValue(const wxString& value, TypeSerialized type);

	private:
		wxString _valueString;
		TypeSerialized _valueType{};
	};

	// RuntimeOption is an Option which is not serialized to disk.
	class RuntimeOption : public Option
	{
	public:
		RuntimeOption() = delete;
		~RuntimeOption() = default;

		// Use this for initializing with an empty string value.
		explicit RuntimeOption(const std::string& aName) :
			Option(aName)
		{
			_shouldSerialize = false;
		}

		// Use this also for storing a bool value.
		RuntimeOption(const std::string& aName, int value) :
			Option(aName, value)
		{
			_shouldSerialize = false;
		}

		RuntimeOption(const std::string& aName, double value) :
			Option(aName, value)
		{
			_shouldSerialize = false;
		}

		RuntimeOption(const std::string& aName, const wxString& value) :
			Option(aName, value)
		{
			_shouldSerialize = false;
		}
	};
}
