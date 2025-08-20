#include "protocol_handler.h"
#include <spdlog/spdlog.h>
#include <arpa/inet.h>
#include <cstring>

/**
 * @brief 将 protobuf 的 Packet 序列化为网络帧字符串
 *
 * 该函数将一个 `Packet` 对象（protobuf 消息）序列化为字符串，见messages.proto
 * 并加上网络帧长度前缀，生成可直接发送到网络的帧数据。
 *
 * 帧格式如下：
 * [4 字节长度（网络字节序）][protobuf 序列化数据]
 *
 * 函数执行流程：
 * 1. 使用 `SerializeToString()` 将 protobuf Packet 序列化为字符串。
 *    - 如果序列化失败，会记录错误并返回空字符串。
 * 2. 检查序列化后的数据是否超过 `MAX_FRAME_SIZE`。
 *    - 如果超过，会记录错误并返回空字符串。
 * 3. 将 protobuf 数据长度转换为网络字节序（大端）使用 `host_to_network()`。
 * 4. 构造最终帧字符串：
 *    - 前 4 字节：protobuf 数据的长度（网络字节序），因为TCP是流式的
 *    - 后续字节：protobuf 序列化数据
 * 5. 返回构造好的帧字符串，可直接用于 socket 发送。
 *
 * @param packet 待序列化的 protobuf `Packet` 对象
 *
 * @return std::string 返回序列化后的帧字符串。
 *         如果序列化失败或帧超长，则返回空字符串。
 *
 * @note 调用者负责将返回的帧发送到网络。
 * @note 长度前缀为网络字节序（大端），保证跨平台兼容。
 * @note 本函数内部会使用 `spdlog` 打印错误，但不会抛出异常。
 */
std::string ProtocolHandler::serialize_frame(const Packet &packet)
{
    try
    {
        // Serialize protobuf to string
        std::string protobuf_data;
        if (!packet.SerializeToString(&protobuf_data))
        {
            spdlog::error("Failed to serialize packet to protobuf");
            return "";
        }

        // Check frame size limit
        if (protobuf_data.size() > MAX_FRAME_SIZE)
        {
            spdlog::error("Frame size {} exceeds maximum {}", protobuf_data.size(), MAX_FRAME_SIZE);
            return "";
        }

        // Create frame: [4-byte length][protobuf data]
        uint32_t length = static_cast<uint32_t>(protobuf_data.size());
        uint32_t network_length = host_to_network(length);

        std::string frame;
        frame.reserve(4 + protobuf_data.size());

        // Append length in network byte order
        /**
         * std::string& append(const char* s, size_t n); 需要一个const char* 和数据长度n
         * 但是 network_length 是一个整数类型 (uint32_t)，不是 char*。
         */
        frame.append(reinterpret_cast<const char *>(&network_length), 4);

        // Append protobuf data
        frame.append(protobuf_data);

        return frame;
    }
    catch (const std::exception &e)
    {
        spdlog::error("Exception in serialize_frame: {}", e.what());
        return "";
    }
}

bool ProtocolHandler::deserialize_frame(const std::string &frame_data, Packet &packet)
{
    try
    {
        if (!packet.ParseFromString(frame_data))
        {
            spdlog::error("Failed to parse protobuf data");
            return false;
        }

        return validate_packet(packet);
    }
    catch (const std::exception &e)
    {
        spdlog::error("Exception in deserialize_frame: {}", e.what());
        return false;
    }
}

/**
 * @brief 解析一个网络帧（Frame）数据
 *
 * 该函数用于从接收到的字节缓冲区中解析一个完整的网络帧。
 * 网络帧的格式为：
 * [4 字节长度（网络字节序）][长度为 N 的数据]
 *
 * 功能：
 * 1. 检查缓冲区是否至少包含 4 个字节的长度头；
 * 2. 读取前 4 个字节的长度信息，并将网络字节序转换为主机字节序；
 * 3. 校验帧长度是否超过允许的最大值；
 * 4. 检查缓冲区是否包含完整的数据帧；
 * 5. 提取帧数据到 Frame 结构体，并记录消耗的字节数。
 *
 * @param buffer [in] 待解析的字节缓冲区，类型为 std::vector<uint8_t>。
 * @param frame [out] 输出解析得到的 Frame 对象，其中包含帧长度和帧数据。
 * @param consumed_bytes [out] 输出已消耗的字节数，成功解析为完整帧则为 4 + length，失败或帧无效时可能为 4。
 *
 * @return true 如果成功解析出完整的帧；
 * @return false 如果缓冲区不足以解析出完整帧，或帧长度超过最大限制。
 *
 * @note
 * - 函数假设缓冲区中帧的长度头是网络字节序（big-endian）。
 * - 如果帧长度超过 MAX_FRAME_SIZE，函数会记录错误并跳过长度头。
 * - 如果缓冲区中数据不足，函数会返回 false，等待后续数据到达。
 * - 解析后的 frame.data 是一个长度为 length 的字节向量，frame.length = length。
 */
bool ProtocolHandler::parse_frame(const std::vector<uint8_t> &buffer, Frame &frame, size_t &consumed_bytes)
{
    consumed_bytes = 0;

    // Need at least 4 bytes for length header
    if (buffer.size() < 4)
    {
        return false;
    }

    // Read length from first 4 bytes (network byte order)
    uint32_t network_length;
    std::memcpy(&network_length, buffer.data(), 4);
    uint32_t length = network_to_host(network_length);

    // Validate frame length
    if (length > MAX_FRAME_SIZE)
    {
        spdlog::error("Frame length {} exceeds maximum {}", length, MAX_FRAME_SIZE);
        consumed_bytes = 4; // Skip the invalid length header
        return false;
    }

    // Check if we have complete frame
    if (buffer.size() < 4 + length)
    {
        return false; // Need more data
    }

    // Extract frame data
    frame.length = length;
    frame.data.assign(buffer.begin() + 4, buffer.begin() + 4 + length);
    consumed_bytes = 4 + length;

    return true;
}

Packet ProtocolHandler::create_error_response(uint32_t error_code, const std::string &message, uint32_t sequence)
{
    Packet packet;
    packet.set_version(PROTOCOL_VERSION);
    packet.set_sequence(sequence);

    /**
     * 返回一个指向 ErrorResponse 的指针，你可以直接调用它的 setter 方法设置字段。
     * 自动把 Packet 的 oneof payload 当前选项设置为 error。
     */
    ErrorResponse *error = packet.mutable_error();
    error->set_error_code(error_code);
    error->set_message(message);

    return packet;
}

Packet ProtocolHandler::create_echo_response(const std::string &content, uint32_t sequence)
{
    Packet packet;
    packet.set_version(PROTOCOL_VERSION);
    packet.set_sequence(sequence);

    EchoResponse *echo = packet.mutable_echo_response();
    echo->set_content(content);

    return packet;
}

/**
 * 判断Version不要使用旧版的协议
 * 检查echo_request, echo_response, error这三个字段要有一个
 */
bool ProtocolHandler::validate_packet(const Packet &packet)
{
    // Check protocol version
    if (packet.version() != PROTOCOL_VERSION)
    {
        spdlog::warn("Invalid protocol version: {}, expected: {}", packet.version(), PROTOCOL_VERSION);
        return false;
    }

    // Check if packet has valid payload
    if (!packet.has_echo_request() && !packet.has_echo_response() && !packet.has_error())
    {
        spdlog::warn("Packet has no valid payload");
        return false;
    }

    return true;
}

/**
 * htonl：Host TO Network Long把 主机字节序（Host byte order） 的 32 位整数转换为 网络字节序（Network byte order）。大端（big-endian）。
 */
uint32_t ProtocolHandler::host_to_network(uint32_t value)
{
    return htonl(value);
}

/**
 * ntohl 把 网络字节序（Network byte order） 的 32 位整数转换回 主机字节序（Host byte order）。
 */
uint32_t ProtocolHandler::network_to_host(uint32_t value)
{
    return ntohl(value);
}