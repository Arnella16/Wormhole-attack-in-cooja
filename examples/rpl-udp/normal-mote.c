#include "contiki.h"
#include "contiki-net.h"
#include "lib/random.h"
#include "sys/ctimer.h"
#include "net/ipv6/uip.h"
#include "net/ipv6/uip-ds6.h"
#include "net/ipv6/uip-udp-packet.h"
#include "sys/log.h"

#include <stdio.h>
#include <string.h>

#define DEBUG DEBUG_PRINT
#include "net/ipv6/uip-debug.h"

#ifndef PERIOD
#define PERIOD 60
#endif

#define LOG_MODULE "UDP Client"
#define LOG_LEVEL LOG_LEVEL_INFO

#define START_INTERVAL		(15 * CLOCK_SECOND)
#define SEND_INTERVAL		(PERIOD * CLOCK_SECOND)

#define UIP_IP_BUF   ((struct uip_ip_hdr *)&uip_buf[UIP_LLH_LEN])
#define UIP_UDP_BUF  ((struct uip_udp_hdr *)&uip_buf[UIP_LLIPH_LEN])
#define UIP_ICMP_BUF ((struct uip_icmp_hdr *)&uip_buf[uip_l2_l3_hdr_len])

static struct uip_udp_conn *client_conn;
static uip_ipaddr_t server_ipaddr;

#define MAPPER_ADD_PACKETDATA(dest, source) \
  memcpy(dest, &source, sizeof(source)); dest += sizeof(source)

#define MAPPER_GET_PACKETDATA(dest, source) \
  memcpy(&dest, source, sizeof(dest)); source += sizeof(dest)

static void send_packet(void *ptr) {
    uip_ipaddr_t my_ip;
    uip_ds6_addr_t *addr;
    uip_ipaddr_t *parent_ipaddr = NULL;

    // Get the node's IPv6 address
    addr = &uip_ds6_if.addr_list[0];
    if (addr != NULL) {
        my_ip = addr->ipaddr;
    }

    PRINTF("Node sending data to %d\n", server_ipaddr.u8[sizeof(server_ipaddr.u8) - 1]);

    unsigned char buf[64];
    unsigned char *buf_p = buf;

    // Prepare the data to be sent
    MAPPER_ADD_PACKETDATA(buf_p, my_ip.u8[15]);

    // Example data: Sending node location and some extra information
    uint8_t location_x = 10;  // Dummy value for location X
    uint8_t location_y = 20;  // Dummy value for location Y
    MAPPER_ADD_PACKETDATA(buf_p, location_x);
    MAPPER_ADD_PACKETDATA(buf_p, location_y);

    // Send the packet via UDP
    uip_udp_packet_sendto(client_conn, buf, sizeof(buf), &server_ipaddr, UIP_HTONS(2345));
}

static void print_local_addresses(void) {
    int i;
    uint8_t state;

    PRINTF("Client IPv6 addresses: ");
    for (i = 0; i < UIP_DS6_ADDR_NB; i++) {
        state = uip_ds6_if.addr_list[i].state;
        if (uip_ds6_if.addr_list[i].isused &&
            (state == ADDR_TENTATIVE || state == ADDR_PREFERRED)) {
            PRINT6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
            PRINTF("\n");
        }
    }
}
static void print_local_addresses(void) {
  int i;
  for(i = 0; i < UIP_DS6_ADDR_NB; i++) {
    if(uip_ds6_if.addr_list[i].isused) {
      LOG_INFO("IPv6 Address: ");
      LOG_INFO_6ADDR(&uip_ds6_if.addr_list[i].ipaddr);
      LOG_INFO_("\n");
    }
  }
}
void set_global_address(void) {
    uip_ipaddr_t ipaddr;
    uip_ip6addr(&ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 0);
    uip_ds6_set_addr_iid(&ipaddr, &uip_lladdr);
    uip_ds6_addr_add(&ipaddr, 0, ADDR_AUTOCONF);

    // Set server address
    uip_ip6addr(&server_ipaddr, 0xaaaa, 0, 0, 0, 0, 0, 0, 1);
}

PROCESS(udp_client_process, "UDP client process");
AUTOSTART_PROCESSES(&udp_client_process);

PROCESS_THREAD(udp_client_process, ev, data) {
    static struct etimer periodic;
    static struct ctimer backoff_timer;

    PROCESS_BEGIN();

    set_global_address();
    PRINTF("UDP client process started\n");

    print_local_addresses();

    // Create a new UDP connection with the remote server
    client_conn = udp_new(NULL, UIP_HTONS(2345), NULL);
    if (client_conn == NULL) {
        PRINTF("No UDP connection available, exiting the process!\n");
        PROCESS_EXIT();
    }
    udp_bind(client_conn, UIP_HTONS(12345));

    PRINTF("Created a connection with the server ");
    PRINT6ADDR(&client_conn->ripaddr);
    PRINTF(" local/remote port %u/%u\n",
           UIP_HTONS(client_conn->lport), UIP_HTONS(client_conn->rport));

    etimer_set(&periodic, SEND_INTERVAL);

    while (1) {
        PROCESS_YIELD();
        if (ev == tcpip_event) {
            // Handle incoming data (not implemented in this version)
        }

        if (etimer_expired(&periodic)) {
            etimer_reset(&periodic);
            ctimer_set(&backoff_timer, CLOCK_SECOND * 1, send_packet, NULL);
        }
    }

    PROCESS_END();
}

