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

#ifndef NDEBUG

#include <iostream>
#include <chrono>
#include <string>

/// @brief Reminder: don't forget to comment-out the -mwindows in CMakeLists.txt to avoid wxWidgets eating the stdout.
namespace Profiling
{
    class StackTag
    {
        public:
        StackTag() = delete;

        /// @brief Measures the interval between construction and destruction of this class.
        explicit StackTag(const std::string& name) :
            _name(name),
            _start(std::chrono::high_resolution_clock::now())
        {
        }

        ~StackTag() {
            auto end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - _start).count();
            std::cout << "[PROFILED] " << _name << ": " << (duration / 1000.0) << " ms\n";
        }

        StackTag(const StackTag&) = delete;
        StackTag& operator=(const StackTag&) = delete;
        StackTag(StackTag&&) = delete;
        StackTag& operator=(StackTag&&) = delete;

    private:
        std::string _name;
        std::chrono::time_point<std::chrono::high_resolution_clock> _start;
    };

    class Time
    {
    public:
        Time() = delete;

        /// @brief Measures the interval between construction of this class instance and the explicit call to its End().
        explicit Time(const std::string& name) :
            _name(name),
            _tag(std::make_unique<StackTag>(name))
        {
        }

        Time(const Time&) = delete;
        Time& operator=(const Time&) = delete;
        Time(Time&&) = delete;
        Time& operator=(Time&&) = delete;

        ~Time()
        {
            if (_tag != nullptr)
            {
                _tag = nullptr;
                std::cout << "[Warning] Profile::Time \"" << _name << "\"" << " not manually ended!\n"; // May indicate suboptimal measurement placement.
            }
        }

    public:
        /// @brief Ends profiling.
        void End()
        {
            _tag = nullptr;
        }

    private:
        std::string _name;
        std::unique_ptr<StackTag> _tag;
    };
}

#endif
