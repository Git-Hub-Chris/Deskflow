/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2004 Chris Schoeneman
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

#include "filesystem.h"
#if SYSAPI_WIN32
#include "common/win32/encoding_utilities.h"
#endif
#include <fstream>

namespace deskflow {

namespace {

template <class Stream> void open_utf8_path_impl(Stream &stream, const fs::path &path, std::ios_base::openmode mode)
{
  stream.open(path.native().c_str(), mode);
}

} // namespace

void open_utf8_path(std::ifstream &stream, const fs::path &path, std::ios_base::openmode mode)
{
  open_utf8_path_impl(stream, path, mode);
}

void open_utf8_path(std::ofstream &stream, const fs::path &path, std::ios_base::openmode mode)
{
  open_utf8_path_impl(stream, path, mode);
}

void open_utf8_path(std::fstream &stream, const fs::path &path, std::ios_base::openmode mode)
{
  open_utf8_path_impl(stream, path, mode);
}

std::FILE *fopen_utf8_path(const fs::path &path, const std::string &mode)
{
#if SYSAPI_WIN32
  auto wchar_mode = utf8_to_win_char(mode);
  return _wfopen(path.native().c_str(), reinterpret_cast<wchar_t *>(wchar_mode.data()));
#else
  std::string narrow_path = path.string();
  return std::fopen(narrow_path.c_str(), mode.c_str());
#endif
}

} // namespace deskflow
