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

#include "platform/MSWindowsClipboardHTMLConverter.h"

#include <base/String.h>

//
// MSWindowsClipboardHTMLConverter
//

MSWindowsClipboardHTMLConverter::MSWindowsClipboardHTMLConverter()
{
  m_format = RegisterClipboardFormat("HTML Format");
}

MSWindowsClipboardHTMLConverter::~MSWindowsClipboardHTMLConverter()
{
  // do nothing
}

IClipboard::EFormat MSWindowsClipboardHTMLConverter::getFormat() const
{
  return IClipboard::kHTML;
}

UINT MSWindowsClipboardHTMLConverter::getWin32Format() const
{
  return m_format;
}

std::string MSWindowsClipboardHTMLConverter::doFromIClipboard(const std::string &data) const
{
  // prepare to CF_HTML format prefix and suffix
  std::string prefix("Version:0.9\r\nStartHTML:0000000105\r\n"
                     "EndHTML:ZZZZZZZZZZ\r\n"
                     "StartFragment:XXXXXXXXXX\r\nEndFragment:YYYYYYYYYY\r\n"
                     "<!DOCTYPE><HTML><BODY><!--StartFragment-->");
  std::string suffix("<!--EndFragment--></BODY></HTML>\r\n");

  // Get byte offsets for header
  uint32_t StartFragment = (uint32_t)prefix.size();
  uint32_t EndFragment = StartFragment + (uint32_t)data.size();
  // StartHTML is constant by the design of the prefix
  uint32_t EndHTML = EndFragment + (uint32_t)suffix.size();

  prefix.replace(prefix.find("XXXXXXXXXX"), 10, deskflow::string::sprintf("%010u", StartFragment));
  prefix.replace(prefix.find("YYYYYYYYYY"), 10, deskflow::string::sprintf("%010u", EndFragment));
  prefix.replace(prefix.find("ZZZZZZZZZZ"), 10, deskflow::string::sprintf("%010u", EndHTML));

  // concatenate
  prefix += data;
  prefix += suffix;
  return prefix;
}

std::string MSWindowsClipboardHTMLConverter::doToIClipboard(const std::string &data) const
{
  // get fragment start/end args
  std::string startArg = findArg(data, "StartFragment");
  std::string endArg = findArg(data, "EndFragment");
  if (startArg.empty() || endArg.empty()) {
    return std::string();
  }

  // convert args to integers
  int32_t start = (int32_t)atoi(startArg.c_str());
  int32_t end = (int32_t)atoi(endArg.c_str());
  if (start <= 0 || end <= 0 || start >= end) {
    return std::string();
  }

  // extract the fragment
  return data.substr(start, end - start);
}

std::string MSWindowsClipboardHTMLConverter::findArg(const std::string &data, const std::string &name) const
{
  std::string::size_type i = data.find(name);
  if (i == std::string::npos) {
    return std::string();
  }

  i = data.find_first_of(":\r\n", i);
  if (i == std::string::npos || data[i] != ':') {
    return std::string();
  }
  i = data.find_first_of("0123456789\r\n", i + 1);
  if (i == std::string::npos || data[i] == '\r' || data[i] == '\n') {
    return std::string();
  }

  std::string::size_type j = data.find_first_not_of("0123456789", i);
  if (j == std::string::npos) {
    j = data.size();
  }
  return data.substr(i, j - i);
}
