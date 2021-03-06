/*  Museek - A SoulSeek client written in C++
    Copyright (C) 2006-2007 Ingmar K. Steen (iksteen@gmail.com)
    Copyright 2008 little blue poney <lbponey@users.sourceforge.net>

    This program is free software; you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation; either version 2 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program; if not, write to the Free Software
    Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA

 */

#ifdef HAVE_CONFIG_H
# include "config.h"
#endif // HAVE_CONFIG_H
#include "codesetmanager.h"
#include "usersocket.h"
#include "museekd.h"
#include "configmanager.h"
#include "handshakesocket.h"
#include "handshakemessages.h"
#include "servermanager.h"
#include "peermanager.h"
#include <NewNet/nnreactor.h>
#include <limits.h>

Museek::UserSocket::UserSocket(Museek::Museekd * museekd, const std::string & type, bool obfuscated) : NewNet::TcpClientSocket(), m_Museekd(museekd), m_Type(type)
{
  setNeedsObfuscated(obfuscated);
  disconnectedEvent.connect(this, &UserSocket::onDisconnected);
  m_Museekd->server()->cannotConnectNotifyReceivedEvent.connect(this, &UserSocket::onCannotConnectNotify);
}

Museek::UserSocket::UserSocket(Museek::HandshakeSocket * that, const std::string & type) : NewNet::TcpClientSocket(), m_Museekd(that->museekd()), m_Type(type)
{
  disconnectedEvent.connect(this, &UserSocket::onDisconnected);
  m_Museekd->server()->cannotConnectNotifyReceivedEvent.connect(this, &UserSocket::onCannotConnectNotify);

  m_Token = that->token();
  m_User = that->user();

  setNeedsObfuscated(that->obfuscated());

  setDescriptor(that->descriptor());
  setSocketState(SocketConnected);
  receiveBuffer() = that->receiveBuffer();
}

Museek::UserSocket::~UserSocket()
{
    if(m_PassiveConnectTimeout.isValid())
      m_Museekd->reactor()->removeTimeout(m_PassiveConnectTimeout);
}

void
Museek::UserSocket::onDisconnected(NewNet::ClientSocket *)
{
  // Just in case this socket is still registered
  m_Museekd->peers()->removePassiveConnectionWaiting(m_Token);

  if(reactor())
    reactor()->remove(this);
}

void
Museek::UserSocket::initiate(const std::string & user)
{
  m_User = user;
  m_Token = m_Museekd->token();

  if(m_Museekd->config()->get("clients", "connectmode", "active") == "passive")
    initiatePassive();
  else
    initiateActive();
}

void
Museek::UserSocket::initiateActive()
{
  NNLOG("museekd.user.debug", "Initiating active user connection to %s (type %s).", m_User.c_str(), m_Type.c_str());

  m_Museekd->server()->peerAddressReceivedEvent.connect(this, &UserSocket::onServerPeerAddressReceived);
  SGetPeerAddress msg(m_User);
  m_Museekd->server()->sendMessage(msg.make_network_packet());
}

void
Museek::UserSocket::initiatePassive()
{
  NNLOG("museekd.user.debug", "Initiating passive user connection to %s (type %s).", m_User.c_str(), m_Type.c_str());

  m_PassiveConnectTimeout = m_Museekd->reactor()->addTimeout(60000, this, &UserSocket::onFirewallPierceTimedOut);
  m_Museekd->peers()->waitingPassiveConnection(this);

  SConnectToPeer msg(m_Token, m_User, m_Type);
  m_Museekd->server()->sendMessage(msg.make_network_packet());
}

void
Museek::UserSocket::firewallPierced(Museek::HandshakeSocket * socket)
{
    NNLOG("museekd.user.debug", "%s's firewall successfully pierced.", m_User.c_str());
    if(m_PassiveConnectTimeout.isValid())
      m_Museekd->reactor()->removeTimeout(m_PassiveConnectTimeout);

    setSocketState(SocketConnected);
    setNeedsObfuscated(socket->obfuscated());
    setDescriptor(socket->descriptor());
    receiveBuffer() = socket->receiveBuffer();
    if(! receiveBuffer().empty())
        dataReceivedEvent(this);
    socket->receiveBuffer().clear();
    connectedEvent(this);
}

void
Museek::UserSocket::onFirewallPierceTimedOut(long)
{
  m_Museekd->peers()->removePassiveConnectionWaiting(m_Token);

  if(socketState() == SocketUninitialized)
  {
    NNLOG("museekd.user.debug", "Passive connection failed: pierce firewall timed out. Trying an active connection.");
    initiateActive();
  }
}

void
Museek::UserSocket::onCannotConnectNotify(const SCannotConnect * msg)
{
    m_Museekd->peers()->removePassiveConnectionWaiting(m_Token);

    if (msg->token == token()) {
        NNLOG("museekd.user.debug", "Cannot connect to the peer.");
        disconnect();
    }
}

void
Museek::UserSocket::onServerPeerAddressReceived(const SGetPeerAddress * message)
{
    if((message->user != m_User) || (socketState() != SocketUninitialized))
        return;

    if (message->use_obfuscation) {
        NNLOG("museekd.user.debug", "Received address of user %s: %s:%u and obfuscated port %u", m_User.c_str(), message->ip.c_str(), message->port, message->obfuscated_port);

        museekd()->codeset()->addModernPeer(message->user);

        if((message->ip == "0.0.0.0") || (message->obfuscated_port == 0)) {
            cannotConnectEvent(this);
            return;
        }

        setNeedsObfuscated(message->use_obfuscation);
        HInitiate handshake(m_Museekd->server()->username(), m_Type, m_Token);
        sendMessage(handshake.make_network_packet());

        connect(message->ip, message->obfuscated_port);
    }
    else {
        NNLOG("museekd.user.debug", "Received address of user %s: %s:%u", m_User.c_str(), message->ip.c_str(), message->port);

        if((message->ip == "0.0.0.0") || (message->port == 0)) {
            cannotConnectEvent(this);
            return;
        }

        HInitiate handshake(m_Museekd->server()->username(), m_Type, m_Token);
        sendMessage(handshake.make_network_packet());

        connect(message->ip, message->port);
    }
}

/**
  * This is called when a peer asks us to connect to him.
  */
void
Museek::UserSocket::reverseConnect(const std::string & user, uint token, const std::string & ip, uint port)
{
  NNLOG("museekd.user.debug", "Trying to reverse connect to %s at %s:%u, token=%u", user.c_str(), ip.c_str(), port, token);

  cannotConnectEvent.connect(this, &UserSocket::onCannotReverseConnect);

  m_User = user;
  m_Token = token;

  HPierceFirewall handshake(m_Token);
  sendMessage(handshake.make_network_packet());

  connect(ip, port);
}

void
Museek::UserSocket::onCannotReverseConnect(NewNet::ClientSocket *)
{
  NNLOG("museekd.user.debug", "Could not fulfill %s's connection request.", m_User.c_str());
  if (m_Museekd->server()->loggedIn()) {
    SCannotConnect msg(m_User, m_Token);
    m_Museekd->server()->sendMessage(msg.make_network_packet());
  }
  // Try to disconnect even if it's probably not necessary
  // then remove from reactor as this will probably not be done by disconnect()
  disconnect();
}

void
Museek::UserSocket::sendMessage(const NewNet::Buffer & buffer)
{
    // If obfuscated, generate and send the key
    unsigned char key[4];
    if (needsObfuscated()) {
        generateObfKey(key);
        send(key, 4);
    }

    // Send the message length
    unsigned char len_buf[4];
    len_buf[0] = buffer.count() & 0xff;
    len_buf[1] = (buffer.count() >> 8) & 0xff;
    len_buf[2] = (buffer.count() >> 16) & 0xff;
    len_buf[3] = (buffer.count() >> 24) & 0xff;

    if (needsObfuscated()) {
        encodeMessage(len_buf, key, 4);
    }

    send(len_buf, 4);

    // Send the data
    if (needsObfuscated()) {
        NewNet::Buffer * obfBuffer = new NewNet::Buffer(buffer);
        encodeMessage(obfBuffer->data(), key, buffer.count());
        send(obfBuffer->data(), buffer.count());
        delete obfBuffer;
    }
    else {
        send(buffer.data(), buffer.count());
    }

}

void
Museek::UserSocket::generateObfKey(unsigned char *buf)
{
    srand(time(NULL));
    for (int i = 0; i < 4; i++) {
        buf[i] = (rand() % 255) + 1;
    }
}

void
Museek::UserSocket::rotrKey(unsigned char *buf, unsigned int c)
{
    const unsigned int mask = (CHAR_BIT*sizeof(buf) - 1);

    uint32_t key_orig = buf[0] + (buf[1] << 8) + (buf[2] << 16) + (buf[3] << 24);

    c &= mask;
    uint32_t key_rotated = (key_orig>>c) | (key_orig<<( (-c)&mask ));

    buf[0] = key_rotated & 0xff;
    buf[1] = (key_rotated >> 8) & 0xff;
    buf[2] = (key_rotated >> 16) & 0xff;
    buf[3] = (key_rotated >> 24) & 0xff;
}

void
Museek::UserSocket::encodeMessage(unsigned char *buf, unsigned char *key, uint32 len)
{
    uint keySize = 4;

    uint32 key_pos = 0;
    for (int i = 0; i < len; i++) {
        key_pos = i % keySize;
        if (key_pos == 0) {
            rotrKey(key, 31);
        }
        buf[i] = buf[i] ^ key[key_pos];
    }
}
