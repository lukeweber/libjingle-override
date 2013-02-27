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

// This file contain SDP strings used for testing.

#ifndef TALK_APP_WEBRTC_TEST_TESTSDPSTRINGS_H_
#define TALK_APP_WEBRTC_TEST_TESTSDPSTRINGS_H_

namespace webrtc {

// SDP offer string from a Nightly FireFox build.
static const char kFireFoxSdpOffer[] =
    "v=0\r\n"
    "o=Mozilla-SIPUA 23551 0 IN IP4 0.0.0.0\r\n"
    "s=SIP Call\r\n"
    "t=0 0\r\n"
    "a=ice-ufrag:e5785931\r\n"
    "a=ice-pwd:36fb7878390db89481c1d46daa4278d8\r\n"
    "a=fingerprint:sha-256 A7:24:72:CA:6E:02:55:39:BA:66:DF:6E:CC:4C:D8:B0:1A:"
    "BF:1A:56:65:7D:F4:03:AD:7E:77:43:2A:29:EC:93\r\n"
    "m=audio 36993 RTP/SAVPF 109 0 8 101\r\n"
    "c=IN IP4 74.95.2.170\r\n"
    "a=rtpmap:109 opus/48000/2\r\n"
    "a=ptime:20\r\n"
    "a=rtpmap:0 PCMU/8000\r\n"
    "a=rtpmap:8 PCMA/8000\r\n"
    "a=rtpmap:101 telephone-event/8000\r\n"
    "a=fmtp:101 0-15\r\n"
    "a=sendrecv\r\n"
    "a=candidate:0 1 UDP 2112946431 172.16.191.1 61725 typ host\r\n"
    "a=candidate:2 1 UDP 2112487679 172.16.131.1 58798 typ host\r\n"
    "a=candidate:4 1 UDP 2113667327 10.0.254.2 58122 typ host\r\n"
    "a=candidate:5 1 UDP 1694302207 74.95.2.170 36993 typ srflx raddr "
    "10.0.254.2 rport 58122\r\n"
    "a=candidate:0 2 UDP 2112946430 172.16.191.1 55025 typ host\r\n"
    "a=candidate:2 2 UDP 2112487678 172.16.131.1 63576 typ host\r\n"
    "a=candidate:4 2 UDP 2113667326 10.0.254.2 50962 typ host\r\n"
    "a=candidate:5 2 UDP 1694302206 74.95.2.170 41028 typ srflx raddr"
    " 10.0.254.2 rport 50962\r\n"
    "m=video 38826 RTP/SAVPF 120\r\n"
    "c=IN IP4 74.95.2.170\r\n"
    "a=rtpmap:120 VP8/90000\r\n"
    "a=sendrecv\r\n"
    "a=candidate:0 1 UDP 2112946431 172.16.191.1 62017 typ host\r\n"
    "a=candidate:2 1 UDP 2112487679 172.16.131.1 59741 typ host\r\n"
    "a=candidate:4 1 UDP 2113667327 10.0.254.2 62652 typ host\r\n"
    "a=candidate:5 1 UDP 1694302207 74.95.2.170 38826 typ srflx raddr"
    " 10.0.254.2 rport 62652\r\n"
    "a=candidate:0 2 UDP 2112946430 172.16.191.1 63440 typ host\r\n"
    "a=candidate:2 2 UDP 2112487678 172.16.131.1 51847 typ host\r\n"
    "a=candidate:4 2 UDP 2113667326 10.0.254.2 58890 typ host\r\n"
    "a=candidate:5 2 UDP 1694302206 74.95.2.170 33611 typ srflx raddr"
    " 10.0.254.2 rport 58890\r\n"
    "m=application 45536 SCTP/DTLS 5000\r\n"
    "c=IN IP4 74.95.2.170\r\n"
    "a=fmtp:5000 protocol=webrtc-datachannel;streams=16\r\n"
    "a=sendrecv\r\n"
    "a=candidate:0 1 UDP 2112946431 172.16.191.1 60248 typ host\r\n"
    "a=candidate:2 1 UDP 2112487679 172.16.131.1 55925 typ host\r\n"
    "a=candidate:4 1 UDP 2113667327 10.0.254.2 65268 typ host\r\n"
    "a=candidate:5 1 UDP 1694302207 74.95.2.170 45536 typ srflx raddr"
    " 10.0.254.2 rport 65268\r\n"
    "a=candidate:0 2 UDP 2112946430 172.16.191.1 49162 typ host\r\n"
    "a=candidate:2 2 UDP 2112487678 172.16.131.1 59635 typ host\r\n"
    "a=candidate:4 2 UDP 2113667326 10.0.254.2 61232 typ host\r\n"
    "a=candidate:5 2 UDP 1694302206 74.95.2.170 45468 typ srflx raddr"
    " 10.0.254.2 rport 61232\r\n";

}  // namespace webrtc

#endif  // TALK_APP_WEBRTC_TEST_TESTSDPSTRINGS_H_
