#define LUA_LIB

#include "luapb.h"

using namespace std;
using namespace luakit;

namespace luapb {

    int load_pb(lua_State* L) {
        size_t len;
        auto data = (uint8_t*)lua_tolstring(L, 2, &len);
        auto buf = luakit::get_buff();
        auto lbuf = buf->peek_space(len);
        buf->push_data(data, len);
        buf->pop_space(len);
        read_file_descriptor_set(buf->get_slice());
        return 1;
    }

    int load_file(lua_State* L, const char* filename) {
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
        auto msg = find_message(pbname);
        if (msg == nullptr) luaL_error(L, "invalid pb cmd type");
        try {
            encode_message(L, buf, msg);
        } catch (const exception& e){
            luaL_error(L, e.what());
        }
        lua_pushlstring(L, (char*)buf->head(), buf->size());
        return 1;
    }

    int pb_decode(lua_State* L, string pbname) {
        size_t len;
        auto val = (uint8_t*)lua_tolstring(L, 2, &len);
        auto msg = find_message(pbname);
        if (msg == nullptr) luaL_error(L, "invalid pb cmd type");
        slice s = slice(val, len);
        try {
            decode_message(L, &s, msg);
        } catch (const exception& e) {
            luaL_error(L, e.what());
        }
        return 1;
    }

    int pb_enum_id(lua_State* L, string efullname) {
        auto penum = find_enum(efullname);
        if (penum) {
            if (lua_isstring(L, 2)) {
                string key = lua_tostring(L, 2);
                auto it = penum->kvpair.find(key);
                if (it != penum->kvpair.end()) {
                    lua_pushinteger(L, it->second);
                    return 1;
                }
            }
            if (lua_isnumber(L, 2)) {
                int32_t key = lua_tointeger(L, 2);
                auto it = penum->vkpair.find(key);
                if (it != penum->vkpair.end()) {
                    lua_pushlstring(L, it->second.data(), it->second.size());
                    return 1;
                }
            }
        }
        return 0;
    }

    luakit::lua_table open_luapb(lua_State* L) {
        kit_state kit_state(L);
        lua_table luapb = kit_state.new_table("protobuf");
        luapb.set_function("load", load_pb);
        luapb.set_function("enum", pb_enum_id);
        luapb.set_function("enums", pb_enums);
        luapb.set_function("clear", pb_clear);
        luapb.set_function("decode", pb_decode);
        luapb.set_function("encode", pb_encode);
        luapb.set_function("loadfile", load_file);
        luapb.set_function("messages", pb_messages);
        return luapb;
    }
}

extern "C" {
    LUALIB_API int luaopen_luapb(lua_State* L) {
        auto luapb = luapb::open_luapb(L);
        return luapb.push_stack();
    }
}
