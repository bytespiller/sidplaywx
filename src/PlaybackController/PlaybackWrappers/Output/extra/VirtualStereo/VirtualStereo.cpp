/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "VirtualStereo.h"

#include <algorithm>
#include <cstring> // memcpy
#include <cmath>
#include <cstdint>

static constexpr size_t CHANNELS = 2;
static constexpr size_t FULL_TAIL_OFFSET = 2;

VirtualStereo::VirtualStereo(unsigned int sampleRate, unsigned int offsetMs, float sideVolumeFactor) :
	_framesOffset(static_cast<unsigned int>(std::floor(sampleRate * (offsetMs / 1000.0)))),
	_delayBuffer(_framesOffset * CHANNELS * FULL_TAIL_OFFSET)
{
	ChangeSideVolumeFactor(sideVolumeFactor);
}

void VirtualStereo::Apply(short* const out, const size_t framesPerBuffer)
{
	const size_t samplesPerBuffer = framesPerBuffer * CHANNELS;

	// Warm-up
	if (!_delayBuffer.IsSaturated())
	{
		const size_t delayBufferRemaining = _delayBuffer.GetCapacity() - _delayBuffer.GetLength();
		const size_t samplesToWrite = (samplesPerBuffer > delayBufferRemaining) ? delayBufferRemaining : samplesPerBuffer;
		_delayBuffer.CopyFrom(out, samplesToWrite);
	}

	const bool readyFirst = _delayBuffer.GetLength() >= _framesOffset * CHANNELS;
	const bool readySecond = _delayBuffer.IsSaturated();

	short* tailSampleStraight = nullptr;
	size_t tailLenStraight = 0;

	short* tailSampleWrapped = nullptr;
	size_t tailLenWrapped = 0;

	if (readyFirst)
	{
		// Store pristine samples
		_snapshotBuffer.CopyFrom(out, samplesPerBuffer);

		// Get tail
		_delayBuffer.Peek(tailSampleStraight, tailLenStraight, tailSampleWrapped, tailLenWrapped, _delayBuffer.GetLength());
	}

	// Pan the original left (HaaS (-n) ms virtual left preceding)
	for (size_t frame = 0; frame < framesPerBuffer; ++frame)
	{
		out[2 * frame] = std::lrintf(out[2 * frame] * _sideVolumeFactor); // quieter left
		out[(2 * frame) + 1] = 0; // mute right
	}

	// Center 1x delay (center "present" in HaaS domain)
	if (readyFirst)
	{
		const size_t halfTailFrames = std::min(framesPerBuffer, _framesOffset);
		const size_t halfTailSamples = halfTailFrames * CHANNELS;

		const size_t tailSamplesReadOffset = (!readySecond) ? _warmupReadSamples : _framesOffset * CHANNELS;
		if (!readySecond)
		{
			_warmupReadSamples += halfTailSamples;
		}

		// Render the 50% of the previous tail first
		for (size_t frame = 0; frame < halfTailFrames; ++frame)
		{
			const size_t writeSample = frame * CHANNELS;

			size_t readSample = tailSamplesReadOffset + writeSample;
			const short* const tailChunk = ((readSample < tailLenStraight) ? tailSampleStraight : tailSampleWrapped);
			readSample = (readSample < tailLenStraight) ? readSample : readSample - tailLenStraight;

			out[writeSample] += std::lrintf(*(tailChunk + readSample) * _centerVolumeFactor); //  left
			out[writeSample + 1] += std::lrintf(*(tailChunk + readSample + 1) * _centerVolumeFactor); // right
		}

		// Render rest of the current frame shifted forward (as needed, depending on the current output buffer size)
		const short* shiftSamples = _snapshotBuffer.GetData();
		for (size_t frame = halfTailFrames; frame < framesPerBuffer; ++frame)
		{
			const size_t sample = frame * CHANNELS;
			out[sample] += std::lrintf(*shiftSamples++ * _centerVolumeFactor); // left
			out[sample + 1] += std::lrintf(*shiftSamples++ * _centerVolumeFactor); // right
		}
	}

	// Expansion right -- 2x delay (HaaS (n) ms virtual right delayed)
	if (readySecond)
	{
		const size_t maxTailFrames = std::min(framesPerBuffer, _framesOffset * FULL_TAIL_OFFSET);
		const size_t maxTailSamples = maxTailFrames * CHANNELS;

		// Render the previous tail first
		for (size_t frame = 0; frame < maxTailFrames; ++frame)
		{
			const size_t writeSample = frame * CHANNELS;
			const short* const tailChunk = ((writeSample < tailLenStraight) ? tailSampleStraight : tailSampleWrapped);
			const size_t readSample = (writeSample < tailLenStraight) ? writeSample : writeSample - tailLenStraight;

			// skipped left
			out[writeSample + 1] += std::lrintf(*(tailChunk + readSample + 1) * _sideVolumeFactor); // quieter right
		}

		// Render rest of the current frame shifted forward (as needed, depending on the current output buffer size)
		const short* shiftSamples = _snapshotBuffer.GetData();
		for (size_t frame = maxTailFrames; frame < framesPerBuffer; ++frame)
		{
			const size_t sample = frame * CHANNELS;
			++shiftSamples; // skip left
			out[sample + 1] += std::lrintf(*shiftSamples++ * _sideVolumeFactor); // quieter right
		}

		// Remember the new full tail
		_delayBuffer.Advance(maxTailSamples);
		_delayBuffer.CopyFrom(_snapshotBuffer.GetData() + samplesPerBuffer - maxTailSamples, maxTailSamples);
	}
}

void VirtualStereo::Reset()
{
	_delayBuffer.Reset();
	_warmupReadSamples = 0;
}

void VirtualStereo::ChangeSideVolumeFactor(float factor)
{
	_sideVolumeFactor = factor;
	_centerVolumeFactor = 1.0f - (_sideVolumeFactor * 2);
}
