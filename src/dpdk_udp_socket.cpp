#include "dpdk_udp_socket.h"

DpdkUdpSocket::DpdkUdpSocket(uint16_t port_id) :
    m_port_id(port_id)
{
}

DpdkUdpSocket::~DpdkUdpSocket()
{
    rte_eth_dev_stop(m_port_id);
    rte_eth_dev_close(m_port_id);
    rte_mempool_free(m_mbuf_pool);
}

bool DpdkUdpSocket::initialize()
{
    // Create memory pool for mbufs
    m_mbuf_pool = rte_pktmbuf_pool_create("MBUF_POOL", 8192, 32, 0, RTE_MBUF_DEFAULT_BUF_SIZE, rte_socket_id());
    if (m_mbuf_pool == NULL)
        return false;

    // Get device information
    rte_eth_dev_info_get(m_port_id, &m_dev_info);

    // Configure the port
    memset(&m_port_conf, 0, sizeof(m_port_conf));
    m_port_conf.rxmode.mq_mode = ETH_MQ_RX_RSS;
    m_port_conf.rx_adv_conf.rss_conf.rss_key = NULL;
    m_port_conf.rx_adv_conf.rss_conf.rss_hf = ETH_RSS_UDP;
    rte_eth_dev_configure(m_port_id, 1, 1, &m_port_conf);

    // Setup RX and TX queues
    if(!setupSocketBuffers(m_port_id, m_dev_info, m_mbuf_pool))
       return false;
    return true;
}

bool DpdkUdpSocket::setupSocketBuffers(uint16_t port_id, struct rte_eth_dev_info dev_info, struct rte_mempool *mbuf_pool)
{
    struct rte_eth_rxconf rx_conf;
    struct rte_eth_txconf tx_conf;
    int ret;

    // Setup RX queues
    rx_conf = dev_info.default_rxconf;
    rx_conf.rx_thresh.pthresh = 8;
    rx_conf.rx_thresh.hthresh = 8;
    rx_conf.rx_thresh.wthresh = 4;
    rx_conf.offloads = DEV_RX_OFFLOAD_CHECKSUM;
    ret = rte_eth_rx_queue_setup(port_id, 0, RX_RING_SIZE,
                                 rte_eth_dev_socket_id(port_id), &rx_conf, mbuf_pool);
    if (ret < 0)
        return false;

    // Setup TX queues
    tx_conf = dev_info.default_txconf;
    tx_conf.tx_thresh.pthresh = 36;
    tx_conf.tx_thresh.hthresh = 0;
    tx_conf.tx_thresh.wthresh = 0;
    tx_conf.offloads = DEV_TX_OFFLOAD_UDP_CKSUM;
    ret = rte_eth_tx_queue_setup(port_id, 0, TX_RING_SIZE,
                                 rte_eth_dev_socket_id(port_id), &tx_conf);
    if (ret < 0)
        return false;

    // Start the port
    ret = rte_eth_dev_start(port_id);
    if (ret < 0)
        return false;

    return true;
}

// TODO move out the event loop logic from socket class
void DpdkUdpSocket::run()
{
    struct rte_mbuf *pkts_tx[MAX_PKT_BURST];
    struct rte_mbuf *pkts_rx[MAX_PKT_BURST];

    // Create epoll file descriptor
    int epfd = epoll_create1(0);
    if (epfd == -1) {
        // Handle error
    }

    // Add DPDK port fd to epoll
    struct epoll_event ev;
    ev.events = EPOLLIN;
    ev.data.fd = rte_eth_dev_socket_id(m_port_id);
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, ev.data.fd, &ev) == -1) {
        // Handle error
    }

    while (true) {
        // Wait for events
        int nfds = epoll_wait(epfd, &ev, 1, -1);
        if (nfds == -1) {
            // Handle error
        }

        // Allocate packets for transmission
        for (int i = 0; i < MAX_PKT_BURST; i++) {
            pkts_tx[i] = rte_pktmbuf_alloc(m_mbuf_pool);
            if (pkts_tx[i] == NULL) {
                // Handle error
            }
            // Fill in Ethernet, IPv4, and UDP headers
        }

        // Transmit packets
        int nb_tx = rte_eth_tx_burst(m_port_id, 0, pkts_tx, MAX_PKT_BURST);

        // Receive packets
        int nb_rx = rte_eth_rx_burst(m_port_id, 0, pkts_rx, MAX_PKT_BURST);

        // Handle received packets
        for (int i = 0; i < nb_rx; i++) {
            // Process packet
            rte_pktmbuf_free(pkts_rx[i]);
        }

        // check if any packet left to receive
        int ret = rte_eth_rx_queue_count(m_port_id,0);
        if(ret == 0)
            // no packets left to receive
            break;
    }

    // Close epoll file descriptor
    close(epfd);
}