/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2025 Jasmin Rutic (bytespiller@gmail.com)
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

#include "PlaylistModel.h"
#include "../../../Helpers/HelpersWx.h"
#include <stdexcept>

// ----------------------------------------------------------------------------
// PlaylistTreeModelNode
// ----------------------------------------------------------------------------

PlaylistTreeModelNode::PlaylistTreeModelNode(unsigned int uid, PlaylistTreeModelNode* parent, const wxString& title, const wxString& filepath, int defaultSubsong, uint_least32_t duration, const wxString& hvscPath, const char* md5, const wxString& author, const wxString& copyright, RomRequirement romRequirement, bool playable, const wxString& musCompanionStrFilePath) :
	uid(uid),
	_parent(parent),
	title(title),
	filepath(filepath),
	defaultSubsong(defaultSubsong),
	duration(duration),
	hvscPath(hvscPath),
	md5(md5),
	author(author),
	copyright(copyright),
	type((parent == nullptr) ? ItemType::Song : ItemType::Subsong),
	romRequirement(romRequirement),
	_playable(playable),
	musCompanionStrFilePath(musCompanionStrFilePath)
{
};

PlaylistTreeModelNode* const PlaylistTreeModelNode::GetParent()
{
	return _parent;
}

PlaylistTreeModelNodePtrArray& PlaylistTreeModelNode::GetChildren()
{
	return _children;
}

int PlaylistTreeModelNode::GetSubsongCount() const
{
	return _children.size();
}

PlaylistTreeModelNode& PlaylistTreeModelNode::GetSubsong(int subsong)
{
	assert(GetChildren().size()); // No children, a subsong was passed instead of mainsong?
	return *GetChildren().at((subsong == 0) ? defaultSubsong : subsong - 1);
}

bool PlaylistTreeModelNode::IsPlayable() const
{
	return _playable;
}

bool PlaylistTreeModelNode::IsAutoPlayable() const
{
	if(!IsPlayable() || _tag != PlaylistTreeModelNode::ItemTag::Normal)
	{
		return false;
	}

	// If it has children, check that there is at least one which isn't tagged for auto-navigation skip.
	return _children.empty() || std::any_of(_children.cbegin(), _children.cend(), [](const PlaylistTreeModelNodePtr& subsong)
	{
		return subsong->GetTag() == PlaylistTreeModelNode::ItemTag::Normal;
	});
}

PlaylistTreeModelNode::ItemTag PlaylistTreeModelNode::GetTag() const
{
	return _tag;
}

UIElements::Playlist::PlaylistIconId PlaylistTreeModelNode::GetIconId() const
{
	return _iconId;
}

PlaylistTreeModelNode& PlaylistTreeModelNode::AddChild(PlaylistTreeModelNode* childToAdopt, PlaylistTreeModelNode::PassKey<UIElements::Playlist::Playlist>)
{
	assert(_parent == nullptr); // Adding children to children is unexpected usecase.
	return *_children.emplace_back(childToAdopt).get();
}

wxDataViewItemAttr& PlaylistTreeModelNode::GetItemAttr(PlaylistTreeModelNode::PassKey<UIElements::Playlist::Playlist>)
{
	return _itemAttr;
}

wxDataViewItemAttr& PlaylistTreeModelNode::GetItemAttrForModel(PlaylistTreeModelNode::PassKey<PlaylistTreeModel>)
{
	return _itemAttr;
}

void PlaylistTreeModelNode::ResetItemAttr(PassKey<UIElements::Playlist::Playlist>)
{
	_itemAttr = wxDataViewItemAttr();
}

void PlaylistTreeModelNode::SetPlayable(bool playable, PassKey<UIElements::Playlist::Playlist>)
{
	_playable = playable;
}

void PlaylistTreeModelNode::SetTag(ItemTag tag, PassKey<UIElements::Playlist::Playlist>)
{
	_tag = tag;
}

void PlaylistTreeModelNode::SetIconId(UIElements::Playlist::PlaylistIconId newIconId, PassKey<UIElements::Playlist::Playlist>)
{
	_iconId = newIconId;
}

// ----------------------------------------------------------------------------
// PlaylistTreeModel
// ----------------------------------------------------------------------------

PlaylistTreeModel::PlaylistTreeModel(const UIElements::Playlist::PlaylistIcons& playlistIcons) :
	_playlistIcons(playlistIcons)
{
}

const UIElements::Playlist::PlaylistIcons& PlaylistTreeModel::GetPlaylistIcons() const
{
	return _playlistIcons;
}

std::unique_ptr<void, std::function<void(void*)>> PlaylistTreeModel::PrepareDirty(std::function<void()> notifier)
{
#ifndef WIN32
	BeforeReset();
#endif

	// Use a dummy non-null pointer so that the deleter always runs
	void* const dummy = reinterpret_cast<void*>(1);

	// Return a unique_ptr with a custom deleter lambda. When the unique_ptr goes out of scope, our lambda gets called, automatically invoking the AfterReset() / notifier.
	return std::unique_ptr<void, std::function<void(void*)>>
	(
		dummy, [this, notifier = std::move(notifier)](void*)
		{
#ifndef WIN32
			AfterReset();
#else
			notifier();
#endif
		}
	);
}

bool PlaylistTreeModel::HasContainerColumns(const wxDataViewItem& item) const
{
	const PlaylistTreeModelNode* const node = TreeItemToModelNode(item);
	return node->type == PlaylistTreeModelNode::ItemType::Song; // Allows the main song's columns other than the first one being processed in the GetValue().
}

void PlaylistTreeModel::GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const
{
	wxASSERT(item.IsOk());

	PlaylistTreeModelNode* node = TreeItemToModelNode(item);
	switch (static_cast<ColumnId>(col))
	{
		case ColumnId::Icon:
		{
			const UIElements::Playlist::PlaylistIconId nodeIconId = node->GetIconId();
			if (nodeIconId != UIElements::Playlist::PlaylistIconId::NoIcon)
			{
				variant = wxVariant(*_playlistIcons.GetIconList().at(nodeIconId).bitmap); // This severely impacts the scrolling performance on MSW (issue with wxWidgets' wxDataViewCtrl indirection design).
			}
			else
			{
				variant = wxVariant(wxNullBitmap);
			}
			break;
		}
		case ColumnId::Title:
		{
			const int count = node->GetSubsongCount();
			if (count == 0)
			{
				variant = node->title;
			}
			else // Faster than using wxString::Format.
			{
				variant = Helpers::Wx::FastJoin(node->title, " (", std::to_string(count).c_str(), ")");
			}
			break;
		}
		case ColumnId::Duration:
		{
			if (node->GetSubsongCount() == 0)
			{
				variant = Helpers::Wx::GetTimeFormattedString(node->duration, true);
			}
			else
			{
				variant = wxEmptyString;
			}
			break;
		}
		case ColumnId::Author:
		{
			variant = node->author;
			break;
		}
		case ColumnId::Copyright:
		{
			variant = node->copyright;
			break;
		}
		//default: // Reminder: dummy column always ends up here.
			//throw std::out_of_range(std::to_string(col)); // Note: wxWidgets will eat this and won't throw any visible errors. We shouldn't throw here as it's expensive in vain.
	}
}

bool PlaylistTreeModel::SetValue(const wxVariant &variant, const wxDataViewItem& item, unsigned int col)
{
	wxASSERT(item.IsOk());
	return true;
}

wxDataViewItem PlaylistTreeModel::GetParent(const wxDataViewItem& item) const
{
	// The invisible root node has no parent
	if (!item.IsOk())
	{
		return wxDataViewItem(0);
	}

	PlaylistTreeModelNode* node = TreeItemToModelNode(item);

	if (node->type == PlaylistTreeModelNode::ItemType::Song)
	{
		return wxDataViewItem(0);
	}

	return wxDataViewItem(node->GetParent());
}

bool PlaylistTreeModel::IsContainer(const wxDataViewItem& item) const
{
	// The invisible root node can have children
	if (!item.IsOk())
	{
		return true;
	}

	const PlaylistTreeModelNode& node = *TreeItemToModelNode(item);
	return node.type == PlaylistTreeModelNode::ItemType::Song && node.GetSubsongCount() > 0; // Top-level item (song) which can contain subsongs.
}

unsigned int PlaylistTreeModel::GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const
{
	const PlaylistTreeModelNodePtrArray* children;

	if (!parent.IsOk())
	{
		children = &entries;
	}
	else
	{
		PlaylistTreeModelNode* node = TreeItemToModelNode(parent);
		children = &node->GetChildren();
	}

	if (children->size() == 0)
	{
		return 0;
	}

	for (const PlaylistTreeModelNodePtr& child: *children)
	{
		array.Add(wxDataViewItem(child.get()));
	}

	return array.size();
}

bool PlaylistTreeModel::GetAttr(const wxDataViewItem& item, unsigned int col, wxDataViewItemAttr& attr) const
{
	PlaylistTreeModelNode* node = TreeItemToModelNode(item);
	attr = node->GetItemAttrForModel({});
	return true;
}
