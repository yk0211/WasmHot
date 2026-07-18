;; WASM 模块接口规范（WAT 文本格式）
;; 该模块需由宿主 WasmEdge 插件加载，接口必须与 C++ 端严格对应。

(module
  ;; 导入 host 模块的 3 个 host 函数
  (import "host" "get_current_object_id"
    (func $host_get_current_object_id (result i64)))

  (import "host" "get_component"
    (func $host_get_component (param i64 i32 i32 i32) (result i32)))

  (import "host" "set_component"
    (func $host_set_component (param i64 i32 i32 i32) (result i32)))

  ;; 导出线性内存，最小 1 页（64KB），足够容纳 input(0~4095) + output(4096~8191)
  (memory (export "memory") 1)

  ;; 辅助函数：获取当前对象 ID（直接调用 host）
  (func $get_current_object_id (result i64)
    call $host_get_current_object_id)

  ;; 辅助函数：读取组件到内存
  ;; param: object_id(i64), type(i32), buf_ptr(i32), buf_cap(i32)
  ;; result: i32 (实际读取长度，负值表示错误)
  (func $get_component (param i64 i32 i32 i32) (result i32)
    local.get 0
    local.get 1
    local.get 2
    local.get 3
    call $host_get_component)

  ;; 辅助函数：从内存写入组件
  ;; param: object_id(i64), type(i32), buf_ptr(i32), len(i32)
  ;; result: i32 (1 表示成功，0 表示失败)
  (func $set_component (param i64 i32 i32 i32) (result i32)
    local.get 0
    local.get 1
    local.get 2
    local.get 3
    call $host_set_component)

  ;; 导出 execute 函数——宿主调用的唯一入口
  ;; param:
  ;;   0: i64  action_hash        (动作名 CityHash64)
  ;;   1: i32  input_offset       (输入数据在内存中的偏移)
  ;;   2: i32  input_size         (输入数据长度)
  ;;   3: i32  output_offset      (输出缓冲区在内存中的偏移)
  ;;   4: i32  output_cap         (输出缓冲区容量)
  ;; result: i32
  ;;   >= 0 : 成功，返回写入 output 区的字节数
  ;;   <  0 : 失败
  (func (export "execute") (param i64 i32 i32 i32 i32) (result i32)
    ;; TODO: 根据 action_hash 分发逻辑
    ;; 示例：
    ;; 1. 调用 $get_current_object_id 获取 object_id
    ;; 2. 调用 $get_component / $set_component 读写组件数据
    ;; 3. 将结果写入 memory + output_offset
    ;; 4. 返回写入长度

    i32.const 0   ;; 返回 0 表示无输出（占位）
  )
)
