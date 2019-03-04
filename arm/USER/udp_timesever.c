#include "uip.h"
#include "tcpip.h"
#include "sntpc.h"

#include <stdio.h>
#include <string.h>

static struct uip_udp_conn *sntp_conn = NULL;

void sntp_conf(u16_t *sntpserver)
{
  if(sntp_conn != NULL) {
    uip_udp_remove(sntp_conn);
  }

	sntp_conn = uip_udp_new((uip_ipaddr_t *)sntpserver, HTONS(123));
}

void sntp_appcall(void)
{
  if(uip_udp_conn->rport == HTONS(53)) { 
    if(uip_poll()) {
      check_entries();
    }
    if(uip_newdata()) {
      newdata();
    }
  }
}