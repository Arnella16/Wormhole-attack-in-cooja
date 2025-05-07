#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include <stdint.h>
#include <inttypes.h>
#include "random.h"

#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678
#define SEND_INTERVAL (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn;
static uint32_t tx_count = 0;
static uint32_t rx_count = 0;

/* Hardcoded wormhole IP */
static uip_ipaddr_t wormhole_ip;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP Client through Wormhole");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  LOG_INFO("Received response of length %u from ", datalen);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\nPayload: '%.*s'\n", datalen, (char *) data);
  rx_count++;
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char str[32];

  PROCESS_BEGIN();

  /* Set hardcoded wormhole IP */
  uip_ip6addr(&wormhole_ip, 0xaaaa,0,0,0,0x0212,0x7400,0,0xbbbb); // Example IP

  simple_udp_register(&udp_conn, UDP_CLIENT_PORT, NULL, UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    snprintf(str, sizeof(str), "hello %" PRIu32 "", tx_count);
    LOG_INFO("Sending request %" PRIu32 " through wormhole node ", tx_count);
    LOG_INFO_6ADDR(&wormhole_ip);
    LOG_INFO_("\n");

    simple_udp_sendto(&udp_conn, str, strlen(str), &wormhole_ip);
    tx_count++;

    etimer_set(&periodic_timer, SEND_INTERVAL - CLOCK_SECOND +
                                    (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}

