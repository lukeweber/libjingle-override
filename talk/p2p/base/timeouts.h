/*
 * libjingle
 * Copyright 2012, Tuenti Technologies.
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

#ifndef TALK_P2P_BASE_TIMEOUTS_H_
#define TALK_P2P_BASE_TIMEOUTS_H_

// This file contains timeouts related to signaling that are used in various
// classes in libjingle

namespace cricket {
enum SessionTimeout {
    kSessionTimeoutWritable = 50000,
    kSessionTimeoutInitAck = 8000,
};

enum PortTimeout {
    kPortTimeoutConnectionReadable = 90000,//30000
    kPortTimeoutConnectionWriteable = 45000,//15000
    kPortTimeoutConnectionWriteConnect = 15000,//5000
    kPortTimeoutConnectionResponse = 15000,//5000
};

// When the socket is unwritable, we will use 10 Kbps (ignoring IP+UDP headers)
// for pinging.  When the socket is writable, we will use only 1 Kbps because
// we don't want to degrade the quality on a modem.  These numbers should work
// well on a 28.8K modem, which is the slowest connection on which the voice
// quality is reasonable at all.
enum P2PTransportChannelPingTimeout {
    kPingPacketSize = 60*8,
    kPingTimeoutWritableDelay = 30000 * kPingPacketSize / 1000,//1000 * PING_PACKET_SIZE / 1000
    kPingTimeoutUnWritableDelay = 30000 * kPingPacketSize / 10000,//1000 * PING_PACKET_SIZE / 10000
    kPingMaxCurrentWritableDelay = 900,//2*WRITABLE_DELAY - bit
};

enum BasicPortAllocatorTimeout {
    kAllocatorTimeoutAllocateDelay = 4000,//250
    kAllocatorTimeoutAllocateStepDelay = 4000,
};
} //namespace cricket
#endif  // TALK_P2P_BASE_TIMEOUTS_H_
