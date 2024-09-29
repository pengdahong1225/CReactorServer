//
// Created by Messi on 2023/6/9.
//

#ifndef CREACTORSERVER_CODE_C_H
#define CREACTORSERVER_CODE_C_H

#include "noncopyable.h"
#include "Net/Buffer.h"
#include <sys/types.h>
#include <string>
#include <cstring>
#include <iostream>

/*
 * 解析器 -- 数据包解析
 */

// Protocol format:
//
// * 0                       4           6
// * +-----------------------+-----------+
// * |     packet len        |magic code |
// * +-----------+-----------+-----------+
// * |                                   |
// * +                                   +
// * |           body bytes              |
// * +                                   +
// * |            ... ...                |
// * +-----------------------------------+.

static int packetSize = 4;
static std::string magicCode = "XX";
static int magicCodeSize = 2;

class Codec : noncopyable {
public:
    std::string EnCodeData(const std::string &buf) {
        // std::string data;
        // const size_t length = packetSize + magicCodeSize + buf.size();
        // char frame[length];

        // int32_t header = buf.size();
        // memcpy(frame, &header, packetSize);
        // memcpy(frame + packetSize, magicCode.c_str(), magicCodeSize);
        // // content
        // memcpy(frame + packetSize + magicCodeSize, buf.c_str(), buf.size());

        // data = std::string(frame);
        return buf;
    }

    std::string DeCodeData(reactor::Buffer &buffer) {
        // std::string header, magic, body;
        // if (buffer.readableBytes() > 0) {
        //     header = buffer.retrieveAsString(packetSize);
        // }
        // if (buffer.readableBytes() > 0) {
        //     magic = buffer.retrieveAsString(magicCodeSize);
        // }
        // if (buffer.readableBytes() > 0) {
        //     body = buffer.retrieveAsString(packetSize);
        // }

        // if (header.empty() || magic.empty() || body.empty()) {
        //     return "";
        // }

        // int32_t length = std::atoi(header.c_str());
        // if (length != body.size()) {
        //     std::cout << "Codec::DeCodeData"
        //               << "->"
        //               << "the length of package is error" << std::endl;
        // }
        // if (strcmp(magic.c_str(), magicCode.c_str()) != 0) {
        //     std::cout << "Codec::DeCodeData"
        //               << "->"
        //               << "the magic of package is error" << std::endl;
        // }
        // return body;
        return buffer.retrieveAllAsString();
    }
};

#endif //CREACTORSERVER_CODE_C_H
