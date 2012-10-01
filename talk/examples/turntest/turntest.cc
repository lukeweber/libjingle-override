#include <iostream>

#include "talk/base/logging.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/socketaddress.h"
#include "talk/base/testclient.h"
#include "talk/p2p/base/turnserver.h"

#include <boost/thread.hpp>
#include <boost/scoped_ptr.hpp>
#include <boost/make_shared.hpp>


using namespace cricket;

static const talk_base::SocketAddress turn_int_addr("127.0.0.1", 3478);

class TestConnection {
  public:
    TestConnection(const talk_base::SocketAddress& client_addr, 
        const talk_base::SocketAddress& peer_addr, const uint32 channel,
        const char* data)
      : main_(talk_base::Thread::Current()), ss_(main_->socketserver()) {
        client_.reset(new talk_base::TestClient(
              talk_base::AsyncUDPSocket::Create(ss_, client_addr)));
        peer_.reset(new talk_base::TestClient(
              talk_base::AsyncUDPSocket::Create(ss_, peer_addr)));
        data_ = data;
        channel_ = channel;
        peer_addr_ = peer_addr;
      }

    void Run() {
      std::cout << "Running" << std::endl;
      Allocate();
      BindChannel();
      // Send Data 
      for (int i = 0; i < 100; i++) {
        std::string client_data = std::string("client") + data_;
        ClientSendData(client_data.c_str());
        std::string received_data = PeerReceiveData();
        if (received_data != client_data) {
          std::cout << "!!! Client -> Peer Data | Validation Failed !!!" << std::endl;
        }
      }

      for (int i = 0; i < 100; i++) {
        std::string peer_data = std::string("peer") + data_;
        PeerSendData(peer_data.c_str());
        std::string received_data = ClientReceiveData();
        if (received_data != peer_data) {
          std::cout << "!!! Peer -> Client Data | Validation Failed !!!" << std::endl;
          std::cout << received_data << std::endl;
        }
      }
    }

  private:
    void Allocate() {
      std::cout << "Allocate" << std::endl;
      StunMessage* allocate_request = new TurnMessage();
      allocate_request->SetType(STUN_ALLOCATE_REQUEST);

      // Set transport attribute
      StunUInt32Attribute * transport_attr =
        StunAttribute::CreateUInt32(STUN_ATTR_REQUESTED_TRANSPORT);
      transport_attr->SetValue(0x11000000);
      allocate_request->AddAttribute(transport_attr);

      // Send allocate request
      SendStunMessage(allocate_request);

      // Check allocate response
      TurnMessage* allocate_response = ReceiveStunMessage();

      switch (allocate_response->type()) {
        case STUN_ALLOCATE_RESPONSE:
          std::cout << "allocate response" << std::endl;
          break;
        case STUN_ALLOCATE_ERROR_RESPONSE:
          std::cout << "allocate error" << std::endl;
          break;
        default:
          std::cout << "bogus" << allocate_response->type() << std::endl;
      }

      // Extract assigned relayed address
      const StunAddressAttribute* raddr_attr =
        allocate_response->GetAddress(STUN_ATTR_XOR_RELAYED_ADDRESS);
      if (!raddr_attr) {
          std::cout << "No relayed address" << std::endl;
      } else {
        relayed_addr_ = talk_base::SocketAddress(raddr_attr->ipaddr(), raddr_attr->port());
      }

      const StunAddressAttribute* maddr_attr =
        allocate_response->GetAddress(STUN_ATTR_XOR_MAPPED_ADDRESS);
      if (!maddr_attr) {
          std::cout << "No relayed address" << std::endl;
      } else {
        mapped_addr_ = talk_base::SocketAddress(maddr_attr->ipaddr(), maddr_attr->port());
      }

      delete allocate_response;
      delete allocate_request;
    }

    void BindChannel() {
      std::cout << "Bind Channel " << channel_ << std::endl;
      StunMessage* bind_request = new TurnMessage();
      bind_request->SetType(STUN_CHANNEL_BIND_REQUEST);

      // Set channel number attribute
      StunUInt32Attribute* channum_attr =
        StunAttribute::CreateUInt32(STUN_ATTR_CHANNEL_NUMBER);
      channum_attr->SetValue(channel_);
      bind_request->AddAttribute(channum_attr);

      // Set peer address attribute
      StunXorAddressAttribute* addr_attr = StunAttribute::CreateXorAddress(
          STUN_ATTR_XOR_PEER_ADDRESS);
      addr_attr->SetIP(peer_addr_.ipaddr());
      addr_attr->SetPort(peer_addr_.port());
      bind_request->AddAttribute(addr_attr);

      // Send Channel Bind request
      SendStunMessage(bind_request);

      // Receive Channel Bind response
      TurnMessage* bind_response = ReceiveStunMessage();
      switch (bind_response->type()) {
        case STUN_CHANNEL_BIND_RESPONSE:
          std::cout << "bind response" << std::endl;
          break;
        case STUN_CHANNEL_BIND_ERROR_RESPONSE:
          std::cout << "bind error" << std::endl;
          break;
        default:
          std::cout << "bogus: " << bind_response->type() << std::endl;
      }

      delete bind_response;
      delete bind_request;
    }

    void ClientSendData(const char* data) {
      std::cout << "Client Send Data" << std::endl;
      talk_base::ByteBuffer buff;
      uint32 val = channel_ | std::strlen(data);
      buff.WriteUInt32(val);
      buff.WriteBytes(data, std::strlen(data));
      client_->SendTo(buff.Data(), buff.Length(), turn_int_addr);
    }

    std::string PeerReceiveData() {
      std::cout << "Peer Receive Data" << std::endl;
      std::string raw;
      talk_base::TestClient::Packet* packet = peer_->NextPacket();
      if (packet) {
        raw = std::string(packet->buf, packet->size);
        delete packet;
      }
      return raw;
    }

    void PeerSendData(const char* data) {
      std::cout << "Peer Send Data" << std::endl;
      talk_base::ByteBuffer buff;
      buff.WriteBytes(data, std::strlen(data));
      peer_->SendTo(buff.Data(), buff.Length(), relayed_addr_);
    }

    std::string ClientReceiveData() {
      std::cout << "Client Receive Data" << std::endl;
      std::string raw;
      talk_base::TestClient::Packet* packet = client_->NextPacket();
      if (packet) {
        uint16 val1, val2;
        memcpy(&val1, packet->buf, 2);
        val2 = talk_base::NetworkToHost16(val1);
        if ( val2 & 0x4000 )  {
          uint32 received_channel = val2 << 16;
          if (received_channel == channel_) {
            raw = std::string(packet->buf + 4, packet->size - 4);
          } else {
            std::cout << "!!! Client -> Peer | Wrong Channel Number !!!" << std::endl;
          }
        }
      }
      return raw;
    }

    void SendStunMessage(const StunMessage* msg) {
      talk_base::ByteBuffer buff;
      msg->Write(&buff);
      client_->SendTo(buff.Data(), buff.Length(), turn_int_addr);
    }

    TurnMessage* ReceiveStunMessage() {
      TurnMessage* response = NULL;
      talk_base::TestClient::Packet* packet = client_->NextPacket();
      if (packet) {
        talk_base::ByteBuffer buf(packet->buf, packet->size);
        response = new TurnMessage();
        response->Read(&buf);
        delete packet;
      }
      return response;
    }

    talk_base::Thread* main_;
    talk_base::SocketServer* ss_;
    talk_base::scoped_ptr<talk_base::TestClient> client_;
    talk_base::scoped_ptr<talk_base::TestClient> peer_;
    talk_base::SocketAddress peer_addr_;
    talk_base::SocketAddress mapped_addr_;
    talk_base::SocketAddress relayed_addr_;
    const char* data_;
    uint32 channel_;
};

int main(int argc, char **argv) {
  talk_base::SocketAddress client_addr("127.0.0.1", 6000);
  talk_base::SocketAddress peer_addr("127.0.0.1", 6001);
  const char* test_data = "datadatadatadatadatadatadatadatadatadatadatadata";
  const uint32 channel_min = 0x40000000;
  const uint32 channel_max = 0x80000000;
  // uint32 channel = 0x40010000;
  uint32 channel = channel_min + 0x10000;
  if (channel == channel_max) {
    channel = channel_min;
  }

  std::cout << "Creating clients" << std::endl;
  TestConnection *conn = new TestConnection(client_addr, peer_addr, channel, test_data);
  conn->Run();
  delete conn;
}
