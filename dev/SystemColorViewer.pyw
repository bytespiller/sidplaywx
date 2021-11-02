## Copyright (C) 2021 Jasmin Rutic (bytespiller@gmail.com)
##
## This program is free software: you can redistribute it and/or modify
## it under the terms of the GNU General Public License as published by
## the Free Software Foundation, either version 3 of the License, or
## (at your option) any later version.
##
## This program is distributed in the hope that it will be useful,
## but WITHOUT ANY WARRANTY; without even the implied warranty of
## MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
## GNU General Public License for more details.
##
## You should have received a copy of the GNU General Public License
## along with this program.  If not, see https://www.gnu.org/licenses/gpl-3.0.html

import wx

colors = {
	"SYS_COLOUR_SCROLLBAR": wx.SYS_COLOUR_SCROLLBAR,
	"SYS_COLOUR_DESKTOP": wx.SYS_COLOUR_DESKTOP,
	"SYS_COLOUR_ACTIVECAPTION": wx.SYS_COLOUR_ACTIVECAPTION,
	"SYS_COLOUR_INACTIVECAPTION": wx.SYS_COLOUR_INACTIVECAPTION,
	"SYS_COLOUR_MENU": wx.SYS_COLOUR_MENU,
	"SYS_COLOUR_WINDOW": wx.SYS_COLOUR_WINDOW,
	"SYS_COLOUR_WINDOWFRAME": wx.SYS_COLOUR_WINDOWFRAME,
	"SYS_COLOUR_MENUTEXT": wx.SYS_COLOUR_MENUTEXT,
	"SYS_COLOUR_WINDOWTEXT": wx.SYS_COLOUR_WINDOWTEXT,
	"SYS_COLOUR_CAPTIONTEXT": wx.SYS_COLOUR_CAPTIONTEXT,
	"SYS_COLOUR_ACTIVEBORDER": wx.SYS_COLOUR_ACTIVEBORDER,
	"SYS_COLOUR_INACTIVEBORDER": wx.SYS_COLOUR_INACTIVEBORDER,
	"SYS_COLOUR_APPWORKSPACE": wx.SYS_COLOUR_APPWORKSPACE,
	"SYS_COLOUR_HIGHLIGHT": wx.SYS_COLOUR_HIGHLIGHT,
	"SYS_COLOUR_HIGHLIGHTTEXT": wx.SYS_COLOUR_HIGHLIGHTTEXT,
	"SYS_COLOUR_BTNFACE": wx.SYS_COLOUR_BTNFACE,
	"SYS_COLOUR_BTNSHADOW": wx.SYS_COLOUR_BTNSHADOW,
	"SYS_COLOUR_GRAYTEXT": wx.SYS_COLOUR_GRAYTEXT,
	"SYS_COLOUR_BTNTEXT": wx.SYS_COLOUR_BTNTEXT,
	"SYS_COLOUR_INACTIVECAPTIONTEXT": wx.SYS_COLOUR_INACTIVECAPTIONTEXT,
	"SYS_COLOUR_BTNHIGHLIGHT": wx.SYS_COLOUR_BTNHIGHLIGHT,
	"SYS_COLOUR_3DDKSHADOW": wx.SYS_COLOUR_3DDKSHADOW,
	"SYS_COLOUR_3DLIGHT": wx.SYS_COLOUR_3DLIGHT,
	"SYS_COLOUR_INFOTEXT": wx.SYS_COLOUR_INFOTEXT,
	"SYS_COLOUR_INFOBK": wx.SYS_COLOUR_INFOBK,
	"SYS_COLOUR_LISTBOX": wx.SYS_COLOUR_LISTBOX,
	"SYS_COLOUR_HOTLIGHT": wx.SYS_COLOUR_HOTLIGHT,
	"SYS_COLOUR_GRADIENTACTIVECAPTION": wx.SYS_COLOUR_GRADIENTACTIVECAPTION,
	"SYS_COLOUR_GRADIENTINACTIVECAPTION": wx.SYS_COLOUR_GRADIENTINACTIVECAPTION,
	"SYS_COLOUR_MENUHILIGHT": wx.SYS_COLOUR_MENUHILIGHT,
	"SYS_COLOUR_MENUBAR": wx.SYS_COLOUR_MENUBAR,
	"SYS_COLOUR_LISTBOXTEXT": wx.SYS_COLOUR_LISTBOXTEXT,
	"SYS_COLOUR_LISTBOXHIGHLIGHTTEXT": wx.SYS_COLOUR_LISTBOXHIGHLIGHTTEXT
}

app = wx.App()
frame = wx.Frame(None, size = (640, 200), title = "System colors viewer")
panel = wx.Panel(frame)

label = wx.StaticText(panel, -1, style = wx.ALIGN_LEFT | wx.ST_ELLIPSIZE_END)
font = wx.Font(16, wx.DEFAULT, wx.NORMAL, wx.BOLD)
label.SetFont(font)
label.SetBackgroundColour((255,255,255))
label.SetLabel("Use arrow/wasd keys and SPACE to reset.")

def UpdateLabelSize():
	global label
	label.SetSize(size=(frame.GetClientSize().GetWidth(), 25))

UpdateLabelSize()
colorIndex = -1

def OnKeyUp(event):
	global colorIndex

	keycode = event.GetKeyCode()
	if keycode == wx.WXK_LEFT or keycode == 65:
		colorIndex -= 1
		if colorIndex < 0:
			colorIndex = len(colors) - 1
	elif keycode == wx.WXK_RIGHT or keycode == 68:
		colorIndex += 1
		if colorIndex > len(colors) - 1:
			colorIndex = 0
	elif keycode == wx.WXK_SPACE:
		colorIndex = 0

	color = list(colors.values())[colorIndex]
	panel.SetBackgroundColour(wx.SystemSettings.GetColour(color))
	label.SetLabel("[" + str(colorIndex) + "] " + list(colors)[colorIndex])

	UpdateLabelSize()
	frame.Refresh()
	frame.Update()

panel.Bind(wx.EVT_KEY_UP, OnKeyUp)

frame.Show();
app.MainLoop()
