/*
 * This file is part of sidplaywx, a GUI player for Commodore 64 SID music files.
 * Copyright (C) 2023-2024 Jasmin Rutic (bytespiller@gmail.com)
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

#include "../Components/PlaylistIcons.h"

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
	#include <wx/wx.h>
#endif

#include <wx/dataview.h>

#include <memory>
#include <vector>

// Forward declarations
namespace UIElements
{
	namespace Playlist
	{
		class Playlist;
	}
}

class PlaylistTreeModel;
class PlaylistTreeModelNode;

// Typedefs
using PlaylistTreeModelNodePtr = std::unique_ptr<PlaylistTreeModelNode>;
using PlaylistTreeModelNodePtrArray = std::vector<PlaylistTreeModelNodePtr>;

// ----------------------------------------------------------------------------
// PlaylistTreeModelNode: a node inside PlaylistTreeModel
// ----------------------------------------------------------------------------

class PlaylistTreeModelNode
{
public:
    template <typename T>
    class PassKey { friend T; PassKey() {} PassKey(PassKey const&) {} };

public:
	enum class ItemType
	{
		Song,
		Subsong
	};

	enum class ItemTag
	{
		Normal,
		ShortDuration,
		Blacklisted,
		RequiresRomBasic,
		RequiresRomKernal
	};

public:
	PlaylistTreeModelNode() = delete;
	PlaylistTreeModelNode(PlaylistTreeModelNode&) = delete;
	PlaylistTreeModelNode(PlaylistTreeModelNode* parent, const wxString& title, const wxString& filepath, int defaultSubsong, uint_least32_t duration, const wxString& author, bool playable);

public:
	/// @brief Returns a nullptr for a mainsong or a mainsong for a subsong.
	PlaylistTreeModelNode* const GetParent();

	/// @brief Returns children nodes i.e., subsongs.
	PlaylistTreeModelNodePtrArray& GetChildren();

	/// @brief A number of subsong child nodes, can be zero.
	int GetSubsongCount() const;

	/// @brief Gets a subsong by index starting from 1. Specify 0 for a default subsong.
	PlaylistTreeModelNode& GetSubsong(int subsong);

	/// @brief Whether the item is playable (e.g., not missing any required ROM).
	bool IsPlayable() const;

	/// @brief Whether the item is playable (e.g., not missing any required ROM) and isn't tagged for auto-navigation skip (either itself or all its children).
	bool IsAutoPlayable() const;

	ItemTag GetTag() const;
	UIElements::Playlist::PlaylistIconId GetIconId() const;

public:
	/// @brief This is a protected method that can only be called by the controller due to mandatory model refresh requirement.
	PlaylistTreeModelNode& AddChild(PlaylistTreeModelNode* childToAdopt, PassKey<UIElements::Playlist::Playlist>);

	/// @brief This is a protected method that can only be called by the controller to ensure visual presentation consistency.
	wxDataViewItemAttr& GetItemAttr(PassKey<UIElements::Playlist::Playlist>);

	/// @brief This is a protected method that can only be called by the model due to wxDataViewModel virtual override requirement.
	wxDataViewItemAttr& GetItemAttrForModel(PassKey<PlaylistTreeModel>);

	/// @brief This is a protected method that can only be called by the controller to ensure visual presentation consistency.
	void ResetItemAttr(PassKey<UIElements::Playlist::Playlist>);

	/// @brief This is a protected method that can only be called by the controller to ensure visual presentation consistency.
	void SetPlayable(bool playable, PassKey<UIElements::Playlist::Playlist>);

	/// @brief This is a protected method that can only be called by the controller to ensure visual presentation consistency.
	void SetTag(ItemTag tag, PassKey<UIElements::Playlist::Playlist>);

	/// @brief This is a protected method that can only be called by the controller to ensure visual presentation consistency.
	void SetIconId(UIElements::Playlist::PlaylistIconId newIconId, PassKey<UIElements::Playlist::Playlist>);

public:
	const ItemType type;
	const wxString title;
	const wxString author;
	const wxString filepath;

	/// @brief Indicates a default subsong for a song item, or a self-index (1-based) for a subsong item.
	const int defaultSubsong;
	const uint_least32_t duration;


private:
	PlaylistTreeModelNode* _parent = nullptr;
	PlaylistTreeModelNodePtrArray _children;
	wxDataViewItemAttr _itemAttr;
	bool _playable = true;
	ItemTag _tag = ItemTag::Normal;
	UIElements::Playlist::PlaylistIconId _iconId = UIElements::Playlist::PlaylistIconId::NoIcon;
};

// ----------------------------------------------------------------------------
// PlaylistTreeModel
// ----------------------------------------------------------------------------

class PlaylistTreeModel: public wxDataViewModel
{
public:
	enum class ColumnId : unsigned int
	{
		Icon = 0,
		Title,
		Duration,
		Author,
		PlaceholderLast // Must exist for proper column layouting, especially for a dynamically shown "Author" column.
	};

	/// @brief Convenience method to downcast a wxDataViewItem to PlaylistTreeModelNode.
	static inline PlaylistTreeModelNode* TreeItemToModelNode(wxDataViewItem treeItem)
	{
		assert(treeItem.IsOk());
		return static_cast<PlaylistTreeModelNode*>(treeItem.GetID());
	}

	/// @brief Convenience method to upcast a PlaylistTreeModelNode to wxDataViewItem.
	static inline wxDataViewItem ModelNodeToTreeItem(const PlaylistTreeModelNode& node)
	{
		return wxDataViewItem(const_cast<PlaylistTreeModelNode*>(&node));
	}

public:
	explicit PlaylistTreeModel(const UIElements::Playlist::PlaylistIcons& playlistIcons);

public:
	PlaylistTreeModelNodePtrArray entries;

public:
	virtual wxDataViewItem GetParent(const wxDataViewItem& item) const override;

protected: // Implementation of base class virtuals to define model
	virtual bool HasContainerColumns(const wxDataViewItem& item) const override;
	virtual void GetValue(wxVariant& variant, const wxDataViewItem& item, unsigned int col) const override;
	virtual bool SetValue(const wxVariant& variant, const wxDataViewItem& item, unsigned int col) override;

	virtual bool IsContainer(const wxDataViewItem& item) const override;
	virtual unsigned int GetChildren(const wxDataViewItem& parent, wxDataViewItemArray& array) const override;

	virtual bool GetAttr(const wxDataViewItem & item, unsigned int col, wxDataViewItemAttr& attr) const override;

private:
	UIElements::Playlist::PlaylistIcons _playlistIcons;
};
