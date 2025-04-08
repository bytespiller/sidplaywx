/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2023 Jasmin Rutic (bytespiller@gmail.com)
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

#include <codecvt>
#include <fstream> // TEMP
#include <limits>
#include <locale> // TEMP
#include <string>
#include <vector>

namespace Helpers
{
	namespace General
	{
		inline std::string GetZeroPaddedString(long number, unsigned int fill = 1)
		{
			if (number >= 0 && number < 10)
			{
				return std::string(fill, '0') + std::to_string(number);
			}
			return std::to_string(number);
		}

		template <typename T>
		bool AreFloatsEqual(T f1, T f2)
		{
			const T epsilon = std::numeric_limits<T>::epsilon();
			return (f1 + epsilon >= f2) && (f1 - epsilon <= f2);
		}

		template <typename T>
		std::vector<T> SplitString(const T& str, const char delimiter)
		{
			std::vector<T> items;

			size_t start = 0;
			while (true)
			{
				const size_t end = str.find(delimiter, start);
				if (end == T::npos)
				{
					if (start == 0) // First and only one.
					{
						items.emplace_back(str);
					}
					else // Don't forget the last one.
					{
						items.emplace_back(str.substr(start));
					}
					break;
				}

				items.emplace_back(str.substr(start, end - start));
				start = end + 1;
			}

			return items;
		}

		inline std::string WideStringToUtf8(const std::wstring& wstr) // TODO: get rid of this temporary patch for initial Linux build
		{
			std::wstring_convert<std::codecvt_utf8<wchar_t>> converter;
			return converter.to_bytes(wstr);
		}
	}
}
