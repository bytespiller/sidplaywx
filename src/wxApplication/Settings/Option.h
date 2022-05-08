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

#pragma once

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <variant>

namespace Settings
{
	class Option
	{
	public:
		enum class TypeSerialized : size_t
		{
			Undefined = 0,
			Int,
			Double,
			String
		};

		using Var = std::variant<std::monostate, int, double, wxString>;

	public:
		Option() = delete;
		Option(const wxString& aName, Var value, bool valueIsDefault = false);

		virtual ~Option() = default;

	public:
		void UpdateValue(const Var& newValue);
		void UpdateValue(const Option& other);

	public:
		bool HasValue() const;
		TypeSerialized GetValueType() const;
		Var GetValue() const;

		int GetValueAsInt() const;
		double GetValueAsDouble() const;
		const wxString& GetValueAsString() const;

#pragma region Fungible getters
		bool GetValueAsBool() const;
		float GetValueAsFloat() const;
#pragma endregion

		bool ShouldSerialize() const;
		bool IsUnchangedDefault() const;

	public:
		const wxString name;

	protected:
		bool _shouldSerialize = true;

	private:
		Var _value = std::monostate{};
		bool _valueIsDefault = false;
	};

	// RuntimeOption is an Option which is not serialized to disk.
	class RuntimeOption : public Option
	{
	public:
		RuntimeOption() = delete;
		RuntimeOption(const std::string& aName, Option::Var value, bool valueIsDefault = false) :
			Option(aName, value, valueIsDefault)
		{
			_shouldSerialize = false;
		}

		~RuntimeOption() = default;
	};

	// Syntactic sugar to avoid the need to pass an unsightly additional parameter for the default-value Options.
	class DefaultOption : public Option
	{
	public:
		DefaultOption() = delete;
		DefaultOption(const std::string& aName, Option::Var value) :
			Option(aName, value, true)
		{
		}

		~DefaultOption() = default;
	};
}
