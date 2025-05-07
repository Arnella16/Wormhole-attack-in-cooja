#include "contiki.h"
#include "contiki-net.h"                   // includes uip, uip-ds6, uip-udp-packet
#include "net/netstack.h"
#include "net/routing/routing.h"           // NETSTACK_ROUTING
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include <string.h>

#define LOG_MODULE "UDP Server"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT   8765
#define UDP_SERVER_PORT1  5678
#define UDP_SERVER_PORT2  2346

static struct simple_udp_connection server_conn1, server_conn2;

/*---------------------------------------------------------------------------*/
PROCESS(udp_server_process, "UDP server process");
AUTOSTART_PROCESSES(&udp_server_process);
/*---------------------------------------------------------------------------*/

#define MAPPER_GET_PACKETDATA(dest, source) \
  memcpy(&dest, source, sizeof(dest)); source += sizeof(dest)


static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{
  uint8_t  nodeid, parent_node;
  uint16_t node_rank, parent_rank;
  int      numof_nbr;
  uint8_t *appdata = (uint8_t *)uip_appdata;
  int      i;

  /* Unpack fixed header */
  MAPPER_GET_PACKETDATA(nodeid,       appdata);
  MAPPER_GET_PACKETDATA(node_rank,    appdata);
  MAPPER_GET_PACKETDATA(parent_node,  appdata);
  MAPPER_GET_PACKETDATA(parent_rank,  appdata);
  MAPPER_GET_PACKETDATA(numof_nbr,    appdata);

  LOG_INFO("Node %u rank %u parent %u prank %u nbrs %d: ",
           nodeid, node_rank, parent_node, parent_rank, numof_nbr);

  /* Unpack each neighbor */
  for(i = 0; i < numof_nbr; i++) {
    uint8_t  nbr_id;
    uint16_t nbr_r;
    MAPPER_GET_PACKETDATA(nbr_id, appdata);
    MAPPER_GET_PACKETDATA(nbr_r,  appdata);
    LOG_INFO("(%u:%u) ", nbr_id, nbr_r);
  }
  LOG_INFO_("\n");
}
/*---------------------------------------------------------------------------*/
PROCESS_THREAD(udp_server_process, ev, data)
{
  uip_ipaddr_t prefix;
  PROCESS_BEGIN();

  /* 1) Assign a manual /64 prefix AAAA::1 */
  uip_ip6addr(&prefix, 0xaaaa,0,0,0,0,0,0,1);
  uip_ds6_set_addr_iid(&prefix, &uip_lladdr);
  uip_ds6_addr_add(&prefix, 0, ADDR_MANUAL);
  LOG_INFO("Border router IPv6: ");
  LOG_INFO_6ADDR(&prefix);
  LOG_INFO_("\n");

  /* 2) Start RPL as root */
  NETSTACK_ROUTING.root_start();
  LOG_INFO("RPL DAG root started\n");

  /* 3) Open two UDP server ports */
  simple_udp_register(&server_conn1, UDP_SERVER_PORT1,
                      NULL, UDP_CLIENT_PORT, udp_rx_callback);
  simple_udp_register(&server_conn2, UDP_SERVER_PORT2,
                      NULL, UDP_CLIENT_PORT, udp_rx_callback);
  LOG_INFO("Listening on UDP ports %u and %u\n",
           UDP_SERVER_PORT1, UDP_SERVER_PORT2);

  /* 4) Nothing more to do—packets handled in callback */
  while(1) {
    PROCESS_YIELD();
  }
  PROCESS_END();
}

