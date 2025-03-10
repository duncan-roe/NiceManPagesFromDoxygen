/*
 * (C) 2012 by Pablo Neira Ayuso <pablo@netfilter.org>
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This code has been sponsored by Vyatta Inc. <http://www.vyatta.com>
 */

#include <errno.h>
#include <stdlib.h>
#include <string.h> /* for memcpy */
#include <stdbool.h>

#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>

#include "internal.h"

/**
 * \defgroup pktbuff User-space network packet buffer
 *
 * These functions provide the user-space network packet buffer.
 * This abstraction is strongly inspired by Linux kernel network buffer,
 * the so-called sk_buff.
 *
 * \manonly
.SH SYNOPSIS
.nf
\fB
#include <libmnl/libmnl.h>
#include <libnetfilter_queue/pktbuff.h>
\endmanonly
 *
 * @{
 */

static int __pktb_setup(int family, struct pkt_buff *pktb)
{
	struct ethhdr *ethhdr;

	switch (family) {
	case AF_INET:
	case AF_INET6:
		pktb->network_header = pktb->data;
		break;
	case AF_BRIDGE:
		ethhdr = (struct ethhdr *)pktb->data;
		pktb->mac_header = pktb->data;

		switch(ethhdr->h_proto) {
		case ETH_P_IP:
		case ETH_P_IPV6:
			pktb->network_header = pktb->data + ETH_HLEN;
			break;
		default:
			/* This protocol is unsupported. */
			errno = EPROTONOSUPPORT;
			return -1;
		}
		break;
	}

	return 0;
}

static void pktb_setup_metadata(struct pkt_buff *pktb, void *pkt_data,
				size_t len, size_t extra)
{
	pktb->len = len;
	pktb->data_len = len + extra;
	pktb->data = pkt_data;
}

/**
 * pktb_alloc - allocate a new packet buffer
 * \note
 * pktb_alloc copies the IP datagram to the (<b>calloc</b>'d) packet buffer.
 * This is essential if your application needs to hold on to multiple
 * datagrams.
 * <br>If your application processes each datagram as it arrives, you can avoid
 * the datagram copy by calling pktb_setup_raw() instead.
 *
 * \param family Indicate what family. Currently supported families are
 * AF_BRIDGE, AF_INET & AF_INET6.
 * \param data Pointer to packet data
 * \param len Packet length
 * \param extra Extra memory in the tail to be allocated (for mangling)
 *
 * This function returns a packet buffer that contains the packet data and
 * some extra memory room in the tail (if requested). This function copies
 * the memory area provided as a pointer to packet data into the packet buffer
 * structure.
 *
 * The extra length provides extra packet data room at the tail of the packet
 * buffer in case you need to mangle it.
 *
 * \return Pointer to a new userspace packet buffer or NULL on failure.
 * \par Errors
 * __ENOMEM__ From __calloc__()
 * \n
 * __EPROTONOSUPPORT__ _family_ was __AF_BRIDGE__ and this is not an IP packet
 * (v4 or v6)
 * \sa __calloc__(3)
 */

struct pkt_buff *pktb_alloc(int family, void *data, size_t len, size_t extra)
{
	struct pkt_buff *pktb;
	void *pkt_data;

	pktb = calloc(1, sizeof(struct pkt_buff) + len + extra);
	if (pktb == NULL)
		return NULL;

	/* Better make sure alignment is correct. */
	pkt_data = (uint8_t *)pktb + sizeof(struct pkt_buff);
	memcpy(pkt_data, data, len);

	pktb_setup_metadata(pktb, pkt_data, len, extra);

	if (__pktb_setup(family, pktb) < 0) {
		free(pktb);
		return NULL;
	}

	return pktb;
}

/**
 * pktb_setup_raw - set up a packet buffer from memory area
 * \param pktb Pointer to memory of length pktb_head_size() bytes
 * \param family Supported families are AF_BRIDGE, AF_INET & AF_INET6.
 * \param data Pointer to packet data
 * \param len Packet data length
 * \param extra Extra memory available after packet data (for mangling).
 *
 * Use this function to set up a packet buffer in a memory area of size
 * pktb_head_size(). The created packet buffer addresses the packet data
 * in the buffer filled by <b>mnl_socket_recvfrom</b>() in the mainline.
 * This function avoids a packet copy and uses less memory than the alternative
 * pktb_alloc() interface.
 *
 * \note
 * The extra space available for mangling is the buffer size minus the number of
 * bytes returned by <b>mnl_socket_recvfrom</b>().
 * The mainline needs to pass this datum to the callback,
 * since that is where pktb_setup_raw() is called.
 * One can use the \b data argument of <b>mnl_cb_run</b>() to pass this datum.
 *
 * \return Pointer to a new userspace packet buffer or NULL on failure.
 * \par Errors
 * __EPROTONOSUPPORT__ _family_ was __AF_BRIDGE__ and this is not an IP packet
 * (v4 or v6)
 */

struct pkt_buff *pktb_setup_raw(void *pktb, int family, void *data,
				size_t len, size_t extra)
{
	memset(pktb, 0, sizeof (struct pkt_buff));
	pktb_setup_metadata(pktb, data, len, extra);
	if (__pktb_setup(family, pktb) < 0)
		pktb = NULL;

	return pktb;
}

/**
 * pktb_data - get pointer to network packet
 * \param pktb Pointer to userspace packet buffer
 * \return Pointer to start of network packet data within __pktb__
 * \par
 * It is appropriate to use _pktb_data_ as the second argument of
 * nfq_nlmsg_verdict_put_pkt()
 */

uint8_t *pktb_data(struct pkt_buff *pktb)
{
	return pktb->data;
}

/**
 * pktb_len - get length of packet buffer
 * \param pktb Pointer to userspace packet buffer
 * \return Length of packet contained within __pktb__
 * \par
 * It is appropriate to use _pktb_len_ as the third argument of
 * nfq_nlmsg_verdict_put_pkt()
 */

uint32_t pktb_len(struct pkt_buff *pktb)
{
	return pktb->len;
}

/**
 * pktb_free - release packet buffer
 * \param pktb Pointer to userspace packet buffer
 */

void pktb_free(struct pkt_buff *pktb)
{
	free(pktb);
}

/**
 * \defgroup otherfns Other functions
 *
 * The library provides a number of other functions which many user-space
 * programs will never need. These divide into 2 groups:
 * \n
 * 1. Functions to get values of members of opaque __struct pktbuff__, described
 * below
 *
 * 2. Internal functions, described in Topic __Internal functions__
 *
 * \manonly
.SH SYNOPSIS
.nf
\fB
#include <libmnl/libmnl.h>
#include <libnetfilter_queue/pktbuff.h>
\endmanonly
 *
 * @{
 */

/**
 * \defgroup do_not_use Internal functions
 *
 * Do not use these functions. Instead, always use the mangle
 * function appropriate to the level at which you are working.\n
pktb_mangle() uses all the below functions except pktb_pull(), which is not
 * used by anything.
 *
 * \manonly
.SH SYNOPSIS
.nf
\fB
#include <libmnl/libmnl.h>
#include <libnetfilter_queue/pktbuff.h>
\endmanonly
 *
 * @{
 */

/**
 * pktb_push - decrement pointer to packet buffer
 * \param pktb Pointer to userspace packet buffer
 * \param len Number of bytes to subtract from packet start address
 */

void pktb_push(struct pkt_buff *pktb, unsigned int len)
{
	pktb->data -= len;
	pktb->len += len;
}

/**
 * pktb_pull - increment pointer to packet buffer
 * \param pktb Pointer to userspace packet buffer
 * \param len Number of bytes to add to packet start address
 */

void pktb_pull(struct pkt_buff *pktb, unsigned int len)
{
	pktb->data += len;
	pktb->len -= len;
}

/**
 * pktb_put - add extra bytes to the tail of the packet buffer
 * \param pktb Pointer to userspace packet buffer
 * \param len Number of bytes to add to packet tail (and length)
 */

void pktb_put(struct pkt_buff *pktb, unsigned int len)
{
	pktb->len += len;
}

/**
 * pktb_trim - set new length for this packet buffer
 * \param pktb Pointer to userspace packet buffer
 * \param len New packet length (tail is adjusted to reflect this)
 */

void pktb_trim(struct pkt_buff *pktb, unsigned int len)
{
	pktb->len = len;
}

/**
 * @}
 */

/**
 * pktb_tailroom - get room available for packet expansion
 * \param pktb Pointer to userspace packet buffer
 * \return room in bytes after the tail of the packet buffer
 * \n
 * This starts off as the __extra__ argument to pktb_alloc().
 * Programmers should ensure this __extra__ argument is sufficient for any
 * packet mangle, as packet buffers cannot be expanded dynamically.
 */

unsigned int pktb_tailroom(struct pkt_buff *pktb)
{
	return pktb->data_len - pktb->len;
}

/**
 * pktb_mac_header - get address of layer 2 header (if any)
 * \param pktb Pointer to userspace packet buffer
 * \return Pointer to MAC header or NULL if no such header present.
 * \n
 * Only packet buffers in family __AF_BRIDGE__ have a non-NULL MAC header.
 */

uint8_t *pktb_mac_header(struct pkt_buff *pktb)
{
	return pktb->mac_header;
}

/**
 * pktb_network_header - get address of layer 3 header
 * \param pktb Pointer to userspace packet buffer
 * \return Pointer to layer 3 header or NULL if the packet buffer was created
 * with an unsupported family
 */

uint8_t *pktb_network_header(struct pkt_buff *pktb)
{
	return pktb->network_header;
}

/**
 * pktb_transport_header - get address of layer 4 header (if known)
 * \param pktb Pointer to userspace packet buffer
 * \return Pointer to layer 4 header or NULL if not (yet) set
 * \note
 * Unlike the lower-level headers, it is the programmer's responsibility to
 * create the level 4 (transport) header pointer by caling e.g.
 * nfq_ip_set_transport_header()
 */

uint8_t *pktb_transport_header(struct pkt_buff *pktb)
{
	return pktb->transport_header;
}

/**
 * @}
 */

static int pktb_expand_tail(struct pkt_buff *pktb, int extra)
{
	/* No room in packet, cannot mangle it. We don't support dynamic
	 * reallocation. Instead, increase the size of the extra room in
	 * the tail in pktb_alloc.
	 */
	if (pktb->len + extra > pktb->data_len)
		return 0;

	pktb->len += extra;
	return 1;
}

static int enlarge_pkt(struct pkt_buff *pktb, unsigned int extra)
{
	if (pktb->len + extra > 65535)
		return 0;

	if (!pktb_expand_tail(pktb, extra - pktb_tailroom(pktb)))
		return 0;

	return 1;
}

/**
 * pktb_mangle - adjust contents of a packet
 * \param pktb Pointer to userspace packet buffer
 * \param dataoff Supplementary offset, usually offset from layer 3 (IP) header
 * to the layer 4 (TCP or UDP) header. Specify zero to access the layer 3
 * header. If \b pktb was created in family \b AF_BRIDGE, specify
 * \b -ETH_HLEN (a negative offset) to access the layer 2 (MAC) header.
 * \param match_offset Further offset to content that you want to mangle
 * \param match_len Length of the existing content you want to mangle
 * \param rep_buffer Pointer to data you want to use to replace current content
 * \param rep_len Length of data you want to use to replace current content
 * \returns 1 for success and 0 for failure. Failure will occur if the \b extra
 * argument to the pktb_alloc() call that created \b pktb is less than the
 * excess of \b rep_len over \b match_len
 \warning pktb_mangle does not update any checksums. Developers should use the
 appropriate mangler for the protocol level: nfq_ip_mangle(),
 nfq_tcp_mangle_ipv4(), nfq_udp_mangle_ipv4() or IPv6 variants.
 \n
 It is appropriate to use pktb_mangle to change the MAC header.
 */

int pktb_mangle(struct pkt_buff *pktb,
		int dataoff,
		unsigned int match_offset,
		unsigned int match_len,
		const char *rep_buffer,
		unsigned int rep_len)
{
	unsigned char *data;

	if (rep_len > match_len &&
	    rep_len - match_len > pktb_tailroom(pktb) &&
	    !enlarge_pkt(pktb, rep_len - match_len))
		return 0;

	data = pktb->network_header + dataoff;

	/* move post-replacement */
	memmove(data + match_offset + rep_len,
		data + match_offset + match_len,
		pktb_tail(pktb) - (pktb->network_header + dataoff +
			     match_offset + match_len));

	/* insert data from buffer */
	memcpy(data + match_offset, rep_buffer, rep_len);

	/* update packet info */
	if (rep_len > match_len)
		pktb_put(pktb, rep_len - match_len);
	else
		pktb_trim(pktb, pktb->len + rep_len - match_len);

	pktb->mangled = true;
	return 1;
}

/**
 * pktb_mangled - test whether packet has been mangled
 * \param pktb Pointer to userspace packet buffer
 * \return __true__ if packet has been mangled (modified), else __false__
 * \par
 * When assembling a verdict, it is not necessary to return the contents of
 * un-modified packets. Use _pktb_mangled_ to decide whether packet contents
 * need to be returned.
 */

bool pktb_mangled(const struct pkt_buff *pktb)
{
	return pktb->mangled;
}

/**
 * pktb_head_size - get number of bytes needed for a packet buffer
 *                  (control part only)
 * \return size of struct pkt_buff
 */


size_t pktb_head_size(void)
{
	return sizeof(struct pkt_buff);
}

/**
 * @}
 */
