#include "contiki.h"
#include "net/netstack.h"
#include "net/packetbuf.h"
#include "net/linkaddr.h"
#include "sys/log.h"
#include <string.h>

#define LOG_MODULE "Wormhole"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(wormhole_process, "Wormhole Mote");
AUTOSTART_PROCESSES(&wormhole_process);

#define MAX_PACKET_SIZE 128
#define WORMHOLE_TAG 0x42

// Check if the packet has already been forwarded by a wormhole node
static int is_forwarded_packet(const uint8_t *data, int len) {
  return len > 0 && data[0] == WORMHOLE_TAG;
}

static void wormhole_input_packet(void) {
  const linkaddr_t *from = packetbuf_addr(PACKETBUF_ADDR_SENDER);
  const uint8_t *data = packetbuf_dataptr();
  int len = packetbuf_datalen();

  LOG_INFO("Received packet from %02x.%02x, len=%d\n", from->u8[0], from->u8[1], len);

  if (is_forwarded_packet(data, len)) {
    LOG_INFO("Already forwarded. Skipping to prevent loop.\n");
    return;
  }

  if (len + 1 <= MAX_PACKET_SIZE) {
    uint8_t buffer[MAX_PACKET_SIZE];
    buffer[0] = WORMHOLE_TAG;
    memcpy(&buffer[1], data, len);
    int new_len = len + 1;

    // Broadcast the packet so other wormhole nodes receive it
    packetbuf_clear();
    packetbuf_copyfrom(buffer, new_len);
    packetbuf_set_addr(PACKETBUF_ADDR_RECEIVER, &linkaddr_null); // Broadcast

    LOG_INFO("Forwarding packet with wormhole tag. New len=%d\n", new_len);
    NETSTACK_LLSEC.send(NULL, NULL);
  }
}

PROCESS_THREAD(wormhole_process, ev, data)
{
  PROCESS_BEGIN();

  LOG_INFO("Wormhole node started at %02x.%02x\n",
           linkaddr_node_addr.u8[0], linkaddr_node_addr.u8[1]);

  NETSTACK_NETWORK.input = wormhole_input_packet;

  PROCESS_END();
}

