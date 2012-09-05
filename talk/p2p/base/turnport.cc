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

#include <iostream>
#include "talk/base/asyncpacketsocket.h"
#include "talk/base/helpers.h"
#include "talk/base/logging.h"
#include "talk/base/byteorder.h"
#include "talk/p2p/base/common.h"
#include "talk/p2p/base/turnport.h"


namespace cricket {

static const uint32 kMessageConnectTimeout = 1;
static const int kKeepAliveDelay           = 10 * 60 * 1000;
static const int kRetryTimeout             = 50 * 1000;  // ICE says 50 secs
// How long to wait for a socket to connect to remote host in milliseconds
// before trying another connection.
static const int kSoftConnectTimeoutMs     = 3 * 1000;

const char TURN_PORT_TYPE[] = "relay";

//NFHACK SHOULD NOT HARDCODE
//MAKE SURE IT IS EXACTLY 39 CHARS + 1 NULL TERM = 40 UNTIL WE FIX THE HASH
//                       0        10        20        30        40
//                       01234567890123456789012345678901234567890
const char password[] = "supersecretpass56789";//01234567890123456789";

TurnPort::TurnPort(
    talk_base::Thread* thread, talk_base::PacketSocketFactory* factory,
    talk_base::Network* network, const talk_base::IPAddress& ip,
    int min_port, int max_port, const std::string& username,
    const std::string& password)
    : Port(thread, TURN_PORT_TYPE, factory, network, ip, min_port, max_port,
           username, password),
      requests_(thread),
      socket_(NULL),
      ready_(false),
      channel_nmbr_(0x40000000),
      error_(0),
      //NFHACK SHOULD NOT HARDCODE: maybe this is wy it is working unidirectional
      turn_username_(""),//nicktuen"),//nicktuentitesting@gmail.com"),
      nonce_(""),
      //NFHACK SHOULD NOT HARDCODE
      realm_("myrealm") {
  requests_.SignalSendPacket.connect(this, &TurnPort::OnSendPacket);
}

TurnPort::~TurnPort() {
  LOG_J(LS_INFO, this) << "RelayPort destructed";
  delete socket_;
  thread()->Clear(this);
}

bool TurnPort::Init() {
  socket_ = socket_factory()->CreateUdpSocket(
     talk_base::SocketAddress(ip(), 0),
     min_port(), max_port());
  if (!socket_) {
    LOG_J(LS_WARNING, this) << "UDP socket creation failed";
    return false;
  }

  socket_->SignalReadPacket.connect(this, &TurnPort::OnReadPacket);

  return true;
}

void TurnPort::AddServerAddress(const ProtocolAddress& addr) {
  LOG_J(LS_INFO, this) << "RelayPort::AddServerAddress called " << addr.address;
  server_addr_ = addr.address;
  alt_server_addr_ = addr.address;
  if (!this->Init()) {
    LOG_J(LS_WARNING, this) <<
                   "RelayPort::AddServerAddress; unable to initialize port";
  }
}

void TurnPort::AddExternalAddress(const ProtocolAddress& addr) {
}

void TurnPort::PrepareAddress() {
  LOG(INFO) << __FUNCTION__;
  //
  // Send a AllocateRequest to Relay Server
  //
  requests_.SendDelayed(new TurnAllocateRequest(this, server_addr_), 0);
  ready_ = false;
}

void TurnPort::PrepareSecondaryAddress() {
  LOG(INFO) << __FUNCTION__;
  requests_.SendDelayed(new TurnAllocateRequest(this, alt_server_addr_), 0);
}

void TurnPort::SetReady() {
  if (!ready_) {
    ready_ = true;
    SignalAddressReady(this);
  }
}

void TurnPort::OnConnectionDestroyed(Connection* conn) {
  LOG_J(LS_INFO, this) << "RelayPort::OnConnectionDestroyed";
  RelayProxyConnection* rpc = reinterpret_cast<RelayProxyConnection*>(conn);
  connectionMap_.erase(rpc->GetChannelNumber());
  Port::OnConnectionDestroyed(conn);
}

Connection* TurnPort::CreateConnection(const Candidate& address,
                                        CandidateOrigin origin) {
  if (address.protocol() != "udp")
    return NULL;

  RelayProxyConnection* conn = new RelayProxyConnection(this, channel_nmbr_,
                                                        address);
  connectionMap_[channel_nmbr_] = conn;
  AddConnection(conn);
  channel_nmbr_ += 0x10000;
  // 0x4000 through 0x7FFF: These values are the allowed channel numbers
  // (16,383 possible values).
  if ( channel_nmbr_ == 0x80000000 )
    channel_nmbr_ = 0x40000000;
  return conn;
}

int TurnPort::SetOption(talk_base::Socket::Option opt, int value) {
  int result = 0;
  socket_->SetOption(opt, value);
  options_.push_back(OptionValue(opt, value));
  return result;
}

int TurnPort::GetError() {
  return error_;
}

void TurnPort::OnReadPacket(talk_base::AsyncPacketSocket* socket,
                             const char* data, size_t size,
                             const talk_base::SocketAddress& remote_addr) {
  ASSERT(socket == socket_);

  // if we received a channel data request, send it to appropriate
  // connection and let it handle
  uint16 val1, val2;
  memcpy(&val1, data, 2);
  val2 = talk_base::NetworkToHost16(val1);

  if ( val2 & 0x4000 )  {
    std::map<uint32, RelayProxyConnection*>::iterator it;
    uint32 val = val2 << 16;
    // LOG(LS_INFO) << "channum " << val;
    if ((it = connectionMap_.find(val)) != connectionMap_.end()) {
      RelayProxyConnection * pRPC = it->second;
      return pRPC->OnReadPacket (data+4, size-4);
    }
  }

  // this is not channel data. Check if this is a response for a STUN request
  // we sent out (Allocate Request) if yes,
  // let our request map handle the request
  talk_base::ByteBuffer buf(data, size);
  TurnMessage msg;
  if (!msg.Read(&buf)) {
    // LOG(LS_INFO) << "RelayPort::OnReadPacket Incoming packet was not STUN";
    return;
  }

  // LOG(LS_INFO) << "requests_.CheckResponse";

  if (!requests_.CheckResponse(&msg)) {
    // LOG(LS_INFO) << "RelayProxyConnection.CheckResponse";
    //
    // this is not a request we sent out; ask each connection to handle request
    //
    bool bHandled = false;
    std::map<uint32, RelayProxyConnection *>::iterator it =
                connectionMap_.begin();
    for ( ; (it != connectionMap_.end() && !bHandled); it++ ) {
      bHandled = it->second->CheckResponse(&msg);
    }
  }
}

int TurnPort::SendTo(const void* data, size_t size,
                      const talk_base::SocketAddress& addr, bool payload) {
  // LOG_J(LS_INFO, this) << " RelayPort::SendTo; Sending Packet to: " << addr;

  int sent = socket_->SendTo(data, size, alt_server_addr_);
  if (sent < 0) {
    error_ = socket_->GetError();
    LOG_J(LS_ERROR, this) << "UDP send of " << size
                          << " bytes failed with error " << error_;
  }
  return sent;
}

void TurnPort::OnSendPacket(const void* data, size_t size, StunRequest* req) {
  talk_base::SocketAddress addr;

  if (req->type() == STUN_ALLOCATE_REQUEST) {
    TurnAllocateRequest* aReq = static_cast<TurnAllocateRequest*>(req);
    addr = aReq->server_addr();
  } else if (req->type() == STUN_CHANNEL_BIND_REQUEST) {
    ChannelBindRequest* aReq = static_cast<ChannelBindRequest*>(req);
    addr = aReq->server_addr();
  } else if (req->type() == STUN_REFRESH_REQUEST) {
    RefreshRequest* aReq = static_cast<RefreshRequest*>(req);
    addr = aReq->server_addr();
  }

  if (addr.IsNil()) {
    addr = this->Alt_server_addr();
  }

  // LOG(LS_INFO) << "RelayPort::OnSendPacket; Sending request to: " << addr;

  if (socket_->SendTo(data, size, addr) < 0)
    LOG(LS_ERROR) << socket_->GetError() <<
      "RelayPort::OnSendPacket sendto error";
}

void TurnPort::SendBindingResponse(StunMessage* request,
                                    const talk_base::SocketAddress& addr) {
  // LOG_J(LS_VERBOSE, this)<< "RelayPort::SendBindingResponse";

  ASSERT(request->type() == STUN_BINDING_REQUEST);

  std::map<uint32, RelayProxyConnection*>::iterator it;
  uint32 chanNum;

  Connection* conn2 = this->GetConnection(addr);

  if (NULL != conn2) {
      RelayProxyConnection* pRPC =
          reinterpret_cast<RelayProxyConnection*>(conn2);
      chanNum = pRPC->GetChannelNumber();
  }

  // Retrieve the username from the request.
  const StunByteStringAttribute* username_attr =
               request->GetByteString(STUN_ATTR_USERNAME);
  ASSERT(username_attr != NULL);
  if (username_attr == NULL) {
    // No valid username, skip the response.
    return;
  }

  // Fill in the response message.
  TurnMessage response;
  response.SetType(STUN_BINDING_RESPONSE);
  response.SetTransactionID(request->transaction_id());

  StunByteStringAttribute* username2_attr =
             StunAttribute::CreateByteString(STUN_ATTR_USERNAME);
  username2_attr->CopyBytes(username_attr->bytes(), username_attr->length());
  response.AddAttribute(username2_attr);

  StunAddressAttribute* addr_attr =
      StunAttribute::CreateAddress(STUN_ATTR_MAPPED_ADDRESS);
  addr_attr->SetPort(addr.port());
  addr_attr->SetIP(addr.ipaddr());
  response.AddAttribute(addr_attr);

  // Send the response message.
  // NOTE: If we wanted to, this is where we would add the HMAC.
  talk_base::ByteBuffer buf;
  response.Write(&buf);

  talk_base::ByteBuffer buff2;
  uint32 val2 = chanNum | buf.Length();
  buff2.WriteUInt32(val2);
  buff2.WriteBytes((const char*) buf.Data(), buf.Length());

  if (SendTo(buff2.Data(), buff2.Length(), alt_server_addr_, false) < 0) {
    LOG_J(LS_ERROR, this) << "Failed to send STUN ping response to "
                          << addr.ToString();
  }

  // The fact that we received a successful request means that this connection
  // (if one exists) should now be readable.
  Connection* conn = GetConnection(addr);
  ASSERT(conn != NULL);
  if (conn)
    conn->ReceivedPing();
}

void TurnPort::SendBindingErrorResponse(StunMessage* request,
                                         const talk_base::SocketAddress& addr,
                                         int error_code,
                                         const std::string& reason) {
  ASSERT(request->type() == STUN_BINDING_REQUEST);

  std::map<uint32, RelayProxyConnection*>::iterator it;
  uint32 chanNum;
  Connection* conn2 = this->GetConnection(addr);
  if (NULL != conn2)  {
      RelayProxyConnection* pRPC =
            reinterpret_cast<RelayProxyConnection*>(conn2);
      chanNum = pRPC->GetChannelNumber();
  }

  // Retrieve the username from the request. If it didn't have one, we
  // shouldn't be responding at all.
  const StunByteStringAttribute* username_attr =
      request->GetByteString(STUN_ATTR_USERNAME);
  ASSERT(username_attr != NULL);
  if (username_attr == NULL) {
    // No valid username, skip the response.
    return;
  }

  // Fill in the response message.
  TurnMessage response;
  response.SetType(STUN_BINDING_ERROR_RESPONSE);
  response.SetTransactionID(request->transaction_id());

  StunByteStringAttribute* username2_attr =
      StunAttribute::CreateByteString(STUN_ATTR_USERNAME);
  username2_attr->CopyBytes(username_attr->bytes(), username_attr->length());
  response.AddAttribute(username2_attr);

  StunErrorCodeAttribute* error_attr = StunAttribute::CreateErrorCode();
  error_attr->SetCode(error_code);
  error_attr->SetReason(reason);
  response.AddAttribute(error_attr);

  // Send the response message.
  // NOTE: If we wanted to, this is where we would add the HMAC.
  talk_base::ByteBuffer buf;
  response.Write(&buf);

  talk_base::ByteBuffer buff2;
  uint32 val2 = chanNum | buf.Length();
  buff2.WriteUInt32(val2);
  buff2.WriteBytes((const char*) buf.Data(), buf.Length());

  LOG_J(LS_INFO, this) << "Sending STUN binding error: reason=" << reason
                       << " to " << addr.ToString();

  if (SendTo(buff2.Data(), buff2.Length(), alt_server_addr_, false) < 0) {
    LOG_J(LS_ERROR, this) << "Failed to send STUN binding error response to "
                          << addr.ToString();
  }
}

//
// TurnAllocateRequest implementation
//
TurnAllocateRequest::TurnAllocateRequest(TurnPort* port,
                                 const talk_base::SocketAddress& server_addr) :
    StunRequest(new TurnMessage()),
    port_(port),
    server_addr_(server_addr) {
  start_time_ = talk_base::Time();
}

void TurnAllocateRequest::Prepare(StunMessage* request) {
  LOG(LS_INFO) << "TurnAllocateRequest::Prepare";

  bool AddMI = false;
  request->SetType(STUN_ALLOCATE_REQUEST);
  if (port_->GetNonce().length() != 0) {
    AddMI = true;
    StunByteStringAttribute* username_attr =
              StunAttribute::CreateByteString(STUN_ATTR_USERNAME);
    username_attr->CopyBytes(port_->turn_username_.c_str(), 8);
    request->AddAttribute(username_attr);

    StunByteStringAttribute* nonce_attr =
              StunAttribute::CreateByteString(STUN_ATTR_NONCE);
    nonce_attr->CopyBytes(port_->GetNonce().c_str(),
                          port_->GetNonce().length());
    request->AddAttribute(nonce_attr);
  }

  if (port_->GetRealm().length() != 0) {
    StunByteStringAttribute* realm_attr =
            StunAttribute::CreateByteString(STUN_ATTR_REALM);
    realm_attr->CopyBytes(port_->GetRealm().c_str(),
                          port_->GetRealm().length());
    request->AddAttribute(realm_attr);
  }

  StunUInt32Attribute * transport_attr =
             StunAttribute::CreateUInt32(STUN_ATTR_REQUESTED_TRANSPORT);
  transport_attr->SetValue(0x11000000);
  request->AddAttribute(transport_attr);
  
  if ( AddMI ) {
    request->AddMessageIntegrity(password);
  }
  LOG(LS_INFO) << "LOGT REQ = " << request->ToString() << std::endl;

}

int TurnAllocateRequest::GetNextDelay() {
  int delay = 100 * talk_base::_max(1 << count_, 2);
  count_ += 1;
  if (count_ == 5)
    timeout_ = true;
  return delay;
}

void TurnAllocateRequest::OnResponse(StunMessage* response) {
  const StunAddressAttribute* addr_attr =
          response->GetAddress(STUN_ATTR_XOR_RELAYED_ADDRESS);
  if (!addr_attr) {
    LOG(LS_ERROR) << "Allocate response missing mapped address.";
    LOG(LS_ERROR) << response->ToString();
  } else if (addr_attr->family() != STUN_ADDRESS_IPV4) {
    LOG(LS_ERROR) << "Mapped address has bad family";
  } else {
    talk_base::SocketAddress addr(addr_attr->ipaddr(), addr_attr->port());
    ProtocolType proto = PROTO_UDP;
    LOG(INFO) << "Relay allocate succeeded: " << ProtoToString(proto)
              << " @ " << addr.ToString();
    port_->AddAddress(addr, port_->socket_->GetLocalAddress(), "udp", true);
  }
}

void TurnAllocateRequest::OnErrorResponse(StunMessage* response) {
  const StunErrorCodeAttribute* attr = response->GetErrorCode();
  if (!attr) {
    LOG(LS_ERROR) << "Bad allocate response error code";
  } else {
    LOG(LS_ERROR) << "Allocate error response:"
              << " code=" << static_cast<int>(attr->code())
              << " reason='" << attr->reason() << "'";
  }

  if (attr->code() == cricket::STUN_ERROR_UNAUTHORIZED) {
    const StunByteStringAttribute* nonce_attr =
          response->GetByteString(STUN_ATTR_NONCE);
     ASSERT(nonce_attr != NULL);
     port_->SetNonce(std::string(nonce_attr->bytes(), nonce_attr->length()));

     const StunByteStringAttribute* realm_attr =
                        response->GetByteString(STUN_ATTR_REALM);
     ASSERT(realm_attr != NULL);
     port_->SetRealm(std::string(realm_attr->bytes(), realm_attr->length()));

     std::string nonce(nonce_attr->bytes(), nonce_attr->length());
     std::string realm(realm_attr->bytes(), realm_attr->length());

     LOG(INFO) << "nonce = " << nonce << " realm = " << realm;

     port_->PrepareSecondaryAddress();
  } else if (attr->code() == cricket::STUN_ERROR_TRY_ALTERNATE) {
    const StunAddressAttribute* addr_attr =
                response->GetAddress(STUN_ATTR_ALTERNATE_SERVER);
    if (!addr_attr) {
      LOG(LS_ERROR) << "Response has no source address";
      return;
    } else if (addr_attr->family() != 1) {
      LOG(LS_ERROR) << "Source address has bad family";
      return;
    }

    talk_base::SocketAddress remote_addr(addr_attr->ipaddr(),
                                         addr_attr->port());

    LOG(INFO) << "Alternate Server: " << remote_addr.ToString();

    port_->Set_alt_server_addr(remote_addr);
    port_->PrepareSecondaryAddress();
  } else {
    LOG(LS_ERROR) << "Unhandled error code";
  }
}

void TurnAllocateRequest::OnTimeout() {
  LOG(LS_ERROR) << "Allocate request timed out";
  port_->SignalAddressError(port_);
}

//
// RelayProxyConnection implementation
//
RelayProxyConnection::RelayProxyConnection(TurnPort* port, uint32 channelNum,
                                            const Candidate& candidate)
                                          : ProxyConnection(port, 0, candidate),
                                            error_(0) {
  chan_ = channelNum;
  nextping_ = talk_base::Time();
  bChannelBindSuccess = false;
}

void RelayProxyConnection::Ping(uint32 now) {
  if ( talk_base::Time() >= nextping_ ) {
    LOG(LS_VERBOSE) << "Sending ChannelBindRequest";
    SendChannelBindRequest();
    if (bChannelBindSuccess) {
      LOG(LS_VERBOSE) << "Sending RefreshRequest";
      SendRefreshRequest();
    }
    nextping_ += 60000;
  }
  //
  // let base class send out keep alives
  //
  if (bChannelBindSuccess)  {
    Connection::Ping(now);
  }
}

void RelayProxyConnection::SendChannelBindRequest() {
  TurnPort* port = reinterpret_cast<TurnPort*> (this->port());
  ChannelBindRequest *request = new ChannelBindRequest(this,
                                                       port->Alt_server_addr());
  requests_.Send(request);
}

void RelayProxyConnection::SendRefreshRequest() {
  TurnPort* port = reinterpret_cast<TurnPort*> (this->port());
  RefreshRequest *request = new RefreshRequest(this,
                                               port->Alt_server_addr());
  requests_.Send(request);
}

//
// Called when a packet is received on this connection.
//
void RelayProxyConnection::OnReadPacket(const char* data, size_t size) {
  // LOG(LS_VERBOSE) << "RelayProxyConnection::OnReadPacket; invoking base" ;
  Connection::OnReadPacket(data, size);
}

void RelayProxyConnection::OnSendStunPacket(const void* data, size_t size,
                                            StunRequest* req) {
  if ( (req->type() == STUN_REFRESH_REQUEST) ||
       (req->type() == STUN_CHANNEL_BIND_REQUEST) ) {
    // LOG(LS_INFO) << "RelayProxyConnection::OnSendStunPacket";
    TurnPort* port = reinterpret_cast<TurnPort*>(this->port());
    port->OnSendPacket(data, size, req);
  } else {
    //
    // All requests other than STUN_REFRESH/CHANNEL_BIND need to be
    // wrapped inside of a Channel Data request
    talk_base::ByteBuffer buff;
    uint32 val = chan_ | size;
    buff.WriteUInt32(val);
    buff.WriteBytes((const char*) data, size);
    TurnPort* port = reinterpret_cast<TurnPort*>(this->port());
    port->SendTo(buff.Data(), buff.Length(), port->Alt_server_addr(), false);
  }
}

int RelayProxyConnection::Send(const void* data, size_t size)  {
  // LOG(LS_VERBOSE) << "RelayProxyConnection::Send ....";

  talk_base::ByteBuffer buff;
  uint32 val = chan_ | size;
  buff.WriteUInt32(val);
  buff.WriteBytes((const char*) data, size);
  TurnPort* port = reinterpret_cast<TurnPort*>(this->port());
  return (port->SendTo(buff.Data(), buff.Length(), port->Alt_server_addr(),
          false));

}  // namespace cricket

//
// ChannelBindRequest implementation
//
ChannelBindRequest::ChannelBindRequest(RelayProxyConnection* conn,
                                   const talk_base::SocketAddress& server_addr)
                                   : StunRequest(new TurnMessage()),
                                     conn_(conn),
                                     server_addr_(server_addr) {
}

ChannelBindRequest::~ChannelBindRequest() {
}

void ChannelBindRequest::Prepare(StunMessage* request) {
  LOG(LS_VERBOSE) << "ChannelBindRequest::Prepare";

  request->SetType(STUN_CHANNEL_BIND_REQUEST);

  StunByteStringAttribute* username_attr =
       StunAttribute::CreateByteString(STUN_ATTR_USERNAME);
  username_attr->CopyBytes(conn_->GetTurnUserName().c_str(),
                           conn_->GetTurnUserName().length());
  request->AddAttribute(username_attr);

  StunByteStringAttribute* nonce_attr =
      StunAttribute::CreateByteString(STUN_ATTR_NONCE);
  nonce_attr->CopyBytes(conn_->GetNonce().c_str(),
                        conn_->GetNonce().length());
  request->AddAttribute(nonce_attr);

  StunByteStringAttribute* realm_attr =
      StunAttribute::CreateByteString(STUN_ATTR_REALM);
  realm_attr->CopyBytes(conn_->GetRealm().c_str(),
                        conn_->GetRealm().length());
  request->AddAttribute(realm_attr);

  StunUInt32Attribute* channum_attr =
                 StunAttribute::CreateUInt32(STUN_ATTR_CHANNEL_NUMBER);
  channum_attr->SetValue(conn_->GetChannelNumber());
  request->AddAttribute(channum_attr);

  StunXorAddressAttribute* addr_attr = StunAttribute::CreateXorAddress(
                                            STUN_ATTR_XOR_PEER_ADDRESS);
  addr_attr->SetIP(conn_->remote_candidate().address().ipaddr());
  addr_attr->SetPort(conn_->remote_candidate().address().port());
  request->AddAttribute(addr_attr);

  request->AddMessageIntegrity(password);
  LOG(LS_INFO) << "LOGT REQ = " << request->ToString() << std::endl;

}

void ChannelBindRequest::OnResponse(StunMessage* response) {
  LOG(LS_INFO) << "ChannelBindRequest::OnResponse";
  conn_->ChannelBindSucess();
}

void ChannelBindRequest::OnErrorResponse(StunMessage* response) {
  LOG(LS_ERROR) << "ChannelBindRequest::OnErrorResponse!";
  const StunErrorCodeAttribute* attr = response->GetErrorCode();
  if (!attr) {
    LOG(LS_ERROR) << "Bad ChannelBind response error code";
  } else {
    LOG(LS_ERROR) << "ChannelBind error response:"
                  << " code=" << static_cast<int>(attr->code())
                  << " reason='" << attr->reason() << "'";
    if (attr->code() == cricket::STUN_ERROR_STALE_NONCE) {
      const StunByteStringAttribute* nonce_attr = response->GetByteString(
                                                    STUN_ATTR_NONCE);
      ASSERT(nonce_attr != NULL);
      if ( nonce_attr != NULL ) {
        TurnPort* port = reinterpret_cast<TurnPort*>(conn_->port());
        port->SetNonce(std::string(nonce_attr->bytes(), nonce_attr->length()));
        LOG(LS_INFO) << "Re-Sending ChannelBindRequest with new Nonce";
        conn_->SendChannelBindRequest();
      }
    }
  }
}

void ChannelBindRequest::OnTimeout() {
  LOG(LS_ERROR) << "ChannelBindRequest::OnTimeOut!";
}

//
// RefreshRequest implementation
//
RefreshRequest::RefreshRequest(RelayProxyConnection* conn,
                               const talk_base::SocketAddress& server_addr)
                               : StunRequest(new TurnMessage()),
                                 conn_(conn),
                                 server_addr_(server_addr) {
}

RefreshRequest::~RefreshRequest() {
}

void RefreshRequest::Prepare(StunMessage* request) {
  request->SetType(STUN_REFRESH_RESPONSE);

  StunByteStringAttribute* username_attr = StunAttribute::CreateByteString(
                                            STUN_ATTR_USERNAME);
  username_attr->CopyBytes(conn_->GetTurnUserName().c_str(),
                           conn_->GetTurnUserName().length());
  request->AddAttribute(username_attr);

  StunByteStringAttribute* nonce_attr = StunAttribute::CreateByteString(
                                         STUN_ATTR_NONCE);
  nonce_attr->CopyBytes(conn_->GetNonce().c_str(),
                        conn_->GetNonce().length());
  request->AddAttribute(nonce_attr);

  StunByteStringAttribute* realm_attr = StunAttribute::CreateByteString(
                                          STUN_ATTR_REALM);
  realm_attr->CopyBytes(conn_->GetRealm().c_str(),
                        conn_->GetRealm().length());
  request->AddAttribute(realm_attr);

  request->AddMessageIntegrity(password);
}

void RefreshRequest::OnResponse(StunMessage* response) {
  // LOG(LS_INFO) << "RefreshRequest::OnResponse; refresh request succeeded";
}

void RefreshRequest::OnErrorResponse(StunMessage* response) {
  LOG(LS_ERROR) << "RefreshRequest::OnErrorResponse!";
  const StunErrorCodeAttribute* attr = response->GetErrorCode();
  if (!attr) {
    LOG(LS_ERROR) << "Bad RefreshRequest response error code";
  } else {
    LOG(LS_ERROR) << "RefreshRequest error response:"
                  << " code=" << static_cast<int>(attr->code())
                  << " reason='" << attr->reason() << "'";
    if (attr->code() == cricket::STUN_ERROR_STALE_NONCE) {
      const StunByteStringAttribute* nonce_attr = response->GetByteString(
                                                            STUN_ATTR_NONCE);
      ASSERT(nonce_attr != NULL);
      if ( nonce_attr != NULL ) {
        TurnPort* port = reinterpret_cast<TurnPort*>(conn_->port());
        port->SetNonce(std::string(nonce_attr->bytes(),
                       nonce_attr->length()));
        // Resend the request with the new Nonce
        LOG(LS_INFO) <<"Re-Sending RefreshRequest with new Nonce";
        conn_->SendRefreshRequest();
      }
    }
  }
}

void RefreshRequest::OnTimeout() {
  LOG(LS_ERROR) << "RefreshRequest::OnTimeOut!";
}

}  // namespace cricket
