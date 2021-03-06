/*
 * raw socket based virtual network interface feature for LKL
 * Copyright (c) 2015,2016 Ryo Nakamura, Hajime Tazaki
 *
 * Author: Ryo Nakamura <upa@wide.ad.jp>
 *         Hajime Tazaki <thehajime@gmail.com>
 *
 * Current implementation is linux-specific.
 */

#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <unistd.h>
#include <net/if.h>
#include <linux/if_ether.h>
#include <linux/if_packet.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <net/ethernet.h>

#include "virtio.h"
#include "virtio_net_fd.h"

/* since Linux 3.14 (man 7 packet) */
#ifndef PACKET_QDISC_BYPASS
#define PACKET_QDISC_BYPASS 20
#endif

struct lkl_netdev *lkl_netdev_raw_create(const char *ifname)
{
	int ret;
	struct sockaddr_ll *ll;
	int fd, fd_flags, val;

	//fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_ALL));
	//fd = socket(PF_PACKET, SOCK_RAW, htons(ETH_P_IP));
	fd = socket(PF_PACKET, SOCK_DGRAM, htons(ETH_P_IP));
	//fd = socket(PF_INET, SOCK_RAW, htons(IPPROTO_TCP));
	if (fd < 0) {
		perror("socket");
		return NULL;
	}

	//memset(&ll, 0, sizeof(ll));
    ll = malloc(sizeof(*ll));
	ll->sll_family = PF_PACKET;
	ll->sll_ifindex = if_nametoindex(ifname);
	//ll.sll_protocol = htons(ETH_P_ALL);
	ll->sll_protocol = htons(ETH_P_IP);
    //const unsigned char ether_broadcast_addr[] = {0xff, 0xff, 0xff, 0xff, 0xff, 0xff};
    //ll.sll_halen = ETHER_ADDR_LEN;
    //memcpy(ll.sll_addr, ether_broadcast_addr, ETHER_ADDR_LEN);

	ret = bind(fd, (struct sockaddr *)ll, sizeof(*ll));
	if (ret) {
		perror("bind");
		close(fd);
		return NULL;
	}


   // printf("%s\n", ifname);

	val = 1;
	ret = setsockopt(fd, SOL_PACKET, PACKET_QDISC_BYPASS, &val,
			 sizeof(val));
	if (ret)
		perror("PACKET_QDISC_BYPASS, ignoring");

	fd_flags = fcntl(fd, F_GETFD, NULL);
	fcntl(fd, F_SETFL, fd_flags | O_NONBLOCK);

	return lkl_register_netdev_fd(fd, ll);
}
