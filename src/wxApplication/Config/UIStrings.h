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

// This is temporary until localization support is implemented one day perhaps.
// Reminder: "inline" must be to (try to) avoid unnecessary duplication of string literals in the executable.

namespace Strings
{
	namespace FramePlayer
	{
#ifndef NDEBUG
		inline constexpr const char* const WINDOW_TITLE("sidplaywx (DEBUG)");
#else
		inline constexpr const char* const WINDOW_TITLE("sidplaywx");
#endif

		inline constexpr const char* const MENU_FILE("&File");

		inline constexpr const char* const MENU_ITEM_OPEN_FILES("&Open Files...");
		inline constexpr const char* const MENU_ITEM_OPEN_FOLDERS("Open Fol&ders...");

		inline constexpr const char* const MENU_ITEM_ENQUEUE_FILES("&Enqueue Files...");
		inline constexpr const char* const MENU_ITEM_ENQUEUE_FOLDERS("En&queue Folders...");

		inline constexpr const char* const MENU_ITEM_EXIT("E&xit");

		inline constexpr const char* const MENU_EDIT("&Edit");
		inline constexpr const char* const MENU_ITEM_PLAYBACK_MODS("&Modify Playback");
		inline constexpr const char* const MENU_ITEM_PREFERENCES("&Preferences");

		inline constexpr const char* const MENU_HELP("&Help");
		inline constexpr const char* const MENU_ITEM_ABOUT("&About");

		inline constexpr const char* const REPEAT_MODE_SEQUENTIAL("Sequential");
		inline constexpr const char* const REPEAT_MODE_REPEAT_ONE("Repeat One");
		inline constexpr const char* const REPEAT_MODE_REPEAT_ALL("Repeat All");
		inline constexpr const char* const REPEAT_MODE_LEAVE_RUNNING("Leave Running");
		inline constexpr const char* const REPEAT_MODE_PLAY_ONCE("Play Once");

		inline constexpr const char* const VOL_MENU_PREFIX("Volume:");
		inline constexpr const char* const VOL_SET_MAX("Set Max Volume");
		inline constexpr const char* const VOL_SLIDER_DISABLE("Disable Slider");
		inline constexpr const char* const VOL_SLIDER_ENABLE("Enable Slider");

		inline constexpr const char* const BROWSE_FILES_SUPPORTED("All Supported");
		inline constexpr const char* const BROWSE_FILES_SID("SID Files");
		inline constexpr const char* const BROWSE_FILES_ZIP("Zip Archives");
		inline constexpr const char* const BROWSE_FILES_ALL("All Files");

		inline constexpr const char* const STATUS_DISCOVERING_FILES("Discovering files...");
		inline constexpr const char* const STATUS_CLEARING_PLAYLIST("Busy clearing playlist...");
		inline constexpr const char* const STATUS_ADDING_FILES_WITH_COUNT("Adding %i files");

		inline constexpr const char* const STATUS_PAUSED("Paused");
		inline constexpr const char* const STATUS_PLAYING("Playing");
		inline constexpr const char* const STATUS_MODIFIED_SUFFIX(" (MODIFIED)");
		inline constexpr const char* const STATUS_BYTES_SUFFIX(" bytes");
		inline constexpr const char* const STATUS_SEEKING("Seeking...");
		inline constexpr const char* const STATUS_STOPPED("Stopped");
		inline constexpr const char* const STATUS_ERROR("Error");
		inline constexpr const char* const STATUS_EXTERNAL_FILES_INCOMING("Incoming files...");
	}

	namespace PlaylistTree
	{
		inline constexpr const char* const MENU_ITEM_REMOVE("Skip this");
		inline constexpr const char* const MENU_ITEM_RESTORE("Unskip this");

		inline constexpr const char* const MENU_ITEM_SCROLL_TO_CURRENT("Scroll to current");

		inline constexpr const char* const MENU_ITEM_EXPAND_ALL("Expand all");
		inline constexpr const char* const MENU_ITEM_COLLAPSE_ALL("Collapse all");

		inline constexpr const char* const SUBSONG("Subsong");
	}

	namespace PlaybackMods
	{
		inline constexpr const char* const WINDOW_TITLE("Modify Playback");

		inline constexpr const char* const SID_VOICES_TITLE("Voices");
		inline constexpr const char* const SID_TITLE("SID");
		inline constexpr const char* const SID_VOICE("Voice");
		inline constexpr const char* const VOICE_ACTIVE("Active");
		inline constexpr const char* const VOICE_MENU_ITEM_SOLO("Solo");

		inline constexpr const char* const SPEED_SLIDER("Playback Speed (%)");
		inline constexpr const char* const SPEED_SLIDER_MENU_ITEM_RESET("Reset to 100%");
	}

	namespace Preferences
	{
		inline constexpr const char* const WINDOW_TITLE("Preferences");
		inline constexpr const char* const MSG_APP_RESTART_REQUIRED("One or more changes will take effect after the application is restarted.");
		inline constexpr const char* const SUFFIX_NOTE_APP_RESTART("PLEASE NOTE: change becomes effective after app restart.");

		inline constexpr const char* const TITLE_ROM_INFO("Detected ROM");

		// Audio Output
		inline constexpr const char* const CATEGORY_AUDIO_OUTPUT("Audio Output");
		inline constexpr const char* const OPT_DEVICE("Device");
		inline constexpr const char* const DESC_DEVICE("Additional notes:\n- Altering the playback speed is not supported on all device interfaces.\n- On MSW use DS or MME for full functionality.\n- Ongoing playback will stop if the output sample rate changes.");

		inline constexpr const char* const OPT_LOW_LATENCY("Low Latency");
		inline constexpr const char* const DESC_LOW_LATENCY("Enable for more responsive controls.\nDisable if experiencing stuttering.");

		inline constexpr const char* const OPT_FORCE_MONO("Force mono");
		inline constexpr const char* const DESC_FORCE_MONO("Disables stereo/panning effects of multi-SID tunes (2SID, 3SID).\nNote: ongoing playback will stop when changing this setting.");

		// Playback behavior
		inline constexpr const char* const CATEGORY_PLAYBACK_BEHAVIOR("Playback behavior");

		inline constexpr const char* const OPT_AUTOPLAY("Autoplay");
		inline constexpr const char* const DESC_AUTOPLAY("- Play added files immediately (unless enqueued).\n- Always start playback on track navigation.");

		inline constexpr const char* const OPT_START_DEFAULT_SUBSONG("Start default subsong");
		inline constexpr const char* const DESC_START_DEFAULT_SUBSONG("Start multi-tunes from their default subsong (indicated with a crown icon, isn't necessarily the first subsong).\nTurn this off to always start a multi-tune from its first subsong.\n(This option is also available in a Repeat Mode button's context menu.)");

		inline constexpr const char* const OPT_INCLUDE_SUBSONGS("Include subsongs");
		inline constexpr const char* const DESC_INCLUDE_SUBSONGS("Sequential playback, looping and Media Keys should include subsongs.\n(This option is also available in a Repeat Mode button's context menu.)");

		inline constexpr const char* const OPT_FALLBACK_DURATION("Fallback duration");
		inline constexpr const char* const DESC_FALLBACK_DURATION("Song duration (in seconds) when its real duration is unknown (i.e., song is not in a Songlengths.md5 database).");

		inline constexpr const char* const OPT_SKIP_SHORTER("Auto-skip shorter");
		inline constexpr const char* const DESC_SKIP_SHORTER("Auto-skip (sub)songs with durations below the specified threshold (in seconds). Affected (sub)songs will be indicated with a timer icon. You can still force playback by activating them in the playlist directly. Set to 0 to disable this feature.");

		inline constexpr const char* const OPT_POP_SILENCER("Pop suppression");
		inline constexpr const char* const DESC_POP_SILENCER("Mute duration (in milliseconds) on song start to try reduce the audible pop.");

		inline constexpr const char* const OPT_DRAGDROP_MODE("Drag and drop");
		inline constexpr const char* const DESC_DRAGDROP_MODE("Governs how to handle the external files dropped onto the app window.\n- Dual: drop to window area to replace, drop to playlist area to enqueue.\n- Always replace: drop anywhere to replace.\n- Always enqueue: drop anywhere to enqueue.\n- Disabled: ignore \"drag and dropped\" files.");
		inline constexpr const char* const ITEM_DRAGDROP_MODE_DUAL("Dual");
		inline constexpr const char* const ITEM_DRAGDROP_MODE_REPLACE("Always replace");
		inline constexpr const char* const ITEM_DRAGDROP_MODE_ENQUEUE("Always enqueue");
		inline constexpr const char* const ITEM_DRAGDROP_MODE_DISABLED("Disabled");

		// Visual
		inline constexpr const char* const CATEGORY_VISUAL_BEHAVIOR("Visual");

		inline constexpr const char* const OPT_SELECTION_FOLLOWS_PLAYBACK("Autoselect active song");
		inline constexpr const char* const DESC_SELECTION_FOLLOWS_PLAYBACK("Selection will automatically follow the playback and the active (sub)song will be scrolled into view. Tunes with subsongs will be expanded.");

		inline constexpr const char* const OPT_AUTOEXPAND_SUBSONGS("Autoexpand/collapse subsongs");
		inline constexpr const char* const DESC_AUTOEXPAND_SUBSONGS("Automatically expand/collapse tunes with subsongs upon playback enter/exit.");

		inline constexpr const char* const OPT_TASKBAR_PROGRESS("Progress in Taskbar");
		inline constexpr const char* const DESC_TASKBAR_PROGRESS("Display playback state in the Taskbar.");
		inline constexpr const char* const ITEM_TASKBAR_PROGRESS_ENABLED("Enabled");
		inline constexpr const char* const ITEM_TASKBAR_PROGRESS_COLOR("Color only");
		inline constexpr const char* const ITEM_TASKBAR_PROGRESS_DISABLED("Disabled");

		// HVSC
		inline constexpr const char* const CATEGORY_HVSC("HVSC support");
		inline constexpr const char* const DESC_CATEGORY_HVSC("High Voltage SID Collection support.");

		inline constexpr const char* const OPT_SONGLENGTHS_PATH("Path to Songlengths.md5 file");
		inline constexpr const char* const DESC_SONGLENGTHS_PATH("If missing, a bundled songlengths database will be used instead (which is likely older).");
		inline constexpr const char* const WILDCARD_DESC_MD5("HVSC Songlengths database");

		inline constexpr const char* const OPT_SONGLENGTHS_TRIM("Songlengths trim offset");
		inline constexpr const char* const DESC_SONGLENGTHS_TRIM("Playback duration offset (in milliseconds, limited to 1 second).\nFor example a value of -500 would play the tunes for a half second shorter than the displayed time.");

		// Emulation
		inline constexpr const char* const CATEGORY_EMULATION("Emulation");
		inline constexpr const char* const DESC_CATEGORY_EMULATION("Some tunes (indicated with a chip) require C64 system ROMs to play. Since distribution of the C64 system ROM files is legally gray area, you'll have to source them yourself. They are usually distributed with C64 emulators for example.");

		inline constexpr const char* const OPT_DEFAULT_C64_MODEL("Default C64 model");
		inline constexpr const char* const DESC_DEFAULT_C64_MODEL("- Prefer: C64 model to use if not specified by the tune.\n- Force: ignore tune specification and always use the selected C64 model.\nAdditional notes:\n- First option in the list is recommended.\n- Ongoing playback will stop when changing this setting.");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_PREFER_PAL("Prefer PAL (Europe)");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_PREFER_NTSC("Prefer NTSC (US/JP)");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_PREFER_OLD_NTSC("Prefer OLD NTSC");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_PREFER_DREAN("Prefer Drean (PAL-N)");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_PREFER_PAL_M("Prefer PAL-M (Brazil)");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_FORCE_PAL("Force PAL (Europe)");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_FORCE_NTSC("Force NTSC (US/JP)");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_FORCE_OLD_NTSC("Force OLD NTSC");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_FORCE_DREAN("Force Drean (PAL-N)");
		inline constexpr const char* const ITEM_DEFAULT_C64_MODEL_FORCE_PAL_M("Force PAL-M (Brazil)");

		inline constexpr const char* const OPT_DEFAULT_SID_MODEL("Default SID model");
		inline constexpr const char* const DESC_DEFAULT_SID_MODEL("- Prefer: SID model to use if not specified by the tune.\n- Force: ignore tune specification and always use the selected SID model.\nAdditional notes:\n- First option in the list is recommended.\n- Ongoing playback will stop when changing this setting.");
		inline constexpr const char* const ITEM_DEFAULT_SID_MODEL_PREFER_6581("Prefer MOS 6581");
		inline constexpr const char* const ITEM_DEFAULT_SID_MODEL_PREFER_8580("Prefer MOS 8580");
		inline constexpr const char* const ITEM_DEFAULT_SID_MODEL_FORCE_6581("Force MOS 6581");
		inline constexpr const char* const ITEM_DEFAULT_SID_MODEL_FORCE_8580("Force MOS 8580");

		inline constexpr const char* const OPT_FILTER_ENABLED("Filter enabled");
		inline constexpr const char* const DESC_FILTER_ENABLED("Enable filter emulation.\nNote: ongoing playback will stop when changing this setting.");

		inline constexpr const char* const OPT_FILTER_CURVE_6581("Filter curve (SID 6581)");
		inline constexpr const char* const OPT_FILTER_CURVE_8580("Filter curve (SID 8580)");
		inline constexpr const char* const DESC_FILTER_CURVE_COMMON("Adjust the center frequency value from 0.0 (high/light) to 1.0 (low/dark), default is 0.5.\nNote: ongoing playback will stop when changing this setting.");

		inline constexpr const char* const OPT_BOOSTVOLUME8580("Volume boost (SID 8580)");
		inline constexpr const char* const DESC_BOOSTVOLUME8580("Boost the output volume when the SID 8580 is used so that it plays with approximately the same loudness as a SID 6581.\n\nThe emulated 8580 is around half the loudness of a 6581. No idea why that is, but the libsidplayfp works that way. This option shouldn't cause clipping.");

		inline constexpr const char* const OPT_DIGIBOOST("DigiBoost (SID 8580)");
		inline constexpr const char* const DESC_DIGIBOOST("The SID 8580 performs volume changes silently. Some tunes are using the volume register to play digitized sounds on the older SID 6581. These sounds are very silent (effectively missing!) on a 8580. DigiBoost hack enables a loud volume changing on 8580 (but may have side effects with \"normal\" tunes made for 8580).\nNote: ongoing playback will stop when enabling, but requires app restart to disable.");

		inline constexpr const char* const OPT_ROM_KERNAL_PATH("Path to KERNAL ROM");
		inline constexpr const char* const DESC_ROM_KERNAL_PATH("Some advanced tunes require KERNAL ROM to play. If unavailable, those tunes are indicated with a RED crossout text.\nFor more info see category description.");

		inline constexpr const char* const OPT_ROM_BASIC_PATH("Path to BASIC ROM");
		inline constexpr const char* const DESC_ROM_BASIC_PATH("Minority of tunes require BASIC ROM to play. If unavailable, those tunes are indicated with a BLUE crossout text.\nFor more info see category description.");

		inline constexpr const char* const OPT_ROM_CHARGEN_PATH("Path to CHARGEN ROM");
		inline constexpr const char* const DESC_ROM_CHARGEN_PATH("Character generator ROM file. Not sure how relevant for tune playback, here for completeness since libsidplayfp accepts it.\nFor more info see category description.");

		// System
		inline constexpr const char* const CATEGORY_SYSTEM("System");

		inline constexpr const char* const OPT_MEDIA_KEYS("Use Media Keys");
		inline constexpr const char* const DESC_MEDIA_KEYS("Use the Media Keys to control the playback.\nIf you don't have Media Keys on your keyboard, you can use an utility such as AutoHotkey to emulate them with hotkeys of your choice.");

		inline constexpr const char* const OPT_SINGLE_INSTANCE("Single instance");
		inline constexpr const char* const DESC_SINGLE_INSTANCE("Do not spawn multiple instances of the app, reuse an existing instance whenever possible.");

		inline constexpr const char* const OPT_RESTORE_DEFAULTS("Restore defaults");
		inline constexpr const char* const DESC_RESTORE_DEFAULTS("Enable this to reset the application settings.\nPlease note: the application will close to apply this option.");
	}

	namespace About
	{
		inline constexpr const char* const DESCRIPTION("A GUI player for Commodore 64 SID music files based on libsidplayfp and wxWidgets");
		inline constexpr const char* const DEVELOPER_LIBRARIES("This program uses the following (unaffiliated) Open Source libraries:");
		inline constexpr const char* const ARTISTS_HVSC("Check out the (unaffiliated) High Voltage SID Collection for SID tunes, Songlengths Database and more: https://www.hvsc.c64.org");
		inline constexpr const char* const LICENSE("sidplaywx - A GUI player for Commodore 64 SID music files based on libsidplayfp and wxWidgets.\nCopyright (C) 2021 Jasmin Rutic (bytespiller@gmail.com)\n\nThis program is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program.  If not, see https://www.gnu.org/licenses/gpl-3.0.html");
	}

	namespace Error
	{
		inline constexpr const char* const ERR_INIT_PLAYBACK("Fatal error: failed to initialize playback subsystems.");

		inline constexpr const char* const MSG_ERR_RESET_DEFAULTS_RECOVERY("Critical init failure.\nDo you want to reset all the settings to defaults?\n(Selecting No will close the application.)");
		inline constexpr const char* const MSG_ERR_RESET_DEFAULTS_EXIT("Critical init failure.\nDo you want to reset all the settings to defaults?\n(Note: you will have to relaunch the application.)");

		inline constexpr const char* const MSG_ERR_SONGLENGTHS_NOT_FOUND("Songlengths database not found. Note: we use relative paths, so if you've moved the executable that could be the reason.");
		inline constexpr const char* const MSG_ERR_SONGLENGTHS_INIT_FAILED("Songlengths database is corrupted.");
		inline constexpr const char* const MSG_ERR_SONGLENGTHS_FALLBACK_SUCCESS("A built-in older database will be used from now on instead.");

		inline constexpr const char* const MSG_ERR_ROM_KERNAL("Failed to load the KERNAL ROM file.");
		inline constexpr const char* const MSG_ERR_ROM_BASIC("Failed to load the BASIC ROM file.");
		inline constexpr const char* const MSG_ERR_ROM_CHARGEN("Failed to load the CHARGEN ROM file.");
		inline constexpr const char* const MSG_ERR_ROM_FILE_NOT_FOUND("File not found. Note: we use relative paths, so if you've moved the executable that could be the reason.");

		inline constexpr const char* const MSG_ERR_TUNE_FILE("Unable to read tune file.");

		inline constexpr const char* const MSG_ERR_AUDIO_CONFIG("Incorrect audio device configuration, reverting settings.");
	}

	namespace Internal
	{
		inline constexpr const char* const UNHANDLED_SWITCH_CASE("Unhandled case!");
	}
}
