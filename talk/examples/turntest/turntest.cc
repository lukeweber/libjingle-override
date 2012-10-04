#include <iostream>

#include "talk/base/logging.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/socketaddress.h"
#include "talk/base/testclient.h"
#include "talk/p2p/base/turnserver.h"

using namespace cricket;

static const talk_base::SocketAddress turn_int_addr("127.0.0.1", 3478);

class TestConnection : public talk_base::Thread {
  public:
    TestConnection(talk_base::SocketAddress& client_addr, 
        talk_base::SocketAddress& peer_addr, const uint32 channel,
        const char* data) {
        data_ = data;
        channel_ = channel;
        client_addr_ = client_addr;
        peer_addr_ = peer_addr;
      }

    virtual void Run() {
      std::cout << "Running on " << channel_ << std::endl;
      th_ = talk_base::Thread::Current();
      ss_ = th_->socketserver();
      // Create client and peer
      client_.reset(new talk_base::TestClient(
            talk_base::AsyncUDPSocket::Create(ss_, client_addr_)));
      peer_.reset(new talk_base::TestClient(
            talk_base::AsyncUDPSocket::Create(ss_, peer_addr_)));
      // Assure that we have allocated, binded and have relay address
      if (Allocate() && BindChannel() && relayed_addr_.port() != 0) {
        // Send data from client to peer
        for (int i = 0; i < 10; i++) {
          std::string client_data = std::string("client") + data_;
          ClientSendData(client_data.c_str());
          std::string received_data = PeerReceiveData();
          if (received_data != client_data) {
            std::cout << "--- " << client_addr_.port() << " -> " 
              << peer_addr_.port() << " | Validation Failed"  << std::endl;
          }
        }

        // Send data from peer to client
        for (int i = 0; i < 10; i++) {
          std::string peer_data = std::string("peer") + data_;
          PeerSendData(peer_data.c_str());
          std::string received_data = ClientReceiveData();
          if (received_data != peer_data) {
            std::cout << "--- " << peer_addr_.port() << " -> "
              << client_addr_.port() << " | Validation Failed"  << std::endl;
            std::cout << received_data << std::endl;
          }
        }
      }
    }

  private:
    bool Allocate() {
      std::cout << "Allocate" << std::endl;
      StunMessage* allocate_request = new TurnMessage();
      allocate_request->SetType(STUN_ALLOCATE_REQUEST);
      bool success = true;

      // Set transport attribute
      StunUInt32Attribute * transport_attr =
        StunAttribute::CreateUInt32(STUN_ATTR_REQUESTED_TRANSPORT);
      transport_attr->SetValue(0x11000000);
      allocate_request->AddAttribute(transport_attr);

      // Send allocate request and get the response
      SendStunMessage(allocate_request);
      TurnMessage* allocate_response = ReceiveStunMessage();

      if (allocate_response != NULL) {
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

        // Extract mapped address
        const StunAddressAttribute* maddr_attr =
          allocate_response->GetAddress(STUN_ATTR_XOR_MAPPED_ADDRESS);
        if (!maddr_attr) {
          std::cout << "No relayed address" << std::endl;
        } else {
          mapped_addr_ = talk_base::SocketAddress(maddr_attr->ipaddr(), maddr_attr->port());
        }
      } else {
        std::cout << "Got NULL on allocate request" << std::endl;
        success = false;
      }

      delete allocate_response;
      delete allocate_request;

      return success;
    }

    bool BindChannel() {
      bool success = true;

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

      // Send Channel Bind request and get the response
      SendStunMessage(bind_request);
      TurnMessage* bind_response = ReceiveStunMessage();

      if (bind_response != NULL) {
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
      } else {
        std::cout << "Got NULL on bind channel" << std::endl;
        success = false;
      }

      delete bind_response;
      delete bind_request;

      return success;
    }

    void ClientSendData(const char* data) {
      // std::cout << "Client Send Data from port " << client_addr_.port() << std::endl;
      talk_base::ByteBuffer buff;
      uint32 val = channel_ | std::strlen(data);
      buff.WriteUInt32(val);
      buff.WriteBytes(data, std::strlen(data));
      client_->SendTo(buff.Data(), buff.Length(), turn_int_addr);
    }

    std::string PeerReceiveData() {
      // std::cout << "Peer Receive Data on port " << peer_addr_.port() << std::endl;
      std::string raw;
      talk_base::TestClient::Packet* packet = peer_->NextPacket();
      if (packet) {
        raw = std::string(packet->buf, packet->size);
        delete packet;
      }
      return raw;
    }

    void PeerSendData(const char* data) {
      // std::cout << "Peer Send Data from port " << peer_addr_.port() << std::endl;
      talk_base::ByteBuffer buff;
      buff.WriteBytes(data, std::strlen(data));
      peer_->SendTo(buff.Data(), buff.Length(), relayed_addr_);
    }

    std::string ClientReceiveData() {
      // std::cout << "Client Receive Data on port " << client_addr_.port() << std::endl;
      std::string raw;
      talk_base::TestClient::Packet* packet = client_->NextPacket();
      if (packet) {
        // First 4 bytes are reserved for channel address
        // But only 2 of them are significant, the rest 2 are zeros
        uint16 val1, val2;
        memcpy(&val1, packet->buf, 2);
        val2 = talk_base::NetworkToHost16(val1);
        if ( val2 & 0x4000 )  {
          uint32 received_channel = val2 << 16;
          if (received_channel == channel_) {
            // Move the pointer and reduce the size to read
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

    talk_base::Thread* th_;
    talk_base::SocketServer* ss_;
    talk_base::scoped_ptr<talk_base::TestClient> client_;
    talk_base::scoped_ptr<talk_base::TestClient> peer_;
    talk_base::SocketAddress peer_addr_;
    talk_base::SocketAddress client_addr_;
    talk_base::SocketAddress mapped_addr_;
    talk_base::SocketAddress relayed_addr_;
    const char* data_;
    uint32 channel_;
};

int main(int argc, char **argv) {
  uint32 client_port = 6000;
  uint32 peer_port = 6001;
  const char* test_data = "datadatadatadatadatadatadatadatadatadatadatadata";
  const uint32 channel_min = 0x40000000;
  const uint32 channel_max = 0x80000000;
  const int thread_count = 10;
  std::vector<talk_base::Thread*> threads;

  uint32 channel = channel_min + 0x10000;
  for (int i = 0; i < thread_count; i++) {
    std::cout << "--- Starting thread " << i << std::endl;
    talk_base::SocketAddress sa1 = talk_base::SocketAddress("127.0.0.1", client_port);
    talk_base::SocketAddress sa2 = talk_base::SocketAddress("127.0.0.1", peer_port);
    threads.push_back(new TestConnection(sa1, sa2, channel, test_data));
    threads[i]->Start();

    client_port += 2;
    peer_port += 2;
    channel += 0x10000;

    // Check if we reached the max value for channel number
    if (channel == channel_max) {
      channel = channel_min;
    }
  }

  for (int i = 0; i < thread_count; i++) {
    threads[i]->Stop();
    delete threads[i];
  }
}
