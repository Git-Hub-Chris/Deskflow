/*
 * synergy -- mouse and keyboard sharing utility
 * Copyright (C) 2012-2022 Symless Ltd.
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

#include "InverseClientSocket.h"

#include "net/NetworkAddress.h"
#include "net/SocketMultiplexer.h"
#include "net/TSocketMultiplexerMethodJob.h"
#include "net/XSocket.h"
#include "mt/Lock.h"
#include "arch/Arch.h"
#include "arch/XArch.h"
#include "base/Log.h"
#include "base/IEventQueue.h"
#include "base/IEventJob.h"

#include <cstring>
#include <cstdlib>
#include <memory>

//
// InverseClientSocket
//

InverseClientSocket::InverseClientSocket(IEventQueue* events, SocketMultiplexer* socketMultiplexer, IArchNetwork::EAddressFamily family) :
    IDataSocket(events),
    m_events(events),
    m_mutex(),
    m_socket(family),
    m_flushed(&m_mutex, true),
    m_socketMultiplexer(socketMultiplexer)
{
}

InverseClientSocket::~InverseClientSocket()
{
    try {
        // warning virtual function in destructor is very danger practice
        InverseClientSocket::close();
    }
    catch (...) {
        LOG((CLOG_DEBUG "error while TCP socket destruction"));
    }
}

void
InverseClientSocket::bind(const NetworkAddress& addr)
{
    m_socket.bindSocket(addr);
}

void
InverseClientSocket::close()
{
    // remove ourself from the multiplexer
    setJob(nullptr);

    Lock lock(&m_mutex);

    // clear buffers and enter disconnected state
    if (m_connected) {
        sendEvent(m_events->forISocket().disconnected());
    }
    onDisconnected();

    // close the socket
    m_socket.closeSocket();
}

void*
InverseClientSocket::getEventTarget() const
{
    return const_cast<void*>(static_cast<const void*>(this));
}

UInt32
InverseClientSocket::read(void* buffer, UInt32 n)
{
    // copy data directly from our input buffer
    Lock lock(&m_mutex);
    UInt32 size = m_inputBuffer.getSize();
    if (n > size) {
        n = size;
    }
    if (buffer != nullptr && n != 0) {
        memcpy(buffer, m_inputBuffer.peek(n), n);
    }
    m_inputBuffer.pop(n);

    // if no more data and we cannot read or write then send disconnected
    if (n > 0 && m_inputBuffer.getSize() == 0 && !m_readable && !m_writable) {
        sendEvent(m_events->forISocket().disconnected());
        m_connected = false;
    }

    return n;
}

void
InverseClientSocket::write(const void* buffer, UInt32 n)
{
    bool wasEmpty;
    {
        Lock lock(&m_mutex);

        // must not have shutdown output
        if (!m_writable) {
            sendEvent(m_events->forIStream().outputError());
            return;
        }

        // ignore empty writes
        if (n == 0) {
            return;
        }

        // copy data to the output buffer
        wasEmpty = (m_outputBuffer.getSize() == 0);
        m_outputBuffer.write(buffer, n);

        // there's data to write
        m_flushed = false;
    }

    // make sure we're waiting to write
    if (wasEmpty) {
        setJob(newJob());
    }
}

void
InverseClientSocket::flush()
{
    Lock lock(&m_mutex);
    while (m_flushed == false) {
        m_flushed.wait();
    }
}

void
InverseClientSocket::shutdownInput()
{
    bool useNewJob = false;
    {
        Lock lock(&m_mutex);

        // shutdown socket for reading
        m_socket.closeSocketForRead();

        // shutdown buffer for reading
        if (m_readable) {
            sendEvent(m_events->forIStream().inputShutdown());
            onInputShutdown();
            useNewJob = true;
        }
    }
    if (useNewJob) {
        setJob(newJob());
    }
}

void
InverseClientSocket::shutdownOutput()
{
    bool useNewJob = false;
    {
        Lock lock(&m_mutex);

        // shutdown socket for writing
        m_socket.closeSocketForWrite();

        // shutdown buffer for writing
        if (m_writable) {
            sendEvent(m_events->forIStream().outputShutdown());
            onOutputShutdown();
            useNewJob = true;
        }
    }
    if (useNewJob) {
        setJob(newJob());
    }
}

bool
InverseClientSocket::isReady() const
{
    Lock lock(&m_mutex);
    return (m_inputBuffer.getSize() > 0);
}

bool
InverseClientSocket::isFatal() const
{
    // TCP sockets aren't ever left in a fatal state.
    LOG((CLOG_ERR "isFatal() not valid for non-secure connections"));
    return false;
}

UInt32
InverseClientSocket::getSize() const
{
    Lock lock(&m_mutex);
    return m_inputBuffer.getSize();
}

void
InverseClientSocket::connect(const NetworkAddress& addr)
{
    {
        Lock lock(&m_mutex);

        // fail on attempts to reconnect
        if (m_socket.getRawSocket() == nullptr || m_connected) {
            sendConnectionFailedEvent("busy");
            return;
        }

        try {
            if (m_socket.connectSocket(addr)) {
                sendEvent(m_events->forIDataSocket().connected());
                onConnected();
            }
            else {
                // connection is in progress
                m_writable = true;
            }
        }
        catch (const XArchNetwork& e) {
            throw XSocketConnect(e.what());
        }
    }
    setJob(newJob());
}

InverseClientSocket::EJobResult
InverseClientSocket::doRead()
{
    UInt8 buffer[4096] = {0};
    size_t bytesRead = m_socket.readSocket(buffer, sizeof(buffer));

    if (bytesRead > 0) {
        bool wasEmpty = (m_inputBuffer.getSize() == 0);

        // slurp up as much as possible
        do {
            m_inputBuffer.write(buffer, static_cast<UInt32>(bytesRead));

            bytesRead = m_socket.readSocket(buffer, sizeof(buffer));
        } while (bytesRead > 0);

        // send input ready if input buffer was empty
        if (wasEmpty) {
            sendEvent(m_events->forIStream().inputReady());
        }
    }
    else {
        // remote write end of stream hungup.  our input side
        // has therefore shutdown but don't flush our buffer
        // since there's still data to be read.
        sendEvent(m_events->forIStream().inputShutdown());
        if (!m_writable && m_inputBuffer.getSize() == 0) {
            sendEvent(m_events->forISocket().disconnected());
            m_connected = false;
        }
        m_readable = false;
        return kNew;
    }

    return kRetry;
}

InverseClientSocket::EJobResult
InverseClientSocket::doWrite()
{
    UInt32 bufferSize = m_outputBuffer.getSize();
    auto buffer = static_cast<const UInt8*>(m_outputBuffer.peek(bufferSize));
    const UInt32 bytesWrote = static_cast<UInt32>(m_socket.writeSocket(buffer, bufferSize));

    if (bytesWrote > 0) {
        discardWrittenData(bytesWrote);
        return kNew;
    }

    return kRetry;
}

void
InverseClientSocket::setJob(ISocketMultiplexerJob* job)
{
    // multiplexer will delete the old job
    if (job == nullptr) {
        m_socketMultiplexer->removeSocket(this);
    }
    else {
        m_socketMultiplexer->addSocket(this, job);
    }
}

ISocketMultiplexerJob*
InverseClientSocket::newJob()
{
    // note -- must have m_mutex locked on entry

    if (m_socket.getRawSocket() == nullptr) {
        return nullptr;
    }
    else if (!m_connected) {
        assert(!m_readable);
        if (!(m_readable || m_writable)) {
            return nullptr;
        }
        return new TSocketMultiplexerMethodJob<InverseClientSocket>(
                                this, &InverseClientSocket::serviceConnecting,
                                m_socket.getRawSocket(), m_readable, m_writable);
    }
    else {
        if (!(m_readable || (m_writable && (m_outputBuffer.getSize() > 0)))) {
            return nullptr;
        }
        return new TSocketMultiplexerMethodJob<InverseClientSocket>(
                                this, &InverseClientSocket::serviceConnected,
                                m_socket.getRawSocket(), m_readable,
                                m_writable && (m_outputBuffer.getSize() > 0));
    }
}

void
InverseClientSocket::sendConnectionFailedEvent(const char* msg)
{
    ConnectionFailedInfo* info = new ConnectionFailedInfo(msg);
    m_events->addEvent(Event(m_events->forIDataSocket().connectionFailed(),
                            getEventTarget(), info, Event::kDontFreeData));
}

void
InverseClientSocket::sendEvent(Event::Type type)
{
    m_events->addEvent(Event(type, getEventTarget()));
}

void
InverseClientSocket::discardWrittenData(int bytesWrote)
{
    m_outputBuffer.pop(bytesWrote);
    if (m_outputBuffer.getSize() == 0) {
        sendEvent(m_events->forIStream().outputFlushed());
        m_flushed = true;
        m_flushed.broadcast();
    }
}

void
InverseClientSocket::onConnected()
{
    m_connected = true;
    m_readable  = true;
    m_writable  = true;
}

void
InverseClientSocket::onInputShutdown()
{
    m_inputBuffer.pop(m_inputBuffer.getSize());
    m_readable = false;
}

void
InverseClientSocket::onOutputShutdown()
{
    m_outputBuffer.pop(m_outputBuffer.getSize());
    m_writable = false;

    // we're now flushed
    m_flushed = true;
    m_flushed.broadcast();
}

void
InverseClientSocket::onDisconnected()
{
    // disconnected
    onInputShutdown();
    onOutputShutdown();
    m_connected = false;
}

ISocketMultiplexerJob*
InverseClientSocket::serviceConnecting(ISocketMultiplexerJob* job,
                bool, bool write, bool error)
{
    Lock lock(&m_mutex);

    // should only check for errors if error is true but checking a new
    // socket (and a socket that's connecting should be new) for errors
    // should be safe and Mac OS X appears to have a bug where a
    // non-blocking stream socket that fails to connect immediately is
    // reported by select as being writable (i.e. connected) even when
    // the connection has failed.  this is easily demonstrated on OS X
    // 10.3.4 by starting a synergy client and telling to connect to
    // another system that's not running a synergy server.  it will
    // claim to have connected then quickly disconnect (i guess because
    // read returns 0 bytes).  unfortunately, synergy attempts to
    // reconnect immediately, the process repeats and we end up
    // spinning the CPU.  luckily, OS X does set SO_ERROR on the
    // socket correctly when the connection has failed so checking for
    // errors works.  (curiously, sometimes OS X doesn't report
    // connection refused.  when that happens it at least doesn't
    // report the socket as being writable so synergy is able to time
    // out the attempt.)
    if (error || true) {
        try {
            // connection may have failed or succeeded
            m_socket.throwErrorOnSocket();
        }
        catch (const XArchNetwork& e) {
            sendConnectionFailedEvent(e.what());
            onDisconnected();
            return newJob();
        }
    }

    if (write) {
        sendEvent(m_events->forIDataSocket().connected());
        onConnected();
        return newJob();
    }

    return job;
}

ISocketMultiplexerJob*
InverseClientSocket::serviceConnected(ISocketMultiplexerJob* job,
                bool read, bool write, bool error)
{
    Lock lock(&m_mutex);

    if (error) {
        sendEvent(m_events->forISocket().disconnected());
        onDisconnected();
        return newJob();
    }

    EJobResult result = kRetry;
    if (write) {
        try {
            result = doWrite();
        }
        catch (XArchNetworkShutdown&) {
            // remote read end of stream hungup.  our output side
            // has therefore shutdown.
            onOutputShutdown();
            sendEvent(m_events->forIStream().outputShutdown());
            if (!m_readable && m_inputBuffer.getSize() == 0) {
                sendEvent(m_events->forISocket().disconnected());
                m_connected = false;
            }
            result = kNew;
        }
        catch (XArchNetworkDisconnected&) {
            // stream hungup
            onDisconnected();
            sendEvent(m_events->forISocket().disconnected());
            result = kNew;
        }
        catch (XArchNetwork& e) {
            // other write error
            LOG((CLOG_WARN "error writing socket: %s", e.what()));
            onDisconnected();
            sendEvent(m_events->forIStream().outputError());
            sendEvent(m_events->forISocket().disconnected());
            result = kNew;
        }
    }

    if (read && m_readable) {
        try {
            result = doRead();
        }
        catch (XArchNetworkDisconnected&) {
            // stream hungup
            sendEvent(m_events->forISocket().disconnected());
            onDisconnected();
            result = kNew;
        }
        catch (XArchNetwork& e) {
            // ignore other read error
            LOG((CLOG_WARN "error reading socket: %s", e.what()));
        }
    }

    if (result == kBreak) {
        return nullptr;
    }

    return result == kNew ? newJob() : job;
}
