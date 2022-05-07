/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2022 Jasmin Rutic (bytespiller@gmail.com)
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
#include "../Config/UIStrings.h"

namespace Settings
{
	Option::Option(const wxString& aName, Var value) :
		name(aName)
	{
		UpdateValue(value);
	}

	void Option::UpdateValue(const Var& newValue)
	{
		const bool initialized = HasValue();
		const Settings::Option::TypeSerialized& canonicalType = (initialized) ? GetValueType() : static_cast<TypeSerialized>(newValue.index());

		assert(!initialized || (newValue.index() == _value.index())); // Type mismatch.
		switch (canonicalType)
		{
			case TypeSerialized::Int:
				_value = std::get<int>(newValue);
				break;
			case TypeSerialized::Double:
				_value = std::get<double>(newValue);
				break;
			case TypeSerialized::String:
				_value = std::get<wxString>(newValue);
				break;
			default:
				throw std::runtime_error(Strings::Internal::UNHANDLED_SWITCH_CASE);
		}
	}

	void Option::UpdateValue(const Option& other)
	{
		assert(other.name == name);
		UpdateValue(other._value);
	}

	bool Option::HasValue() const
	{
		return GetValueType() != Settings::Option::TypeSerialized::Undefined;
	}

	Option::TypeSerialized Option::GetValueType() const
	{
		return static_cast<TypeSerialized>(_value.index());
	}

	Option::Var Option::GetValue() const
	{
		return _value;
	}

	int Option::GetValueAsInt() const
	{
		assert(HasValue());
		assert(GetValueType() == Option::TypeSerialized::Int);
		return std::get<int>(_value);
	}

	double Option::GetValueAsDouble() const
	{
		assert(HasValue());
		assert(GetValueType() == Option::TypeSerialized::Double);
		return std::get<double>(_value);
	}

	const wxString& Option::GetValueAsString() const
	{
		assert(HasValue());
		assert(GetValueType() == Option::TypeSerialized::String);
		return std::get<wxString>(_value);
	}

	bool Option::GetValueAsBool() const
	{
		return !!GetValueAsInt();
	}

	float Option::GetValueAsFloat() const
	{
		return static_cast<float>(GetValueAsDouble());
	}

	bool Option::ShouldSerialize() const
	{
		return _shouldSerialize;
	}
}
