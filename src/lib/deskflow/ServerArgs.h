/*
 * Deskflow -- mouse and keyboard sharing utility
 * Copyright (C) 2014-2020 Symless Ltd.
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

#include "ArgsBase.h"
#include "server/Config.h"

#include <memory>

namespace deskflow {

class ServerArgs : public ArgsBase
{
  using Config = deskflow::server::Config;

public:
  ServerArgs();
  ServerArgs(ServerArgs const &src) = default;
  ServerArgs(ServerArgs &&) = default;
  ~ServerArgs() override;

  ServerArgs &operator=(ServerArgs const &) = default;
  ServerArgs &operator=(ServerArgs &&) = default;

public:
  std::string m_configFile = "";
  std::shared_ptr<Config> m_config;
};

} // namespace deskflow
