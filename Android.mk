LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)


LOCAL_ARM_MODE := arm
LOCAL_MODULE := libjingle
LOCAL_MODULE_TAGS := optional
LOCAL_CPP_EXTENSION := .cc

# disabled: talk/base/filelock.cc
LOCAL_CORE_COMMON_SRC := \
	talk/base/asyncfile.cc \
	talk/base/asynchttprequest.cc \
	talk/base/asyncsocket.cc \
	talk/base/asynctcpsocket.cc \
	talk/base/asyncudpsocket.cc \
	talk/base/autodetectproxy.cc \
	talk/base/bandwidthsmoother.cc \
	talk/base/base64.cc \
	talk/base/basicpacketsocketfactory.cc \
	talk/base/bytebuffer.cc \
	talk/base/checks.cc \
	talk/base/common.cc \
	talk/base/cpumonitor.cc \
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
	talk/base/httpserver.cc \
	talk/base/ipaddress.cc \
	talk/base/logging.cc \
	talk/base/md5.cc \
	talk/base/messagedigest.cc \
	talk/base/messagehandler.cc \
	talk/base/messagequeue.cc \
	talk/base/multipart.cc \
	talk/base/natserver.cc \
	talk/base/natsocketfactory.cc \
	talk/base/nattypes.cc \
	talk/base/nethelpers.cc \
	talk/base/network.cc \
	talk/base/nssidentity.cc \
	talk/base/nssstreamadapter.cc \
	talk/base/optionsfile.cc \
	talk/base/pathutils.cc \
	talk/base/physicalsocketserver.cc \
	talk/base/proxydetect.cc \
	talk/base/proxyinfo.cc \
	talk/base/proxyserver.cc \
	talk/base/ratelimiter.cc \
	talk/base/ratetracker.cc \
	talk/base/sha1.cc \
	talk/base/sharedexclusivelock.cc \
	talk/base/signalthread.cc \
	talk/base/socketadapters.cc \
	talk/base/socketaddress.cc \
	talk/base/socketaddresspair.cc \
	talk/base/socketpool.cc \
	talk/base/socketstream.cc \
	talk/base/ssladapter.cc \
	talk/base/sslsocketfactory.cc \
	talk/base/sslidentity.cc \
	talk/base/sslstreamadapter.cc \
	talk/base/sslstreamadapterhelper.cc \
	talk/base/stream.cc \
	talk/base/stringencode.cc \
	talk/base/stringutils.cc \
	talk/base/systeminfo.cc \
	talk/base/task.cc \
	talk/base/taskparent.cc \
	talk/base/taskrunner.cc \
	talk/base/testclient.cc \
	talk/base/thread.cc \
	talk/base/timeutils.cc \
	talk/base/timing.cc \
	talk/base/transformadapter.cc \
	talk/base/urlencode.cc \
	talk/base/versionparsing.cc \
	talk/base/virtualsocketserver.cc \
	talk/base/worker.cc \
	talk/xmllite/qname.cc \
	talk/xmllite/xmlbuilder.cc \
	talk/xmllite/xmlconstants.cc \
	talk/xmllite/xmlelement.cc \
	talk/xmllite/xmlnsstack.cc \
	talk/xmllite/xmlparser.cc \
	talk/xmllite/xmlprinter.cc \
	talk/xmpp/chatroommoduleimpl.cc \
	talk/xmpp/constants.cc \
	talk/xmpp/discoitemsquerytask.cc \
	talk/xmpp/hangoutpubsubclient.cc \
	talk/xmpp/iqtask.cc \
	talk/xmpp/jid.cc \
	talk/xmpp/moduleimpl.cc \
	talk/xmpp/mucroomconfigtask.cc \
	talk/xmpp/mucroomdiscoverytask.cc \
	talk/xmpp/mucroomlookuptask.cc \
	talk/xmpp/mucroomuniquehangoutidtask.cc \
	talk/xmpp/pingtask.cc \
	talk/xmpp/pubsubclient.cc \
	talk/xmpp/pubsub_task.cc \
	talk/xmpp/pubsubtasks.cc \
	talk/xmpp/receivetask.cc \
	talk/xmpp/rostermoduleimpl.cc \
	talk/xmpp/saslmechanism.cc \
	talk/xmpp/xmppclient.cc \
	talk/xmpp/xmppengineimpl.cc \
	talk/xmpp/xmppengineimpl_iq.cc \
	talk/xmpp/xmpplogintask.cc \
	talk/xmpp/xmppstanzaparser.cc \
	talk/xmpp/xmpptask.cc

LOCAL_CORE_LINUX_SRC := \
	talk/base/dbus.cc \
	talk/base/libdbusglibsymboltable.cc \
	talk/base/linux.cc \
	talk/base/linuxfdwalk.c \

LOCAL_CORE_POSIX_SRC := \
	talk/base/latebindingsymboltable.cc \
	talk/base/openssladapter.cc \
	talk/base/openssldigest.cc \
	talk/base/opensslidentity.cc \
	talk/base/opensslstreamadapter.cc \
	talk/base/posix.cc \
	talk/base/unixfilesystem.cc

LOCAL_MEDIA_SRC := \
	talk/media/base/capturemanager.cc \
	talk/media/base/capturerenderadapter.cc \
	talk/media/base/codec.cc \
	talk/media/base/constants.cc \
	talk/media/base/cpuid.cc \
	talk/media/base/filemediaengine.cc \
	talk/media/base/hybridvideoengine.cc \
	talk/media/base/mediaengine.cc \
	talk/media/base/rtpdataengine.cc \
	talk/media/base/rtpdump.cc \
	talk/media/base/rtputils.cc \
	talk/media/base/streamparams.cc \
	talk/media/base/videoadapter.cc \
	talk/media/base/videocapturer.cc \
	talk/media/base/videocommon.cc \
	talk/media/base/videoframe.cc \
	talk/media/devices/devicemanager.cc \
	talk/media/devices/filevideocapturer.cc \
	talk/media/devices/dummydevicemanager.cc \
	talk/media/webrtc/webrtcpassthroughrender.cc \
	talk/media/webrtc/webrtcvideocapturer.cc \
	talk/media/webrtc/webrtcvideoengine.cc \
	talk/media/webrtc/webrtcvideoframe.cc \
	talk/media/webrtc/webrtcvoiceengine.cc

LOCAL_P2P_SRC := \
	talk/p2p/base/constants.cc \
	talk/p2p/base/dtlstransportchannel.cc \
	talk/p2p/base/p2ptransport.cc \
	talk/p2p/base/p2ptransportchannel.cc \
	talk/p2p/base/parsing.cc \
	talk/p2p/base/port.cc \
	talk/p2p/base/portallocator.cc \
	talk/p2p/base/portallocatorsessionproxy.cc \
	talk/p2p/base/portproxy.cc \
	talk/p2p/base/pseudotcp.cc \
	talk/p2p/base/relayport.cc \
	talk/p2p/base/relayserver.cc \
	talk/p2p/base/rawtransport.cc \
	talk/p2p/base/rawtransportchannel.cc \
	talk/p2p/base/session.cc \
	talk/p2p/base/sessiondescription.cc \
	talk/p2p/base/sessionmanager.cc \
	talk/p2p/base/sessionmessages.cc \
	talk/p2p/base/stun.cc \
	talk/p2p/base/stunport.cc \
	talk/p2p/base/stunrequest.cc \
	talk/p2p/base/stunserver.cc \
	talk/p2p/base/tcpport.cc \
	talk/p2p/base/transport.cc \
	talk/p2p/base/transportchannel.cc \
	talk/p2p/base/transportchannelproxy.cc \
	talk/p2p/base/transportdescriptionfactory.cc \
	talk/p2p/base/turnport.cc \
	talk/p2p/base/turnserver.cc \
	talk/p2p/client/basicportallocator.cc \
	talk/p2p/client/connectivitychecker.cc \
	talk/p2p/client/httpportallocator.cc \
	talk/p2p/client/socketmonitor.cc \
	talk/session/tunnel/pseudotcpchannel.cc \
	talk/session/tunnel/tunnelsessionclient.cc \
	talk/session/tunnel/securetunnelsessionclient.cc \
	talk/session/media/audiomonitor.cc \
	talk/session/media/call.cc \
	talk/session/media/channel.cc \
	talk/session/media/channelmanager.cc \
	talk/session/media/currentspeakermonitor.cc \
	talk/session/media/mediamessages.cc \
	talk/session/media/mediamonitor.cc \
	talk/session/media/mediarecorder.cc \
	talk/session/media/mediasession.cc \
	talk/session/media/mediasessionclient.cc \
	talk/session/media/rtcpmuxfilter.cc \
	talk/session/media/soundclip.cc \
	talk/session/media/srtpfilter.cc \
	talk/session/media/ssrcmuxfilter.cc \
	talk/session/media/typingmonitor.cc


LOCAL_SRC_FILES := \
	$(LOCAL_CORE_COMMON_SRC) \
	$(LOCAL_CORE_LINUX_SRC) \
	$(LOCAL_CORE_POSIX_SRC) \
	$(LOCAL_MEDIA_SRC) \
	$(LOCAL_P2P_SRC) \


LOCAL_C_INCLUDES := \
	$(MY_THIRD_PARTY_PATH)/openssl/openssl/include \
	$(MY_THIRD_PARTY_PATH)/openssl/config/android \
	$(MY_THIRD_PARTY_PATH)/libsrtp/source/config \
	$(MY_THIRD_PARTY_PATH)/libsrtp/source/srtp/include \
	$(MY_THIRD_PARTY_PATH)/libsrtp/source/srtp/crypto/include \
	$(MY_THIRD_PARTY_PATH) \
	$(MY_THIRD_PARTY_PATH)/webrtc \
	$(MY_THIRD_PARTY_PATH)/webrtc/modules/interface \
	$(MY_THIRD_PARTY_PATH)/expat/files/lib \
	$(MY_GTEST_PATH) \
	$(MY_GTEST_PATH)/include \
	$(MY_THIRD_PARTY_PATH)/libyuv/include/

LOCAL_C_INCLUDES += $(LIBJINGLE_C_INCLUDE)

#single big lib build
# LOCAL_SHARED_LIBRARIES := \
	# libstlport_shared \
	# libwebrtc_voice

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
	-DHAVE_UINT_T \
	-DUSE_WEBRTC_313_BRANCH

include $(BUILD_STATIC_LIBRARY)

ifeq ($(ENABLE_UNITTEST), 1)
$(info Build unittest for libjingle)
include $(LOCAL_PATH)/unittest.mk
endif
