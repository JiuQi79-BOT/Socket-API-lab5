# 许可证认证软件

基于Socket API实现的许可认证系统，用于远程桌面连接软件的许可证管理。

---

##  功能特性

-  许可证序列号生成与验证
-  并发用户数限制管理
-  心跳检测与超时自动释放（30分钟）
-  客户端异常退出处理
-  Windows控制台可执行程序接口
-  PHP网页管理界面（实时显示许可证状态和活跃客户端）

---

##  项目结构

```
lab5/
├── server/                     # 服务器端程序
│   ├── license_server.cpp      # Socket服务器源码
│   ├── license_server.exe      # 编译后的服务器（需自行编译）
│   ├── serial_generator.cpp    # 序列号生成器源码
│   └── serial_generator.exe    # 编译后的生成器（需自行编译）
├── client/                     # 客户端程序
│   ├── license_client.cpp      # 客户端源码
│   └── license_client.exe      # 编译后的客户端（需自行编译）
├── web/                        # PHP管理界面
│   └── index.php               # 许可证管理网页
├── database/                   # 数据文件目录
│   ├── licenses.txt            # 许可证数据存储（服务器使用）
│   ├── active_clients.txt      # 活跃客户端列表（服务器自动生成）
│   └── schema.sql              # SQLite数据库初始化脚本
├── build.bat                   # Windows编译脚本
└── README.md                   # 项目说明文档
```

---

##  环境要求

### 编译环境（Windows）
- Windows 10/11 操作系统
- MinGW (g++) 编译器（建议版本：8.1.0 或更高）
- Winsock2 库（Windows系统内置，无需额外安装）

### PHP运行环境（网页管理）
- PHP 7.4 或更高版本
- SQLite3 扩展（需在php.ini中启用）

---

##  编译方法

### 方法一：使用编译脚本

```bash
# 运行编译脚本
build.bat
```

### 方法二：手动编译

打开命令提示符（CMD）或PowerShell，进入项目根目录：

```bash
# 编译许可证服务器
g++ -o server/license_server.exe server/license_server.cpp -lws2_32

# 编译序列号生成器
g++ -o server/serial_generator.exe server/serial_generator.cpp

# 编译客户端
g++ -o client/license_client.exe client/license_client.cpp -lws2_32
```

### 编译成功验证

编译成功后，在 `server/` 和 `client/` 目录下会生成对应的 `.exe` 文件：
- `server/license_server.exe`
- `server/serial_generator.exe`
- `client/license_client.exe`

---

##  使用方法

### 1. 启动许可证服务器

```bash
# 进入项目根目录
cd d:\MyGit\"利用Socket API实现许可认证软件（lab5）"

# 启动服务器
server\license_server.exe
```

**启动成功输出：**
```
License Server started successfully, listening on port 8888
Loaded 3 licenses
```

**参数说明：**
- 服务器默认监听端口：**8888**
- 许可证数据文件：`database/licenses.txt`
- 活跃客户端文件：`database/active_clients.txt`（自动生成）

### 2. 生成新许可证

使用序列号生成器创建新许可证：

```bash
server\serial_generator.exe
```

**示例操作：**
```
=== 许可证生成器 ===
请输入用户名: mycompany
请输入口令: mysecretpassword
请输入许可人数(如: 10, 50): 20

=== 许可证生成成功 ===
用户名: mycompany
许可人数: 20
序列号: 1234567890
请妥善保存此序列号!
```

### 3. 客户端认证

```bash
client\license_client.exe <服务器IP> <序列号>
```

**示例：**

```bash
# 本地测试（服务器和客户端在同一机器）
client\license_client.exe 127.0.0.1 1234567890

# 远程服务器（服务器在另一台机器）
client\license_client.exe 192.168.1.100 0987654321
```

**认证成功输出：**
```
Connecting to license server 127.0.0.1:8888
Connected, authenticating...

=== [许可证认证流程] ===
步骤1/3: 构造认证请求
         请求格式: AUTH:<序列号>
         发送内容: AUTH:1234567890
步骤2/3: 等待服务器响应...
步骤3/3: 解析服务器响应
         响应内容: OK:1234567890_1777551780_41

[认证成功]
├─ 服务器已验证序列号有效性
├─ 许可证容量充足
└─ 授予客户端ID: 1234567890_1777551780_41
Software A started, press Ctrl+C to exit
```

**认证失败输出：**
```
Connecting to license server 127.0.0.1:8888
Connected, authenticating...

=== [许可证认证流程] ===
步骤1/3: 构造认证请求
         请求格式: AUTH:<序列号>
         发送内容: AUTH:1111111111
步骤2/3: 等待服务器响应...
步骤3/3: 解析服务器响应
         响应内容: DENY:Invalid serial number

[认证失败]
└─ 原因: Invalid serial number
```

### 4. 退出客户端

按 `Ctrl+C` 退出，许可证将自动释放：

```
License released, exiting...
```

---

##  PHP网页管理界面

### 启动方法

#### 方法一：使用PHP内置服务器（推荐）

```bash
# 进入项目根目录
cd d:\MyGit\利用Socket API实现许可认证软件（lab5）

# 启动PHP内置服务器，监听8080端口
cd web
php -S localhost:8080
```

#### 方法二：使用Apache/Nginx

1. 将 `web/index.php` 复制到Web服务器的根目录
2. 确保PHP配置文件（php.ini）中启用了 SQLite3 扩展：
   ```ini
   extension_dir = "ext"
   extension=sqlite3
   ```
3. 启动Web服务器

### 访问网页

打开浏览器，访问：

```
http://localhost:8080
```

### 网页功能

| 功能 | 说明 |
|------|------|
| 生成新许可证 | 通过表单输入用户名、口令和许可人数，自动生成序列号 |
| 许可证列表 | 显示所有许可证的序列号、用户名、许可人数、当前使用人数和状态 |
| 活跃客户端 | 显示当前连接的客户端信息（序列号、客户端ID、客户端IP、最后心跳时间） |

### 实时更新说明

- 许可证使用状态：实时读取 `database/licenses.txt`
- 活跃客户端列表：实时读取 `database/active_clients.txt`
- 数据由服务器自动更新，刷新网页即可看到最新状态

---

##  测试文档

### 测试环境准备

1. 确保已编译所有程序
2. 启动许可证服务器
3. 准备两个命令提示符窗口（一个用于服务器，一个用于客户端）

### 测试用例

#### 测试1：有效序列号认证

**步骤：**
```bash
# 窗口1：启动服务器
cd d:\MyGit\利用Socket API实现许可认证软件（lab5）
server\license_server.exe

# 窗口2：运行客户端
cd d:\MyGit\利用Socket API实现许可认证软件（lab5）
client\license_client.exe 127.0.0.1 1234567890
```

**预期结果：**
- 服务器显示认证成功日志
- 客户端显示认证成功，输出客户端ID
- 网页显示许可证 `1234567890` 当前使用人数 +1

#### 测试2：无效序列号认证

**步骤：**
```bash
client\license_client.exe 127.0.0.1 1111111111
```

**预期结果：**
- 客户端显示认证失败，提示"无效的序列号"

#### 测试3：许可证人数上限

**步骤：**
```bash
# 使用许可人数为5的序列号 0987654321

# 启动第1-5个客户端（应全部成功）
client\license_client.exe 127.0.0.1 0987654321
client\license_client.exe 127.0.0.1 0987654321
client\license_client.exe 127.0.0.1 0987654321
client\license_client.exe 127.0.0.1 0987654321
client\license_client.exe 127.0.0.1 0987654321

# 启动第6个客户端（应失败）
client\license_client.exe 127.0.0.1 0987654321
```

**预期结果：**
- 前5个客户端认证成功
- 第6个客户端认证失败，提示"Max users exceeded"

#### 测试4：客户端异常退出处理

**步骤：**
1. 启动一个客户端并认证成功
2. 强制关闭客户端窗口（不按Ctrl+C）
3. 等待约60秒

**预期结果：**
- 服务器检测到客户端超时
- 自动释放许可证
- 网页显示许可证当前使用人数 -1

#### 测试5：服务器重启恢复

**步骤：**
1. 启动服务器和客户端（认证成功）
2. 关闭服务器（直接关闭窗口）
3. 重新启动服务器
4. 启动新客户端

**预期结果：**
- 新客户端可以正常连接认证
- 原客户端需要重新认证

#### 测试6：网页管理功能

**步骤：**
1. 启动服务器和PHP网页
2. 在网页上生成新许可证
3. 使用新生成的序列号启动客户端

**预期结果：**
- 新许可证成功添加到 `licenses.txt`
- 客户端可以使用新序列号认证
- 网页实时显示许可证状态

---

##  默认测试数据

系统内置三个测试许可证：

| 序列号 | 用户名 | 口令 | 最大人数 | 当前使用 |
|--------|--------|------|----------|----------|
| 1234567890 | admin | password123 | 10 | 0 |
| 0987654321 | testorg | testpass | 5 | 0 |
| 9079926355 | user1 | 1147852 | 10 | 0 |

---

##  协议说明

### 应用层协议

服务器支持三种请求格式：

| 请求类型 | 格式 | 说明 |
|----------|------|------|
| 认证 | `AUTH:<序列号>` | 客户端首次连接时发送，提交序列号进行认证 |
| 心跳 | `HEARTBEAT:<客户端ID>` | 每30分钟发送一次，保持连接活跃状态 |
| 释放 | `RELEASE:<客户端ID>` | 客户端退出时发送，释放许可证 |

### 响应格式

| 响应类型 | 格式 | 说明 |
|----------|------|------|
| 成功 | `OK:<客户端ID>` | 认证成功，返回客户端唯一标识 |
| 成功 | `OK` | 心跳/释放操作成功 |
| 失败 | `DENY:<原因>` | 操作失败，返回失败原因 |

### 数据文件格式

**licenses.txt**（许可证数据）：
```
序列号|用户名|口令|最大人数|当前使用人数
```

**active_clients.txt**（活跃客户端）：
```
序列号|客户端ID|客户端IP|最后心跳时间戳
```

---

##  故障排除

### 问题1：服务器无法启动

**症状：**
```
Bind failed: 0
```

**原因：** 端口8888被其他程序占用

**解决方案：**
```bash
# 查找占用端口的进程
netstat -ano | findstr "8888"

# 终止占用进程（将PID替换为实际进程ID）
taskkill /F /PID <PID>

# 重新启动服务器
server\license_server.exe
```

### 问题2：客户端无法连接

**症状：**
```
Failed to connect to server: 10061
```

**原因：** 服务器未启动或网络不通

**解决方案：**
1. 确保服务器已启动
2. 检查防火墙是否允许8888端口
3. 确认服务器IP地址正确

### 问题3：网页无法显示活跃客户端

**症状：** 网页显示"暂无活跃客户端"

**原因：** 服务器未运行或未创建 `active_clients.txt`

**解决方案：**
1. 确保服务器已启动
2. 确保至少有一个客户端已认证成功
3. 检查 `database/active_clients.txt` 是否存在

### 问题4：PHP SQLite3扩展未启用

**症状：**
```
Fatal error: Uncaught Error: Class "SQLite3" not found
```

**解决方案：**
1. 找到PHP安装目录
2. 复制 `php.ini-development` 或 `php.ini-production` 为 `php.ini`
3. 在 `php.ini` 中取消注释：
   ```ini
   extension_dir = "ext"
   extension=sqlite3
   ```

---

##  技术实现

| 组件 | 技术栈 |
|------|--------|
| 服务端 | C++ + Winsock2 + 文件存储 |
| 客户端 | C++ + Winsock2 |
| 管理界面 | PHP + 文件读取 |

---

##  注意事项

1. **服务器启动顺序**：必须先启动服务器，客户端才能连接
2. **端口访问**：客户端需要网络访问服务器的8888端口
3. **心跳超时**：心跳超时时间为30分钟，超时后许可证自动释放
4. **数据持久化**：许可证数据存储在 `database/licenses.txt`，活跃客户端存储在 `database/active_clients.txt`
5. **安全建议**：生产环境中应使用加密传输（如TLS）和更强的认证机制
