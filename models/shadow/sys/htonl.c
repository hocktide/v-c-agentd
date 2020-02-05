#include <stdint.h>
#include <string.h>

uint32_t htonl(uint32_t const net)
{
    uint8_t data[4];
    memcpy(&data, &net, sizeof(data));

    return (((uint32_t)data[3]) << 0) | (((uint32_t)data[2]) << 8) | (((uint32_t)data[1]) << 16) | (((uint32_t)data[0]) << 24);
}
