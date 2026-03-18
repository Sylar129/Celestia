# Lua C API 参考说明

这份文档根据仓库中的以下头文件整理：

- `lua/lua.h`
- `lua/lauxlib.h`
- `lua/lualib.h`

目标是给公开接口补齐中文说明，方便在不改动原始源码的前提下查阅。文档覆盖：

- 核心 C API
- 辅助库 API
- 标准库入口 API
- 常用宏、常量、伪索引、调试结构

当前仓库内置 Lua 版本为 `5.5.1`。

## 阅读约定

- 栈索引支持正索引和负索引。负索引 `-1` 表示栈顶。
- 除非特别说明，`get` 类接口通常会把结果压入栈顶。
- `set` 类接口通常会消费栈顶的值作为写入内容。
- `raw` 前缀表示跳过元方法。
- `x` / `k` 后缀一般表示“扩展版本”，例如带状态返回、续体或额外参数。
- 宏接口本质上是已有函数的包装，适合简化常见调用。

## 1. 编译期常量、类型与基础约定

### 1.1 版本与标识

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `LUA_COPYRIGHT` | 版权信息字符串。 | 一般用于打印版本信息。 |
| `LUA_AUTHORS` | 作者信息字符串。 | 用于展示，不参与运行时逻辑。 |
| `LUA_VERSION_MAJOR_N` | 主版本号整数。 | 当前为 `5`。 |
| `LUA_VERSION_MINOR_N` | 次版本号整数。 | 当前为 `5`。 |
| `LUA_VERSION_RELEASE_N` | 修订版本号整数。 | 当前为 `1`。 |
| `LUA_VERSION_NUM` | 组合后的版本号整数。 | 例如 `505`。 |
| `LUA_VERSION_RELEASE_NUM` | 更细粒度的版本号整数。 | 例如 `50501`。 |
| `LUA_VERSION_MAJOR` | 主版本号字符串。 | 由宏展开得到。 |
| `LUA_VERSION_MINOR` | 次版本号字符串。 | 由宏展开得到。 |
| `LUA_VERSION_RELEASE` | 修订版本号字符串。 | 由宏展开得到。 |
| `LUA_VERSION` | 版本字符串。 | 例如 `Lua 5.5`。 |
| `LUA_RELEASE` | 完整发布字符串。 | 例如 `Lua 5.5.1`。 |
| `lua_ident` | Lua 标识字符串。 | `extern const char[]`，可用于诊断或展示。 |

### 1.2 类型标记

| 常量 | 说明 |
| --- | --- |
| `LUA_TNONE` | 非法索引或不存在的值。 |
| `LUA_TNIL` | `nil`。 |
| `LUA_TBOOLEAN` | 布尔值。 |
| `LUA_TLIGHTUSERDATA` | 轻量用户数据，本质是裸指针。 |
| `LUA_TNUMBER` | 数值。 |
| `LUA_TSTRING` | 字符串。 |
| `LUA_TTABLE` | 表。 |
| `LUA_TFUNCTION` | 函数。 |
| `LUA_TUSERDATA` | 完整用户数据。 |
| `LUA_TTHREAD` | 线程 / 协程。 |
| `LUA_NUMTYPES` | 类型总数。 |

### 1.3 状态码

| 常量 | 说明 |
| --- | --- |
| `LUA_OK` | 成功。 |
| `LUA_YIELD` | 协程被挂起。 |
| `LUA_ERRRUN` | 运行期错误。 |
| `LUA_ERRSYNTAX` | 语法错误。 |
| `LUA_ERRMEM` | 内存分配失败。 |
| `LUA_ERRERR` | 错误处理函数自身出错。 |
| `LUA_ERRFILE` | 文件加载错误。定义于 `lauxlib.h`。 |

### 1.4 伪索引与注册表索引

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `LUA_REGISTRYINDEX` | 注册表伪索引。 | 用来访问注册表。 |
| `lua_upvalueindex(i)` | 第 `i` 个上值的伪索引。 | 常用于 C 闭包读取上值。 |
| `LUA_RIDX_GLOBALS` | 注册表中的全局表索引。 | 可配合 `lua_rawgeti` 取得全局表。 |
| `LUA_RIDX_MAINTHREAD` | 注册表中的主线程索引。 | 常用于跨协程判断。 |
| `LUA_RIDX_LAST` | 当前预定义注册表键的最大值。 | 便于保留自定义范围。 |

### 1.5 其他常量

| 接口 | 说明 |
| --- | --- |
| `LUA_SIGNATURE` | 预编译块签名，内容为 `"\x1bLua"`。 |
| `LUA_MULTRET` | 表示“返回所有结果”。常见于 `lua_call` / `lua_pcall`。 |
| `LUA_MINSTACK` | Lua 保证为 C 函数预留的最小额外栈空间。 |
| `LUA_N2SBUFFSZ` | `lua_numbertocstring` 推荐使用的最小缓冲区大小。 |

### 1.6 公共类型

| 类型 | 说明 |
| --- | --- |
| `lua_State` | Lua 状态机对象。所有 API 都围绕它操作。 |
| `lua_Number` | Lua 浮点数类型。具体定义受 `luaconf.h` 影响。 |
| `lua_Integer` | Lua 整数类型。 |
| `lua_Unsigned` | Lua 无符号整数类型。 |
| `lua_KContext` | 续体上下文值类型。 |
| `lua_CFunction` | C 函数签名：`int (*)(lua_State *L)`。返回值为返回结果个数。 |
| `lua_KFunction` | 续体回调签名。用于 `lua_callk` / `lua_pcallk` / `lua_yieldk`。 |
| `lua_Reader` | 加载代码块时的读取回调。 |
| `lua_Writer` | 导出字节码时的写入回调。 |
| `lua_Alloc` | 自定义内存分配器签名。 |
| `lua_WarnFunction` | 警告回调签名。 |
| `lua_Hook` | 调试钩子回调签名。 |
| `lua_Debug` | 调试信息结构体。 |
| `luaL_Reg` | `name + CFunction` 的注册表项。常用于批量注册库函数。 |
| `luaL_Buffer` | 构造字符串结果时使用的辅助缓冲区。 |
| `luaL_Stream` | 标准 IO 库文件句柄的起始结构。 |

## 2. `lua.h` 核心 C API

### 2.1 状态管理

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_newstate(f, ud, seed)` | 创建新的 Lua 状态机。 | 需要提供分配器；`seed` 用于随机化相关内部行为。 |
| `lua_close(L)` | 关闭状态机并释放全部资源。 | 仅对主状态调用；关闭后 `L` 不再可用。 |
| `lua_newthread(L)` | 在当前状态中创建新协程。 | 新线程与主状态共享全局状态和 GC 堆。 |
| `lua_closethread(L, from)` | 关闭一个线程。 | Lua 5.5 新接口；常用于显式结束协程并清理待关闭变量。 |
| `lua_atpanic(L, panicf)` | 设置恐慌处理函数。 | 返回旧的 panic 函数。发生不可恢复错误时调用。 |
| `lua_version(L)` | 获取 Lua 版本号。 | 返回 `lua_Number` 指向的版本值语义。常用于 ABI 检查。 |

### 2.2 栈操作

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_absindex(L, idx)` | 把相对索引转换成绝对索引。 | 在后续压栈/弹栈前固定位置很有用。 |
| `lua_gettop(L)` | 返回当前栈顶索引。 | 也可视为当前元素个数。 |
| `lua_settop(L, idx)` | 把栈调整到指定顶端。 | 可用于截断栈或补 `nil`。 |
| `lua_pushvalue(L, idx)` | 复制指定索引处的值到栈顶。 | 不会移除原值。 |
| `lua_rotate(L, idx, n)` | 旋转从 `idx` 到栈顶这段区间。 | 插入、删除、交换局部区间时很常用。 |
| `lua_copy(L, fromidx, toidx)` | 把一个位置的值复制到另一个位置。 | 不会弹栈；目标位置会被覆盖。 |
| `lua_checkstack(L, n)` | 确保至少还能再压入 `n` 个值。 | 成功返回非零。 |
| `lua_xmove(from, to, n)` | 在两个线程之间移动 `n` 个值。 | 两个线程必须属于同一个全局状态。 |

### 2.3 值检查与转换

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_isnumber(L, idx)` | 判断值是否是数字或可转换为数字。 | 字符串数字也可能返回真。 |
| `lua_isstring(L, idx)` | 判断值是否是字符串或数字。 | 因为数字可转换为字符串。 |
| `lua_iscfunction(L, idx)` | 判断值是否是 C 函数。 | 对 Lua 函数返回假。 |
| `lua_isinteger(L, idx)` | 判断值是否是整数。 | 仅对数值且内部为整数时成立。 |
| `lua_isuserdata(L, idx)` | 判断值是否是完整或轻量用户数据。 | 两种 userdata 都会返回真。 |
| `lua_type(L, idx)` | 返回值类型标记。 | 不存在则返回 `LUA_TNONE`。 |
| `lua_typename(L, tp)` | 把类型标记转成可读字符串。 | 适合错误信息输出。 |
| `lua_tonumberx(L, idx, isnum)` | 转成 `lua_Number`。 | `isnum` 可选，用于区分失败与 `0`。 |
| `lua_tointegerx(L, idx, isnum)` | 转成 `lua_Integer`。 | 同上，适合精确整数读取。 |
| `lua_toboolean(L, idx)` | 转成布尔值。 | 只有 `false` 和 `nil` 为假。 |
| `lua_tolstring(L, idx, len)` | 转成字符串并返回指针。 | 可能触发转换并修改该栈位的表示。 |
| `lua_rawlen(L, idx)` | 取原始长度。 | 不调用 `__len`；适用于字符串、表、userdata。 |
| `lua_tocfunction(L, idx)` | 取出 C 函数指针。 | 不是 C 函数则返回 `NULL`。 |
| `lua_touserdata(L, idx)` | 取出 userdata 地址。 | 对 light/full userdata 语义不同，但都可取指针。 |
| `lua_tothread(L, idx)` | 取出线程对象。 | 不是线程则返回 `NULL`。 |
| `lua_topointer(L, idx)` | 取对象的稳定标识指针。 | 仅用于比较/日志，不应用于解引用。 |

### 2.4 比较与算术

#### 比较操作码

| 常量 | 说明 |
| --- | --- |
| `LUA_OPEQ` | 相等比较。 |
| `LUA_OPLT` | 小于比较。 |
| `LUA_OPLE` | 小于等于比较。 |

#### 算术操作码

| 常量 | 说明 |
| --- | --- |
| `LUA_OPADD` | 加法。 |
| `LUA_OPSUB` | 减法。 |
| `LUA_OPMUL` | 乘法。 |
| `LUA_OPMOD` | 取模。 |
| `LUA_OPPOW` | 幂运算。 |
| `LUA_OPDIV` | 浮点除法。 |
| `LUA_OPIDIV` | 整除。 |
| `LUA_OPBAND` | 按位与。 |
| `LUA_OPBOR` | 按位或。 |
| `LUA_OPBXOR` | 按位异或。 |
| `LUA_OPSHL` | 左移。 |
| `LUA_OPSHR` | 右移。 |
| `LUA_OPUNM` | 一元负号。 |
| `LUA_OPBNOT` | 按位取反。 |

#### 比较与算术函数

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_arith(L, op)` | 按指定操作码执行算术或位运算。 | 一元运算消耗 1 个值，二元运算消耗 2 个值并压回结果。 |
| `lua_rawequal(L, idx1, idx2)` | 原始相等比较。 | 不触发 `__eq`。 |
| `lua_compare(L, idx1, idx2, op)` | 按 `LUA_OPEQ/LUA_OPLT/LUA_OPLE` 比较两个值。 | 可能触发元方法。 |

### 2.5 压栈接口

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_pushnil(L)` | 压入 `nil`。 | 最基础的占位值。 |
| `lua_pushnumber(L, n)` | 压入浮点数。 | 使用 `lua_Number`。 |
| `lua_pushinteger(L, n)` | 压入整数。 | 使用 `lua_Integer`。 |
| `lua_pushlstring(L, s, len)` | 压入指定长度的字符串。 | 允许中间含 `\0`。 |
| `lua_pushexternalstring(L, s, len, falloc, ud)` | 压入外部管理的字符串。 | Lua 5.5 新接口；字符串存储生命周期由外部分配器约定辅助管理。 |
| `lua_pushstring(L, s)` | 压入以 `\0` 结尾的 C 字符串。 | `s == NULL` 时通常等价于压入 `nil`。 |
| `lua_pushvfstring(L, fmt, argp)` | 按格式化模板压入字符串。 | `va_list` 版本。 |
| `lua_pushfstring(L, fmt, ...)` | 按格式化模板压入字符串。 | 可直接用于拼错误消息。 |
| `lua_pushcclosure(L, fn, n)` | 压入带 `n` 个上值的 C 闭包。 | 上值需预先压栈。 |
| `lua_pushboolean(L, b)` | 压入布尔值。 | 非 0 为真。 |
| `lua_pushlightuserdata(L, p)` | 压入轻量用户数据。 | 仅保存指针，不受 GC 管理。 |
| `lua_pushthread(L)` | 压入当前线程对象。 | 返回值表示当前线程是否为主线程。 |

### 2.6 取值接口

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_getglobal(L, name)` | 读取全局变量并压栈。 | 返回压入值的类型。 |
| `lua_gettable(L, idx)` | 以栈顶键访问表并压入结果。 | 会触发 `__index`；通常消费键。 |
| `lua_getfield(L, idx, k)` | 以字符串字段名读取表。 | `k` 不需要先压栈。 |
| `lua_geti(L, idx, n)` | 以整数键读取表。 | 常用于数组风格访问。 |
| `lua_rawget(L, idx)` | 原始读取表项。 | 不触发元方法。 |
| `lua_rawgeti(L, idx, n)` | 原始读取整数键。 | 适合性能敏感路径。 |
| `lua_rawgetp(L, idx, p)` | 以指针为键进行原始读取。 | 常用于注册表私有键。 |
| `lua_createtable(L, narr, nrec)` | 创建新表并压栈。 | 预估数组/哈希容量可减少扩容。 |
| `lua_newuserdatauv(L, sz, nuvalue)` | 创建完整用户数据并压栈。 | 可为 userdata 预留 `nuvalue` 个关联值。 |
| `lua_getmetatable(L, objindex)` | 取得对象元表并压栈。 | 有元表时返回非零。 |
| `lua_getiuservalue(L, idx, n)` | 取得 userdata 的第 `n` 个关联值。 | 成功会把值压栈，并返回其类型。 |

### 2.7 设值接口

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_setglobal(L, name)` | 设置全局变量。 | 消费栈顶值。 |
| `lua_settable(L, idx)` | 按键设置表项。 | 期望栈顶为 value，其下为 key。 |
| `lua_setfield(L, idx, k)` | 设置字符串字段。 | 只消费栈顶 value。 |
| `lua_seti(L, idx, n)` | 设置整数键。 | 常用于数组下标写入。 |
| `lua_rawset(L, idx)` | 原始设置表项。 | 不触发元方法。 |
| `lua_rawseti(L, idx, n)` | 原始设置整数键。 | 适合性能场景。 |
| `lua_rawsetp(L, idx, p)` | 以指针键进行原始设置。 | 适合放入注册表私有表。 |
| `lua_setmetatable(L, objindex)` | 设置对象元表。 | 消费栈顶元表，成功返回非零。 |
| `lua_setiuservalue(L, idx, n)` | 设置 userdata 的第 `n` 个关联值。 | 消费栈顶值。 |

### 2.8 调用、加载与导出

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_callk(L, nargs, nresults, ctx, k)` | 调用函数，支持续体。 | 被调函数和参数必须已按协议压栈。 |
| `lua_call(L, n, r)` | `lua_callk` 的简化宏。 | 不支持续体。 |
| `lua_pcallk(L, nargs, nresults, errfunc, ctx, k)` | 保护模式调用。 | 出错时返回状态码而非长跳转到 C。 |
| `lua_pcall(L, n, r, f)` | `lua_pcallk` 的简化宏。 | `f` 是错误处理函数索引。 |
| `lua_load(L, reader, dt, chunkname, mode)` | 通过回调加载源码或字节码。 | 成功后把编译后的函数压栈。 |
| `lua_dump(L, writer, data, strip)` | 导出函数为预编译块。 | `strip != 0` 时剥离调试信息。 |

### 2.9 协程接口

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_yieldk(L, nresults, ctx, k)` | 挂起当前协程。 | 仅可在可挂起上下文中调用。 |
| `lua_yield(L, n)` | `lua_yieldk` 的简化宏。 | 不带续体。 |
| `lua_resume(L, from, narg, nres)` | 恢复协程运行。 | `narg` 为传入参数数；`nres` 输出结果个数。 |
| `lua_status(L)` | 获取线程状态。 | 常见值包括 `LUA_OK`、`LUA_YIELD` 等。 |
| `lua_isyieldable(L)` | 检查当前线程是否允许挂起。 | 在 C 调用链里尤其有用。 |

### 2.10 警告接口

| 接口 | 说明 |
| --- | --- |
| `lua_setwarnf(L, f, ud)` | 设置警告回调及其用户数据。 |
| `lua_warning(L, msg, tocont)` | 发送警告消息。`tocont` 指示是否还有后续片段。 |

### 2.11 垃圾回收接口

#### GC 操作码

| 常量 | 说明 |
| --- | --- |
| `LUA_GCSTOP` | 停止 GC。 |
| `LUA_GCRESTART` | 重新启动 GC。 |
| `LUA_GCCOLLECT` | 完整执行一次 GC。 |
| `LUA_GCCOUNT` | 取已使用内存的 K 字节数。 |
| `LUA_GCCOUNTB` | 取已使用内存的余数字节。 |
| `LUA_GCSTEP` | 进行一次增量步骤。 |
| `LUA_GCISRUNNING` | 查询 GC 是否启用。 |
| `LUA_GCGEN` | 切换到分代模式。 |
| `LUA_GCINC` | 切换到增量模式。 |
| `LUA_GCPARAM` | 设置或获取 GC 参数。 |

#### GC 参数码

| 常量 | 说明 |
| --- | --- |
| `LUA_GCPMINORMUL` | 分代模式下控制 minor collection 频率。 |
| `LUA_GCPMAJORMINOR` | 控制 major 到 minor 的切换阈值。 |
| `LUA_GCPMINORMAJOR` | 控制 minor 到 major 的切换阈值。 |
| `LUA_GCPPAUSE` | 增量模式下的暂停参数。 |
| `LUA_GCPSTEPMUL` | 增量模式下的步进倍率。 |
| `LUA_GCPSTEPSIZE` | 增量模式下的单步粒度。 |
| `LUA_GCPN` | GC 参数总数。 |

#### GC 函数

| 接口 | 说明 |
| --- | --- |
| `lua_gc(L, what, ...)` | GC 总控接口。根据 `what` 执行查询、切换或调参。 |

### 2.12 其他常用接口

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_error(L)` | 抛出错误。 | 使用栈顶对象作为错误值，通常不会返回。 |
| `lua_next(L, idx)` | 遍历表中的下一个键值对。 | 遍历前先压入 `nil` 作为首键。 |
| `lua_concat(L, n)` | 拼接栈顶 `n` 个值。 | 结果压回栈顶。 |
| `lua_len(L, idx)` | 计算长度并压栈。 | 会触发 `__len`。 |
| `lua_numbertocstring(L, idx, buff)` | 把数字转换到 C 缓冲区。 | Lua 5.5 新接口；缓冲区建议至少 `LUA_N2SBUFFSZ`。 |
| `lua_stringtonumber(L, s)` | 把 C 字符串解析为 Lua 数字并压栈。 | 成功返回已消费字符数。 |
| `lua_getallocf(L, ud)` | 获取当前分配器和用户数据。 | 常用于接管或包装分配器。 |
| `lua_setallocf(L, f, ud)` | 设置当前分配器。 | 需要保证实现与 Lua 内部约定兼容。 |
| `lua_toclose(L, idx)` | 将栈位标记为待关闭值。 | 与 `__close` 元方法配合。 |
| `lua_closeslot(L, idx)` | 关闭指定栈位。 | Lua 5.5 新接口；显式触发关闭逻辑。 |

### 2.13 调试 API

#### 调试事件与掩码

| 常量 | 说明 |
| --- | --- |
| `LUA_HOOKCALL` | 调用事件。 |
| `LUA_HOOKRET` | 返回事件。 |
| `LUA_HOOKLINE` | 行事件。 |
| `LUA_HOOKCOUNT` | 指令计数事件。 |
| `LUA_HOOKTAILCALL` | 尾调用事件。 |
| `LUA_MASKCALL` | 调用事件掩码。 |
| `LUA_MASKRET` | 返回事件掩码。 |
| `LUA_MASKLINE` | 行事件掩码。 |
| `LUA_MASKCOUNT` | 指令计数事件掩码。 |

#### 调试函数

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `lua_getstack(L, level, ar)` | 获取某一调用层级的激活记录。 | 成功返回非零。 |
| `lua_getinfo(L, what, ar)` | 按 `what` 指定的信息掩码提取调试信息。 | 可获取源码、行号、参数、函数对象等。 |
| `lua_getlocal(L, ar, n)` | 获取第 `n` 个局部变量并压栈。 | 返回局部变量名。 |
| `lua_setlocal(L, ar, n)` | 设置第 `n` 个局部变量。 | 消费栈顶值，返回变量名。 |
| `lua_getupvalue(L, funcindex, n)` | 获取函数第 `n` 个上值并压栈。 | 返回上值名。 |
| `lua_setupvalue(L, funcindex, n)` | 设置函数第 `n` 个上值。 | 消费栈顶值。 |
| `lua_upvalueid(L, fidx, n)` | 获取上值身份标识。 | 可判断两个闭包是否共享同一上值。 |
| `lua_upvaluejoin(L, fidx1, n1, fidx2, n2)` | 让两个闭包共享同一个上值。 | 属于高级调试/元编程接口。 |
| `lua_sethook(L, func, mask, count)` | 设置调试钩子。 | `count` 用于计数钩子。 |
| `lua_gethook(L)` | 获取当前钩子函数。 | 返回 `lua_Hook`。 |
| `lua_gethookmask(L)` | 获取当前钩子掩码。 | 与 `LUA_MASK*` 组合。 |
| `lua_gethookcount(L)` | 获取当前钩子计数步长。 | 常与 `LUA_MASKCOUNT` 配合。 |

#### `lua_Debug` 字段说明

| 字段 | 说明 |
| --- | --- |
| `event` | 当前触发的调试事件。 |
| `name` | 函数名。 |
| `namewhat` | 名称来源，如 `global`、`local`、`field`、`method`。 |
| `what` | 函数类别，如 `Lua`、`C`、`main`、`tail`。 |
| `source` | 原始源码标识。 |
| `srclen` | `source` 长度。 |
| `currentline` | 当前执行到的行号。 |
| `linedefined` | 函数起始行。 |
| `lastlinedefined` | 函数结束行。 |
| `nups` | 上值数量。 |
| `nparams` | 参数数量。 |
| `isvararg` | 是否可变参数。 |
| `extraargs` | 额外参数数量。 |
| `istailcall` | 是否处于尾调用。 |
| `ftransfer` | 传输值的起始索引。 |
| `ntransfer` | 传输值的数量。 |
| `short_src` | 截短后的源码说明。 |

### 2.14 `lua.h` 便捷宏与兼容宏

| 接口 | 说明 | 等价或用途 |
| --- | --- | --- |
| `lua_getextraspace(L)` | 取得 `lua_State` 前置附加空间。 | 适合存放宿主侧轻量上下文。 |
| `lua_tonumber(L, i)` | 简化版数字转换。 | 等价于 `lua_tonumberx(L, i, NULL)`。 |
| `lua_tointeger(L, i)` | 简化版整数转换。 | 等价于 `lua_tointegerx(L, i, NULL)`。 |
| `lua_pop(L, n)` | 弹出 `n` 个值。 | 等价于 `lua_settop(L, -(n) - 1)`。 |
| `lua_newtable(L)` | 创建空表。 | 等价于 `lua_createtable(L, 0, 0)`。 |
| `lua_register(L, n, f)` | 注册全局 C 函数。 | 先压函数，再 `lua_setglobal`。 |
| `lua_pushcfunction(L, f)` | 压入无上值的 C 函数。 | 等价于 `lua_pushcclosure(L, f, 0)`。 |
| `lua_isfunction(L, n)` | 检查是否为函数。 | `lua_type(...) == LUA_TFUNCTION`。 |
| `lua_istable(L, n)` | 检查是否为表。 | 类型判定宏。 |
| `lua_islightuserdata(L, n)` | 检查是否为轻量用户数据。 | 类型判定宏。 |
| `lua_isnil(L, n)` | 检查是否为 `nil`。 | 类型判定宏。 |
| `lua_isboolean(L, n)` | 检查是否为布尔值。 | 类型判定宏。 |
| `lua_isthread(L, n)` | 检查是否为线程。 | 类型判定宏。 |
| `lua_isnone(L, n)` | 检查索引是否无效。 | `lua_type(...) == LUA_TNONE`。 |
| `lua_isnoneornil(L, n)` | 检查值不存在或为 `nil`。 | 对可选参数很实用。 |
| `lua_pushliteral(L, s)` | 压入字面量字符串。 | 通过 `"" s` 保证编译期字符串。 |
| `lua_pushglobaltable(L)` | 把全局表压栈。 | 从注册表读取 `LUA_RIDX_GLOBALS`。 |
| `lua_tostring(L, i)` | 简化版字符串转换。 | 等价于 `lua_tolstring(L, i, NULL)`。 |
| `lua_insert(L, idx)` | 把栈顶元素插入到 `idx`。 | 基于 `lua_rotate`。 |
| `lua_remove(L, idx)` | 删除 `idx` 处元素。 | 基于 `lua_rotate` + `lua_pop`。 |
| `lua_replace(L, idx)` | 用栈顶替换 `idx`。 | 基于 `lua_copy` + `lua_pop`。 |
| `lua_newuserdata(L, s)` | 兼容版创建 userdata。 | 等价于 `lua_newuserdatauv(L, s, 1)`。 |
| `lua_getuservalue(L, idx)` | 兼容版读取第 1 个 userdata 关联值。 | 等价于 `lua_getiuservalue(L, idx, 1)`。 |
| `lua_setuservalue(L, idx)` | 兼容版设置第 1 个 userdata 关联值。 | 等价于 `lua_setiuservalue(L, idx, 1)`。 |
| `lua_resetthread(L)` | 兼容版重置线程。 | 等价于 `lua_closethread(L, NULL)`。 |

## 3. `lauxlib.h` 辅助库 API

### 3.1 辅助库常量与注册表键

| 接口 | 说明 |
| --- | --- |
| `LUA_GNAME` | 全局表默认名称，值为 `_G`。 |
| `LUA_LOADED_TABLE` | 注册表中的已加载模块表键名。 |
| `LUA_PRELOAD_TABLE` | 注册表中的预加载模块表键名。 |
| `LUAL_NUMSIZES` | 用于校验数值 ABI 的尺寸组合常量。 |
| `LUA_NOREF` | 无效引用。 |
| `LUA_REFNIL` | 对应 `nil` 的引用。 |
| `LUA_FILEHANDLE` | 标准文件句柄元表名。 |

### 3.2 参数检查、错误与元表辅助

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `luaL_checkversion_(L, ver, sz)` | 检查运行时 Lua 版本和数值 ABI。 | 嵌入式宿主初始化时很重要。 |
| `luaL_checkversion(L)` | `luaL_checkversion_` 的宏包装。 | 使用当前编译版本常量。 |
| `luaL_getmetafield(L, obj, e)` | 获取对象元表中的字段。 | 成功时把字段值压栈。 |
| `luaL_callmeta(L, obj, e)` | 调用对象元表中的方法。 | 成功返回非零。 |
| `luaL_tolstring(L, idx, len)` | 生成可读字符串。 | 等价于 Lua 层 `tostring` 语义。 |
| `luaL_argerror(L, arg, extramsg)` | 抛出参数错误。 | 一般不会返回。 |
| `luaL_typeerror(L, arg, tname)` | 抛出类型不匹配错误。 | 可用于自定义校验。 |
| `luaL_checklstring(L, arg, l)` | 读取必选字符串参数。 | 失败会报错。 |
| `luaL_optlstring(L, arg, def, l)` | 读取可选字符串参数。 | 参数缺失或为 `nil` 时返回默认值。 |
| `luaL_checknumber(L, arg)` | 读取必选数字参数。 | 类型不符时报错。 |
| `luaL_optnumber(L, arg, def)` | 读取可选数字参数。 | 无值时用默认值。 |
| `luaL_checkinteger(L, arg)` | 读取必选整数参数。 | 要求可转换为整数。 |
| `luaL_optinteger(L, arg, def)` | 读取可选整数参数。 | 无值时用默认值。 |
| `luaL_checkstack(L, sz, msg)` | 确保有足够栈空间。 | 失败会抛错，不返回状态码。 |
| `luaL_checktype(L, arg, t)` | 检查参数类型。 | 不是指定类型就报错。 |
| `luaL_checkany(L, arg)` | 检查参数是否存在。 | 适合必选但类型不限的参数。 |
| `luaL_newmetatable(L, tname)` | 按名称创建或获取元表。 | 首次创建返回非零。 |
| `luaL_setmetatable(L, tname)` | 给栈顶对象设置命名元表。 | 元表必须已在注册表里。 |
| `luaL_testudata(L, ud, tname)` | 尝试把 userdata 视为指定类型。 | 不匹配时返回 `NULL`。 |
| `luaL_checkudata(L, ud, tname)` | 强制检查 userdata 类型。 | 不匹配时报错。 |
| `luaL_where(L, lvl)` | 把错误位置前缀压栈。 | 常用于自定义错误消息。 |
| `luaL_error(L, fmt, ...)` | 抛出格式化错误。 | 常用于 C 函数返回错误。 |
| `luaL_checkoption(L, arg, def, lst)` | 检查字符串参数是否属于候选列表。 | 返回命中的列表索引。 |
| `luaL_fileresult(L, stat, fname)` | 把文件操作结果转换为 Lua 返回值。 | 常见返回是 `true` 或 `nil,error,code`。 |
| `luaL_execresult(L, stat)` | 把 `system` / 子进程结果转换为 Lua 返回值。 | 适合实现 `os.execute` 风格接口。 |
| `luaL_alloc(ud, ptr, osize, nsize)` | 辅助库默认分配器。 | 常用于构造 `luaL_newstate`。 |

### 3.3 引用、加载与状态创建

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `luaL_ref(L, t)` | 在表中创建整型引用。 | 通常用于注册表保活对象。 |
| `luaL_unref(L, t, ref)` | 释放此前创建的引用。 | 避免注册表泄漏。 |
| `luaL_loadfilex(L, filename, mode)` | 加载文件为 Lua chunk。 | 成功时把函数压栈。 |
| `luaL_loadfile(L, f)` | `luaL_loadfilex` 简化宏。 | `mode == NULL`。 |
| `luaL_loadbufferx(L, buff, sz, name, mode)` | 从内存缓冲区加载 chunk。 | 适合嵌入脚本文本。 |
| `luaL_loadbuffer(L, s, sz, n)` | `luaL_loadbufferx` 简化宏。 | `mode == NULL`。 |
| `luaL_loadstring(L, s)` | 从 C 字符串加载 chunk。 | 常用于测试和小脚本。 |
| `luaL_newstate(void)` | 创建带默认分配器的新状态。 | 是最常见的初始化入口。 |
| `luaL_makeseed(L)` | 生成随机种子。 | 可辅助宿主自定义初始化。 |
| `luaL_len(L, idx)` | 读取长度并转为 `lua_Integer`。 | 比 `lua_len` 更便于 C 侧直接取值。 |

### 3.4 字符串与模块辅助

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `luaL_addgsub(b, s, p, r)` | 向缓冲区追加替换结果。 | 把 `s` 中的 `p` 替换为 `r`。 |
| `luaL_gsub(L, s, p, r)` | 返回替换后的新字符串。 | 结果会压栈。 |
| `luaL_setfuncs(L, l, nup)` | 批量注册 C 函数表。 | 每个函数共享栈顶的 `nup` 个上值。 |
| `luaL_getsubtable(L, idx, fname)` | 取得或创建子表。 | 成功后把子表压栈。 |
| `luaL_traceback(L, L1, msg, level)` | 生成回溯字符串。 | 常用于错误包装。 |
| `luaL_requiref(L, modname, openf, glb)` | 按模块方式加载 C 库。 | 可选择是否同步写入全局表。 |

### 3.5 `lauxlib.h` 常用宏

| 接口 | 说明 | 等价或用途 |
| --- | --- | --- |
| `luaL_newlibtable(L, l)` | 预分配合适大小的新库表。 | 按 `luaL_Reg` 数组大小估计容量。 |
| `luaL_newlib(L, l)` | 创建新库表并批量注册函数。 | 常用于 `luaopen_xxx`。 |
| `luaL_argcheck(L, cond, arg, extramsg)` | 条件不满足时报参数错误。 | 轻量断言宏。 |
| `luaL_argexpected(L, cond, arg, tname)` | 条件不满足时报类型错误。 | 更偏向“期望某类型”的表达。 |
| `luaL_checkstring(L, n)` | 简化版字符串必选参数读取。 | 等价于 `luaL_checklstring(..., NULL)`。 |
| `luaL_optstring(L, n, d)` | 简化版字符串可选参数读取。 | 等价于 `luaL_optlstring(..., NULL)`。 |
| `luaL_typename(L, i)` | 返回指定栈位的类型名。 | 组合 `lua_type` 与 `lua_typename`。 |
| `luaL_dofile(L, fn)` | 加载并执行文件。 | 相当于 `loadfile + pcall`。 |
| `luaL_dostring(L, s)` | 加载并执行字符串。 | 相当于 `loadstring + pcall`。 |
| `luaL_getmetatable(L, n)` | 从注册表按名字取元表。 | 本质调用 `lua_getfield(L, LUA_REGISTRYINDEX, n)`。 |
| `luaL_opt(L, f, n, d)` | 统一的“可选参数或默认值”包装。 | 参数缺失时直接取默认值。 |
| `luaL_intop(op, v1, v2)` | 按 Lua 规则执行整数位级运算。 | 用无符号中间值实现环绕语义。 |
| `luaL_pushfail(L)` | 压入失败值。 | 取决于配置，可能是 `nil` 或 `false`。 |

### 3.6 缓冲区接口 `luaL_Buffer`

#### 结构字段

| 字段 | 说明 |
| --- | --- |
| `b` | 当前缓冲区地址。 |
| `size` | 当前缓冲区容量。 |
| `n` | 已写入字节数。 |
| `L` | 关联的 Lua 状态。 |
| `init` | 内置初始缓冲区联合体。 |

#### 缓冲区宏与函数

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `luaL_bufflen(bf)` | 获取当前缓冲区长度。 | 直接返回 `n`。 |
| `luaL_buffaddr(bf)` | 获取当前缓冲区地址。 | 适合只读查看。 |
| `luaL_addchar(B, c)` | 追加一个字符。 | 必要时自动扩容。 |
| `luaL_addsize(B, s)` | 手动增加已写入长度。 | 常配合手工写缓冲区使用。 |
| `luaL_buffsub(B, s)` | 回退缓冲区长度。 | 用于撤销尾部字节。 |
| `luaL_buffinit(L, B)` | 初始化缓冲区。 | 开始构造字符串前先调用。 |
| `luaL_prepbuffsize(B, sz)` | 预留 `sz` 字节可写空间。 | 返回可直接写入的地址。 |
| `luaL_addlstring(B, s, l)` | 追加定长字符串。 | 支持二进制数据。 |
| `luaL_addstring(B, s)` | 追加 C 字符串。 | 以 `\0` 结尾。 |
| `luaL_addvalue(B)` | 追加栈顶字符串值。 | 会消费栈顶值。 |
| `luaL_pushresult(B)` | 把缓冲区内容作为 Lua 字符串压栈。 | 结束构造流程。 |
| `luaL_pushresultsize(B, sz)` | 以指定长度提交结果。 | 适合手工写入后一次性完成。 |
| `luaL_buffinitsize(L, B, sz)` | 初始化并一次性预留空间。 | 返回可写指针。 |
| `luaL_prepbuffer(B)` | 预留默认缓冲区大小。 | 等价于 `luaL_prepbuffsize(B, LUAL_BUFFERSIZE)`。 |

### 3.7 `luaL_Stream` 与兼容宏

| 接口 | 说明 |
| --- | --- |
| `luaL_Stream::f` | 底层 `FILE*`。`NULL` 表示未完成创建。 |
| `luaL_Stream::closef` | 关闭函数。`NULL` 表示已关闭。 |
| `luaL_checkunsigned` | 把参数读为无符号整数。仅在兼容宏开启时提供。 |
| `luaL_optunsigned` | 读取可选无符号整数。 |
| `luaL_checkint` | 读取为 `int`。 |
| `luaL_optint` | 读取可选 `int`。 |
| `luaL_checklong` | 读取为 `long`。 |
| `luaL_optlong` | 读取可选 `long`。 |

## 4. `lualib.h` 标准库入口

### 4.1 标准库名称与加载位

| 接口 | 说明 |
| --- | --- |
| `LUA_VERSUFFIX` | 环境变量版本后缀，例如 `_5_5`。 |
| `LUA_GLIBK` | `base` 库加载位。 |
| `LUA_LOADLIBK` | `package` 库加载位。 |
| `LUA_COLIBK` | `coroutine` 库加载位。 |
| `LUA_DBLIBK` | `debug` 库加载位。 |
| `LUA_IOLIBK` | `io` 库加载位。 |
| `LUA_MATHLIBK` | `math` 库加载位。 |
| `LUA_OSLIBK` | `os` 库加载位。 |
| `LUA_STRLIBK` | `string` 库加载位。 |
| `LUA_TABLIBK` | `table` 库加载位。 |
| `LUA_UTF8LIBK` | `utf8` 库加载位。 |
| `LUA_LOADLIBNAME` | `package` 库名。 |
| `LUA_COLIBNAME` | `coroutine` 库名。 |
| `LUA_DBLIBNAME` | `debug` 库名。 |
| `LUA_IOLIBNAME` | `io` 库名。 |
| `LUA_MATHLIBNAME` | `math` 库名。 |
| `LUA_OSLIBNAME` | `os` 库名。 |
| `LUA_STRLIBNAME` | `string` 库名。 |
| `LUA_TABLIBNAME` | `table` 库名。 |
| `LUA_UTF8LIBNAME` | `utf8` 库名。 |

### 4.2 标准库打开函数

| 接口 | 说明 |
| --- | --- |
| `luaopen_base(L)` | 打开基础库。 |
| `luaopen_package(L)` | 打开包管理库。 |
| `luaopen_coroutine(L)` | 打开协程库。 |
| `luaopen_debug(L)` | 打开调试库。 |
| `luaopen_io(L)` | 打开 IO 库。 |
| `luaopen_math(L)` | 打开数学库。 |
| `luaopen_os(L)` | 打开操作系统库。 |
| `luaopen_string(L)` | 打开字符串库。 |
| `luaopen_table(L)` | 打开表库。 |
| `luaopen_utf8(L)` | 打开 UTF-8 库。 |

### 4.3 批量打开标准库

| 接口 | 说明 | 关键点 |
| --- | --- | --- |
| `luaL_openselectedlibs(L, load, preload)` | 按位掩码加载 / 预加载标准库。 | Lua 5.5 新增，适合最小化运行时。 |
| `luaL_openlibs(L)` | 打开全部标准库。 | 等价于 `luaL_openselectedlibs(L, ~0, 0)`。 |

## 5. 实际使用建议

### 5.1 宿主初始化的最常见流程

1. `luaL_newstate()` 创建状态。
2. `luaL_openlibs(L)` 或 `luaL_openselectedlibs(L, ...)` 打开库。
3. 使用 `luaL_loadfile` / `luaL_loadstring` 加载脚本。
4. 用 `lua_pcall` 执行，统一处理错误。

### 5.2 写 C 函数时最常见的辅助组合

1. 用 `luaL_check*` / `luaL_opt*` 取参数。
2. 用 `lua_push*` 压回返回值。
3. 返回整数 `n`，表示返回值数量。
4. 发生错误时使用 `luaL_error`，不要手工拼长跳转。

### 5.3 注册模块的最常见写法

1. 定义 `luaL_Reg lib[]`。
2. 在 `luaopen_xxx` 中调用 `luaL_newlib(L, lib)`。
3. 如需共享状态，可先压上值再用 `luaL_setfuncs`。

### 5.4 容易踩坑的点

1. `lua_tolstring` 可能改变栈位表示，遍历时不要忽略副作用。
2. `lua_settable` / `lua_rawset` 会消费键和值，栈平衡必须自己维护。
3. `lua_pcall` 的 `errfunc` 必须是有效栈索引，且错误处理函数本身也可能失败。
4. `lua_xmove` 只能用于同一个全局状态下的不同线程。
5. `lightuserdata` 不受 GC 管理，生命周期完全由宿主负责。
6. `lua_topointer` 只适合做身份比较，不代表可安全解引用。

## 6. 文档边界说明

这份文档以当前仓库中的头文件声明为准，重点是“接口用途”和“调用语义”。如果你后续还需要，我可以继续在这份文档基础上补两类内容：

1. 每个 API 的最小示例代码。
2. 结合栈变化的调用时序图。
