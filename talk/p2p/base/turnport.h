/*
 * libjingle
 * Copyright 2004--2005, Google Inc.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 *  1. Redistributions of source code must retain the above copyright notice,
 *     this list of conditions and the following disclaimer.
 *  2. Redistributions in binary form must reproduce the above copyright notice,
 *     this list of conditions and the following disclaimer in the documentation
 *     and/or other materials provided with the distribution.
 *  3. The name of the author may not be used to endorse or promote products
 *     derived from this software without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE AUTHOR ``AS IS'' AND ANY EXPRESS OR IMPLIED
 * WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF
 * MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO
 * EVENT SHALL THE AUTHOR BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef TALK_P2P_BASE_TURNPORT_H_
#define TALK_P2P_BASE_TURNPORT_H_

#include <deque>
#include <string>
#include <utility>
#include <vector>
#include <map>

#include "talk/p2p/base/port.h"
#include "talk/p2p/base/stunrequest.h"

namespace cricket {

extern const char RELAY_PORT_TYPE[];
class RelayProxyConnection;

// Communicates using an allocated port on the relay server. For each
// remote candidate that we try to send data to a RelayEntry instance
// is created. The RelayEntry will try to reach the remote destination
// by connecting to all available server addresses in a pre defined
// order with a small delay in between. When a connection is
// successful all other connection attemts are aborted.
class TurnPort : public Port {
 public:
  typedef std::pair<talk_base::Socket::Option, int> OptionValue;

  // RelayPort doesn't yet do anything fancy in the ctor.
  static TurnPort* Create(
      talk_base::Thread* thread, talk_base::PacketSocketFactory* factory,
      talk_base::Network* network, const talk_base::IPAddress& ip,
      int min_port, int max_port, const std::string& username,
      const std::string& password) {
    return new TurnPort(thread, factory, network, ip, min_port, max_port,
                         username, password);
  }
  virtual ~TurnPort();
  virtual std::string GetClassname() const { return "TurnPort"; }
  std::string ToString() const;

  void AddServerAddress(const ProtocolAddress& addr);
  void AddExternalAddress(const ProtocolAddress& addr);

  void Set_server_addr(const talk_base::SocketAddress& addr) {
    server_addr_ = addr;
  }
  const talk_base::SocketAddress& Server_addr() const {
    return server_addr_;
  }
  void Set_alt_server_addr(const talk_base::SocketAddress& addr) {
    alt_server_addr_ = addr;
  }
  const talk_base::SocketAddress& Alt_server_addr() const {
    return alt_server_addr_;
  }

  const std::vector<OptionValue>& options() const { return options_; }

  virtual void PrepareAddress();
  // This will contact the secondary server and signal another candidate
  // address for this port (which may be the same as the first address).
  void PrepareSecondaryAddress();
  virtual Connection* CreateConnection(const Candidate& address,
                                       CandidateOrigin origin);
  virtual int SetOption(talk_base::Socket::Option opt, int value);
  virtual int GetError();
  virtual void OnConnectionDestroyed(Connection* conn);

  void SetNonce(const std::string& nonce) { nonce_ = nonce; }
  const std::string& GetNonce() { return nonce_; }
  void SetRealm(const std::string& realm) { realm_ = realm; }
  const std::string& GetRealm() { return realm_; }
  const std::string& GetTurnUserName() { return turn_username_; }

  void OnSendPacket(const void* data, size_t size, StunRequest* req);

  virtual void SendBindingResponse(StunMessage* request,
                                   const talk_base::SocketAddress& addr);

  virtual void SendBindingErrorResponse(StunMessage* request,
                                        const talk_base::SocketAddress& addr,
                                        int error_code,
                                        const std::string& reason);

  bool IsReady() { return ready_; }

  // Used for testing.
  sigslot::signal1<const ProtocolAddress*> SignalConnectFailure;
  sigslot::signal1<const ProtocolAddress*> SignalSoftTimeout;

 protected:
  TurnPort(talk_base::Thread* thread, talk_base::PacketSocketFactory* factory,
           talk_base::Network*, const talk_base::IPAddress& ip,
           int min_port, int max_port, const std::string& username,
           const std::string& password);
  bool Init();

  void SetReady();

  virtual int SendTo(const void* data, size_t size,
                     const talk_base::SocketAddress& addr, bool payload);

  // Dispatches the given packet to the port or connection as appropriate.
  void OnReadPacket(talk_base::AsyncPacketSocket* socket,
                    const char* data, size_t size,
                    const talk_base::SocketAddress& remote_addr);

 private:
  StunRequestManager requests_;
  talk_base::AsyncPacketSocket* socket_;
  talk_base::SocketAddress server_addr_;
  talk_base::SocketAddress alt_server_addr_;
  bool ready_;
  std::vector<OptionValue> options_;
  uint32 channel_nmbr_;
  int error_;
  std::string turn_username_;
  std::string nonce_;
  std::string realm_;
  std::map<uint32, RelayProxyConnection* > connectionMap_;

  friend class TurnAllocateRequest;
  friend class RelayProxyConnection;
};

// Send an allocate request
class TurnAllocateRequest : public StunRequest {
 public:
  TurnAllocateRequest(TurnPort* port, const talk_base::SocketAddress& server_addr);
  virtual ~TurnAllocateRequest() {}
  virtual std::string GetClassname() const { return "TurnAllocateRequest"; }
  virtual void Prepare(StunMessage* request);
  const talk_base::SocketAddress& server_addr() const { return server_addr_; }
  virtual int GetNextDelay();
  virtual void OnResponse(StunMessage* response);
  virtual void OnErrorResponse(StunMessage* response);
  virtual void OnTimeout();

 private:
  TurnPort* port_;
  uint32 start_time_;
  talk_base::SocketAddress server_addr_;
};

// Send an ChannelBindRequest
class ChannelBindRequest : public StunRequest {
 public:
  ChannelBindRequest(RelayProxyConnection* conn,
                     const talk_base::SocketAddress& server_addr);
  virtual ~ChannelBindRequest();
  virtual std::string GetClassname() const { return "ChannelBindRequest"; }
  //virtual void Prepare(TurnMessage* request);
  virtual void Prepare(StunMessage* request);
  const talk_base::SocketAddress& server_addr() const {
    return server_addr_;
  }
  virtual void OnResponse(StunMessage* response);
  virtual void OnErrorResponse(StunMessage* response);
  virtual void OnTimeout();

 private:
  RelayProxyConnection* conn_;
  talk_base::SocketAddress server_addr_;
};

// Send an RefreshRequest
class RefreshRequest : public StunRequest {
 public:
  RefreshRequest(RelayProxyConnection* conn,
                 const talk_base::SocketAddress& server_addr);
  virtual ~RefreshRequest();
  virtual std::string GetClassname() const { return "RefreshRequest"; }
  //virtual void Prepare(TurnMessage* request);
  virtual void Prepare(StunMessage* request);
  const talk_base::SocketAddress& server_addr() const {
    return server_addr_;
  }
  virtual void OnResponse(StunMessage* response);
  virtual void OnErrorResponse(StunMessage* response);
  virtual void OnTimeout();

 private:
  RelayProxyConnection* conn_;
  talk_base::SocketAddress server_addr_;
};

class RelayProxyConnection : public ProxyConnection  {
  public:
    RelayProxyConnection(TurnPort* port, uint32 channelNum,
                         const Candidate& candidate);
    virtual std::string GetClassname() const { return "RelayProxyConnection"; }
    virtual int Send(const void* data, size_t size);
    virtual int GetError()  { return error_ ; }
    virtual void Ping(uint32 now);
    void SendChannelBindRequest();
    void SendRefreshRequest();
    void ChannelBindSucess()   { bChannelBindSuccess = true; }
    uint32 GetChannelNumber()   { return chan_ ; }
    bool CheckResponse(StunMessage* msg) {
      return requests_.CheckResponse(msg);
    }
    virtual void OnSendStunPacket(const void* data,
                                  size_t size,
                                  StunRequest* req);
    // Called when a packet is received on this connection.
    virtual void OnReadPacket(const char* data, size_t size);
    const std::string& GetNonce() {
      TurnPort* port = reinterpret_cast<TurnPort*> (this->port());
      return port->GetNonce();
    }

    const std::string& GetRealm() {
      TurnPort* port = reinterpret_cast<TurnPort*> (this->port());
      return port->GetRealm();
    }

    const std::string& GetTurnUserName() {
      TurnPort* port = reinterpret_cast<TurnPort*> (this->port());
      return port->GetTurnUserName();
    }

  private:
    int    error_;
    uint32 chan_;
    uint32 nextping_;
    bool   bChannelBindSuccess;
};

}  // namespace cricket

#endif  // TALK_P2P_BASE_TURNPORT_H_
