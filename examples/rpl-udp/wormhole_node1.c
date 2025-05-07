#include "contiki.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "sys/log.h"
#include <stdint.h>
#include <inttypes.h>
#include "random.h"
#include <stdio.h>
#include "net/ipv6/uip-ds6.h"
#include "net/routing/routing.h"

#define LOG_MODULE "Wormhole-A"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT	8765
#define UDP_SERVER_PORT	5678

static struct simple_udp_connection udp_conn_client;
static struct simple_udp_connection udp_conn_forward;

struct message {
  uint8_t dest_id;
  uint8_t forwarded_flag;
};


PROCESS(wormhole_node_a_process, "Wormhole Node A");
AUTOSTART_PROCESSES(&wormhole_node_a_process);

static void
set_ipaddr(uip_ipaddr_t *ip, uint8_t id)
{
  //LOG_INFO("ID: %u\n", id);
  uint16_t seg_5 = 0x7400 + id;         // 7400 + id → 7402
  uint16_t seg_7 = (0x100 * id) + id;    // id0id → 202 when id=2

  uip_ip6addr(ip, 0xfd00, 0, 0, 0, 0x212, seg_5, id, seg_7);
}

static void
get_next_hop(uint8_t dest_id, uip_ipaddr_t *next_hop)
{
  switch(dest_id) {
    case 1: set_ipaddr(next_hop, 6); break; // via 3 (Link-local) exp
    case 2: set_ipaddr(next_hop, 2); break; // via 7 (Link-local)
    case 3: set_ipaddr(next_hop, 6); break; // direct (Link-local)
    case 4: set_ipaddr(next_hop, 6); break; // via 8 (Link-local)
    case 5: set_ipaddr(next_hop, 2); break; // via 2 (Link-local)
    case 6: set_ipaddr(next_hop, 6); break; // direct (Link-local)
    case 8: set_ipaddr(next_hop, 6); break; // via 3 (Link-local)
    default: memset(next_hop, 0, sizeof(uip_ipaddr_t)); break; // No route
  }
}
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
	uip_ipaddr_t peer_ip;
	
	struct message msg;
	msg.dest_id = data[0];
	LOG_INFO("Received packet for ID %u", msg.dest_id);
	  LOG_INFO_6ADDR(sender_addr);
	  LOG_INFO_("\n");

	  uint8_t my_id = linkaddr_node_addr.u8[7];

	  if(my_id == msg.dest_id) {
	    LOG_INFO("This node (ID %u) is the final destination.\n", my_id);
	  } else {
	  	set_ipaddr(&peer_ip, 8);
		if(!data[1]){
  			msg.dest_id = data[0];
  			msg.forwarded_flag = 1;
  			simple_udp_sendto(&udp_conn_client, &msg, sizeof(msg), &peer_ip);
  			return;
  		}
	    uip_ipaddr_t next_hop;
	    get_next_hop(msg.dest_id, &next_hop);
	    
	    

	    if(!uip_is_addr_unspecified(&next_hop)) {
	      LOG_INFO("Forwarding to destination ID %u via next hop: ", msg.dest_id);
	      LOG_INFO_6ADDR(&next_hop);
	      LOG_INFO_("\n");
	      simple_udp_sendto(&udp_conn_client, &msg, sizeof(msg), &next_hop);
	    } else {
	      LOG_WARN("No route for dest ID: %u\n", msg.dest_id);
	    }
	  }
 }

/*---------------------------------------------------------------------------*/
PROCESS_THREAD(wormhole_node_a_process, ev, data)
{
 
  PROCESS_BEGIN();
  
  LOG_INFO("Wormhole Node A started\n");

  /* Use a manual IP within the DAG's prefix (assuming root uses aaaa::/64) */
  uip_ipaddr_t my_ip;
  set_ipaddr(&my_ip, 7);
  uip_ds6_addr_add(&my_ip, 0, ADDR_MANUAL);

  LOG_INFO("My IP address: ");
  LOG_INFO_6ADDR(&my_ip);
  LOG_INFO_("\n");
  
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn_client,
                    UDP_SERVER_PORT, NULL,
                    UDP_SERVER_PORT, udp_rx_callback);
  simple_udp_register(&udp_conn_forward, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
          
  LOG_INFO("Initialised\n");        
  
  PROCESS_END();
}

/*---------------------------------------------------------------------------*/
