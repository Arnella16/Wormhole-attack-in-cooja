#include "contiki.h"
#include "net/routing/routing.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include <string.h>

#define LOG_MODULE "UDP-Server"
#define LOG_LEVEL LOG_LEVEL_INFO

#define UDP_CLIENT_PORT 8765
#define UDP_SERVER_PORT 5678

static struct simple_udp_connection udp_conn_client;
static struct simple_udp_connection udp_conn_forward;

struct message {
  uint8_t dest_id;
  uint8_t forwarded_flag;
};


/* Helper to build an IP like fd00::212:740<id>:<id>:<id>0<id> */
static void
set_ipaddr(uip_ipaddr_t *ip, uint8_t id)
{
  //LOG_INFO("ID: %u\n", id);
  uint16_t seg_5 = 0x7400 + id;         // 7400 + id → 7402
  uint16_t seg_7 = (0x100 * id) + id;    // id0id → 202 when id=2

  uip_ip6addr(ip, 0xfd00, 0, 0, 0, 0x212, seg_5, id, seg_7);
}

/* Routing table logic using link-local addresses */
static void
get_next_hop(uint8_t dest_id, uip_ipaddr_t *next_hop)
{
  switch(dest_id) {
    case 1: set_ipaddr(next_hop, 3); break; // via 3 (Link-local) exp
    case 2: set_ipaddr(next_hop, 7); break; // via 7 (Link-local)
    case 3: set_ipaddr(next_hop, 3); break; // direct (Link-local)
    case 4: set_ipaddr(next_hop, 3); break; // via 8 (Link-local)
    case 5: set_ipaddr(next_hop, 7); break; // via 2 (Link-local)
    case 7: set_ipaddr(next_hop, 7); break; // direct (Link-local)
    case 8: set_ipaddr(next_hop, 3); break; // via 3 (Link-local)
    default: memset(next_hop, 0, sizeof(uip_ipaddr_t)); break; // No route
  }
}
static void
udp_rx_callback(struct simple_udp_connection *c,
                const uip_ipaddr_t *sender_addr,
                uint16_t sender_port,
                const uip_ipaddr_t *receiver_addr,
                uint16_t receiver_port,
                const uint8_t *data,
                uint16_t datalen)
{

  struct message msg;
  
  if(datalen <= 2) {
    // First byte is the destination ID
    msg.dest_id = data[0];
    msg.forwarded_flag = data[1];
  } else {
    // No explicit destination ID in payload → infer from sender address
    msg.dest_id = sender_addr->u8[15]; // Last byte of IPv6 address
    msg.forwarded_flag = 0;
  }
	
  LOG_INFO("Received packet for ID %u", msg.dest_id);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_("\n");

  uint8_t my_id = linkaddr_node_addr.u8[7];

  if(my_id == msg.dest_id) {
    LOG_INFO("This node (ID %u) is the final destination.\n", my_id);
  } else {
    uip_ipaddr_t next_hop;
    get_next_hop(msg.dest_id, &next_hop);
    
    

    if(!uip_is_addr_unspecified(&next_hop)) {
      LOG_INFO("Forwarding to destination ID %u via next hop: ", msg.dest_id);
      LOG_INFO_6ADDR(&next_hop);
      //LOG_INFO("\nForwarded flag: %u\n", msg.forwarded_flag);
      LOG_INFO_("\n");
      simple_udp_sendto(&udp_conn_client, &msg, sizeof(msg), &next_hop);
    } else {
      LOG_WARN("No route for dest ID: %u\n", msg.dest_id);
    }
  }
}



PROCESS(udp_server_process, "UDP Server");
AUTOSTART_PROCESSES(&udp_server_process);

PROCESS_THREAD(udp_server_process, ev, data)
{
  PROCESS_BEGIN();

  NETSTACK_ROUTING.root_start();

  simple_udp_register(&udp_conn_forward, UDP_SERVER_PORT, NULL,
                      UDP_CLIENT_PORT, udp_rx_callback);
                      
  simple_udp_register(&udp_conn_client, UDP_SERVER_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);

  PROCESS_END();
}

