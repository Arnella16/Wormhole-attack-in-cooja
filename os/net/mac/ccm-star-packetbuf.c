/*
 * Copyright (c) 2013, Hasso-Plattner-Institut.
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the Institute nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE INSTITUTE AND CONTRIBUTORS ``AS IS'' AND
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED.  IN NO EVENT SHALL THE INSTITUTE OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS
 * OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * This file is part of the Contiki operating system.
 *
 */

/**
 * \file
 *         CCM* convenience functions for LLSEC use
 * \author
 *         Justin King-Lacroix <justin.kinglacroix@gmail.com>
 *         Konrad Krentz <konrad.krentz@gmail.com>
 */

#include "net/mac/ccm-star-packetbuf.h"
#include "net/linkaddr.h"
#include "net/packetbuf.h"
#include "net/mac/llsec802154.h"
#include <string.h>

#if LLSEC802154_USES_AUX_HEADER && LLSEC802154_USES_FRAME_COUNTER

/*---------------------------------------------------------------------------*/
void
ccm_star_packetbuf_set_nonce(uint8_t *nonce, int forward)
{
  const linkaddr_t *source_addr = forward
                                  ? &linkaddr_node_addr
                                  : packetbuf_addr(PACKETBUF_ADDR_SENDER);
  memcpy(nonce, source_addr->u8, LINKADDR_SIZE);
  memset(nonce + LINKADDR_SIZE, 0, 8 - LINKADDR_SIZE);
  nonce[8] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) >> 8;
  nonce[9] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_2_3) & 0xff;
  nonce[10] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) >> 8;
  nonce[11] = packetbuf_attr(PACKETBUF_ATTR_FRAME_COUNTER_BYTES_0_1) & 0xff;
  nonce[12] = packetbuf_attr(PACKETBUF_ATTR_SECURITY_LEVEL);
}
/*---------------------------------------------------------------------------*/
#endif /* LLSEC802154_USES_AUX_HEADER && LLSEC802154_USES_FRAME_COUNTER */
