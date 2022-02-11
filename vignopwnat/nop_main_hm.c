#include "nat_config.h"
#include "nf.h"
#include "nf-util.h"
//#include "../nf-log.h" //Needed?

struct nf_config config;

bool nf_has_rte_vlan_header(struct rte_ether_hdr *header) {
  return header->ether_type == rte_be_to_cpu_16(RTE_ETHER_TYPE_VLAN);
}

static inline struct rte_vlan_hdr *nf_then_get_rte_vlan_header(void *p) {
  //CHUNK_LAYOUT_N(p, rte_ether_hdr, rte_ether_fields, rte_ether_nested_fields); //TODO: Verify
  void *hdr = nf_borrow_next_chunk(p, sizeof(struct rte_vlan_hdr));
  return (struct rte_vlan_hdr *)hdr;
}

bool nf_init(void) {
  return true;
}

int nf_process(uint16_t device, uint8_t **buffer, uint16_t packet_length, vigor_time_t now, struct rte_mbuf *mbuf) {
  (void)now;

  int wan_vlan = -1; // "-1" represents an access port.
  int lan_vlan = 10;

  printf("Processing packet ... \n");

  uint16_t dst_device;

  if (device == config.wan_device) {
    dst_device = config.lan_main_device;

    struct rte_ether_hdr *rte_ether_header = nf_then_get_rte_ether_header(buffer);
    rte_ether_header->s_addr = config.device_macs[dst_device];
    rte_ether_header->d_addr = config.endpoint_macs[dst_device];

    if (nf_has_rte_vlan_header(rte_ether_header)) {
      printf("Path 1/5 = inc-wan-acs, has-hdr\n");
      return device; //Drop
    }

    struct rte_vlan_hdr* vlan_header = (struct rte_vlan_hdr*) nf_insert_new_chunk(buffer, sizeof(struct rte_vlan_hdr), mbuf);
    rte_ether_header = (struct rte_ether_hdr *) nf_get_borrowed_chunk(0);

    vlan_header->eth_proto = rte_ether_header->ether_type;
    rte_ether_header->ether_type = rte_be_to_cpu_16(RTE_ETHER_TYPE_VLAN);
    vlan_header->vlan_tci = rte_cpu_to_be_16((uint16_t)lan_vlan); //TODO: Confirm Priority and CFI

    printf("Path 2/5 = inc-wan-acs, no-hdr\n");
    return dst_device;

  } else {
    dst_device = config.wan_device;

    struct rte_ether_hdr *rte_ether_header = nf_then_get_rte_ether_header(buffer);
    rte_ether_header->s_addr = config.device_macs[dst_device];
    rte_ether_header->d_addr = config.endpoint_macs[dst_device];

    if (!nf_has_rte_vlan_header(rte_ether_header)) {
      printf("Path 3/5 = inc-lan-trk, no-hdr\n");
      return device; //Drop
    }

    struct rte_vlan_hdr *rte_vlan_header = nf_then_get_rte_vlan_header(buffer);

    if (rte_cpu_to_be_16((uint16_t)lan_vlan) != rte_vlan_header->vlan_tci) {
      printf("Path 4/5 = inc-lan-trk, wrong-vlan\n");
      return device; //Drop
    }

    rte_be16_t ether_type = rte_vlan_header->eth_proto;
    nf_shrink_chunk(buffer, 0, mbuf);
    rte_ether_header = (struct rte_ether_hdr *) nf_get_borrowed_chunk(0);
    rte_ether_header->ether_type = ether_type;
    printf("Path 5/5 = inc-lan-trk, rmv-hdr\n");
    return dst_device;
  }
}
