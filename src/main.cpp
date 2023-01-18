#include "dpdk_udp_socket.h"

int main(int argc, char *argv[]) {
    int ret;
    uint16_t port_id;

    // Initialize DPDK EAL
    ret = rte_eal_init(argc, argv);
    if (ret < 0)
        rte_exit(EXIT_FAILURE, "Error with EAL initialization\n");

    // Get the first available port
    port_id = get_available_port();
    if (port_id == rte_eth_dev_count())
        rte_exit(EXIT_FAILURE, "No available ethernet ports\n");

    // Initialize and start the UDP client
    DpdkUdpSocket client(port_id);
    if (!client.initialize())
        rte_exit(EXIT_FAILURE, "Error initializing UDP client\n");

    // Send and receive packets
    client.run();

    return 0;
}