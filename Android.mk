LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

include $(MY_ROOT_PATH)/android-webrtc.mk
include $(LOCAL_PATH)/config.mk

LOCAL_ARM_MODE := arm
LOCAL_MODULE := libjingle
LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cc

#jingle core
LOCAL_SRC_FILES :=\
		talk/base/logging.cc \
		talk/base/messagequeue.cc \
		talk/base/thread.cc \
		talk/base/asyncfile.cc \
		talk/base/asynchttprequest.cc \
		talk/base/asyncsocket.cc \
		talk/base/asynctcpsocket.cc \
		talk/base/asyncudpsocket.cc \
		talk/base/autodetectproxy.cc \
		talk/base/base64.cc \
		talk/base/basicpacketsocketfactory.cc \
		talk/base/bytebuffer.cc \
		talk/base/checks.cc \
		talk/base/common.cc \
		talk/base/crc32.cc \
		talk/base/diskcache.cc \
		talk/base/event.cc \
		talk/base/fileutils.cc \
		talk/base/firewallsocketserver.cc \
		talk/base/flags.cc \
		talk/base/helpers.cc \
		talk/base/host.cc \
		talk/base/httpbase.cc \
		talk/base/httpclient.cc \
		talk/base/httpcommon.cc \
		talk/base/httprequest.cc \
		talk/base/ipaddress.cc \
		talk/base/md5.cc \
		talk/base/messagedigest.cc \
		talk/base/messagehandler.cc \
		talk/base/nethelpers.cc \
		talk/base/network.cc \
		talk/base/pathutils.cc \
		talk/base/physicalsocketserver.cc \
		talk/base/proxydetect.cc \
		talk/base/proxyinfo.cc \
		talk/base/ratelimiter.cc \
		talk/base/ratetracker.cc \
		talk/base/sha1.cc \
		talk/base/signalthread.cc \
		talk/base/socketadapters.cc \
		talk/base/socketaddress.cc \
		talk/base/socketaddresspair.cc \
		talk/base/socketpool.cc \
		talk/base/socketstream.cc \
		talk/base/ssladapter.cc \
		talk/base/sslsocketfactory.cc \
		talk/base/sslstreamadapter.cc \
		talk/base/stream.cc \
		talk/base/stringencode.cc \
		talk/base/stringutils.cc \
		talk/base/task.cc \
		talk/base/taskparent.cc \
		talk/base/taskrunner.cc \
		talk/base/timeutils.cc \
		talk/base/timing.cc \
		talk/base/urlencode.cc \
		talk/base/worker.cc \
		talk/xmllite/qname.cc \
		talk/xmllite/xmlbuilder.cc \
		talk/xmllite/xmlconstants.cc \
		talk/xmllite/xmlelement.cc \
		talk/xmllite/xmlnsstack.cc \
		talk/xmllite/xmlparser.cc \
		talk/xmllite/xmlprinter.cc \
		talk/xmpp/constants.cc \
		talk/xmpp/jid.cc \
		talk/xmpp/saslmechanism.cc \
		talk/xmpp/xmppclient.cc \
		talk/xmpp/xmppengineimpl.cc \
		talk/xmpp/xmppengineimpl_iq.cc \
		talk/xmpp/xmpplogintask.cc \
		talk/xmpp/xmppstanzaparser.cc \
		talk/xmpp/xmpptask.cc \
		talk/base/openssladapter.cc \
		talk/base/unixfilesystem.cc \
		talk/base/openssldigest.cc \
		talk/base/opensslidentity.cc \
		talk/base/posix.cc \
		talk/base/sslidentity.cc \
		talk/base/linux.cc \
		talk/base/linuxfdwalk.c \
		talk/base/opensslstreamadapter.cc

#p2p
LOCAL_SRC_FILES += \
		talk/p2p/base/constants.cc \
		talk/p2p/base/dtlstransportchannel.cc \
		talk/p2p/base/p2ptransport.cc \
		talk/p2p/base/p2ptransportchannel.cc \
		talk/p2p/base/port.cc \
		talk/p2p/base/portallocator.cc \
		talk/p2p/base/portallocatorsessionproxy.cc \
		talk/p2p/base/portproxy.cc \
		talk/p2p/base/pseudotcp.cc \
		talk/p2p/base/rawtransport.cc \
		talk/p2p/base/rawtransportchannel.cc \
		talk/p2p/base/relayport.cc \
		talk/p2p/base/session.cc \
		talk/p2p/base/sessiondescription.cc \
		talk/p2p/base/sessionmanager.cc \
		talk/p2p/base/sessionmessages.cc \
		talk/p2p/base/parsing.cc \
		talk/p2p/base/stun.cc \
		talk/p2p/base/stunport.cc \
		talk/p2p/base/stunrequest.cc \
		talk/p2p/base/tcpport.cc \
		talk/p2p/base/transport.cc \
		talk/p2p/base/transportchannel.cc \
		talk/p2p/base/transportchannelproxy.cc \
		talk/p2p/base/udpport.cc \
		talk/p2p/client/basicportallocator.cc \
		talk/p2p/client/httpportallocator.cc \
		talk/p2p/client/socketmonitor.cc \
		talk/session/tunnel/pseudotcpchannel.cc \
		talk/session/tunnel/tunnelsessionclient.cc \

#xmpp helper
LOCAL_SRC_FILES += \
  talk/examples/login/jingleinfotask.cc \
  talk/examples/login/xmppauth.cc \
  talk/examples/login/xmpppump.cc \
  talk/examples/login/xmppsocket.cc

#webrtc code
LOCAL_SRC_FILES += \
		talk/session/phone/audiomonitor.cc \
		talk/session/phone/call.cc \
		talk/session/phone/channel.cc \
		talk/session/phone/channelmanager.cc \
		talk/session/phone/codec.cc \
		talk/session/phone/constants.cc \
		talk/session/phone/currentspeakermonitor.cc \
		talk/session/phone/dataengine.cc \
		talk/session/phone/devicemanager.cc \
		talk/session/phone/dummydevicemanager.cc \
		talk/session/phone/filemediaengine.cc \
		talk/session/phone/mediaengine.cc \
		talk/session/phone/mediamessages.cc \
		talk/session/phone/mediamonitor.cc \
		talk/session/phone/mediasession.cc \
		talk/session/phone/mediasessionclient.cc \
		talk/session/phone/rtcpmuxfilter.cc \
		talk/session/phone/rtpdump.cc \
		talk/session/phone/rtputils.cc \
		talk/session/phone/soundclip.cc \
		talk/session/phone/srtpfilter.cc \
		talk/session/phone/ssrcmuxfilter.cc \
		talk/session/phone/streamparams.cc \
		talk/session/phone/typingmonitor.cc \
		talk/session/phone/webrtcvoiceengine.cc \
		talk/session/phone/filevideocapturer.cc \
		talk/session/phone/webrtcvideoengine.cc \
		talk/session/phone/webrtcvideoframe.cc \
		talk/app/webrtc/audiotrack.cc \
		talk/app/webrtc/videotrack.cc \
		talk/app/webrtc/videotrackrenderers.cc \
		talk/session/phone/videocapturer.cc \
		talk/session/phone/videocommon.cc \
		talk/session/phone/videoframe.cc

LOCAL_C_INCLUDES := \
	$(MY_THIRD_PARTY_PATH)/openssl/openssl/include \
	$(MY_THIRD_PARTY_PATH)/libsrtp/source/config \
	$(MY_THIRD_PARTY_PATH)/libsrtp/source/srtp/include \
	$(MY_THIRD_PARTY_PATH)/libsrtp/source/srtp/crypto/include \
	$(MY_THIRD_PARTY_PATH)/webrtc \
	$(MY_THIRD_PARTY_PATH)/webrtc/modules/interface \
	$(MY_THIRD_PARTY_PATH)/expat/files/lib

LOCAL_C_INCLUDES += $(LIBJINGLE_C_INCLUDE)

#single big lib build
LOCAL_SHARED_LIBRARIES := \
	libstlport_shared \
	libwebrtc_voice

LOCAL_LDLIBS := \
	-llog \
	-ldl

LOCAL_WHOLE_STATIC_LIBRARIES := \
	libexpat_static \
	libsrtp_static \
	libssl_static

LOCAL_CFLAGS += \
    $(MY_WEBRTC_COMMON_DEFS) \
    $(LIBJINGLE_CPPFLAGS) \
	-DHAVE_STDLIB_H \
	-DHAVE_STRING_H \
	-DCPU_RISC \
	-DSIZEOF_UNSIGNED_LONG=4 \
	-DSIZEOF_UNSIGNED_LONG_LONG=8 \
	-DHAVE_STDINT_H \
	-DHAVE_INTTYPES_H \
	-DHAVE_NETINET_IN_H \
	-DHAVE_UINT64_T \
	-DHAVE_UINT32_T \
	-DHAVE_UINT16_T \
	-DHAVE_UINT8_T \
	-DHAVE_UINT_T

include $(BUILD_SHARED_LIBRARY)
