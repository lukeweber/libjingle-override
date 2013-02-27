/*
 * libjingle
 * Copyright 2011, Google Inc.
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

#include "talk/app/webrtc/mediastreamproxy.h"
#include "talk/base/refcount.h"
#include "talk/base/scoped_ref_ptr.h"

namespace {

enum {
  MSG_REGISTER_OBSERVER,
  MSG_UNREGISTER_OBSERVER,
  MSG_LABEL,
  MSG_ADD_AUDIO_TRACK,
  MSG_ADD_VIDEO_TRACK,
  MSG_REMOVE_AUDIO_TRACK,
  MSG_REMOVE_VIDEO_TRACK,
  MSG_GET_AUDIO_TRACKS,
  MSG_FIND_AUDIO_TRACK,
  MSG_GET_VIDEO_TRACKS,
  MSG_FIND_VIDEO_TRACK,
  MSG_COUNT,
  MSG_AT,
  MSG_FIND,
};

typedef talk_base::TypedMessageData<std::string*> LabelMessageData;
typedef talk_base::TypedMessageData<size_t> SizeTMessageData;
typedef talk_base::TypedMessageData<webrtc::ObserverInterface*>
    ObserverMessageData;

template<typename T>
class MediaStreamTrackMessageData : public talk_base::MessageData {
 public:
  explicit MediaStreamTrackMessageData(T* track)
      : track_(track),
        result_(false) {
  }

  std::string id;
  talk_base::scoped_refptr<T> track_;
  bool result_;
};

typedef MediaStreamTrackMessageData<webrtc::AudioTrackInterface>
    AudioTrackMsgData;
typedef MediaStreamTrackMessageData<webrtc::VideoTrackInterface>
    VideoTrackMsgData;

template <class TrackType>
class MediaStreamTrackAtMessageData : public talk_base::MessageData {
 public:
  explicit MediaStreamTrackAtMessageData(size_t index)
      : index_(index) {
  }

  size_t index_;
  talk_base::scoped_refptr<TrackType> track_;
};

template <class TrackType>
class MediaStreamTrackFindMessageData : public talk_base::MessageData {
 public:
  explicit MediaStreamTrackFindMessageData(const std::string& id)
      : id_(id) {
  }

  std::string id_;
  talk_base::scoped_refptr<TrackType> track_;
};

template <class TrackVector>
class MediaStreamTracksMessageData : public talk_base::MessageData {
 public:
  explicit MediaStreamTracksMessageData() {
  }

  TrackVector tracks;
};

}  // namespace anonymous

namespace webrtc {

talk_base::scoped_refptr<MediaStreamProxy> MediaStreamProxy::Create(
    const std::string& label,
    talk_base::Thread* signaling_thread) {
  ASSERT(signaling_thread != NULL);
  talk_base::RefCountedObject<MediaStreamProxy>* stream =
      new talk_base::RefCountedObject<MediaStreamProxy>(
          label, signaling_thread,
          reinterpret_cast<LocalMediaStreamInterface*>(NULL));
  return stream;
}

talk_base::scoped_refptr<MediaStreamProxy> MediaStreamProxy::Create(
    const std::string& label,
    talk_base::Thread* signaling_thread,
    LocalMediaStreamInterface* media_stream_impl) {
  ASSERT(signaling_thread != NULL);
  ASSERT(media_stream_impl != NULL);
  talk_base::RefCountedObject<MediaStreamProxy>* stream =
      new talk_base::RefCountedObject<MediaStreamProxy>(label, signaling_thread,
                                                        media_stream_impl);
  return stream;
}

MediaStreamProxy::MediaStreamProxy(const std::string& label,
                                   talk_base::Thread* signaling_thread,
                                   LocalMediaStreamInterface* media_stream_impl)
    : signaling_thread_(signaling_thread),
      media_stream_impl_(media_stream_impl) {
  if (media_stream_impl_ == NULL) {
    media_stream_impl_ = MediaStream::Create(label);
  }
}

std::string MediaStreamProxy::label() const {
  if (!signaling_thread_->IsCurrent()) {
    std::string label;
    LabelMessageData msg(&label);
    Send(MSG_LABEL, &msg);
    return label;
  }
  return media_stream_impl_->label();
}

bool MediaStreamProxy::AddTrack(AudioTrackInterface* track) {
  if (!signaling_thread_->IsCurrent()) {
    AudioTrackMsgData msg(track);
    Send(MSG_ADD_AUDIO_TRACK, &msg);
    return msg.result_;
  }
  return media_stream_impl_->AddTrack(track);
}

bool MediaStreamProxy::RemoveTrack(AudioTrackInterface* track) {
  if (!signaling_thread_->IsCurrent()) {
    AudioTrackMsgData msg(track);
    Send(MSG_REMOVE_AUDIO_TRACK, &msg);
    return msg.result_;
  }
  return media_stream_impl_->RemoveTrack(track);
}

bool MediaStreamProxy::AddTrack(VideoTrackInterface* track) {
  if (!signaling_thread_->IsCurrent()) {
    VideoTrackMsgData msg(track);
    Send(MSG_ADD_VIDEO_TRACK, &msg);
    return msg.result_;
  }
  return media_stream_impl_->AddTrack(track);
}

bool MediaStreamProxy::RemoveTrack(VideoTrackInterface* track) {
  if (!signaling_thread_->IsCurrent()) {
    VideoTrackMsgData msg(track);
    Send(MSG_REMOVE_VIDEO_TRACK, &msg);
    return msg.result_;
  }
  return media_stream_impl_->RemoveTrack(track);
}

AudioTrackVector MediaStreamProxy::GetAudioTracks() {
  if (!signaling_thread_->IsCurrent()) {
    MediaStreamTracksMessageData<AudioTrackVector> msg;
    Send(MSG_GET_AUDIO_TRACKS, &msg);
    return msg.tracks;
  }
  return media_stream_impl_->GetAudioTracks();
}

VideoTrackVector MediaStreamProxy::GetVideoTracks() {
  if (!signaling_thread_->IsCurrent()) {
    MediaStreamTracksMessageData<VideoTrackVector> msg;
    Send(MSG_GET_VIDEO_TRACKS, &msg);
    return msg.tracks;
  }
  return media_stream_impl_->GetVideoTracks();
}

talk_base::scoped_refptr<AudioTrackInterface>
MediaStreamProxy::FindAudioTrack(const std::string& track_id) {
  if (!signaling_thread_->IsCurrent()) {
    MediaStreamTrackMessageData<AudioTrackInterface> msg(NULL);
    msg.id = track_id;
    Send(MSG_FIND_AUDIO_TRACK, &msg);
    return msg.track_;
  }
  return media_stream_impl_->FindAudioTrack(track_id);
}

talk_base::scoped_refptr<VideoTrackInterface>
MediaStreamProxy::FindVideoTrack(const std::string& track_id) {
  if (!signaling_thread_->IsCurrent()) {
    MediaStreamTrackMessageData<VideoTrackInterface> msg(NULL);
    msg.id = track_id;
    Send(MSG_FIND_VIDEO_TRACK, &msg);
    return msg.track_;
  }
  return media_stream_impl_->FindVideoTrack(track_id);
}

void MediaStreamProxy::RegisterObserver(ObserverInterface* observer) {
  if (!signaling_thread_->IsCurrent()) {
    ObserverMessageData msg(observer);
    Send(MSG_REGISTER_OBSERVER, &msg);
    return;
  }
  media_stream_impl_->RegisterObserver(observer);
}

void MediaStreamProxy::UnregisterObserver(ObserverInterface* observer) {
  if (!signaling_thread_->IsCurrent()) {
    ObserverMessageData msg(observer);
    Send(MSG_UNREGISTER_OBSERVER, &msg);
    return;
  }
  media_stream_impl_->UnregisterObserver(observer);
}

void MediaStreamProxy::Send(uint32 id, talk_base::MessageData* data) const {
  signaling_thread_->Send(const_cast<MediaStreamProxy*>(this), id,
                          data);
}

// Implement MessageHandler
void MediaStreamProxy::OnMessage(talk_base::Message* msg) {
  talk_base::MessageData* data = msg->pdata;
  switch (msg->message_id) {
    case MSG_REGISTER_OBSERVER: {
      ObserverMessageData* observer = static_cast<ObserverMessageData*>(data);
      media_stream_impl_->RegisterObserver(observer->data());
      break;
    }
    case MSG_UNREGISTER_OBSERVER: {
      ObserverMessageData* observer = static_cast<ObserverMessageData*>(data);
      media_stream_impl_->UnregisterObserver(observer->data());
      break;
    }
    case MSG_LABEL: {
      LabelMessageData * label = static_cast<LabelMessageData*>(data);
      *(label->data()) = media_stream_impl_->label();
      break;
    }
    case MSG_GET_AUDIO_TRACKS: {
      MediaStreamTracksMessageData<AudioTrackVector>* tracks =
          static_cast<MediaStreamTracksMessageData<AudioTrackVector> *>(data);
      tracks->tracks = media_stream_impl_->GetAudioTracks();
      break;
    }
    case MSG_GET_VIDEO_TRACKS: {
      MediaStreamTracksMessageData<VideoTrackVector>* tracks =
          static_cast<MediaStreamTracksMessageData<VideoTrackVector> *>(data);
      tracks->tracks = media_stream_impl_->GetVideoTracks();
      break;
    }
    case MSG_FIND_AUDIO_TRACK: {
      AudioTrackMsgData * track =
          static_cast<AudioTrackMsgData *>(data);
      track->track_ = media_stream_impl_->FindAudioTrack(track->id);
      break;
    }
    case MSG_FIND_VIDEO_TRACK: {
      VideoTrackMsgData * track =
          static_cast<VideoTrackMsgData *>(data);
      track->track_ = media_stream_impl_->FindVideoTrack(track->id);
      break;
    }
    case MSG_ADD_AUDIO_TRACK: {
      AudioTrackMsgData * track =
          static_cast<AudioTrackMsgData *>(data);
      track->result_ = media_stream_impl_->AddTrack(track->track_.get());
      break;
    }
    case MSG_ADD_VIDEO_TRACK: {
      VideoTrackMsgData * track =
          static_cast<VideoTrackMsgData *>(data);
      track->result_ = media_stream_impl_->AddTrack(track->track_.get());
      break;
    }
    case MSG_REMOVE_AUDIO_TRACK: {
      AudioTrackMsgData * track =
          static_cast<AudioTrackMsgData *>(data);
      track->result_ = media_stream_impl_->RemoveTrack(track->track_.get());
      break;
    }
    case MSG_REMOVE_VIDEO_TRACK: {
      VideoTrackMsgData * track =
          static_cast<VideoTrackMsgData *>(data);
      track->result_ = media_stream_impl_->RemoveTrack(track->track_.get());
      break;
    }
    default:
      ASSERT(!"Not Implemented!");
      break;
  }
}

}  // namespace webrtc
