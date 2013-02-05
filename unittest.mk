LOCAL_PATH := $(call my-dir)
include $(CLEAR_VARS)

include $(MY_ROOT_PATH)/android-webrtc.mk
include $(MY_ROOT_PATH)/libjingle_config.mk

include $(CLEAR_VARS)
LOCAL_MODULE:= libjingle_unittest_main
LOCAL_ARM_MODE := arm
LOCAL_SRC_FILES := talk/android_unittest_main.cc
LOCAL_C_INCLUDES:= \
	$(GTEST_C_INCLUDES) \
	$(MY_LIBJINGLE_C_INCLUDES)

LOCAL_CFLAGS :=  $(MY_UNITTEST_CFLAGS)
LOCAL_CPP_EXTENSION:= .cc
LOCAL_WHOLE_STATIC_LIBRARIES := gunit

include $(BUILD_STATIC_LIBRARY)

include $(CLEAR_VARS)
LOCAL_MODULE:= libjingle_unittest

LOCAL_C_INCLUDES:= $(GTEST_C_INCLUDES) \
		$(MY_LIBJINGLE_C_INCLUDES)
LOCAL_ARM_MODE := arm
LOCAL_MODULE_TAGS := optional

#
# disabled_tests: talk/base/cpumonitor_unittest.cc
#                 talk/base/atomicops_unittest.cc
# Not used currently libjingle. Impl doesn't support platforms < armv7
# Could see base/atomicops.h from chromium if we wanted to implement this in
# the future as an Optimization. CriticalSeciton makes reference to replacing
# their AtomicOps class with something more efficient.
#
LOCAL_BASE_SRC_FILES := \
	talk/base/asynchttprequest_unittest.cc \
	talk/base/autodetectproxy_unittest.cc \
	talk/base/bandwidthsmoother_unittest.cc \
	talk/base/base64_unittest.cc \
	talk/base/basictypes_unittest.cc \
	talk/base/buffer_unittest.cc \
	talk/base/bytebuffer_unittest.cc \
	talk/base/byteorder_unittest.cc \
	talk/base/crc32_unittest.cc \
	talk/base/event_unittest.cc \
	talk/base/fileutils_unittest.cc \
	talk/base/helpers_unittest.cc \
	talk/base/host_unittest.cc \
	talk/base/httpbase_unittest.cc \
	talk/base/httpcommon_unittest.cc \
	talk/base/httpserver_unittest.cc \
	talk/base/ipaddress_unittest.cc \
	talk/base/logging_unittest.cc \
	talk/base/md5digest_unittest.cc \
	talk/base/messagedigest_unittest.cc \
	talk/base/messagequeue_unittest.cc \
	talk/base/multipart_unittest.cc \
	talk/base/nat_unittest.cc \
	talk/base/network_unittest.cc \
	talk/base/nullsocketserver_unittest.cc \
	talk/base/optionsfile_unittest.cc \
	talk/base/pathutils_unittest.cc \
	talk/base/physicalsocketserver_unittest.cc \
	talk/base/proxy_unittest.cc \
	talk/base/proxydetect_unittest.cc \
	talk/base/ratelimiter_unittest.cc \
	talk/base/ratetracker_unittest.cc \
	talk/base/referencecountedsingletonfactory_unittest.cc \
	talk/base/rollingaccumulator_unittest.cc \
	talk/base/sha1digest_unittest.cc \
	talk/base/sharedexclusivelock_unittest.cc \
	talk/base/signalthread_unittest.cc \
	talk/base/sigslot_unittest.cc \
	talk/base/socket_unittest.cc \
	talk/base/socketaddress_unittest.cc \
	talk/base/stream_unittest.cc \
	talk/base/stringencode_unittest.cc \
	talk/base/stringutils_unittest.cc \
	talk/base/task_unittest.cc \
	talk/base/testclient_unittest.cc \
	talk/base/thread_unittest.cc \
	talk/base/timeutils_unittest.cc \
	talk/base/urlencode_unittest.cc \
	talk/base/versionparsing_unittest.cc \
	talk/base/virtualsocket_unittest.cc \
	talk/xmllite/qname_unittest.cc \
	talk/xmllite/xmlbuilder_unittest.cc \
	talk/xmllite/xmlelement_unittest.cc \
	talk/xmllite/xmlnsstack_unittest.cc \
	talk/xmllite/xmlparser_unittest.cc \
	talk/xmllite/xmlprinter_unittest.cc \
	talk/xmpp/hangoutpubsubclient_unittest.cc \
	talk/xmpp/jid_unittest.cc \
	talk/xmpp/mucroomconfigtask_unittest.cc \
	talk/xmpp/mucroomdiscoverytask_unittest.cc \
	talk/xmpp/mucroomlookuptask_unittest.cc \
	talk/xmpp/mucroomuniquehangoutidtask_unittest.cc \
	talk/xmpp/pingtask_unittest.cc \
	talk/xmpp/pubsubclient_unittest.cc \
	talk/xmpp/pubsubtasks_unittest.cc \
	talk/xmpp/util_unittest.cc \
	talk/xmpp/xmppengine_unittest.cc \
	talk/xmpp/xmpplogintask_unittest.cc \
	talk/xmpp/xmppstanzaparser_unittest.cc

# disabled: talk/base/latebindingsymboltable_unittest.cc
LOCAL_LINUX_SRC_FILES := \
	talk/base/linuxfdwalk_unittest.cc

LOCAL_POSIX_SRC_FILES := \
	talk/base/sslidentity_unittest.cc \
	talk/base/sslstreamadapter_unittest.cc

LOCAL_SRC_FILES := \
	$(LOCAL_BASE_SRC_FILES) \
	$(LOCAL_LINUX_SRC_FILES) \
	$(LOCAL_POSIX_SRC_FILES)

LOCAL_CPP_EXTENSION:= .cc
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libjingle \
	libwebrtc_voice \
	libjingle_unittest_main \
	libexpat_static \
	libsrtp_static \
	libssl_static

#LOCAL_STATIC_LIBRARIES := \
#	libjingle_unittest_main \
#	libjingle

LOCAL_CFLAGS +=  $(MY_UNITTEST_CFLAGS)
LOCAL_LDLIBS := -llog -lOpenSLES
include $(BUILD_SHARED_LIBRARY) # libjingle_unittest


##################################################
include $(CLEAR_VARS)
LOCAL_MODULE:= libjingle_media_unittest

LOCAL_C_INCLUDES:= \
	$(GTEST_C_INCLUDES) \
	$(MY_LIBJINGLE_C_INCLUDES)

LOCAL_SRC_FILES := \
	talk/media/base/codec_unittest.cc \
	talk/media/base/filemediaengine_unittest.cc \
	talk/media/base/rtpdataengine_unittest.cc \
	talk/media/base/rtpdump_unittest.cc \
	talk/media/base/rtputils_unittest.cc \
	talk/media/base/testutils.cc \
	talk/media/base/videocapturer_unittest.cc \
	talk/media/base/videocommon_unittest.cc \
	talk/media/devices/dummydevicemanager_unittest.cc \
	talk/media/devices/filevideocapturer_unittest.cc

LOCAL_CPP_EXTENSION := .cc
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libjingle \
	libwebrtc_voice \
	libjingle_unittest_main \
	libexpat_static \
	libsrtp_static \
	libssl_static

LOCAL_CFLAGS +=  $(MY_UNITTEST_CFLAGS)
LOCAL_LDLIBS := -llog -lOpenSLES
include $(BUILD_SHARED_LIBRARY)

##########################################################

include $(CLEAR_VARS)
LOCAL_MODULE:= libjingle_p2p_unittest

LOCAL_C_INCLUDES:= \
	$(GTEST_C_INCLUDES) \
	$(MY_LIBJINGLE_C_INCLUDES)

LOCAL_ARM_MODE := arm
LOCAL_MODULE_TAGS := optional

LOCAL_SRC_FILES := \
	talk/media/base/testutils.cc \
	talk/p2p/base/dtlstransportchannel_unittest.cc \
	talk/p2p/base/p2ptransportchannel_unittest.cc \
	talk/p2p/base/port_unittest.cc \
	talk/p2p/base/portallocatorsessionproxy_unittest.cc \
	talk/p2p/base/pseudotcp_unittest.cc \
	talk/p2p/base/relayport_unittest.cc \
	talk/p2p/base/relayserver_unittest.cc \
	talk/p2p/base/session_unittest.cc \
	talk/p2p/base/stun_unittest.cc \
	talk/p2p/base/stunport_unittest.cc \
	talk/p2p/base/stunrequest_unittest.cc \
	talk/p2p/base/stunserver_unittest.cc \
	talk/p2p/base/transport_unittest.cc \
	talk/p2p/base/transportdescriptionfactory_unittest.cc \
	talk/p2p/client/connectivitychecker_unittest.cc \
	talk/p2p/client/portallocator_unittest.cc \
	talk/session/media/channel_unittest.cc \
	talk/session/media/channelmanager_unittest.cc \
	talk/session/media/currentspeakermonitor_unittest.cc \
	talk/session/media/mediarecorder_unittest.cc \
	talk/session/media/mediamessages_unittest.cc \
	talk/session/media/mediasession_unittest.cc \
	talk/session/media/mediasessionclient_unittest.cc \
	talk/session/media/rtcpmuxfilter_unittest.cc \
	talk/session/media/srtpfilter_unittest.cc \
	talk/session/media/ssrcmuxfilter_unittest.cc

LOCAL_CPP_EXTENSION:= .cc
LOCAL_WHOLE_STATIC_LIBRARIES := \
	libjingle \
	libwebrtc_voice \
	libjingle_unittest_main \
	libexpat_static \
	libsrtp_static \
	libssl_static

LOCAL_CFLAGS +=  $(MY_UNITTEST_CFLAGS)
LOCAL_LDLIBS := -llog -lOpenSLES
include $(BUILD_SHARED_LIBRARY)
