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
    //The ammount of time allowed for a session to become writeable
    kSessionTimeoutWritable = 50000,
    //This is the connection timeout to initiate a connected call
    kSessionTimeoutInitAck = 15000,
};

enum PortTimeout {
    //Times out port if does not receive a readable ping before
    kPortTimeoutConnectionReadable = 30000,
    //Times out port if cannot connect as writeable before
    kPortTimeoutConnectionWriteable = 15000,
    //Times out port if does not receive a writeable ping before
    kPortTimeoutConnectionWriteConnect = 5000,
    //Global timeout request for each port request
    kPortTimeoutConnectionResponse = 5000,
};

// When the socket is unwritable, we will use 10 Kbps (ignoring IP+UDP headers)
// for pinging.  When the socket is writable, we will use only 1 Kbps because
// we don't want to degrade the quality on a modem.  These numbers should work
// well on a 28.8K modem, which is the slowest connection on which the voice
// quality is reasonable at all.
enum P2PTransportChannelPingTimeout {
    kPingPacketSize = 60*8,//original
    //Times out channel if cannot connect as writable before
    kPingTimeoutWritableDelay = 1000 * kPingPacketSize / 1000,
    //Times out channel if does not receive a readable ping before
    kPingTimeoutUnWritableDelay = 1000 * kPingPacketSize / 10000,
    //Times out channel if does not receive a writable ping before
    kPingMaxCurrentWritableDelay = 2*kPingTimeoutWritableDelay,
};

enum BasicPortAllocatorTimeout {
    //The ammount of time to wait before starting allocation
    kAllocatorTimeoutAllocateDelay = 1000,//250
    //The ammount of time to spend on each step of the allocation sequence
    kAllocatorTimeoutAllocateStepDelay = 1000,
};
} //namespace cricket
#endif  // TALK_P2P_BASE_TIMEOUTS_H_
