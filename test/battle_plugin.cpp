// battle_plugin.cpp - 用于编译为 WASM 的测试插件
// 编译命令 (clang):
//   clang --target=wasm32-wasi -O2 -Wl,--no-entry -Wl,--export=execute -fno-exceptions -o battle_plugin.wasm battle_plugin.cpp
// 编译命令 (emcc):
//   emcc -O2 -s WASM=1 -s SIDE_MODULE=1 -s EXPORTED_FUNCTIONS='["_execute"]' -o battle_plugin.wasm battle_plugin.cpp

#include <cstdint>

namespace {

uint32_t djb2(const char* str) {
    uint32_t hash = 5381;
    int c;
    while ((c = *str++)) {
        hash = ((hash << 5) + hash) + c;
    }
    return hash;
}

int write_uint8(uint8_t value, char* out, int32_t cap) {
    if (cap <= 0) return 0;
    if (value == 0) {
        out[0] = '0';
        return 1;
    }
    char buf[4];
    int len = 0;
    while (value > 0 && len < 4) {
        buf[len++] = '0' + (value % 10);
        value /= 10;
    }
    if (len > cap) return 0;
    for (int i = 0; i < len; ++i) {
        out[i] = buf[len - 1 - i];
    }
    return len;
}

} // namespace

extern "C" {

int32_t execute(int32_t action_hash,
                int32_t input_ptr,
                int32_t input_len,
                int32_t output_ptr,
                int32_t output_cap)
{
    if (action_hash != static_cast<int32_t>(djb2("test_action"))) {
        return -1;
    }

    if (input_len < 0) return 0;
    if (output_cap <= 0) return 0;

    const uint8_t* input = reinterpret_cast<const uint8_t*>(input_ptr);
    char* output = reinterpret_cast<char*>(output_ptr);

    int32_t pos = 0;
    const char* prefix = "input: ";
    for (int i = 0; prefix[i] && pos < output_cap; ++i) {
        output[pos++] = prefix[i];
    }

    for (int32_t i = 0; i < input_len && pos < output_cap; ++i) {
        if (i > 0) {
            if (pos < output_cap) output[pos++] = ',';
        }
        int n = write_uint8(input[i], output + pos, output_cap - pos);
        pos += n;
    }

    return pos;
}

}
