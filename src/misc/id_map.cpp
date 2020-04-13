#include <string>

#include <doctest.h>

#include "id_map.h"

namespace ai
{
std::size_t IdMap::operator[](const std::string &audio_name)
{
    mutex.lock_shared();
    if (!map.has_value(audio_name))
    {
        mutex.unlock_shared();
        mutex.lock();
        const auto map_size = map.size();
        map.insert(map_size, audio_name);
        mutex.unlock();
        return map_size;
    }
    auto key = map.get_key(audio_name);
    mutex.unlock_shared();
    return key;
}

const std::string IdMap::operator[](const std::size_t audio_id)
{
    return map.get_value(audio_id);
}

TEST_CASE("IdMap")
{
    IdMap id_map;

    SUBCASE("IDs increase monotonically")
    {
        DOCTEST_CHECK(id_map["asd"] == 0);
        DOCTEST_CHECK(id_map["sdf"] == 1);
        DOCTEST_CHECK(id_map["dfg"] == 2);
        DOCTEST_CHECK(id_map["fgh"] == 3);
    }

    SUBCASE("The same string returns the same ID")
    {
        DOCTEST_CHECK(id_map["asd"] == id_map["asd"]);
    }

    SUBCASE("IDs can be recalled")
    {
        id_map["asd"];
        id_map["sdf"];
        id_map["dfg"];
        id_map["fgh"];
        DOCTEST_CHECK(id_map[id_map["asd"]] == id_map[id_map["asd"]]);
        DOCTEST_CHECK(id_map[id_map["sdf"]] == id_map[id_map["sdf"]]);
        DOCTEST_CHECK(id_map[id_map["dfg"]] == id_map[id_map["dfg"]]);
        DOCTEST_CHECK(id_map[id_map["fgh"]] == id_map[id_map["fgh"]]);
    }
}
}