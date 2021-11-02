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

#include "Option.h"

namespace Settings
{
	Option::Option(const std::string& aName) :
		name(aName)
	{
	}

	Option::Option(const std::string& aName, int value) :
		name(aName)
	{
		UpdateValue(value);
	}

	Option::Option(const std::string& aName, double value) :
		name(aName)
	{
		UpdateValue(value);
	}

	Option::Option(const std::string& aName, const wxString& value) :
		name(aName)
	{
		UpdateValue(value);
	}

	void Option::UpdateValue(int value)
	{
		InternalUpdateValue(std::to_wstring(value), TypeSerialized::Int);
	}

	void Option::UpdateValue(double value)
	{
		InternalUpdateValue(std::to_wstring(value), TypeSerialized::Double);
	}

	void UpdateValue(float value)
	{
		UpdateValue(static_cast<double>(value));
	}

	void Option::UpdateValue(const wxString& value)
	{
		InternalUpdateValue(value, TypeSerialized::String);
	}

	bool Option::HasValue() const
	{
		return !_valueString.empty();
	}

	Option::TypeSerialized Option::GetValueType() const
	{
		return _valueType;
	}

	bool Option::GetValueAsBool() const
	{
		return !!stoi(_valueString.ToStdString());
	}

	int Option::GetValueAsInt() const
	{
		return stoi(_valueString.ToStdString());
	}

	double Option::GetValueAsDouble() const
	{
		return stod(_valueString.ToStdString());
	}

	float Option::GetValueAsFloat() const
	{
		return stof(_valueString.ToStdString());
	}

	const wxString& Option::GetValueAsString() const
	{
		return _valueString;
	}

	bool Option::ShouldSerialize() const
	{
		return _shouldSerialize;
	}

	void Option::UpdateValue(const Option& other)
	{
		assert(other.name == name);
		InternalUpdateValue(other._valueString, other._valueType);
	}

	void Option::InternalUpdateValue(const wxString& value, TypeSerialized type)
	{
		if (_valueType == TypeSerialized::Undefined)
		{
			_valueType = type;
		}

		assert(type == _valueType);
		_valueString = value;
	}
}
