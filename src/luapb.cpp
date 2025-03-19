#define LUA_LIB

#include "luapb.h"

using namespace std;
using namespace luakit;

namespace luapb {

    int load_pb(lua_State* L, const char* filename) {
        FILE* fp = fopen(filename, "rb");
        if (!fp) return 0;
        auto buf = luakit::get_buff();
        auto len = filesystem::file_size(filename);
        auto lbuf = buf->peek_space(len);
        fread(lbuf, 1, len, fp);
        buf->pop_space(len);
        read_file_descriptor_set(buf->get_slice());
        fclose(fp);
        return 1;
    }


    int pb_encode(lua_State* L, string pbname) {
        auto buf = luakit::get_buff();
        buf->clean();
        encode_message(L, buf, find_message(pbname));
        lua_pushlstring(L, (char*)buf->head(), buf->size());
        return 1;
    }

    int pb_decode(lua_State* L, string pbname) {
        size_t len;
        auto val = (uint8_t*)lua_tolstring(L, 2, &len);
        slice s = slice(val, len);
        decode_message(L, &s, find_message(pbname));
        return 1;
    }

    luakit::lua_table open_luapb(lua_State* L) {
        kit_state kit_state(L);
        auto luapb = kit_state.new_table("protobuf");
        luapb.set_function("load_pb", load_pb);
        luapb.set_function("pb_encode", pb_encode);
        luapb.set_function("pb_decode", pb_decode);
        return luapb;
    }
}

extern "C" {
    LUALIB_API int luaopen_luapb(lua_State* L) {
        auto luapb = luapb::open_luapb(L);
        return luapb.push_stack();
    }
}
