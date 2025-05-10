# luapb
一个使用C++编写的面向lua的protobuf编码解码库！

# 依赖
- [lua](https://github.com/xiyoo0812/lua.git)5.3以上
- [luakit](https://github.com/xiyoo0812/luakit.git)一个luabind库
- 项目路径如下<br>
  |--proj <br>
  &emsp;|--lua <br>
  &emsp;|--luapb <br>
  &emsp;|--luakit

# 编译
- msvc: 准备好lua依赖库并放到指定位置，将proj文件加到sln后编译。
- linux: 准备好lua依赖库并放到指定位置，执行make -f luapb.mak

# 其他
- 仅支持lua5.3以上版本，proto3协议
- 性能：lua-protobuf性能对比，测试不是很充分，仅做参考，release/O2编译
  | 平台 | 库  | 编译选项 | 1W次编解码    | 10W次编解码 | 100W次编解码
  |----------|-----------|-----------|-----------|--------|-----|
  | Win11| luapb | -O2 | 40ms   | 407ms   | 3967ms
  | Win11| lua-protobuf | -O2  | 80ms   | 688ms   | 6887ms
  | Debian10| luapb | -O2 | 56ms   | 563ms   | 5637ms
  | Debian10| lua-protobuf | -O2  | 87ms   | 867ms   | 8535ms

# 用法

```protobuf
//protobuf定义

// NCmdId
enum NCmdId
{
    NID_QUANTA_ZERO                     = 0;
    NID_HEARTBEAT_REQ                   = 1001;
    NID_HEARTBEAT_RES                   = 1002;
}

message child_message
{
    uint32 id                       = 1;        // id
    string name                     = 2;        // name
    map<string, string> values      = 3;
}

message test_message
{
    uint32 id                       = 1;        // id
    child_message child             = 2;
    repeated int32 custom2          = 3;
    map<string, string> custom      = 4;
    repeated child_message childs   = 5;
    map<string, child_message> kvs  = 6;
    oneof custom3 {
        string str                  = 8;
        int32 num                   = 9;
    }
}
```
```lua
require("luapb")
local log_debug     = logger.debug
local pb_enum_id    = protobuf.enum
local pbdecode      = protobuf.decode
local pbencode      = protobuf.encode

--从文件加载protobuf二进制描述文件
protobuf.loadfile("xxx.pb")

--从字符串加载protobuf二进制描述文件
local f = io.open("xxx.pb", "rb")
local data = f:read("*a")
protobuf.load(data)
f:close()

--查询所有枚举
for _, name in pairs(protobuf.enums()) do
    log_debug("enum: {}", name)
end

--查询枚举值
local enumval = pb_enum_id("ncmd_cs.NCmdID", "NID_HEARTBEAT_REQ")
log_debug("enumval: {}", enumval)

--查询所有message信息
for full_name, name in pairs(protobuf.messages()) do
    log_debug("message: {}=>{}", full_name, name)
end

--测试数据
local tpb_data = {
    id=1001014162,
    child = {id=1, name="hot", values = {one="xiluo", two="wergins"} },
    childs = {
        {id=2, name="laker", values = {one="doncici", two="james"} },
        {id=3, name="gsa", values = {one="curry", two="green"} },
    },
    kvs = {
        king = {id=4, name="king", values = {one="drozan", two="foxs"} },
        capper = {id=5, name="capper", values = {one="harden", two="lenarde"} },
    },
    custom={role_id="107216333761938434",name="aaa", gender = "2", model = "3"},
    custom2={1, 2, 3, 888, 666},
    str= "oneof str",
    num= 10,
}

--message编码解码
local tpb_str = pbencode("ncmd_cs.test_message", tpb_data)
log_debug("pb encode: {}", #tpb_str)
local tdata = pbdecode("ncmd_cs.test_message", tpb_str)
log_debug("pb decode:{}", tdata)

--清理pb描述
protobuf.clear()

```
