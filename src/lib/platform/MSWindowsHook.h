/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2011 Chris Schoeneman
 *
 * This package is free software; you can redistribute it and/or
 * modify it under the terms of the GNU General Public License
 * found in the file LICENSE that should have accompanied this file.
 *
 * This package is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#pragma once

#include "platform/dfwhook.h"

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>

//! Loads and provides functions for the Windows hook
class MSWindowsHook
{
public:
  MSWindowsHook();
  virtual ~MSWindowsHook();

  void loadLibrary();

  int init(DWORD threadID);

  int cleanup();

  void setSides(uint32_t sides);

  void setZone(int32_t x, int32_t y, int32_t w, int32_t h, int32_t jumpZoneSize);

  void setMode(EHookMode mode);

  static EHookResult install();

  static int uninstall();

  static int installScreenSaver();

  static int uninstallScreenSaver();
};
