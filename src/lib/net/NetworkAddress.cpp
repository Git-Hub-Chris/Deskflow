/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2016 Symless Ltd.
 * Copyright (C) 2002 Chris Schoeneman
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

#include "net/NetworkAddress.h"

#include "net/XSocket.h"
#include "arch/Arch.h"
#include "arch/XArch.h"

#include <algorithm>
#include <cstdlib>

//
// NetworkAddress
//

// name re-resolution adapted from a patch by Brent Priddy.

NetworkAddress::NetworkAddress() :
    m_address(NULL),
    m_hostname(),
    m_port(0)
{
    // note -- make no calls to Network socket interface here;
    // we're often called prior to Network::init().
}

NetworkAddress::NetworkAddress(int port) :
    m_address(NULL),
    m_hostname(),
    m_port(port)
{
    checkPort();
    m_address = ARCH->newAnyAddr(IArchNetwork::kINET);
    ARCH->setAddrPort(m_address, m_port);
}

NetworkAddress::NetworkAddress(const NetworkAddress& addr) :
    m_address(addr.m_address != NULL ? ARCH->copyAddr(addr.m_address) : NULL),
    m_hostname(addr.m_hostname),
    m_port(addr.m_port)
{
    // do nothing
}

NetworkAddress::NetworkAddress(const String& hostname, int port) :
    m_address(NULL),
    m_hostname(hostname),
    m_port(port)
{
    //detect internet protocol version with colom count
    auto isColomPredicate = [](char c){return c == ':';};
    auto colomCount = std::count_if(m_hostname.begin(), m_hostname.end(), isColomPredicate);

    if(colomCount == 1) {
        //ipv4 with port part
        auto hostIt = m_hostname.find(':');
        try {
            m_port = std::stoi(m_hostname.substr(hostIt + 1));
        } catch(...) {
            throw XSocketAddress(XSocketAddress::kBadPort, m_hostname, m_port);
        }

        auto endHostnameIt = static_cast<int>(hostIt);
        m_hostname = m_hostname.substr(0, endHostnameIt > 0 ? endHostnameIt : 0);
    }
    else if (colomCount > 1) {
        //ipv6 part
        if (m_hostname[0] == '[') {
            //ipv6 with port part
            String portDelimeter = "]:";
            auto   hostIt        = m_hostname.find(portDelimeter);

            //bad syntax of ipv6 with port
            if (hostIt == String::npos) {
                throw XSocketAddress(XSocketAddress::kUnknown, m_hostname, m_port);
            }

            auto portSuffix = m_hostname.substr(hostIt + portDelimeter.size());
            //port is implied but omitted
            if (portSuffix.empty()) {
                throw XSocketAddress(XSocketAddress::kBadPort, m_hostname, m_port);
            }
            try {
                m_port = std::stoi(portSuffix);
            } catch(...) {
                //port is not a number
                throw XSocketAddress(XSocketAddress::kBadPort, m_hostname, m_port);
            }

            auto endHostnameIt = static_cast<int>(hostIt) - 1;
            m_hostname = m_hostname.substr(1, endHostnameIt > 0 ? endHostnameIt : 0);
        }

        // ensure that ipv6 link-local adress ended with scope id
        if (m_hostname.rfind("fe80:", 0) == 0 && m_hostname.find('%') == String::npos) {
            throw XSocketAddress(XSocketAddress::kUnknown, m_hostname, m_port);
        }
    }

    // check port number
    checkPort();
}

NetworkAddress::~NetworkAddress()
{
    if (m_address != NULL) {
        ARCH->closeAddr(m_address);
    }
}

NetworkAddress&
NetworkAddress::operator=(const NetworkAddress& addr)
{
    ArchNetAddress newAddr = NULL;
    if (addr.m_address != NULL) {
        newAddr = ARCH->copyAddr(addr.m_address);
    }
    if (m_address != NULL) {
        ARCH->closeAddr(m_address);
    }
    m_address  = newAddr;
    m_hostname = addr.m_hostname;
    m_port     = addr.m_port;
    return *this;
}

void
NetworkAddress::resolve()
{
    // discard previous address
    if (m_address != NULL) {
        ARCH->closeAddr(m_address);
        m_address = NULL;
    }

    try {
        // if hostname is empty then use wildcard address otherwise look
        // up the name.
        if (m_hostname.empty()) {
            m_address = ARCH->newAnyAddr(IArchNetwork::kINET);
        }
        else {
            m_address = ARCH->nameToAddr(m_hostname);
        }
    }
    catch (XArchNetworkNameUnknown&) {
        throw XSocketAddress(XSocketAddress::kNotFound, m_hostname, m_port);
    }
    catch (XArchNetworkNameNoAddress&) {
        throw XSocketAddress(XSocketAddress::kNoAddress, m_hostname, m_port);
    }
    catch (XArchNetworkNameUnsupported&) {
        throw XSocketAddress(XSocketAddress::kUnsupported, m_hostname, m_port);
    }
    catch (XArchNetworkName&) {
        throw XSocketAddress(XSocketAddress::kUnknown, m_hostname, m_port);
    }

    // set port in address
    ARCH->setAddrPort(m_address, m_port);
}

bool
NetworkAddress::operator==(const NetworkAddress& addr) const
{
    return m_address == addr.m_address || ARCH->isEqualAddr(m_address, addr.m_address);
}

bool
NetworkAddress::operator!=(const NetworkAddress& addr) const
{
    return !operator==(addr);
}

bool
NetworkAddress::isValid() const
{
    return (m_address != NULL);
}

const ArchNetAddress&
NetworkAddress::getAddress() const
{
    return m_address;
}

int
NetworkAddress::getPort() const
{
    return m_port;
}

String
NetworkAddress::getHostname() const
{
    return m_hostname;
}

void
NetworkAddress::checkPort()
{
    // check port number
    if (m_port <= 0 || m_port > 65535) {
        throw XSocketAddress(XSocketAddress::kBadPort, m_hostname, m_port);
    }
}
