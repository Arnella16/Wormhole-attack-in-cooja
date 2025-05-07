#include "contiki.h"
#include "net/routing/routing.h"
#include "random.h"
#include "net/netstack.h"
#include "net/ipv6/simple-udp.h"
#include "net/routing/rpl-classic/rpl.h" // <--- Important for DAG and rank access
#include <stdint.h>
#include <stdio.h>
#include <inttypes.h>

#include "sys/log.h"
#define LOG_MODULE "App"
#define LOG_LEVEL LOG_LEVEL_INFO

#define WITH_SERVER_REPLY  1
#define UDP_CLIENT_PORT    8765
#define UDP_SERVER_PORT    5678
#define WORMHOLE_NODE_IP   "aaaa::0212:7400:bbbb"

struct message {
  uint8_t dest_id;
  uint8_t forwarded_flag;
};

#define SEND_INTERVAL      (10 * CLOCK_SECOND)

static struct simple_udp_connection udp_conn_forward;
static struct simple_udp_connection udp_conn_client;
static uint32_t rx_count = 0;

static clock_time_t send_time;  // Time when request was sent
static clock_time_t response_time; // Time when response was received
static uint32_t round_trip_time;

/*---------------------------------------------------------------------------*/
PROCESS(udp_client_process, "UDP client");
AUTOSTART_PROCESSES(&udp_client_process);
/*---------------------------------------------------------------------------*/

#define MAX_NODES 9
static uint8_t forwarding_table[MAX_NODES][MAX_NODES];

static void
set_ipaddr(uip_ipaddr_t *ip, uint8_t id)
{
  //LOG_INFO("ID: %u\n", id);
  uint16_t seg_5 = 0x7400 + id;         // 7400 + id → 7402
  uint16_t seg_7 = (0x100 * id) + id;    // id0id → 202 when id=2

  uip_ip6addr(ip, 0xfd00, 0, 0, 0, 0x212, seg_5, id, seg_7);
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
  LOG_INFO("Received packet of length %u from ", datalen);
  LOG_INFO_6ADDR(sender_addr);
  LOG_INFO_(" to ");
  LOG_INFO_6ADDR(receiver_addr);
  
  struct message msg;
  msg.dest_id = data[0];
  msg.forwarded_flag = data[1];


LOG_INFO("Received msg destined to ID: %u\n", msg.dest_id);

  if(msg.dest_id >= MAX_NODES) {
    LOG_ERR("Invalid destination ID in packet.\n");
    return;
  }

  uint8_t my_id = linkaddr_node_addr.u8[7];
  uint8_t next_hop_id = forwarding_table[my_id][msg.dest_id];

  if(msg.dest_id == my_id) {
    LOG_INFO("Packet has reached final destination: %u\n", msg.dest_id);
    response_time = clock_time(); // Timestamp when response is received
    round_trip_time = response_time - send_time; // Calculate round trip time
    LOG_INFO("Round trip time: %" PRIu32 " ticks\n", round_trip_time);
    return; // Do nothing; we are the destination
  }

  if(next_hop_id == 0) {
    LOG_ERR("No route to destination %u from node %u\n", msg.dest_id, my_id);
    return;
  }

  // Construct IP address of next hop
  uip_ipaddr_t next_hop_ipaddr;
  set_ipaddr(&next_hop_ipaddr, next_hop_id);
  
  simple_udp_sendto(&udp_conn_client, &msg, sizeof(msg), &next_hop_ipaddr);
  LOG_INFO("Forwarding packet to next hop %u [", next_hop_id);
  LOG_INFO_6ADDR(&next_hop_ipaddr);
  //LOG_INFO("\nForwarded flag: %u\n", msg.forwarded_flag);
  LOG_INFO_("]\n");

  
}

/*---------------------------------------------------------------------------*/


static void initialize_forwarding_table() {
  // Initialize all entries to 0 (0 = no route)
  memset(forwarding_table, 0, sizeof(forwarding_table));

  // Example topology (you can expand this based on your exact layout):

  // Node 1
  forwarding_table[1][1] = 1;
  forwarding_table[1][2] = 3;
  forwarding_table[1][3] = 3;
  forwarding_table[1][4] = 3;
  forwarding_table[1][5] = 3;
  forwarding_table[1][6] = 3;
  forwarding_table[1][7] = 3;
  forwarding_table[1][8] = 3;

  // Node 2
  forwarding_table[2][1] = 7;
  forwarding_table[2][2] = 2;
  forwarding_table[2][3] = 7;
  forwarding_table[2][4] = 7;
  forwarding_table[2][5] = 5;
  forwarding_table[2][6] = 7;
  forwarding_table[2][7] = 7;
  forwarding_table[2][8] = 7;

  // Node 3
  forwarding_table[3][1] = 1;
  forwarding_table[3][2] = 6;
  forwarding_table[3][3] = 3;
  forwarding_table[3][4] = 8;
  forwarding_table[3][5] = 6;
  forwarding_table[3][6] = 6;
  forwarding_table[3][7] = 6;
  forwarding_table[3][8] = 8;

  // Node 4
  forwarding_table[4][1] = 8;
  forwarding_table[4][2] = 8;
  forwarding_table[4][3] = 8;
  forwarding_table[4][4] = 4;
  forwarding_table[4][5] = 8;
  forwarding_table[4][6] = 8;
  forwarding_table[4][7] = 8;
  forwarding_table[4][8] = 8;

  // Node 5
  forwarding_table[5][1] = 2;
  forwarding_table[5][2] = 2;
  forwarding_table[5][3] = 2;
  forwarding_table[5][4] = 2;
  forwarding_table[5][5] = 5;
  forwarding_table[5][6] = 2;
  forwarding_table[5][7] = 2;
  forwarding_table[5][8] = 2;
  
  // Node 7
  forwarding_table[7][1] = 6;
  forwarding_table[7][2] = 2;
  forwarding_table[7][3] = 6;
  forwarding_table[7][4] = 6;
  forwarding_table[7][5] = 2;
  forwarding_table[7][6] = 6;
  forwarding_table[7][7] = 7;
  forwarding_table[7][8] = 6;
  
  // Node 8
  forwarding_table[8][1] = 3;
  forwarding_table[8][2] = 3;
  forwarding_table[8][3] = 3;
  forwarding_table[8][4] = 4;
  forwarding_table[8][5] = 3;
  forwarding_table[8][6] = 3;
  forwarding_table[8][7] = 3;
  forwarding_table[8][8] = 8;
  
}



PROCESS_THREAD(udp_client_process, ev, data)
{
  static struct etimer periodic_timer;
  static char str[32];
  uip_ipaddr_t dest_ipaddr;
  static uint32_t tx_count;
  static uint32_t missed_tx_count;
  initialize_forwarding_table();
  PROCESS_BEGIN();
  uip_ipaddr_t my_ipaddr;

  uint8_t node_id = linkaddr_node_addr.u8[7]; // Last byte is usually unique

  set_ipaddr(&my_ipaddr, node_id);
  uip_ds6_addr_add(&my_ipaddr, 0, ADDR_MANUAL);

  LOG_INFO("My IP address: ");
  LOG_INFO_6ADDR(&my_ipaddr);
  LOG_INFO_("\n");

  LOG_INFO_("\n");
  /* Initialize UDP connection */
  simple_udp_register(&udp_conn_forward, UDP_CLIENT_PORT, NULL,
                      UDP_SERVER_PORT, udp_rx_callback);
  simple_udp_register(&udp_conn_client,
                    UDP_SERVER_PORT, NULL,
                    UDP_SERVER_PORT, udp_rx_callback);

  etimer_set(&periodic_timer, random_rand() % SEND_INTERVAL);
  while(1) {
    PROCESS_WAIT_EVENT_UNTIL(etimer_expired(&periodic_timer));

    if(NETSTACK_ROUTING.node_is_reachable() &&
       NETSTACK_ROUTING.get_root_ipaddr(&dest_ipaddr)) {

      /* 🔍 Print RPL rank */
      rpl_dag_t *dag = rpl_get_any_dag();
      if(dag != NULL) {
        LOG_INFO("Current node rank: %u\n", dag->rank);
      } else {
        LOG_INFO("No DAG joined yet\n");
      }

      /* Print statistics every 10th TX */
      if(tx_count % 10 == 0) {
        LOG_INFO("Tx/Rx/MissedTx: %" PRIu32 "/%" PRIu32 "/%" PRIu32 "\n",
                 tx_count, rx_count, missed_tx_count);
      }

      /* Send to DAG root */
      LOG_INFO("Sending request %"PRIu32" to ", tx_count);
      LOG_INFO_6ADDR(&dest_ipaddr);
      LOG_INFO_("\n");
      snprintf(str, sizeof(str), "hello %" PRIu32 "", tx_count);
      send_time = clock_time();
      simple_udp_sendto(&udp_conn_forward, str, strlen(str), &dest_ipaddr);
      tx_count++;
    } else {
      LOG_INFO("Not reachable yet\n");
      if(tx_count > 0) {
        missed_tx_count++;
      }
    }

    /* Add some jitter */
    etimer_set(&periodic_timer, SEND_INTERVAL
      - CLOCK_SECOND + (random_rand() % (2 * CLOCK_SECOND)));
  }

  PROCESS_END();
}
/*---------------------------------------------------------------------------*/

