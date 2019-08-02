#pragma once

#include <string>
#include <sstream>

#include <msgpack.hpp>

namespace SingularityTrainer
{
class MsgPackCodec
{
  public:
    template <typename T>
    static std::string encode(const T &object)
    {
        std::stringstream buffer;
        msgpack::pack(buffer, object);
        return buffer.str();
    }

    template <typename T>
    static T decode(const std::string &message)
    {
        return msgpack::unpack(message.data(), message.size())->as<T>();
    }
};
}