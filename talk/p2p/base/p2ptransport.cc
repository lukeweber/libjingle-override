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

#include "talk/p2p/base/p2ptransport.h"

#include <string>
#include <vector>

#include "talk/base/base64.h"
#include "talk/base/common.h"
#include "talk/base/stringencode.h"
#include "talk/base/stringutils.h"
#include "talk/p2p/base/constants.h"
#include "talk/p2p/base/p2ptransportchannel.h"
#include "talk/p2p/base/parsing.h"
#include "talk/p2p/base/sessionmanager.h"
#include "talk/p2p/base/sessionmessages.h"
#include "talk/xmllite/qname.h"
#include "talk/xmllite/xmlelement.h"
#include "talk/xmpp/constants.h"

namespace {

// Limits for GICE and ICE username sizes.
const size_t kMaxGiceUsernameSize = 64;
const size_t kMaxIceUsernameSize = 512;

}  // namespace

namespace cricket {

static buzz::XmlElement* NewTransportElement(const std::string& name) {
  return new buzz::XmlElement(buzz::QName(name, LN_TRANSPORT), true);
}

P2PTransport::P2PTransport(talk_base::Thread* signaling_thread,
                           talk_base::Thread* worker_thread,
                           const std::string& content_name,
                           PortAllocator* allocator,
                           const std::string& transport_type)
    : Transport(signaling_thread, worker_thread,
                content_name, transport_type, allocator) {
}

P2PTransport::~P2PTransport() {
  DestroyAllChannels();
}

TransportChannelImpl* P2PTransport::CreateTransportChannel(int component) {
  return new P2PTransportChannel(content_name(), component, this,
                                 port_allocator());
}

void P2PTransport::DestroyTransportChannel(TransportChannelImpl* channel) {
  delete channel;
}

bool P2PTransportParser::ParseTransportDescription(
    const buzz::XmlElement* elem,
    const CandidateTranslator* translator,
    TransportDescription* desc,
    ParseError* error) {
  ASSERT(elem->Name().LocalPart() == LN_TRANSPORT);
  desc->transport_type = elem->Name().Namespace();
  if (desc->transport_type != NS_GINGLE_P2P && desc->transport_type != NS_JINGLE_ICE_UDP)
    return BadParse("Unsupported transport type", error);

  if (desc->transport_type == NS_JINGLE_ICE_UDP && elem->HasAttr(QN_ICE_PWD)){
    desc->ice_pwd = elem->Attr(QN_ICE_PWD).data();
    desc->ice_ufrag = elem->Attr(QN_ICE_USER_FRAG).data();
  }

  for (const buzz::XmlElement* transport_elem = elem->FirstElement();
       transport_elem != NULL;
       transport_elem = transport_elem->NextElement()) {
    // Only look at local part because the namespace might (eventually)
    // be NS_GINGLE_P2P or NS_JINGLE_ICE_UDP.
    if (transport_elem->Name().LocalPart() == LN_CANDIDATE) {
      Candidate candidate;
      if (!ParseCandidate(
          desc->transport_type == NS_JINGLE_ICE_UDP ? ICEPROTO_RFC5245 : ICEPROTO_GOOGLE,
          transport_elem, translator, &candidate, error)) {
        return false;
      }

      //TODO: once I find out where we use candidate.password()
      /*if (desc->transport_type == NS_JINGLE_ICE_UDP){
        candidate.set_username(desc->ice_ufrag);
        candidate.set_password(desc->ice_pwd);
      }*/

      desc->candidates.push_back(candidate);
    } else if (transport_elem->Name() == QN_JINGLE_DTLS_FINGERPRINT){
      desc->identity_fingerprint.reset(talk_base::SSLFingerprint::CreateFromRfc4572(
          transport_elem->Attr(QN_JINGLE_DTLS_HASH), transport_elem->BodyText()));
    }
  }
  return true;
}

bool P2PTransportParser::WriteTransportDescription(
    const TransportDescription& desc,
    const CandidateTranslator* translator,
    buzz::XmlElement** out_elem,
    WriteError* error) {
  TransportProtocol proto = TransportProtocolFromDescription(&desc);
  talk_base::scoped_ptr<buzz::XmlElement> trans_elem(
      NewTransportElement(desc.transport_type));

  
  if (proto != ICEPROTO_RFC5245 && proto != ICEPROTO_GOOGLE) {
    LOG(LS_ERROR) << "Failed to serialize unknown ice proto into TransportDescription";
    return false;
  }

  if (proto == ICEPROTO_RFC5245 && desc.ice_ufrag != ""){
    trans_elem.get()->AddAttr(QN_ICE_USER_FRAG, desc.ice_ufrag);
    trans_elem.get()->AddAttr(QN_ICE_PWD, desc.ice_pwd);
  }

  for (std::vector<Candidate>::const_iterator iter = desc.candidates.begin();
       iter != desc.candidates.end(); ++iter) {
    talk_base::scoped_ptr<buzz::XmlElement> cand_elem;
    if (proto == ICEPROTO_GOOGLE){
      cand_elem.reset(new buzz::XmlElement(QN_GINGLE_P2P_CANDIDATE));
      if (!WriteCandidate(proto, *iter, translator, cand_elem.get(), error)) {
        return false;
      }
    } else if (proto == ICEPROTO_RFC5245){
      cand_elem.reset(new buzz::XmlElement(QN_JINGLE_TRANSPORT_CANDIDATE));
      if (!WriteCandidateRfc5245(proto, *iter, translator, cand_elem.get(), error)) {
        return false;
      }
    }
    
    trans_elem->AddElement(cand_elem.release());
  }

  *out_elem = trans_elem.release();
  return true;
}

bool P2PTransportParser::ParseGingleCandidate(
    const buzz::XmlElement* elem,
    const CandidateTranslator* translator,
    Candidate* candidate,
    ParseError* error) {
  return ParseCandidate(ICEPROTO_GOOGLE, elem, translator, candidate, error);
}

bool P2PTransportParser::WriteGingleCandidate(
    const Candidate& candidate,
    const CandidateTranslator* translator,
    buzz::XmlElement** out_elem,
    WriteError* error) {
  talk_base::scoped_ptr<buzz::XmlElement> elem(
      new buzz::XmlElement(QN_GINGLE_CANDIDATE));                                     
  bool ret = WriteCandidate(ICEPROTO_GOOGLE, candidate, translator, elem.get(),
                            error);
  if (ret) {
    *out_elem = elem.release();
  }
  return ret;
}

bool P2PTransportParser::VerifyUsernameFormat(TransportProtocol proto,
                                              const std::string& username,
                                              ParseError* error) {
  if (proto == ICEPROTO_GOOGLE || proto == ICEPROTO_HYBRID) {
    if (username.size() > kMaxGiceUsernameSize)
      return BadParse("candidate username is too long", error);
    if (!talk_base::Base64::IsBase64Encoded(username))
      return BadParse("candidate username has non-base64 encoded characters",
                      error);
  } else if (proto == ICEPROTO_RFC5245) {
    if (username.size() > kMaxIceUsernameSize)
      return BadParse("candidate username is too long", error);
  }
  return true;
}

bool P2PTransportParser::ParseCandidate(TransportProtocol proto,
                                        const buzz::XmlElement* elem,
                                        const CandidateTranslator* translator,
                                        Candidate* candidate,
                                        ParseError* error) {
  
  ASSERT(proto == ICEPROTO_GOOGLE || proto == ICEPROTO_RFC5245);
  ASSERT(translator != NULL);

  if (!elem->HasAttr(buzz::QN_NAME) ||
      !elem->HasAttr(QN_ADDRESS) ||
      !elem->HasAttr(QN_PORT) ||
      // ICEPROTO_RFC5245 gets this from transport ice-ufrag
      (!elem->HasAttr(QN_USERNAME) && proto == ICEPROTO_GOOGLE) || 
      !elem->HasAttr(QN_PROTOCOL) ||
      !elem->HasAttr(QN_GENERATION)) {
    return BadParse("candidate missing required attribute", error);
  }

  talk_base::SocketAddress address;
  if (!ParseAddress(elem, QN_ADDRESS, QN_PORT, &address, error))
    return false;

  std::string channel_name = elem->Attr(buzz::QN_NAME);
  int component = 0;
  if (!translator ||
      !translator->GetComponentFromChannelName(channel_name, &component)) {
    return BadParse("candidate has unknown channel name " + channel_name,
                    error);
  }

  candidate->set_component(component);
  candidate->set_address(address);
  if (proto == ICEPROTO_GOOGLE) {
    float preference = 0.0;
    if (!GetXmlAttr(elem, QN_PREFERENCE, 0.0f, &preference)) {
      return BadParse("candidate has unknown preference", error);
    }
    candidate->set_preference(preference);
    candidate->set_username(elem->Attr(QN_USERNAME));
    if (elem->HasAttr(QN_PASSWORD))
      candidate->set_password(elem->Attr(QN_PASSWORD));
  } else if(proto == ICEPROTO_RFC5245) {
    uint32 priority = GetXmlAttr(elem, QN_PRIORITY, 0);
    candidate->set_priority(priority);
  }
  
  candidate->set_protocol(elem->Attr(QN_PROTOCOL));
  candidate->set_generation_str(elem->Attr(QN_GENERATION));
  
  if (elem->HasAttr(buzz::QN_TYPE))
    candidate->set_type(elem->Attr(buzz::QN_TYPE));
  if (elem->HasAttr(QN_NETWORK))
    candidate->set_network_name(elem->Attr(QN_NETWORK));

  if (!VerifyUsernameFormat(proto, candidate->username(), error))
    return false;

  return true;
}

bool P2PTransportParser::WriteCandidateRfc5245(TransportProtocol proto,
                                        const Candidate& candidate,
                                        const CandidateTranslator* translator,
                                        buzz::XmlElement* elem,
                                        WriteError* error) {
  ASSERT(proto == ICEPROTO_RFC5245);
  ASSERT(translator != NULL);
  std::string channel_name;
  if (!translator ||
      !translator->GetChannelNameFromComponent(
          candidate.component(), &channel_name)) {
    return BadWrite("Cannot write candidate because of unknown component.",
                    error);
  }

  elem->SetAttr(buzz::QN_NAME, channel_name);
  elem->SetAttr(QN_ADDRESS, candidate.address().ipaddr().ToString());
  elem->SetAttr(QN_PORT, candidate.address().PortAsString());
  AddXmlAttr(elem, QN_PRIORITY, candidate.priority());
  elem->SetAttr(QN_PROTOCOL, candidate.protocol());
  elem->SetAttr(QN_GENERATION, candidate.generation_str());
  elem->SetAttr(buzz::QN_TYPE, candidate.type());
  if (!candidate.network_name().empty())
    elem->SetAttr(QN_NETWORK, candidate.network_name());

  return true;

}

bool P2PTransportParser::WriteCandidate(TransportProtocol proto,
                                        const Candidate& candidate,
                                        const CandidateTranslator* translator,
                                        buzz::XmlElement* elem,
                                        WriteError* error) {
  ASSERT(proto == ICEPROTO_GOOGLE);
  ASSERT(translator != NULL);

  std::string channel_name;
  if (!translator ||
      !translator->GetChannelNameFromComponent(
          candidate.component(), &channel_name)) {
    return BadWrite("Cannot write candidate because of unknown component.",
                    error);
  }

  elem->SetAttr(buzz::QN_NAME, channel_name);
  elem->SetAttr(QN_ADDRESS, candidate.address().ipaddr().ToString());
  elem->SetAttr(QN_PORT, candidate.address().PortAsString());
  AddXmlAttr(elem, QN_PREFERENCE, candidate.preference());
  elem->SetAttr(QN_USERNAME, candidate.username());
  if (!candidate.password().empty())
    elem->SetAttr(QN_PASSWORD, candidate.password());
  elem->SetAttr(QN_PROTOCOL, candidate.protocol());
  elem->SetAttr(QN_GENERATION, candidate.generation_str());
  elem->SetAttr(buzz::QN_TYPE, candidate.type());
  if (!candidate.network_name().empty())
    elem->SetAttr(QN_NETWORK, candidate.network_name());

  return true;
}

}  // namespace cricket
