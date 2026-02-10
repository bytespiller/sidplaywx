/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2026 Jasmin Rutic (bytespiller@gmail.com)
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

#include "Playlist.h"
#include "../../Config/UIStrings.h"
#include <wx/renderer.h>

#include <chrono>
#include <random>

namespace UIElements
{
	namespace Playlist
	{
		static constexpr unsigned int COL_PADDING = 10; // Column padding in the respective wxDataViewCtrl, we use this hardcoded value here to avoid PITA.

		using ColumnId = PlaylistTreeModel::ColumnId;

		Playlist::Playlist(wxPanel* parent, const PlaylistIcons& playlistIcons, Settings::AppSettings& appSettings, unsigned long style) :
			wxDataViewCtrl(parent, wxID_ANY, wxDefaultPosition, wxDefaultSize, style),
			_model(*new PlaylistTreeModel(playlistIcons)),
			_appSettings(appSettings)
		{
			AssociateModel(&_model);

			const wxDataViewColumn* const iconColumn = _AddBitmapColumn(ColumnId::Icon);
			_AddTextColumn(ColumnId::Title, Strings::PlaylistTree::COLUMN_TITLE);
			_AddTextColumn(ColumnId::Duration, Strings::PlaylistTree::COLUMN_DURATION);
			_AddTextColumn(ColumnId::Author, Strings::PlaylistTree::COLUMN_AUTHOR);
			_AddTextColumn(ColumnId::Copyright, Strings::PlaylistTree::COLUMN_COPYRIGHT);

#ifdef WIN32
			Bind(wxEVT_MOUSEWHEEL, &_OverrideScrollWheel, this); // Partial workaround for the smooth scrolling performance issues on MSW (especially with lots of icons in rows).
#endif

			// Auto-fit the Title column upon the child item expansion
			Bind(wxEVT_DATAVIEW_ITEM_EXPANDED, [this](const wxDataViewEvent& /*evt*/)
			{
				AutoFitTextColumn(ColumnId::Title);
			});

			// Handle sorting
			Bind(wxEVT_DATAVIEW_COLUMN_HEADER_CLICK, [this](const wxDataViewEvent& evt)
			{
				if (wxDataViewColumn* const col = evt.GetDataViewColumn())
				{
					_SortByColumn(*col);
				}
			});

			// Handle the status icon tooltips
			const int headerHeight = wxRendererNative::Get().GetHeaderButtonHeight(this);
			GetMainWindow()->Bind(wxEVT_MOTION, [this, iconColumn, headerHeight](const wxMouseEvent& evt)
			{
				wxPoint pos(evt.GetPosition()); // Alternatively, doesn't need applying the headerHeight offset, but is sufficient on MSW only: ScreenToClient(GetMainWindow()->ClientToScreen(evt.GetPosition()));
				pos.y += headerHeight;

				wxDataViewItem item;
				wxDataViewColumn* col = nullptr;
				HitTest(pos, item, col);

				if (item.IsOk() && col == iconColumn)
				{
					if (_lastTooltipItem != item) // Update the tooltip only if hovering over another item.
					{
						const PlaylistTreeModelNode* const modelNode = PlaylistTreeModel::TreeItemToModelNode(item);
						const PlaylistIconId iconId = modelNode->GetIconId();
						if (iconId > PlaylistIconId::NoIcon)
						{
							wxString tooltip(_model.GetPlaylistIcons().GetIconList().at(iconId).tooltip);

							switch (iconId)
							{
								case PlaylistIconId::ChipIcon:
									switch (modelNode->romRequirement)
									{
										case PlaylistTreeModelNode::RomRequirement::BasicRom:
											tooltip.Append(" (BASIC)");
											break;
										case PlaylistTreeModelNode::RomRequirement::R64:
											tooltip.Append(" (KERNAL)");
											break;
									}
									break;

								case PlaylistIconId::MusOrStr:
									switch (modelNode->GetTag())
									{
										case PlaylistTreeModelNode::ItemTag::MUS_StandaloneMus:
											tooltip.Append(" (MUS)");
											break;
										case PlaylistTreeModelNode::ItemTag::MUS_StandaloneStr:
											tooltip.Append(" (STR)");
											break;
									}
							}

							_lastTooltipItem = item;
							SetToolTip(tooltip);
							SetCursor(wxCURSOR_QUESTION_ARROW);
						}
						else if (_lastTooltipItem.IsOk()) // Hovering over item without icon. The condition avoids clearing when already cleared.
						{
							_ClearTooltip();
						}
					}
				}
				else if (_lastTooltipItem.IsOk()) // Hovering over irrelevant portion (e.g., not an icon column). The condition avoids clearing when already cleared.
				{
					_ClearTooltip();
				}
			});
		}

		wxWindow* Playlist::GetWxWindow()
		{
			return this;
		}

		void Playlist::AutoFitTextColumn(ColumnId column)
		{
			const unsigned int newWidth = _GetBestTextColumnWidth(column);
			wxDataViewColumn* const col = GetColumn(static_cast<unsigned int>(column));
			col->SetWidth(newWidth);
		}

		void Playlist::Shuffle()
		{
			// Shuffle the model
			if (_model.entries.size() > 1)
			{
				// Snapshot the current items' order
				std::vector<unsigned int> prevOrder(_model.entries.size());
				std::transform(_model.entries.cbegin(), _model.entries.cend(), prevOrder.begin(), [](const PlaylistTreeModelNodePtr& node) { return node->uid; });

				// Do the shuffle (Mersenne Twister)
				std::shuffle(_model.entries.begin(), _model.entries.end(), std::mt19937(std::chrono::system_clock::now().time_since_epoch().count()));

				// Detect if shuffle resulted in unchanged order (can often happen with less items)
				const bool unchangedOrder = std::equal(prevOrder.cbegin(), prevOrder.cend(), _model.entries.cbegin(), [](unsigned int uid, const PlaylistTreeModelNodePtr& node) { return uid == node->uid; } );
				if (unchangedOrder)
				{
					Shuffle(); // Re-shuffle
					return;
				}
			}

			//
			_ResetColumnSortingIndicator();

			// Notify the wx base control of change
			_model.Cleared();

			// Notify the parent (we need to refresh the transport buttons state after sorting in case the first/last positions are swapped)
			GetEventHandler()->AddPendingEvent(wxDataViewEvent(wxEVT_DATAVIEW_COLUMN_SORTED, this, nullptr));
		}

		int Playlist::_GetBestTextColumnWidth(ColumnId column)
		{
			wxClientDC dc(this);
			dc.SetFont(GetFont().MakeBold());

			int width = std::max(10, dc.GetTextExtent(GetColumn(static_cast<unsigned int>(column))->GetTitle()).GetWidth()); // Initial value is minimum auto-width (fit column title).

			{
				const auto DoUpdateWidth = [&](const wxDataViewItem& item) -> void
				{
					wxVariant text;
					_model.GetValue(text, item, static_cast<unsigned int>(column));

					const wxSize& size = dc.GetTextExtent(text.GetString());
					width = std::max(width, size.GetWidth());
				};

				std::for_each(_model.entries.cbegin(), _model.entries.cend(), [&](const PlaylistTreeModelNodePtr& song)
				{
					const wxDataViewItem& songNode = PlaylistTreeModel::ModelNodeToTreeItem(*song);
					if (!songNode.IsOk())
					{
						return;
					}

					// Update by subsongs' titles (if parent expanded)
					const PlaylistTreeModelNodePtrArray& subsongs = song->GetChildren();
					if (!subsongs.empty() && IsExpanded(songNode))
					{
						std::for_each(subsongs.cbegin(), subsongs.cend(), [&](const PlaylistTreeModelNodePtr& subsong)
						{
							const wxDataViewItem& subsongNode = PlaylistTreeModel::ModelNodeToTreeItem(*subsong);
							DoUpdateWidth(subsongNode);
						});
					}

					// Update by the main song title
					DoUpdateWidth(songNode);
				});
			}

			return width + COL_PADDING;
		}

		PlaylistTreeModelNode& Playlist::AddMainSong(const wxString& title, const wxString& filepath, int defaultSubsong, uint_least32_t duration, const wxString& hvscPath, const char* md5, const wxString& author, const wxString& copyright, PlaylistTreeModelNode::RomRequirement romRequirement, bool playable, const wxString& musCompanionStrFilePath)
		{
			_ResetColumnSortingIndicator();

			// Create item
			_model.entries.emplace_back(new PlaylistTreeModelNode(_NextFreeItemUid(), nullptr, title, filepath, defaultSubsong, duration, hvscPath, md5, author, copyright, romRequirement, playable, musCompanionStrFilePath));

			// Notify the wx base control of change
			wxDataViewItem childNotify = wxDataViewItem(_model.entries.back().get());
			_model.ItemAdded(wxDataViewItem(0), childNotify);

			// Return the just-created item for convenience
			return *_model.entries.back().get();
		}

		void Playlist::AddSubsongs(const std::vector<uint_least32_t>& durations, const std::vector<wxString>& titles, PlaylistTreeModelNode& parent)
		{
			assert(durations.size() == titles.size());

			if (durations.size() == 0)
			{
				return;
			}

			wxDataViewItemArray notifyItems(durations.size());
			const auto _ = _model.PrepareDirty([&]()
			{
				_model.ItemsAdded(wxDataViewItem(&parent), notifyItems); // Notify the wx base control of change (MSW).
			});

			// Create multiple items at once
			{
				int cnt = 0;
				for (const uint_least32_t duration : durations)
				{
					++cnt;
					PlaylistTreeModelNode& newChildNode = parent.AddChild(new PlaylistTreeModelNode(_NextFreeItemUid(), &parent, titles.at(cnt - 1), parent.filepath, cnt, duration, parent.hvscPath, parent.md5, "", "", parent.romRequirement, parent.IsPlayable(), parent.musCompanionStrFilePath), {});
					notifyItems.Add(wxDataViewItem(&newChildNode));

					// Indicate if default subsong
					if (parent.defaultSubsong == cnt)
					{
						newChildNode.SetIconId((newChildNode.musCompanionStrFilePath.IsEmpty()) ? PlaylistIconId::DefaultSubsongIndicator : PlaylistIconId::MusAndStr, {});
					}
				}
			}
		}

		void Playlist::Remove(PlaylistTreeModelNode* item)
		{
			void* parent = item->GetParent(); // Remember the parent (or nullptr if no parent) here *before* the item is removed.

			if (PlaylistTreeModelNode* const activeSong = GetActiveSong())
			{
				if (item->uid == activeSong->uid || (activeSong->type == PlaylistTreeModelNode::ItemType::Subsong && item->uid == activeSong->GetParent()->uid))
				{
					_activeItem.Unset();
				}
			}

			// Find and remove the item from the model (in case of a main song it is removed from the root, in case of a subsong it is removed from its parent main song)
			const auto it = std::find_if(_model.entries.cbegin(), _model.entries.cend(), [&item](const PlaylistTreeModelNodePtr& qItemNode) { return qItemNode.get() == item; });
			if (it != _model.entries.cend())
			{
				_model.entries.erase(it); // "item" is now invalid (but not nullptr), do not access it beyond this point.
			}

			// Notify the wx base control of change
			_model.ItemDeleted(static_cast<wxDataViewItem>(parent), static_cast<wxDataViewItem>(item));
		}

		void Playlist::Clear()
		{
			_ResetColumnSortingIndicator();

			_activeItem.Unset();

			// Clear all entries (entries are unique ptrs so they'll be destroyed since the vector is their owner)
			_model.entries.clear();

			// Notify the wx base control of change
			_model.Cleared();
		}

		void Playlist::ExpandSongNode(const PlaylistTreeModelNode& node)
		{
			Expand(PlaylistTreeModel::ModelNodeToTreeItem(node));
		}

		void Playlist::ExpandAll()
		{
			for (const PlaylistTreeModelNodePtr& node : GetSongs())
			{
				Expand(PlaylistTreeModel::ModelNodeToTreeItem(*node.get()));
			}
		}

		void Playlist::CollapseAll()
		{
			for (const PlaylistTreeModelNodePtr& node : GetSongs())
			{
				Collapse(PlaylistTreeModel::ModelNodeToTreeItem(*node.get()));
			}
		}

		PlaylistTreeModelNode* Playlist::GetEffectiveInitialSubsong(const PlaylistTreeModelNode& mainSongItem) const
		{
			assert(mainSongItem.type == PlaylistTreeModelNode::ItemType::Song);
			PlaylistTreeModelNode& node = const_cast<PlaylistTreeModelNode&>(mainSongItem);

			// No subsongs, return self.
			if (node.GetSubsongCount() == 0)
			{
				if (node.IsAutoPlayable())
				{
					return &node;
				}

				return nullptr;
			}

			// Try return effective default subsong if it's playable.
			{
				const int preferredStartSubsong = (_appSettings.GetOption(Settings::AppSettings::ID::RepeatModeDefaultSubsong)->GetValueAsBool()) ? node.defaultSubsong : 1;
				PlaylistTreeModelNode* preferredStartSubsongItem = &node.GetSubsong(preferredStartSubsong);

				if (preferredStartSubsongItem != nullptr && preferredStartSubsongItem->IsAutoPlayable())
				{
					return preferredStartSubsongItem;
				}
			}

			// Effective default subsong is not playable, so find the first playable one.
			const PlaylistTreeModelNodePtrArray& subsongs = node.GetChildren();
			const auto itSubsong = std::find_if(subsongs.cbegin(), subsongs.cend(), [](const PlaylistTreeModelNodePtr& subsongItem)
			{
				return subsongItem->IsAutoPlayable();
			});

			if (itSubsong == subsongs.cend())
			{
				return nullptr;
			}

			return itSubsong->get();
		}

		const PlaylistTreeModelNodePtrArray& Playlist::GetSongs() const
		{
			return _model.entries;
		}

		PlaylistTreeModelNode* Playlist::GetSongAtPlaylistPosition(unsigned int position) const
		{
			if (position == 0 || position > _model.entries.size())
			{
				return nullptr;
			}

			return _model.entries.at(position - 1).get();
		}

		PlaylistTreeModelNode& Playlist::GetSubsong(PlaylistTreeModelNode& song, int subsong) const
		{
			PlaylistTreeModelNode& mainSongNode = (song.GetParent() == nullptr) ? song : *song.GetParent();

			if (subsong == 0)
			{
				return *GetEffectiveInitialSubsong(mainSongNode);
			}

			return mainSongNode.GetSubsong(subsong);
		}

		PlaylistTreeModelNode* Playlist::GetActiveSong() const
		{
			if (_activeItem.IsOk())
			{
				return PlaylistTreeModel::TreeItemToModelNode(_activeItem);
			}

			return nullptr;
		}

		PlaylistTreeModelNode* Playlist::GetNextSong(PlaylistTreeModelNode& fromSong) const
		{
			PlaylistTreeModelNode& mainSongNode = (fromSong.GetParent() == nullptr) ? fromSong : *fromSong.GetParent();

			const auto itCurrent = std::find_if(_model.entries.cbegin(), _model.entries.cend(), [&mainSongNode](const PlaylistTreeModelNodePtr& cSongNode)
			{
				return cSongNode->uid == mainSongNode.uid;
			});

			if (itCurrent == _model.entries.cend())
			{
				return nullptr; // Not found (should never happen).
			}

			const auto itNext = std::next(itCurrent);
			if (itNext == _model.entries.cend())
			{
				return nullptr; // No further items.
			}

			PlaylistTreeModelNode* const nextSong = itNext->get(); // Here we know for sure there is at least one more item.
			if (nextSong->IsAutoPlayable())
			{
				return GetEffectiveInitialSubsong(*nextSong);
			}

			return GetNextSong(*nextSong); // Not playable so try the one after it.
		}

		PlaylistTreeModelNode* Playlist::GetNextSong() const
		{
			PlaylistTreeModelNode* const activeSong = GetActiveSong();
			if (activeSong == nullptr)
			{
				return nullptr;
			}

			PlaylistTreeModelNode* const mainSongNode = (activeSong->GetParent() == nullptr) ? activeSong : activeSong->GetParent();
			return GetNextSong(*mainSongNode);
		}

		PlaylistTreeModelNode* Playlist::GetPrevSong(PlaylistTreeModelNode& fromSong) const
		{
			PlaylistTreeModelNode& mainSongNode = (fromSong.GetParent() == nullptr) ? fromSong : *fromSong.GetParent();

			const auto itCurrent = std::find_if(_model.entries.cbegin(), _model.entries.cend(), [&mainSongNode](const PlaylistTreeModelNodePtr& cSongNode)
			{
				return cSongNode->uid == mainSongNode.uid;
			});

			if (itCurrent == _model.entries.cend() || itCurrent == _model.entries.cbegin()) // Not found (should never happen) or there aren't any previous items.
			{
				return nullptr;
			}

			PlaylistTreeModelNode* const prevSong = std::prev(itCurrent)->get(); // Here we know for sure there is at least one previous item.
			if (prevSong->IsAutoPlayable())
			{
				return GetEffectiveInitialSubsong(*prevSong);
			}

			return GetPrevSong(*prevSong); // Not playable so try the one before it.
		}

		PlaylistTreeModelNode* Playlist::GetPrevSong() const
		{
			PlaylistTreeModelNode* const activeSong = GetActiveSong();
			if (activeSong == nullptr)
			{
				return nullptr;
			}

			PlaylistTreeModelNode* const mainSongNode = (activeSong->GetParent() == nullptr) ? activeSong : activeSong->GetParent();
			return GetPrevSong(*mainSongNode);
		}

		PlaylistTreeModelNode* Playlist::GetNextSubsong(PlaylistTreeModelNode& fromSubsong) const
		{
			PlaylistTreeModelNode& mainSongNode = (fromSubsong.GetParent() == nullptr) ? fromSubsong : *fromSubsong.GetParent();

			if (fromSubsong.defaultSubsong >= mainSongNode.GetSubsongCount() || !fromSubsong.musCompanionStrFilePath.IsEmpty())
			{
				return nullptr; // No more subsongs, or the file is MUS/STR so there are no real subsongs.
			}

			PlaylistTreeModelNode& nextSubsong = GetSubsong(fromSubsong, fromSubsong.defaultSubsong + 1); // Here we know for sure there is at least one more item.
			if (nextSubsong.IsAutoPlayable())
			{
				return &nextSubsong;
			}

			return GetNextSubsong(nextSubsong); // Not playable so try the one after it.
		}

		PlaylistTreeModelNode* Playlist::GetNextSubsong() const
		{
			PlaylistTreeModelNode* const activeSong = GetActiveSong();
			if (activeSong == nullptr)
			{
				return nullptr;
			}

			return GetNextSubsong(*activeSong);
		}

		PlaylistTreeModelNode* Playlist::GetPrevSubsong(PlaylistTreeModelNode& fromSubsong) const
		{
			if (fromSubsong.defaultSubsong <= 1 || !fromSubsong.musCompanionStrFilePath.IsEmpty())
			{
				return nullptr; // No more subsongs, or the file is MUS/STR so there are no real subsongs.
			}

			PlaylistTreeModelNode& prevSubsong = GetSubsong(fromSubsong, fromSubsong.defaultSubsong - 1); // Here we know for sure there is at least one previous item.
			if (prevSubsong.IsAutoPlayable())
			{
				return &prevSubsong;
			}

			return GetPrevSubsong(prevSubsong); // Not playable so try the one before it.
		}

		PlaylistTreeModelNode* Playlist::GetPrevSubsong() const
		{
			PlaylistTreeModelNode* const activeSong = GetActiveSong();
			if (activeSong == nullptr)
			{
				return nullptr;
			}

			return GetPrevSubsong(*activeSong);
		}

		int Playlist::GetMainSongPlaylistPosition(PlaylistTreeModelNode& song) const
		{
			const unsigned int targetUid = ((song.GetParent() == nullptr) ? song : *song.GetParent()).uid;

			const auto itTargetSong = std::find_if(_model.entries.cbegin(), _model.entries.cend(), [targetUid](const PlaylistTreeModelNodePtr& songItem)
			{
				return songItem->uid == targetUid;
			});

			if (itTargetSong == _model.entries.cend())
			{
				return -1;
			}

			return std::distance(_model.entries.cbegin(), itTargetSong) + 1;
		}

		bool Playlist::TrySetActiveSong(const PlaylistTreeModelNode& node, bool autoexpand)
		{
			if (!node.IsPlayable())
			{
				return false;
			}

			wxDataViewItemArray notifyItems;

			// Handle old node
			if (_activeItem.IsOk())
			{
				PlaylistTreeModelNode& oldNode = *GetActiveSong();
				notifyItems.Add(wxDataViewItem(&oldNode));

				// Collapse old if necessary
				if (autoexpand && oldNode.type == PlaylistTreeModelNode::ItemType::Subsong)
				{
					Collapse(_model.GetParent(_activeItem)); // Collapse parent item.
				}

				// Un-highlight the old node
				oldNode.ResetItemAttr({});

				// Un-highlight the parent item
				PlaylistTreeModelNode* parent = oldNode.GetParent();
				if (parent != nullptr)
				{
					notifyItems.Add(wxDataViewItem(parent));
					parent->ResetItemAttr({});
				}
			}

			// Highlight new node
			_activeItem = PlaylistTreeModel::ModelNodeToTreeItem(node);
			notifyItems.Add(_activeItem);
			GetActiveSong()->GetItemAttr({}).SetBold(true);

			// Also highlight the parent node if this is a child node
			if (node.type == PlaylistTreeModelNode::ItemType::Subsong)
			{
				PlaylistTreeModelNode* const activeParent = GetActiveSong()->GetParent();
				notifyItems.Add(wxDataViewItem(activeParent));

				wxDataViewItemAttr& attr = activeParent->GetItemAttr({});
				attr.SetBold(true);
				attr.SetColour(wxSystemSettings::GetColour(wxSYS_COLOUR_HOTLIGHT)); // TODO: define the color in the theme XML instead of here.
			}

			// Expand new if necessary
			if (autoexpand && node.type == PlaylistTreeModelNode::ItemType::Subsong)
			{
				Expand(_model.GetParent(_activeItem)); // Expand parent item.
			}

			// Must be done at the end
#ifdef __WXGTK__
			_model.ItemsChanged(notifyItems); // Not necessary on MSW.
#endif
			Refresh();

			return true;
		}

		bool Playlist::IsEmpty() const
		{
			return _model.entries.empty();
		}

		void Playlist::SetItemTag(PlaylistTreeModelNode& node, PlaylistTreeModelNode::ItemTag tag, bool force)
		{
			if (!force && !node.IsPlayable())
			{
				return;
			}

			node.SetTag(tag, {});
			if (force)
			{
				node.ResetItemAttr({}); // Reset attributes only on force, so that the context menu actions don't remove the bold styling for hard-selected items.
			}

			// Apply icon & styling attributes
			switch (tag)
			{
				case PlaylistTreeModelNode::ItemTag::Normal:
				{
					// Reset the icon
					{
						const bool itemIsDefaultSubsong = node.type == PlaylistTreeModelNode::ItemType::Subsong && node.defaultSubsong == node.GetParent()->defaultSubsong;
						const PlaylistIconId nodeIconId = (itemIsDefaultSubsong)
							? ((node.musCompanionStrFilePath.IsEmpty()) ? PlaylistIconId::DefaultSubsongIndicator : PlaylistIconId::MusAndStr)
							: PlaylistIconId::NoIcon;

						node.SetIconId(nodeIconId, {});
					}

					if (node.romRequirement != PlaylistTreeModelNode::RomRequirement::None)
					{
						// Set chip icon to the main/single song only
						if (node.type == PlaylistTreeModelNode::ItemType::Song)
						{
							node.SetIconId(PlaylistIconId::ChipIcon, {});
						}

						// Apply unplayable styling
						if (!node.IsPlayable())
						{
							const wxColour col = (node.romRequirement == PlaylistTreeModelNode::RomRequirement::BasicRom) ? "#054a80" : "#8a5454"; // TODO: these colors should probably be defined in the theme XML and not hardcoded here.
							node.GetItemAttr({}).SetColour(col);
							node.GetItemAttr({}).SetStrikethrough(true);

							// Apply to any subsongs too
							for (const PlaylistTreeModelNodePtr& subnode : node.GetChildren())
							{
								subnode->GetItemAttr({}).SetColour(col);
								subnode->GetItemAttr({}).SetStrikethrough(true);
							}
						}
					}

					break;
				}
				case PlaylistTreeModelNode::ItemTag::ShortDuration:
				{
					node.SetIconId(PlaylistIconId::SkipShort, {});
					break;
				}
				case PlaylistTreeModelNode::ItemTag::Blacklisted:
				{
					node.SetIconId(PlaylistIconId::RemoveSong, {});
					break;
				}
				case PlaylistTreeModelNode::ItemTag::MUS_StandaloneMus: [[fallthrough]];
				case PlaylistTreeModelNode::ItemTag::MUS_StandaloneStr:
				{
					node.SetIconId(PlaylistIconId::MusOrStr, {});
					break;
				}
			}

			_model.ItemChanged(wxDataViewItem(&node)); // Refresh icon immediately.
		}

		bool Playlist::Select(const PlaylistTreeModelNode& node)
		{
			const wxDataViewItem item = PlaylistTreeModel::ModelNodeToTreeItem(node);
			if (item.IsOk())
			{
				wxDataViewCtrl::Select(item);
				return true;
			}

			return false;
		}

		bool Playlist::EnsureVisible(const PlaylistTreeModelNode& node)
		{
			const wxDataViewItem item = PlaylistTreeModel::ModelNodeToTreeItem(node);
			if (item.IsOk())
			{
				CallAfter([this, item]() // Skip frame, otherwise it jumps around on wxGTK. Also, the item must not be captured by reference here.
				{
					wxDataViewCtrl::EnsureVisible(item);
				});

				return true;
			}

			return false;
		}

		wxDataViewColumn* Playlist::_AddBitmapColumn(ColumnId column, wxAlignment align, int flags)
		{
#ifdef WIN32
			static constexpr int PAD = 0;
#else
			static constexpr int PAD = COL_PADDING;
#endif
			static const int COL_WIDTH = (48 + PAD) * GetDPIScaleFactor(); // Col width 48 comes from: 16 * 3 where 16 is playlist icon size and 3 is magic number.
			return AppendBitmapColumn(wxEmptyString, static_cast<unsigned int>(column), wxDATAVIEW_CELL_INERT, COL_WIDTH, align, flags);
		}

		wxDataViewColumn* Playlist::_AddTextColumn(ColumnId column, const wxString& title, wxAlignment align, int flags)
		{
			return AppendTextColumn(title, static_cast<unsigned int>(column), wxDATAVIEW_CELL_INERT, wxCOL_WIDTH_AUTOSIZE, align, flags);
		}

		void Playlist::_SortByColumn(wxDataViewColumn& viewColumn)
		{
			const ColumnId columnId = static_cast<ColumnId>(viewColumn.GetModelColumn());
			const bool ascending = !(_columnSortState.columnId == columnId  && _columnSortState.ascending);
			bool sorted = true;

			// Sort the model (main songs only)
			switch (columnId)
			{
				case PlaylistTreeModel::ColumnId::Title:
				{
					std::sort(_model.entries.begin(), _model.entries.end(), [&](const PlaylistTreeModelNodePtr& a, const PlaylistTreeModelNodePtr& b)
					{
						const int result = a->title.CmpNoCase(b->title);
						return (ascending) ? result < 0 : result > 0;
					});

					break;
				}
				case PlaylistTreeModel::ColumnId::Duration:
				{
					std::sort(_model.entries.begin(), _model.entries.end(), [&](const PlaylistTreeModelNodePtr& a, const PlaylistTreeModelNodePtr& b)
					{
						return (ascending) ? a->duration < b->duration : a->duration > b->duration;
					});

					break;
				}
				case PlaylistTreeModel::ColumnId::Author:
				{
					std::sort(_model.entries.begin(), _model.entries.end(), [&](const PlaylistTreeModelNodePtr& a, const PlaylistTreeModelNodePtr& b)
					{
						const int result = a->author.CmpNoCase(b->author);
						return (ascending) ? result < 0 : result > 0;
					});

					break;
				}
				case PlaylistTreeModel::ColumnId::Copyright:
				{
					std::sort(_model.entries.begin(), _model.entries.end(), [&](const PlaylistTreeModelNodePtr& a, const PlaylistTreeModelNodePtr& b)
					{
						// Ensure years like "200?" are sorted as "2000"
						wxString aCopyright(a->copyright);
						aCopyright.Replace('?', '0');
						wxString bCopyright(b->copyright);
						bCopyright.Replace('?', '0');

						const int result = aCopyright.CmpNoCase(bCopyright);
						return (ascending) ? result < 0 : result > 0;
					});

					break;
				}
				default:
					sorted = false; // Unhandled column
			}

			if (!sorted)
			{
				return;
			}

			// Set canonical new sorting state
			_columnSortState.ascending = ascending;
			_columnSortState.columnId = columnId;

			// TODO: ensure the sorting indicator also gets cleared when the items are reordered (next feature)
			// Update column sorting indicator
			for (unsigned int i = 0; i < GetColumnCount(); ++i)
			{
				GetColumn(i)->SetBitmap(wxNullBitmap);
			}
			viewColumn.SetBitmap(*_model.GetPlaylistIcons().GetIconList().at((_columnSortState.ascending) ? PlaylistIconId::SortAscending : PlaylistIconId::SortDescending).bitmap);

			// Notify the wx base control of change
			_model.Cleared();

			// Notify the parent (we need to refresh the transport buttons state after sorting in case the first/last positions are swapped)
			GetEventHandler()->AddPendingEvent(wxDataViewEvent(wxEVT_DATAVIEW_COLUMN_SORTED, this, &viewColumn));
		}

		void Playlist::_ResetColumnSortingIndicator()
		{
			if (_columnSortState.columnId == ColumnId::Undefined) [[likely]]
			{
				return; // We must skip this to avoid performance penalty for repeated calls when adding lots of items in bulk.
			}

			_columnSortState = ColumnSortState();
			for (unsigned int i = 0; i < GetColumnCount(); ++i)
			{
				GetColumn(i)->SetBitmap(wxNullBitmap);
			}
		}

#ifdef WIN32
		void Playlist::_OverrideScrollWheel(wxMouseEvent& evt)
		{
			// This method prevents smooth scrolling due to performance issues in the wxDataViewCtrl on MSW.
			if (evt.GetWheelAxis() != wxMOUSE_WHEEL_VERTICAL)
			{
				evt.Skip();
				return;
			}

			const int lines = (evt.GetWheelRotation() / evt.GetWheelDelta()) * evt.GetLinesPerAction();
			DoScroll(GetScrollPos(wxHORIZONTAL), std::max(0, GetScrollPos(wxVERTICAL) - lines)); // Not using the ScrollLines() since it uses the performance-problematic smooth scrolling.
		}
#endif
	}
}
