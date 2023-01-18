#pragma once

#include <rte_mempool.h>
#include <rte_ethdev.h>

class DpdkUdpSocket {
public:
    DpdkUdpSocket(uint16_t port_id);
    ~DpdkUdpSocket();

    bool initialize();
    void run();

private:
    uint16_t m_port_id;
    struct rte_mempool *m_mbuf_pool;
    struct rte_eth_dev_info m_dev_info;
    struct rte_eth_conf m_port_conf;
    bool setupSocket(uint16_t port_id, struct rte_eth_dev_info dev_info, struct rte_mempool *mbuf_pool);
};