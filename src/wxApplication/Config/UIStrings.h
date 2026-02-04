/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2021-2026 Jasmin Rutic (bytespiller@gmail.com)
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
// Reminder: "inline" is to avoid unnecessary duplication of string literals in the executable (C++17, retains internal linkage of constexpr that are in the *header* file).

namespace Strings
{
	inline constexpr const char* const APP_VERSION_TAG("0.19.0"); // Reminder: don't forget to increase. Also, don't forget the final zero for x.y.0 versions.

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

		inline constexpr const char* const MENU_ITEM_SUBMENU_PLAYLIST("Playlist");
		inline constexpr const char* const MENU_ITEM_PLAYLIST_OPEN("Open...");
		inline constexpr const char* const MENU_ITEM_PLAYLIST_SAVE("Save As...");
		inline constexpr const char* const MENU_ITEM_PLAYLIST_SHUFFLE("Shuffle");
		inline constexpr const char* const MENU_ITEM_PLAYLIST_CLEAR("Clear");
		inline constexpr const char* const MENU_ITEM_PLAYLIST_RESET_DEMO("Demo songs");

		inline constexpr const char* const MENU_ITEM_EXIT("E&xit");

		inline constexpr const char* const MENU_EDIT("&Edit");
		inline constexpr const char* const MENU_ITEM_FIND("&Find...");
		inline constexpr const char* const MENU_ITEM_FIND_NEXT("Find &Next");
		inline constexpr const char* const MENU_ITEM_FIND_PREV("Find &Prev");
		inline constexpr const char* const MENU_ITEM_PLAYBACK_MODS("&Modify Playback");
		inline constexpr const char* const MENU_ITEM_PREFERENCES("&Preferences");

		inline constexpr const char* const MENU_VIEW("&View");
		inline constexpr const char* const MENU_ITEM_STAY_TOPMOST("&Always on Top");
		inline constexpr const char* const MENU_ITEM_VISUALIZATION_ENABLED("&Oscilloscope");
		inline constexpr const char* const MENU_ITEM_STIL_INFO("&STIL Info");
		inline constexpr const char* const MENU_ITEM_TUNE_INFO("Current Tune &Info...");

		inline constexpr const char* const MENU_HELP("&Help");
		inline constexpr const char* const MENU_ITEM_CHECK_UPDATES("&Check for Updates");
		inline constexpr const char* const MENU_ITEM_ABOUT("&About");

		inline constexpr const char* const REPEAT_MODE_SEQUENTIAL("Sequential");
		inline constexpr const char* const REPEAT_MODE_REPEAT_ONE("Repeat One");
		inline constexpr const char* const REPEAT_MODE_REPEAT_ALL("Repeat All");
		inline constexpr const char* const REPEAT_MODE_LEAVE_RUNNING("Leave Running");
		inline constexpr const char* const REPEAT_MODE_PLAY_ONCE("Play Once");

		inline constexpr const char* const REPEAT_MODE_MENU_ACTION_PLAYLIST_SHUFFLE("Shuffle now");

		inline constexpr const char* const VOL_MENU_PREFIX("Volume:");
		inline constexpr const char* const VOL_SET_MAX("Set Max Volume");
		inline constexpr const char* const VOL_SLIDER_DISABLE("Disable Slider");
		inline constexpr const char* const VOL_SLIDER_ENABLE("Enable Slider");

		inline constexpr const char* const BROWSE_FILES_SUPPORTED("All Supported");
		inline constexpr const char* const BROWSE_FILES_SID("SID Files");
		inline constexpr const char* const BROWSE_FILES_ZIP("Zip Archives");
		inline constexpr const char* const BROWSE_FILES_M3U8("Multimedia Playlist");
		inline constexpr const char* const BROWSE_FILES_ALL("All Files");

		inline constexpr const char* const STATUS_DISCOVERING_FILES("Discovering files...");
		inline constexpr const char* const STATUS_CLEARING_PLAYLIST("Busy clearing playlist...");
		inline constexpr const char* const STATUS_ADDING_FILES_WITH_COUNT("Adding %i files");

		inline constexpr const char* const STATUS_PAUSED("Paused");
		inline constexpr const char* const STATUS_PLAYING("Playing");
		inline constexpr const char* const STATUS_PLAYING_PRERENDER("Playing (instant)");
		inline constexpr const char* const STATUS_MODIFIED_SUFFIX(" (MODIFIED)");
		inline constexpr const char* const STATUS_BYTES_SUFFIX(" bytes");
		inline constexpr const char* const STATUS_SEEKING("Seeking...");
		inline constexpr const char* const STATUS_STOPPED("Stopped");
		inline constexpr const char* const STATUS_ERROR("Error");
		inline constexpr const char* const STATUS_EXTERNAL_FILES_INCOMING("Incoming files...");
		inline constexpr const char* const STATUS_LOADING_STIL("STIL indexing...");

		inline constexpr const char* const UPDATE_CHECK_LATEST("You're up to date!\nVersion %s");
		inline constexpr const char* const UPDATE_CHECK_NEWER("A new release is available!\n\nRelease version: %s\nYour version: v%s\n\nDo you want to open the GitHub release page?");

		inline constexpr const char* const TOOLTIP_PSID_TITLE("Song title");
		inline constexpr const char* const TOOLTIP_PSID_AUTHOR("Song author");
		inline constexpr const char* const TOOLTIP_PSID_COPYRIGHT("Song copyright");

		inline constexpr const char* const TOOLTIP_STIL_NAME_TITLE("STIL Name - Title");
		inline constexpr const char* const TOOLTIP_STIL_ARTIST_AUTHOR("STIL Artist (Author)");
		inline constexpr const char* const TOOLTIP_STIL_COMMENT("STIL Comment");

		inline constexpr const char* const LABEL_PLAYLIST_POS_EMPTY("%i files");
		inline constexpr const char* const LABEL_PLAYLIST_POS_TEMPLATE("File %i / %i");

		inline constexpr const char* const TOOLTIP_BTN_PLAY_PAUSE("Play/Pause");
		inline constexpr const char* const TOOLTIP_BTN_STOP("Stop");
		inline constexpr const char* const TOOLTIP_BTN_PREV_FILE("Previous file");
		inline constexpr const char* const TOOLTIP_BTN_NEXT_FILE("Next file");
		inline constexpr const char* const TOOLTIP_BTN_PREV_SUBSONG("Previous subsong");
		inline constexpr const char* const TOOLTIP_BTN_NEXT_SUBSONG("Next subsong");

		inline constexpr const char* const TOOLTIP_SUBSONG_LABEL("Subsong");
		inline constexpr const char* const TOOLTIP_SHOW_REFRESH_TUNE_INFO("Show/refresh current Tune Info");

		inline constexpr const char* const MSG_MEDIA_KEYS_TAKEN("Media keys already in use by another application.");
	}

	namespace PlaylistTree
	{
		inline constexpr const char* const COLUMN_TITLE("Title");
		inline constexpr const char* const COLUMN_DURATION("Duration");
		inline constexpr const char* const COLUMN_AUTHOR("Author");
		inline constexpr const char* const COLUMN_COPYRIGHT("Copyright");

		inline constexpr const char* const MENU_ITEM_REMOVE("Remove");
		inline constexpr const char* const MENU_ITEM_REMOVE_ALL_ABOVE("Remove all above");
		inline constexpr const char* const MENU_ITEM_REMOVE_ALL_BELOW("Remove all below");

		inline constexpr const char* const MENU_ITEM_BLACKLIST("Skip subsong");
		inline constexpr const char* const MENU_ITEM_UNBLACKLIST("Unskip subsong");

		inline constexpr const char* const MENU_ITEM_EXPAND_ALL("Expand all");
		inline constexpr const char* const MENU_ITEM_COLLAPSE_ALL("Collapse all");

		inline constexpr const char* const MENU_ITEM_SCROLL_TO_CURRENT("Scroll to current");

		inline constexpr const char* const SUBSONG("Subsong");

		inline constexpr const char* const SONG_ICON_TOOLTIP_DEFAULT_SUBSONG("Default subsong");
		inline constexpr const char* const SONG_ICON_TOOLTIP_USES_ROM("Uses ROM");
		inline constexpr const char* const SONG_ICON_TOOLTIP_SKIP_SHORT("Skip short tune");
		inline constexpr const char* const SONG_ICON_TOOLTIP_REMOVE_SONG("Skip subsong");
		inline constexpr const char* const SONG_ICON_TOOLTIP_MUS_OR_STR("C64 Stereo Sidplayer tune component");
		inline constexpr const char* const SONG_ICON_TOOLTIP_MUS_AND_STR("C64 Stereo Sidplayer tune");
	}

	namespace PlaybackMods
	{
		inline constexpr const char* const WINDOW_TITLE("Modify Playback");

		inline constexpr const char* const SID_VOICES_TITLE("Voices");
		inline constexpr const char* const SID_TITLE("SID");
		inline constexpr const char* const SID_VOICE("Voice");
		inline constexpr const char* const SID_DIGI("Digi samples");
		inline constexpr const char* const SID_FILTER("Filter operation");
		inline constexpr const char* const VOICE_ACTIVE("Active");
		inline constexpr const char* const VOICE_MENU_ITEM_SOLO("Solo");
		inline constexpr const char* const VOICES_UNAVAILABLE_PRERENDER("Voices state frozen during playback in Instant seeking mode.");

		inline constexpr const char* const SPEED_SLIDER("Playback Speed (%)");
		inline constexpr const char* const SPEED_SLIDER_MENU_ITEM_RESET("Reset to 100%");
	}

	namespace TuneInfo
	{
		inline constexpr const char* const WINDOW_TITLE("Tune Info");
		inline constexpr const char* const CHECKBOX_FOLLOW_PLAYBACK("Follow navigation");

		inline constexpr const char* const CATEGORY_BASIC("General");
		inline constexpr const char* const TUNE_TITLE("Title");
		inline constexpr const char* const TUNE_AUTHOR("Author");
		inline constexpr const char* const TUNE_RELEASED("Released");
		inline constexpr const char* const TUNE_MUS_COMMENT("MUS comment");
		inline constexpr const char* const TUNE_PATH_FILE("File");

		inline constexpr const char* const CATEGORY_TECHNICAL("Technical");
		inline constexpr const char* const TUNE_ADDR_DRIVER("Driver address");
		inline constexpr const char* const TUNE_ADDR_LOAD("Load address");
		inline constexpr const char* const TUNE_ADDR_INIT("Init address");
		inline constexpr const char* const TUNE_ADDR_PLAY("Play address");
		inline constexpr const char* const TUNE_SIZE("Size data / file");
		inline constexpr const char* const TUNE_SUBSONG("Song (default)");
		inline constexpr const char* const TUNE_MODEL_SID("SID model");
		inline constexpr const char* const TUNE_MODEL_C64("Clock speed");
		inline constexpr const char* const TUNE_TYPE("Type");
		inline constexpr const char* const TUNE_ROM("ROM required");

		inline constexpr const char* const CATEGORY_HVSC("STIL");
		inline constexpr const char* const HVSC_CANONICAL("HVSC");
		inline constexpr const char* const HVSC_NAME("Name");
		inline constexpr const char* const HVSC_TITLE("Title(s)");
		inline constexpr const char* const HVSC_ARTIST("Artist(s)");
		inline constexpr const char* const HVSC_AUTHOR("Author(s)");
		inline constexpr const char* const HVSC_COMMENT("Comment");
		inline constexpr const char* const HVSC_MD5("MD5");
	}

	namespace Preferences
	{
		inline constexpr const char* const WINDOW_TITLE("Preferences");
		inline constexpr const char* const MSG_APP_RESTART_REQUIRED("One or more changes will take effect after the application is restarted.");
		inline constexpr const char* const SUFFIX_NOTE_APP_RESTART("PLEASE NOTE: change becomes effective after app restart.");

		inline constexpr const char* const TITLE_ROM_INFO("Detected ROM");

		// Audio Output
		inline constexpr const char* const CATEGORY_AUDIO_OUTPUT("Audio output");
		inline constexpr const char* const OPT_DEVICE("Device");

#ifdef __WXGTK__
		inline constexpr const char* const DESC_DEVICE("- PulseAudio is recommended.\n- Modifying the playback speed is not supported on all audio backends.\n\nNote: ongoing playback will stop when changing this setting.");
#elif WIN32
		inline constexpr const char* const DESC_DEVICE("- \"MME: Microsoft Sound Mapper\" is recommended (supports bluetooth auto-switch && more).\n- Modifying the playback speed is not supported on all audio backends.\n\nNote: ongoing playback will stop when changing this setting.");
#else
		inline constexpr const char* const DESC_DEVICE("- Modifying the playback speed is not supported on all audio backends.\n\nNote: ongoing playback will stop when changing this setting.");
#endif

		inline constexpr const char* const OPT_LOW_LATENCY("Low latency");
		inline constexpr const char* const DESC_LOW_LATENCY("Enable for more responsive controls.\nDisable if experiencing stuttering.\nNote: ongoing playback will stop when changing this setting.");

		inline constexpr const char* const OPT_OUT_CHANNELS("Channels");
		inline constexpr const char* const DESC_OUT_CHANNELS("- Mono: mono output for all tunes.\n- Normal: stereo for multi-SID tunes.\n- Virtual stereo: wide sound stage (ideal for headphones).");
		inline constexpr const char* const ITEM_OUT_CHANNELS_MONO("Mono");
		inline constexpr const char* const ITEM_OUT_CHANNELS_DEFAULT("Normal");
		inline constexpr const char* const ITEM_OUT_CHANNELS_EXPANDED("Virtual stereo");

		inline constexpr const char* const OPT_OUT_FXVIRTUALSTEREO_SPEAKER_DISTANCE("Virtual sound stage size");
		inline constexpr const char* const DESC_OUT_FXVIRTUALSTEREO_SPEAKER_DISTANCE("Size of the virtual sound stage.\n- Low: small room effect.\n- High: large room effect.\n- Default: 7");

		inline constexpr const char* const OPT_OUT_FXVIRTUALSTEREO_SIDE_VOLUME("Virtual sound stage focus");
		inline constexpr const char* const DESC_OUT_FXVIRTUALSTEREO_SIDE_VOLUME("Volume allocation of sides vs. center.\n- Min: 0.1 = center-focused sound stage (ball).\n- Max: 0.4 = wide-dispersed sound stage (donut).\n- Default: 0.18");

		inline constexpr const char* const OPT_OUT_FXVIRTUALSTEREO_MULTISID("Multi-SID tunes Virtual stereo");
		inline constexpr const char* const DESC_OUT_FXVIRTUALSTEREO_MULTISID("Governs whether the Virtual stereo applies to multi-SID tunes as well.\n- Enable to use the Virtual stereo with multi-SID tunes too (no hard-panning).\n- Disable to use the Multi-SID channel matrix for them instead (Normal). This will also eliminate any edge-case clipping/crackling.\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");

		#pragma region Multi-SID panning matrix (TODO: add support for 4SID once it becomes possible)

		inline constexpr const char* const CATEGORY_MULTISID_PANNING_MATRIX("Multi-SID channel matrix");
		inline constexpr const char* const DESC_CATEGORY_MULTISID_PANNING_MATRIX("Per-chip per-channel volume matrix allows advanced users to tweak multi-SID panning in detail.\nThis applies only to multi-SID tune types (2SID, 3SID, MUS+STR).\n\nThis large category is collapsed by default.");

		// 2SID panning matrix
		inline constexpr const char* const CATEGORY_2SID_PANNING_MATRIX("2SID tunes");
		inline constexpr const char* const DESC_CATEGORY_2SID_PANNING_MATRIX("Per-chip channel volume matrix for tunes utilizing 2 SID chips.");

		inline constexpr const char* const OPT_2SID_FIRST_LEFT("SID 1: Left");
		inline constexpr const char* const DESC_2SID_FIRST_LEFT("SID 1 left volume when playing 2SID tunes.\nDefault: 1.0\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");
		inline constexpr const char* const OPT_2SID_FIRST_RIGHT("SID 1: Right");
		inline constexpr const char* const DESC_2SID_FIRST_RIGHT("SID 1 right volume when playing 2SID tunes.\nDefault: 0.5\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");

		inline constexpr const char* const OPT_2SID_SECOND_LEFT("SID 2: Left");
		inline constexpr const char* const DESC_2SID_SECOND_LEFT("SID 2 left volume when playing 2SID tunes.\nDefault: 0.5\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");
		inline constexpr const char* const OPT_2SID_SECOND_RIGHT("SID 2: Right");
		inline constexpr const char* const DESC_2SID_SECOND_RIGHT("SID 2 right volume when playing 2SID tunes.\nDefault: 1.0\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");

		// 3SID panning matrix
		inline constexpr const char* const CATEGORY_3SID_PANNING_MATRIX("3SID tunes");
		inline constexpr const char* const DESC_CATEGORY_3SID_PANNING_MATRIX("Per-chip channel volume matrix for tunes utilizing 3 SID chips.");

		inline constexpr const char* const OPT_3SID_FIRST_LEFT("SID 1: Left");
		inline constexpr const char* const DESC_3SID_FIRST_LEFT("SID 1 left volume when playing 3SID tunes.\nDefault: 1.0\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");
		inline constexpr const char* const OPT_3SID_FIRST_RIGHT("SID 1: Right");
		inline constexpr const char* const DESC_3SID_FIRST_RIGHT("SID 1 right volume when playing 3SID tunes.\nDefault: 0.5\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");

		inline constexpr const char* const OPT_3SID_SECOND_LEFT("SID 2: Left");
		inline constexpr const char* const DESC_3SID_SECOND_LEFT("SID 2 left volume when playing 3SID tunes.\nDefault: 1.0\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");
		inline constexpr const char* const OPT_3SID_SECOND_RIGHT("SID 2: Right");
		inline constexpr const char* const DESC_3SID_SECOND_RIGHT("SID 2 right volume when playing 3SID tunes.\nDefault: 1.0\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");

		inline constexpr const char* const OPT_3SID_THIRD_LEFT("SID 3: Left");
		inline constexpr const char* const DESC_3SID_THIRD_LEFT("SID 3 left volume when playing 3SID tunes.\nDefault: 0.5\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");
		inline constexpr const char* const OPT_3SID_THIRD_RIGHT("SID 3: Right");
		inline constexpr const char* const DESC_3SID_THIRD_RIGHT("SID 3 right volume when playing 3SID tunes.\nDefault: 1.0\n\nOngoing playback will stop if Instant seeking is enabled when changing this setting while multi-SID tune is playing.");

		#pragma endregion

		// Playback
		inline constexpr const char* const CATEGORY_PLAYBACK_BEHAVIOR("Playback");

		inline constexpr const char* const OPT_PRERENDER("Instant seeking");
		inline constexpr const char* const DESC_PRERENDER("Pre-render the song to allow for instant seeking.\n- Some realtime features (e.g., toggling voices, Leave Running) won't work during playback in this mode.\n- Ongoing playback will stop when changing this setting.\n(This option is also available in a Repeat Mode button's context menu.)");

		inline constexpr const char* const OPT_AUTOPLAY("Autoplay");
		inline constexpr const char* const DESC_AUTOPLAY("- Play added files immediately (unless enqueued).\n- Always start playback on track navigation.");

		inline constexpr const char* const OPT_START_DEFAULT_SUBSONG("Start default subsong");
		inline constexpr const char* const DESC_START_DEFAULT_SUBSONG("Start multi-tunes from their default subsong (indicated with a crown icon, isn't necessarily the first subsong).\nTurn this off to always start a multi-tune from its first subsong.\n(This option is also available in a Repeat Mode button's context menu.)");

		inline constexpr const char* const OPT_INCLUDE_SUBSONGS("Include subsongs");
		inline constexpr const char* const DESC_INCLUDE_SUBSONGS("Sequential playback, looping and Media Keys should include subsongs.\n(This option is also available in a Repeat Mode button's context menu.)");

		inline constexpr const char* const OPT_FALLBACK_DURATION("Fallback duration");
		inline constexpr const char* const DESC_FALLBACK_DURATION("Song duration (in seconds) when its real duration is unknown (i.e., song is not in a Songlengths.md5 database).");

		inline constexpr const char* const OPT_SKIP_SHORTER("Auto-skip shorter");
		inline constexpr const char* const DESC_SKIP_SHORTER("Auto-skip (sub)songs with durations below the specified threshold (in seconds). Affected (sub)songs will be indicated with a timer icon. You can still force playback by activating them in the playlist directly. Set to 0 to disable this feature.\n\nNOTE: displaying lots of timer icons degrades the playlist scrolling performance (wxWidgets issue).");

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

		inline constexpr const char* const OPT_SYSTEM_THEME("System theme");
		inline constexpr const char* const DESC_SYSTEM_THEME("You can enable the experimental dark theme on Windows here.\nDark theme will be improved in the future.");
		inline constexpr const char* const ITEM_SYSTEM_THEME_AUTO("Auto (experimental)");
		inline constexpr const char* const ITEM_SYSTEM_THEME_LIGHT("Light");
		inline constexpr const char* const ITEM_SYSTEM_THEME_DARK("Dark (experimental)");

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

		inline constexpr const char* const OPT_STIL_PATH("Path to STIL.txt file");
		inline constexpr const char* const DESC_STIL_PATH("If missing, a bundled STIL database will be used instead (which is likely older).");
		inline constexpr const char* const WILDCARD_DESC_STIL_TXT("STIL text file");

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

		inline constexpr const char* const OPT_NTSC_C64_MODEL_MUS("Use NTSC for MUS");
		inline constexpr const char* const DESC_NTSC_C64_MODEL_MUS("Old MUS-format tunes were predominately created on NTSC C64 models and thus sound too slow on PAL models.\n- Ongoing playback will stop when changing this setting.");

		inline constexpr const char* const OPT_FILTER_CURVE_6581("Filter curve (SID 6581)");
		inline constexpr const char* const OPT_FILTER_CURVE_8580("Filter curve (SID 8580)");
		inline constexpr const char* const DESC_FILTER_CURVE_COMMON("Adjust the center frequency value from 0.0 (high/light) to 1.0 (low/dark), default is 0.5.\nNote: ongoing playback will stop when changing this setting.");

		inline constexpr const char* const OPT_DIGIBOOST("DigiBoost (SID 8580)");
		inline constexpr const char* const DESC_DIGIBOOST("The SID 8580 performs volume changes silently. Some tunes are using the volume register to play digitized sounds on the older SID 6581. These sounds are very silent (effectively missing!) on a 8580. DigiBoost hack enables a loud volume changing on 8580 (but may have side effects with \"normal\" tunes made for 8580).\nNote: ongoing playback will stop when changing this setting.");

		inline constexpr const char* const OPT_ROM_KERNAL_PATH("Path to KERNAL ROM");
		inline constexpr const char* const DESC_ROM_KERNAL_PATH("Some advanced tunes require KERNAL ROM to play. If unavailable, those tunes are indicated with a RED crossout text.\nFor more info see category description.");

		inline constexpr const char* const OPT_ROM_BASIC_PATH("Path to BASIC ROM");
		inline constexpr const char* const DESC_ROM_BASIC_PATH("Minority of tunes require BASIC ROM to play. If unavailable, those tunes are indicated with a BLUE crossout text.\nFor more info see category description.");

		inline constexpr const char* const OPT_ROM_CHARGEN_PATH("Path to CHARGEN ROM");
		inline constexpr const char* const DESC_ROM_CHARGEN_PATH("Character generator ROM file. Not sure how relevant for tune playback, here for completeness since libsidplayfp accepts it.\nFor more info see category description.");

		// Application
		inline constexpr const char* const CATEGORY_APPLICATION("Application");

		inline constexpr const char* const OPT_REMEMBER_PLAYLIST("Remember playlist");
		inline constexpr const char* const DESC_REMEMBER_PLAYLIST("Restore the previous song list on app launch.");

		inline constexpr const char* const OPT_MEDIA_KEYS("Use Media keys");
		inline constexpr const char* const DESC_MEDIA_KEYS("Use the Media keys to control the playback.\nIf you don't have Media keys on your keyboard, you can use an utility such as AutoHotkey to emulate them with hotkeys of your choice.");

		inline constexpr const char* const OPT_SINGLE_INSTANCE("Single instance");
		inline constexpr const char* const DESC_SINGLE_INSTANCE("Do not spawn multiple instances of the app, reuse an existing instance whenever possible.");

		inline constexpr const char* const OPT_RESTORE_DEFAULTS("Restore defaults");
		inline constexpr const char* const DESC_RESTORE_DEFAULTS("Enable this to reset the application settings.\n- Please note: the application will close to apply this option.");
		inline constexpr const char* const DESC_RESTORE_DEFAULTS_LINUX("- Preferences file: ");
	}

	namespace About
	{
		inline constexpr const char* const DESCRIPTION("A GUI player for Commodore 64 SID music files based on libsidplayfp and wxWidgets");
		inline constexpr const char* const DEVELOPER_LIBRARIES("This program uses the following Open Source libraries:");
		inline constexpr const char* const HVSC("Check out the (unaffiliated) High Voltage SID Collection for SID tunes, Songlengths Database and more: https://www.hvsc.c64.org");
		inline constexpr const char* const LICENSE("sidplaywx - A GUI player for Commodore 64 SID music files based on libsidplayfp and wxWidgets.\nCopyright (C) 2021-2026 Jasmin Rutic (bytespiller@gmail.com)\n\nThis program is free software: you can redistribute it and/or modify\nit under the terms of the GNU General Public License as published by\nthe Free Software Foundation, either version 3 of the License, or\n(at your option) any later version.\n\nThis program is distributed in the hope that it will be useful,\nbut WITHOUT ANY WARRANTY; without even the implied warranty of\nMERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the\nGNU General Public License for more details.\n\nYou should have received a copy of the GNU General Public License\nalong with this program.  If not, see https://www.gnu.org/licenses/gpl-3.0.html");
	}

	namespace Error
	{
		inline constexpr const char* const ERR_INIT_PLAYBACK("Fatal error: failed to initialize playback subsystems.");

		inline constexpr const char* const MSG_ERR_RESET_DEFAULTS_RECOVERY("Critical init failure.\nDo you want to reset all settings to defaults?\n\n(Selecting \"No\" will close the application so you can retry with original settings.)");
		inline constexpr const char* const MSG_ERR_RESET_DEFAULTS_EXIT("Critical init failure.\nDo you want to reset all settings to defaults?\n(Note: you will have to relaunch the application.)");

		inline constexpr const char* const MSG_ERR_SONGLENGTHS_NOT_FOUND("Songlengths database not found. Note: we use relative paths, so if you've moved the executable that could be the reason.");
		inline constexpr const char* const MSG_ERR_SONGLENGTHS_INIT_FAILED("Songlengths database is corrupted.");
		inline constexpr const char* const MSG_ERR_SONGLENGTHS_FALLBACK_SUCCESS("A built-in songlengths database will be used from now on instead.");
		inline constexpr const char* const MSG_ERR_SONGLENGTHS_FALLBACK_FAILED("Songlengths database unavailable. Other depending functionality (such as STIL) also won't work.");

		inline constexpr const char* const MSG_ERR_STIL_NOT_FOUND("STIL database not found. Note: we use relative paths, so if you've moved the executable that could be the reason.");
		inline constexpr const char* const MSG_ERR_STIL_INIT_FAILED("STIL database is corrupted.");
		inline constexpr const char* const MSG_ERR_STIL_FALLBACK_SUCCESS("A built-in STIL database will be used from now on instead.");

		inline constexpr const char* const MSG_ERR_ROM_KERNAL("Failed to load the KERNAL ROM file.");
		inline constexpr const char* const MSG_ERR_ROM_BASIC("Failed to load the BASIC ROM file.");
		inline constexpr const char* const MSG_ERR_ROM_CHARGEN("Failed to load the CHARGEN ROM file.");
		inline constexpr const char* const MSG_ERR_ROM_FILE_NOT_FOUND("File not found. Note: we use relative paths, so if you've moved the executable that could be the reason.");
		inline constexpr const char* const MSG_ERR_ROM_FILE_REQUIRED("This tune requires a %s ROM file.\nPlease check the Emulation page in Preferences to set the ROM file.\n\n%s");

		inline constexpr const char* const MSG_ERR_TUNE_FILE("Unable to read tune file.");

		inline constexpr const char* const MSG_ERR_AUDIO_CONFIG("Incorrect audio device configuration, reverting settings.");
		inline constexpr const char* const MSG_ERR_AUDIO_OUTPUT("Playback failed. Try choosing another audio output device in Preferences.");
	}

	namespace Common
	{
		inline constexpr const char* const ACTION_BROWSE_LOCATION("Browse location");
		inline constexpr const char* const GENERIC_NOT_FOUND("Not found");
		inline constexpr const char* const GENERIC_RETRY("Retry");
	}

	namespace Internal
	{
		inline constexpr const char* const UNHANDLED_SWITCH_CASE("Unhandled case!");
	}
}
