#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-debug.h"
#include "net/ipv6/uip-udp-packet.h"
#include "sys/log.h"
#include "sys/etimer.h"

#define LOG_MODULE "Wormhole"
#define LOG_LEVEL LOG_LEVEL_INFO

PROCESS(wormhole_attack_process, "Wormhole Attack Process");
AUTOSTART_PROCESSES(&wormhole_attack_process);

static struct etimer timer;
static struct uip_udp_conn *udp_conn;
static uip_ipaddr_t peer_ip;  // Other end of wormhole

// Callback function to handle incoming UDP packets
static void
udp_receiver(struct uip_udp_conn *c, const uip_ipaddr_t *source_addr, uint16_t source_port,
             const uip_ipaddr_t *dest_addr, uint16_t dest_port, const uint8_t *data, uint16_t len)
{
  // Print received packet information
  LOG_INFO("Received packet from ");
  uip_debug_ipaddr_print(source_addr);
  LOG_INFO_(": ");
  LOG_INFO_("%s\n", data);
}

PROCESS_THREAD(wormhole_attack_process, ev, data)
{
  PROCESS_BEGIN();

  // Delay startup to ensure radio + routing init
  etimer_set(&timer, CLOCK_SECOND * 10);
  PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

  // Set peer wormhole node's IP (manually assign in Cooja or extract in runtime)
  uip_ds6_nbr_t *nbr = uip_ds6_nbr_head();
  if(nbr != NULL) {
    uip_ipaddr_copy(&peer_ip, &nbr->ipaddr);
    LOG_INFO("Wormhole peer IP set to: ");
    uip_debug_ipaddr_print(&peer_ip);
    LOG_INFO_("\n");
  } else {
    LOG_ERR("No neighbors found to act as wormhole peer\n");
    PROCESS_EXIT();
  }

  // Create UDP connection
  udp_conn = udp_new(&peer_ip, UIP_HTONS(3001), udp_receiver);
  if(udp_conn == NULL) {
    LOG_ERR("Failed to create UDP connection\n");
    PROCESS_EXIT();
  }
  udp_bind(udp_conn, UIP_HTONS(3001));
  LOG_INFO("Wormhole tunnel created\n");

  while(1) {
    etimer_set(&timer, CLOCK_SECOND * 10);
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&timer));

    // Simulate packet tunneling
    const char *fake_msg = "Forwarded RPL control msg";
    uip_udp_packet_send(udp_conn, fake_msg, strlen(fake_msg));
    LOG_INFO("Sent packet to ");
    uip_debug_ipaddr_print(&peer_ip);
    LOG_INFO_("\n");
  }

  PROCESS_END();
}

