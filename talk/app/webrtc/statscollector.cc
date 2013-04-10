/*
 * libjingle
 * Copyright 2012, Google Inc.
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

#include "talk/app/webrtc/statscollector.h"

#include <utility>
#include <vector>

#include "talk/session/media/channel.h"

namespace webrtc {

// The items below are in alphabetical order.
const char StatsReport::kStatsValueNameActualEncBitrate[] =
    "googActualEncBitrate";
const char StatsReport::kStatsValueNameAudioOutputLevel[] = "audioOutputLevel";
const char StatsReport::kStatsValueNameAudioInputLevel[] = "audioInputLevel";
const char StatsReport::kStatsValueNameAvailableReceiveBandwidth[] =
    "googAvailableReceiveBandwidth";
const char StatsReport::kStatsValueNameAvailableSendBandwidth[] =
    "googAvailableSendBandwidth";
const char StatsReport::kStatsValueNameBucketDelay[] = "googBucketDelay";
const char StatsReport::kStatsValueNameBytesReceived[] = "bytesReceived";
const char StatsReport::kStatsValueNameBytesSent[] = "bytesSent";
const char StatsReport::kStatsValueNameCandidateAddress[] = "candidateAddress";
const char StatsReport::kStatsValueNameFirsReceived[] = "googFirsReceived";
const char StatsReport::kStatsValueNameFirsSent[] = "googFirsSent";
const char StatsReport::kStatsValueNameFrameHeightReceived[] =
    "googFrameHeightReceived";
const char StatsReport::kStatsValueNameFrameHeightSent[] =
    "googFrameHeightSent";
const char StatsReport::kStatsValueNameInitiator[] = "googInitiator";
const char StatsReport::kStatsValueNameFrameRateReceived[] =
    "googFrameRateReceived";
const char StatsReport::kStatsValueNameFrameRateDecoded[] =
    "googFrameRateDecoded";
const char StatsReport::kStatsValueNameFrameRateOutput[] =
    "googFrameRateOutput";
const char StatsReport::kStatsValueNameFrameRateInput[] = "googFrameRateInput";
const char StatsReport::kStatsValueNameFrameRateSent[] = "googFrameRateSent";
const char StatsReport::kStatsValueNameFrameWidthReceived[] =
    "googFrameWidthReceived";
const char StatsReport::kStatsValueNameFrameWidthSent[] = "googFrameWidthSent";
const char StatsReport::kStatsValueNameJitterReceived[] = "googJitterReceived";
const char StatsReport::kStatsValueNameNacksReceived[] = "googNacksReceived";
const char StatsReport::kStatsValueNameNacksSent[] = "googNacksSent";
const char StatsReport::kStatsValueNamePacketsReceived[] = "packetsReceived";
const char StatsReport::kStatsValueNamePacketsSent[] = "packetsSent";
const char StatsReport::kStatsValueNamePacketsLost[] = "packetsLost";
const char StatsReport::kStatsValueNameRetransmitBitrate[] =
    "googRetransmitBitrate";
const char StatsReport::kStatsValueNameRtt[] = "googRtt";
const char StatsReport::kStatsValueNameTargetEncBitrate[] =
    "googTargetEncBitrate";
const char StatsReport::kStatsValueNameTransmitBitrate[] =
    "googTransmitBitrate";
const char StatsReport::kStatsValueNameTransportId[] = "transportId";
const char StatsReport::kStatsValueNameTransportType[] = "googTransportType";

const char StatsReport::kStatsReportTypeSession[] = "googLibjingleSession";
const char StatsReport::kStatsReportTypeBwe[] = "VideoBwe";
const char StatsReport::kStatsReportTypeSsrc[] = "ssrc";
const char StatsReport::kStatsReportTypeIceCandidate[] = "iceCandidate";
const char StatsReport::kStatsReportTypeTransport[] = "googTransport";


// Implementations of functions in statstypes.h
void StatsReport::AddValue(const std::string& name, const std::string& value) {
  Value temp;
  temp.name = name;
  temp.value = value;
  values.push_back(temp);
}

void StatsReport::AddValue(const std::string& name, int64 value) {
  AddValue(name, talk_base::ToString<int64>(value));
}

namespace {

typedef std::map<std::string, webrtc::StatsReport> ReportsMap;

void AddEmptyReport(const std::string& label, ReportsMap* reports) {
  reports->insert(std::pair<std::string, webrtc::StatsReport>(
      label, webrtc::StatsReport()));
}

template <class TrackVector>
void CreateTrackReports(const TrackVector& tracks, ReportsMap* reports) {
  for (size_t j = 0; j < tracks.size(); ++j) {
    webrtc::MediaStreamTrackInterface* track = tracks[j];
    // If there is no previous report for this track, add one.
    if (reports->find(track->id()) == reports->end()) {
      AddEmptyReport(track->id(), reports);
    }
  }
}

void ExtractStats(const cricket::VoiceReceiverInfo& info, StatsReport* report) {
  report->AddValue(StatsReport::kStatsValueNameAudioOutputLevel,
                   info.audio_level);
  report->AddValue(StatsReport::kStatsValueNameBytesReceived,
                   info.bytes_rcvd);
  report->AddValue(StatsReport::kStatsValueNameJitterReceived,
                   info.jitter_ms);
  report->AddValue(StatsReport::kStatsValueNamePacketsReceived,
                   info.packets_rcvd);
  report->AddValue(StatsReport::kStatsValueNamePacketsLost,
                   info.packets_lost);
}

void ExtractStats(const cricket::VoiceSenderInfo& info, StatsReport* report) {
  report->AddValue(StatsReport::kStatsValueNameAudioInputLevel,
                   info.audio_level);
  report->AddValue(StatsReport::kStatsValueNameBytesSent,
                   info.bytes_sent);
  report->AddValue(StatsReport::kStatsValueNamePacketsSent,
                   info.packets_sent);

  // TODO(jiayl): Move the remote stuff into a separate function to extract them to
  // a different stats element for v2.
  report->AddValue(StatsReport::kStatsValueNameJitterReceived,
                   info.jitter_ms);
  report->AddValue(StatsReport::kStatsValueNameRtt, info.rtt_ms);
}

void ExtractStats(const cricket::VideoReceiverInfo& info, StatsReport* report) {
  report->AddValue(StatsReport::kStatsValueNameBytesReceived,
                   info.bytes_rcvd);
  report->AddValue(StatsReport::kStatsValueNamePacketsReceived,
                   info.packets_rcvd);
  report->AddValue(StatsReport::kStatsValueNamePacketsLost,
                   info.packets_lost);

  report->AddValue(StatsReport::kStatsValueNameFirsSent,
                   info.firs_sent);
  report->AddValue(StatsReport::kStatsValueNameNacksSent,
                   info.nacks_sent);
  report->AddValue(StatsReport::kStatsValueNameFrameWidthReceived,
                   info.frame_width);
  report->AddValue(StatsReport::kStatsValueNameFrameHeightReceived,
                   info.frame_height);
  report->AddValue(StatsReport::kStatsValueNameFrameRateReceived,
                   info.framerate_rcvd);
  report->AddValue(StatsReport::kStatsValueNameFrameRateDecoded,
                   info.framerate_decoded);
  report->AddValue(StatsReport::kStatsValueNameFrameRateOutput,
                   info.framerate_output);
}

void ExtractStats(const cricket::VideoSenderInfo& info, StatsReport* report) {
  report->AddValue(StatsReport::kStatsValueNameBytesSent,
                   info.bytes_sent);
  report->AddValue(StatsReport::kStatsValueNamePacketsSent,
                   info.packets_sent);

  report->AddValue(StatsReport::kStatsValueNameFirsReceived,
                   info.firs_rcvd);
  report->AddValue(StatsReport::kStatsValueNameNacksReceived,
                   info.nacks_rcvd);
  report->AddValue(StatsReport::kStatsValueNameFrameWidthSent,
                   info.frame_width);
  report->AddValue(StatsReport::kStatsValueNameFrameHeightSent,
                   info.frame_height);
  report->AddValue(StatsReport::kStatsValueNameFrameRateInput,
                   info.framerate_input);
  report->AddValue(StatsReport::kStatsValueNameFrameRateSent,
                   info.framerate_sent);

  // TODO(jiayl): Move the remote stuff into a separate function to extract them to
  // a different stats element for v2.
  report->AddValue(StatsReport::kStatsValueNameRtt, info.rtt_ms);
}

void ExtractStats(const cricket::BandwidthEstimationInfo& info,
                  double stats_gathering_started,
                  StatsReport* report) {
  report->id = "bweforvideo";
  report->type = webrtc::StatsReport::kStatsReportTypeBwe;

  // Clear out stats from previous GatherStats calls if any.
  if (report->timestamp != stats_gathering_started) {
    report->values.clear();
    report->timestamp = stats_gathering_started;
  }

  report->AddValue(StatsReport::kStatsValueNameAvailableSendBandwidth,
                   info.available_send_bandwidth);
  report->AddValue(StatsReport::kStatsValueNameAvailableReceiveBandwidth,
                   info.available_recv_bandwidth);
  report->AddValue(StatsReport::kStatsValueNameTargetEncBitrate,
                   info.target_enc_bitrate);
  report->AddValue(StatsReport::kStatsValueNameActualEncBitrate,
                   info.actual_enc_bitrate);
  report->AddValue(StatsReport::kStatsValueNameRetransmitBitrate,
                   info.retransmit_bitrate);
  report->AddValue(StatsReport::kStatsValueNameTransmitBitrate,
                   info.transmit_bitrate);
  report->AddValue(StatsReport::kStatsValueNameBucketDelay,
                   info.bucket_delay);
}

uint32 ExtractSsrc(const cricket::VoiceReceiverInfo& info) {
  return info.ssrc;
}

uint32 ExtractSsrc(const cricket::VoiceSenderInfo& info) {
  return info.ssrc;
}

uint32 ExtractSsrc(const cricket::VideoReceiverInfo& info) {
  return info.ssrcs[0];
}

uint32 ExtractSsrc(const cricket::VideoSenderInfo& info) {
  return info.ssrcs[0];
}

// Template to extract stats from a data vector.
// ExtractSsrc and ExtractStats must be defined and overloaded for each type.
template<typename T>
void ExtractStatsFromList(const std::vector<T>& data,
                          StatsCollector* collector) {
  typename std::vector<T>::const_iterator it = data.begin();
  for (; it != data.end(); ++it) {
    std::string id;
    uint32 ssrc = ExtractSsrc(*it);
    if (!collector->session()->GetTrackIdBySsrc(ssrc, &id)) {
      LOG(LS_ERROR) << "The SSRC " << ssrc
                    << " is not associated with a track";
      continue;
    }
    StatsReport* report = collector->PrepareReport(id, ssrc);
    if (!report) {
      continue;
    }
    ExtractStats(*it, report);
  }
};

}  // namespace

StatsCollector::StatsCollector()
    : session_(NULL), stats_gathering_started_(0) {
}

// Adds a MediaStream with tracks that can be used as a |selector| in a call
// to GetStats.
void StatsCollector::AddStream(MediaStreamInterface* stream) {
  ASSERT(stream != NULL);

  CreateTrackReports<AudioTrackVector>(stream->GetAudioTracks(),
                                       &track_reports_);
  CreateTrackReports<VideoTrackVector>(stream->GetVideoTracks(),
                                       &track_reports_);
}

bool StatsCollector::GetStats(MediaStreamTrackInterface* track,
                              StatsReports* reports) {
  ASSERT(reports != NULL);
  reports->clear();

  if (session_report_.timestamp > 0) {
    reports->push_back(session_report_);
  }

  if (track) {
    ReportsMap::const_iterator it = track_reports_.find(track->id());
    if (it == track_reports_.end()) {
      LOG(LS_WARNING) << "No StatsReport is available for "<< track->id();
      return false;
    }
    reports->push_back(it->second);
    return true;
  }

  if (bandwidth_estimation_report_.timestamp > 0) {
    reports->push_back(bandwidth_estimation_report_);
  }

  // If no selector given, add all Stats to |reports|.
  ReportsMap::const_iterator it = track_reports_.begin();
  for (; it != track_reports_.end(); ++it) {
    if (!it->second.type.empty())
      reports->push_back(it->second);
  }

  return true;
}

void StatsCollector::UpdateStats() {
  double time_now = GetTimeNow();
  // Calls to UpdateStats() that occur less than kMinGatherStatsPeriod number of
  // ms apart will be ignored.
  const double kMinGatherStatsPeriod = 50;
  if (stats_gathering_started_ + kMinGatherStatsPeriod > time_now) {
    return;
  }
  stats_gathering_started_ = time_now;

  if (session_) {
    ExtractSessionInfo();
    ExtractVoiceInfo();
    ExtractVideoInfo();
  }
}

StatsReport* StatsCollector::PrepareReport(const std::string& id,
                                           uint32 ssrc) {
  std::map<std::string, webrtc::StatsReport>::iterator it =
      track_reports_.find(id);
  if (it == track_reports_.end()) {
    return NULL;
  }

  StatsReport* report= &(it->second);

  report->id = talk_base::ToString<uint32>(ssrc);
  report->type = webrtc::StatsReport::kStatsReportTypeSsrc;

  // Clear out stats from previous GatherStats calls if any.
  if (report->timestamp != stats_gathering_started_) {
    report->values.clear();
    report->timestamp = stats_gathering_started_;
  }
  // TODO(hta): Add a StatsReport::kStatsvalueNameTransportId for the transport.
  return report;
}

void StatsCollector::ExtractSessionInfo() {
  // Extract information from the base session.
  session_report_.id = session_->id();
  session_report_.type = StatsReport::kStatsReportTypeSession;
  session_report_.timestamp = stats_gathering_started_;
  session_report_.values.clear();
  session_report_.AddValue(StatsReport::kStatsValueNameInitiator,
                           session_->initiator()?"true":"false");

  // TODO(hta): Add transport information.
  // Transport info should come from GetStats() on each Transport
  // available from session_.

  // TODO(hta): Add candidate pair information.
  // The ICE candidate pair information is stored in
  // cricket::P2PTransportChannel::connections_ (of type cricket::Connection)
  // It exports a GetStats call that returns a vector of ConnectionInfo
  // declared in transportchannel.h that has the information we want.
}

void StatsCollector::ExtractVoiceInfo() {
  if (!session_->voice_channel()) {
    return;
  }
  cricket::VoiceMediaInfo voice_info;
  if (!session_->voice_channel()->GetStats(&voice_info)) {
    LOG(LS_ERROR) << "Failed to get voice channel stats.";
    return;
  }
  ExtractStatsFromList(voice_info.receivers, this);
  ExtractStatsFromList(voice_info.senders, this);
}

void StatsCollector::ExtractVideoInfo() {
  if (!session_->video_channel()) {
    return;
  }
  cricket::VideoMediaInfo video_info;
  if (!session_->video_channel()->GetStats(&video_info)) {
    LOG(LS_ERROR) << "Failed to get video channel stats.";
    return;
  }
  ExtractStatsFromList(video_info.receivers, this);
  ExtractStatsFromList(video_info.senders, this);
  if (video_info.bw_estimations.size() != 1) {
    LOG(LS_ERROR) << "BWEs count: " << video_info.bw_estimations.size();
  } else {
    ExtractStats(video_info.bw_estimations[0],
                 stats_gathering_started_,
                 &bandwidth_estimation_report_);
  }
}

double StatsCollector::GetTimeNow() {
  return timing_.WallTimeNow() * talk_base::kNumMillisecsPerSec;
}


}  // namespace webrtc
