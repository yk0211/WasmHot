#include <stdint.h>
#include <string.h>

/* ============================================================
 *  WASM 业务模块模板（C 源码）
 *  编译命令示例（wasi-sdk / clang）：
 *    clang --target=wasm32-unknown-wasi \
 *          -Wl,--export=execute \
 *          -Wl,--export=memory \
 *          -Wl,--import-memory=host,memory \   # 若需从 host 导入 memory
 *          -O2 -o plugin.wasm example.c
 *
 *  注意：
 *  - C++ 端期望 WASM 模块导出名为 "memory" 的线性内存
 *  - C++ 端期望 WASM 模块导出名为 "execute" 的函数
 *  - 输入数据位于 memory[0 .. 4095]（最大 4096 字节）
 *  - 输出缓冲区位于 memory[4096 .. 8191]（最大 4096 字节）
 * ============================================================ */

/* ------------------------------------------------------------
 *  Host 函数导入声明（必须与 C++ 端 CreateHostModule 严格对应）
 * ------------------------------------------------------------ */
__attribute__((import_module("host"),
               import_name("get_current_object_id"))) extern int64_t
host_get_current_object_id(void);

__attribute__((import_module("host"),
               import_name("get_component"))) extern int32_t
host_get_component(int64_t object_id, int32_t type, int32_t buf_ptr,
                   int32_t buf_cap);

__attribute__((import_module("host"),
               import_name("set_component"))) extern int32_t
host_set_component(int64_t object_id, int32_t type, int32_t buf_ptr,
                   int32_t len);

/* ------------------------------------------------------------
 *  导出内存（至少 1 页 = 64KB）
 * ------------------------------------------------------------ */
__attribute__((export_name("memory"))) uint8_t memory[8192];

/* ------------------------------------------------------------
 *  execute —— 宿主调用的唯一入口
 * ------------------------------------------------------------
 * 参数:
 *   action_hash  : 动作名的 64 位 CityHash64 哈希值
 *   input_offset : 输入数据在 memory 中的偏移量
 *   input_size   : 输入数据长度（字节）
 *   output_offset: 输出缓冲区在 memory 中的偏移量
 *   output_cap   : 输出缓冲区容量（字节）
 *
 * 返回:
 *   >= 0 : 成功，返回写入 output 区的字节数
 *   <  0 : 失败
 * ------------------------------------------------------------ */
__attribute__((export_name("execute"))) int32_t execute(int64_t action_hash,
                                                        int32_t input_offset,
                                                        int32_t input_size,
                                                        int32_t output_offset,
                                                        int32_t output_cap) {
  /* 1. 获取当前操作的对象 ID */
  int64_t object_id = host_get_current_object_id();
  (void)object_id; /* 未使用，避免警告 */

  /* 2. 根据 action_hash 分发业务逻辑（示例）
   *
   * 读取组件示例：
   *   int32_t len = host_get_component(object_id, type_id,
   *                                    buffer_offset, buffer_cap);
   *
   * 写入组件示例：
   *   int32_t ok = host_set_component(object_id, type_id,
   *                                   buffer_offset, data_len);
   *
   * 3. 将结果写入 output 区
   *   memcpy(&memory[output_offset], result_data, result_len);
   *   return result_len;
   */

  (void)action_hash;
  (void)input_offset;
  (void)input_size;
  (void)output_offset;
  (void)output_cap;

  return 0; /* 返回 0 表示无输出 */
}
