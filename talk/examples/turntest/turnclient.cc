#include <iostream>

#include "talk/base/flags.h"
#include "talk/base/json.h"
#include "talk/base/physicalsocketserver.h"
#include "talk/base/socketaddress.h"
#include "talk/base/testclient.h"
#include "talk/p2p/base/turnserver.h"

using namespace cricket;

static const talk_base::SocketAddress *g_turn_int_addr = NULL;
enum Mode {
  CLIENT_MODE = 0,
  PEER_MODE = 1
};

class TestConnection : public talk_base::Thread {
  public:
    TestConnection(Mode mode, talk_base::SocketAddress& client_addr, 
        talk_base::SocketAddress& peer_addr, const uint32 channel,
        const int msg_count, const char* data) {
      data_ = data;
      channel_ = channel;
      client_addr_ = client_addr;
      peer_addr_ = peer_addr;
      msg_count_ = msg_count;
      mode_ = mode;
      peer_received_cnt_ = 0;
      client_received_cnt_ = 0;
    }

    virtual void Run() {
      th_ = talk_base::Thread::Current();
      ss_ = th_->socketserver();
      switch (mode_) {
        case CLIENT_MODE:
          RunClientMode();
          break;
        case PEER_MODE:
          RunPeerMode();
          break;
        default:
          std::cout << "Out of my mode today" << std::endl;
          break;
      }
    }

  private:
    void RunClientMode() {
      client_.reset(new talk_base::TestClient(
            talk_base::AsyncUDPSocket::Create(ss_, client_addr_)));
      if (Allocate() && BindChannel() && relayed_addr_.port() != 0) {
        // Send data from client to peer
        std::cout << "Allocated and binded a channel, start sending messages" << std::endl;
        for (int i = 0; i < msg_count_; i++) {
          std::string client_data = std::string("client") + data_;
          ClientSendData(client_data.c_str());
          // Sleep 100ms between messages
          usleep(100000);
          std::cout << ".";
        }
        std::cout << "Done sending messages, waiting to receive some" << std::endl;
        while (true) {
          std::string received_data = ClientReceiveData();
          if (received_data != "") {
            std::cout << "Got data " << received_data << std::endl;
            client_received_cnt_++;
            if (client_received_cnt_ == msg_count_) {
              break;
            }
          } else {
            std::cout << "Still nothing..." << std::endl;
          }
        }
      }
    }

    void RunPeerMode() {
      peer_.reset(new talk_base::TestClient(
            talk_base::AsyncUDPSocket::Create(ss_, peer_addr_)));
      while (true) {
        std::string received_data = PeerReceiveData();
        if (received_data != "") {
          std::cout << "Got data " << received_data << std::endl;
          peer_received_cnt_++;
          if (peer_received_cnt_ == msg_count_) {
            break;
          }
        } else {
          std::cout << "Still nothing..." << std::endl;
        }
      }
      sleep(1);
      std::cout << "Done receiving messages, start sending back" << std::endl;
      for (int i = 0; i < msg_count_; i++) {
        std::string peer_data = std::string("peer") + data_;
        PeerSendData(peer_data.c_str());
        usleep(100000);
        std::cout << ".";
      }
      std::cout << std::endl << "Done sending messages back, bye!" << std::endl;
    }

    bool Allocate() {
      // std::cout << "Allocate" << std::endl;
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
            allocation_state = ALLOCATION_SUCCESS;
            break;
          case STUN_ALLOCATE_ERROR_RESPONSE:
            allocation_state = ALLOCATION_ERROR;
            break;
          default:
            allocation_state = ALLOCATION_BOGUS;
        }

        // Extract assigned relayed address
        const StunAddressAttribute* raddr_attr =
          allocate_response->GetAddress(STUN_ATTR_XOR_RELAYED_ADDRESS);
        if (!raddr_attr) {
            allocation_state = ALLOCATION_ERROR;
        } else {
          relayed_addr_ = talk_base::SocketAddress(raddr_attr->ipaddr(), raddr_attr->port());
        }

        // Extract mapped address
        const StunAddressAttribute* maddr_attr =
          allocate_response->GetAddress(STUN_ATTR_XOR_MAPPED_ADDRESS);
        if (!maddr_attr) {
            allocation_state = ALLOCATION_ERROR;
        } else {
          mapped_addr_ = talk_base::SocketAddress(maddr_attr->ipaddr(), maddr_attr->port());
        }
      } else {
        allocation_state = ALLOCATION_NULL;
        success = false;
      }

      delete allocate_response;
      delete allocate_request;

      return success;
    }

    bool BindChannel() {
      bool success = true;

      // std::cout << "Bind Channel " << channel_ << std::endl;
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
            binding_state = BINDING_SUCCESS;
            break;
          case STUN_CHANNEL_BIND_ERROR_RESPONSE:
            binding_state = BINDING_ERROR;
            break;
          default:
            binding_state = BINDING_BOGUS;
        }
      } else {
        binding_state = BINDING_NULL;
        success = false;
      }

      delete bind_response;
      delete bind_request;

      return success;
    }

    void ClientSendData(const char* data) {
      std::cout << "Client Send Data from port " << client_addr_.port() << std::endl;
      talk_base::ByteBuffer buff;
      uint32 val = channel_ | std::strlen(data);
      buff.WriteUInt32(val);
      buff.WriteBytes(data, std::strlen(data));
      client_->SendTo(buff.Data(), buff.Length(), *g_turn_int_addr);
    }

    std::string PeerReceiveData() {
      std::cout << "Peer Receive Data on port " << peer_addr_.port() << std::endl;
      std::string raw;
      talk_base::TestClient::Packet* packet = peer_->NextPacket();
      if (packet) {
        raw = std::string(packet->buf, packet->size);
        if (relayed_addr_.port() == 0) {
          relayed_addr_ = packet->addr;
        }
        delete packet;
      }
      return raw;
    }

    void PeerSendData(const char* data) {
      std::cout << "Peer Send Data from port " << peer_addr_.port() << " to " 
        << relayed_addr_.ipaddr() << ":"<< relayed_addr_.port() << std::endl;
      talk_base::ByteBuffer buff;
      buff.WriteBytes(data, std::strlen(data));
      peer_->SendTo(buff.Data(), buff.Length(), relayed_addr_);
    }

    std::string ClientReceiveData() {
      std::cout << "Client Receive Data on port " << client_addr_.port() << std::endl;
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
      client_->SendTo(buff.Data(), buff.Length(), *g_turn_int_addr);
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
    int msg_count_;
    uint32 channel_;
    Mode mode_;

    // Stats
    enum AllocationState {
      ALLOCATION_SUCCESS = 0,
      ALLOCATION_ERROR = 1,
      ALLOCATION_BOGUS = 2,
      ALLOCATION_NULL = 5,
      ALLOCATION_DEFAULT = 10
    };
    AllocationState allocation_state;

    enum BindingResult {
      BINDING_SUCCESS = 0,
      BINDING_ERROR = 1,
      BINDING_BOGUS = 2,
      BINDING_NULL = 5,
      BINDING_DEFAULT = 10
    };
    BindingResult binding_state;

    int peer_received_cnt_;
    int client_received_cnt_;
};

int main(int argc, char **argv) {
  // Define parameters
  DEFINE_string(mode, "client", "Mode to run in: client or peer");
  DEFINE_string(client_host, "127.0.0.1", "Client's IP");
  DEFINE_int(client_port, 6000, "Client's port");
  DEFINE_string(peer_host, "127.0.0.1", "Peer's IP");
  DEFINE_int(peer_port, 7000, "Peer's port");
  DEFINE_string(turn_host, "127.0.0.1", "TURN host");
  DEFINE_int(turn_port, 3478, "Turn port");
  DEFINE_int(message_cnt, 10, "Number of messages to send");
  DEFINE_bool(help, false, "Prints this message");

  // parse options
  FlagList::SetFlagsFromCommandLine(&argc, argv, true);
  if (FLAG_help) {
    FlagList::Print(NULL, false);
    return 0;
  }

  Mode mode;
  if (strcmp(FLAG_mode, "client") == 0) {
    mode = CLIENT_MODE;
  } else if (strcmp(FLAG_mode, "peer") == 0) {
    mode = PEER_MODE;
  } else {
    std::cout << "Out of my mode today" << std::endl;
    return 1;
  }

  g_turn_int_addr = new talk_base::SocketAddress(FLAG_turn_host, FLAG_turn_port);

  // 100 bytes, avg size of rtp data packet used in call example
  const char* test_data = 
    "datadatadatadatadatadatadatadatadatadatadatadatada"
    "tadatadatadatadatadatadatadatadatadatadatadatadata";
  const uint32 channel = 0x40010000;
  std::vector<talk_base::Thread*> threads;

  for (int i = 0; i < 1; i++) {
    talk_base::SocketAddress sa1 = talk_base::SocketAddress(FLAG_client_host, FLAG_client_port);
    talk_base::SocketAddress sa2 = talk_base::SocketAddress(FLAG_peer_host, FLAG_peer_port);
    threads.push_back(new TestConnection(mode, sa1, sa2, channel, FLAG_message_cnt, test_data));
    threads[i]->Start();
  }

  for (int i = 0; i < 1; i++) {
    threads[i]->Stop();
    delete threads[i];
  }
}
