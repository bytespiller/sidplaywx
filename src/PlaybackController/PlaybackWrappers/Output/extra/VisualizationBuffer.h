/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2024-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include <atomic>
#include <cstddef>

class VisualizationBuffer
{
public:
	VisualizationBuffer() = delete;
    VisualizationBuffer(VisualizationBuffer&) = delete;
	VisualizationBuffer& operator=(const VisualizationBuffer&) = delete;

    explicit VisualizationBuffer(size_t aLength);
    ~VisualizationBuffer();

public:
    /// @brief Copies front-buffer data with an initialized constant maxLength size and returns maxLength. If no initial data is ready yet, doesn't copy anything and returns 0.
    size_t Read(short* out) const;

    void Write(const short* const data, size_t dataLength);

public:
    const size_t maxLength;

private:
    std::atomic_bool _ready = false; // Whether the front-buffer is ready (remains true permanently).
    std::atomic_bool _flipped = false; // Indicates a front-buffer and back-buffer role inversion.
    std::atomic_size_t _level = 0; // Fill-level of the active back-buffer.

    std::atomic_short* _first; // Front/back buffer #1.
    std::atomic_short* _second; // Front/back buffer #2.
};
