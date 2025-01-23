//
// Created by peter on 2025/1/7.
//

#ifndef CORE_PACKETSTREAMPARSER_H
#define CORE_PACKETSTREAMPARSER_H

#include "Buffer.h"
#include <arpa/inet.h>

// Protocol format:
//
// * 0                       4           6
// * +-----------------------+-----------+
// * |   packet len          |magic code |
// * +-----------+-----------+-----------+
// * |                                   |
// * +                                   +
// * |           body bytes              |
// * +                                   +
// * |            ... ...                |
// * +-----------------------------------+.
// * packet len = magic code + body bytes

#define TCP_DEFAULT_BUFFER 1024*400
#pragma pack(1)
struct PacketHeader {
    unsigned int pack_len;
    unsigned char magic_code[2];
};
#pragma pack()

/**
 * 流式数据包解析器
 * 用于从Buffer中读取标准格式的数据包
 */
class PacketStreamParser {
public:
    int parse_packet_length(Buffer &buffer) {
        size_t buf_len = buffer.readableBytes();
        // 不足一个包头，继续接收
        if (buf_len < sizeof(PacketHeader)) {
            return 0;
        }
        // 满足一个包头，校验魔术码
        struct PacketHeader *pHeader = (struct PacketHeader *) buffer.peek();
        if (pHeader->magic_code[0] != 'X' || pHeader->magic_code[1] != 'X') {
            return -1;
        }
        // 包头合法，读取整个数据包的长度
        int pkg_len = sizeof(int) + ntohl(pHeader->pack_len); // 网络字节序转主机字节序
        if (pkg_len > TCP_DEFAULT_BUFFER) {
            printf("packet too big, pkglen=%d\n", pkg_len);
            return -1;
        }

        // 包体还没有接收完，继续接收
        if (buf_len < pkg_len) {
            return 0;
        }

        // 收到一个完整包
        return pkg_len;
    }

    std::string get_packet(Buffer &buffer, size_t len) {
        std::string packet = buffer.retrieveAsString(len);
        std::string body = packet.substr(sizeof(PacketHeader));
        printf("packet len=%ld, body len=%ld\n", packet.size(), body.size());
        return body;
    }

    std::string serialize_packet(const std::string &body) {
        struct PacketHeader header;
        header.pack_len = htonl(body.size());
        header.magic_code[0] = 'X';
        header.magic_code[1] = 'X';

        std::string packet;
        packet.append((char *) &header, sizeof(header));
        packet.append(body);
        return packet;
    }
};


#endif //CORE_PACKETSTREAMPARSER_H
