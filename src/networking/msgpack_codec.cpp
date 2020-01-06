#include <string>
#include <sstream>
#include <tuple>
#include <vector>

#include <doctest.h>
#include <msgpack.hpp>

#include "msgpack_codec.h"

namespace ai
{
TEST_CASE("MsgPackCodec")
{
    typedef std::tuple<int, std::vector<std::string>> TestObject;

    SUBCASE("Encodes object correctly")
    {
        TestObject object{5, {"asd", "sdf"}};

        std::stringstream buffer;
        msgpack::pack(buffer, object);

        auto encoded_object = MsgPackCodec::encode(object);

        DOCTEST_CHECK(buffer.str() == encoded_object);
    }

    SUBCASE("Decodes object correctly")
    {
        TestObject object{5, {"asd", "sdf"}};

        std::stringstream buffer;
        msgpack::pack(buffer, object);

        auto reconstructed_object = MsgPackCodec::decode<TestObject>(buffer.str());

        DOCTEST_CHECK(object == reconstructed_object);
    }

    SUBCASE("Can perform a round trip")
    {
        TestObject object{5, {"asd", "sdf"}};

        auto encoded_object = MsgPackCodec::encode(object);
        auto reconstructed_object = MsgPackCodec::decode<TestObject>(encoded_object);

        DOCTEST_CHECK(object == reconstructed_object);
    }

    SUBCASE("Decodes to msgpack::object_handle correctly")
    {
        TestObject object{5, {"asd", "sdf"}};

        std::stringstream buffer;
        msgpack::pack(buffer, object);

        auto decoded_object = MsgPackCodec::decode<msgpack::object_handle>(buffer.str());

        DOCTEST_CHECK(decoded_object->as<TestObject>() == object);
    }
}
}