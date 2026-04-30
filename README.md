# 许可证认证软件

基于Socket API实现的许可认证系统，用于远程桌面连接软件的许可证管理。

## 功能特性

- 许可证序列号生成与验证
- 并发用户数限制管理
- 心跳检测与超时自动释放
- 客户端异常退出处理
- Windows控制台可执行程序接口
- PHP网页管理界面

## 项目结构

```
lab5/
├── server/              # 服务器端程序
│   ├── license_server.cpp    # Socket服务器源码
│   ├── license_server.exe    # 编译后的服务器
│   ├── serial_generator.cpp  # 序列号生成器源码
│   └── serial_generator.exe  # 编译后的生成器
├── client/              # 客户端程序
│   ├── license_client.cpp    # 客户端源码
│   └── license_client.exe    # 编译后的客户端
├── web/                 # PHP管理界面
│   └── index.php             # 许可证管理网页
├── database/            # 数据库文件
│   └── licenses.txt          # 许可证数据存储
└── README.md            # 本文件
```

## 编译方法

### 环境要求

- Windows 操作系统
- MinGW (g++) 编译器
- Winsock2 库（已内置）

### 编译命令

```bash
# 编译服务器
g++ -o server/license_server.exe server/license_server.cpp -lws2_32

# 编译序列号生成器
g++ -o server/serial_generator.exe server/serial_generator.cpp

# 编译客户端
g++ -o client/license_client.exe client/license_client.cpp -lws2_32
```

## 使用方法

### 1. 启动许可证服务器

```bash
server\license_server.exe
```

服务器将监听端口 **8888**，启动成功后显示：
```
许可证服务器启动成功，监听端口 8888
已加载 X 个许可证
```

### 2. 生成新许可证

运行序列号生成器，按照提示输入信息：

```bash
server\serial_generator.exe
```

示例输出：
```
=== 许可证生成器 ===
请输入用户名: myorg
请输入口令: mypassword
请输入许可人数(如: 10, 50): 10

=== 许可证生成成功 ===
用户名: myorg
许可人数: 10
序列号: 1234567890
请妥善保存此序列号!
```

### 3. 客户端认证

```bash
client/license_client.exe <服务器IP> <序列号>
```

**示例：**

```bash
# 本地测试
client\license_client.exe 127.0.0.1 1234567890

# 远程服务器
client\license_client.exe 192.168.1.100 1234567890
```

**认证成功输出：**
```
正在连接许可证服务器 127.0.0.1:8888
连接成功，正在认证...
认证成功! 客户端ID: 1234567890_1777539324_41
软件A已启动，按 Ctrl+C 退出
```

**认证失败输出：**
```
正在连接许可证服务器 127.0.0.1:8888
连接成功，正在认证...
认证失败: 无效的序列号
```

### 4. 退出客户端

按 `Ctrl+C` 优雅退出，许可证将自动释放。

## 测试场景

### 测试1：有效序列号认证

```bash
# 启动服务器
server\license_server.exe

# 在另一个终端运行客户端
client\license_client.exe 127.0.0.1 1234567890
```

预期结果：认证成功，显示客户端ID。

### 测试2：无效序列号认证

```bash
client\license_client.exe 127.0.0.1 1111111111
```

预期结果：认证失败，提示"无效的序列号"。

### 测试3：许可证人数上限

使用许可人数为5的序列号 `0987654321`，启动6个客户端：

```bash
# 启动5个客户端（应全部成功）
client\license_client.exe 127.0.0.1 0987654321  # 成功
client\license_client.exe 127.0.0.1 0987654321  # 成功
client\license_client.exe 127.0.0.1 0987654321  # 成功  
client\license_client.exe 127.0.0.1 0987654321  # 成功
client\license_client.exe 127.0.0.1 0987654321  # 成功

# 启动第6个客户端（应失败）
client\license_client.exe 127.0.0.1 0987654321  # 失败：许可证已达最大使用人数      
```

### 测试4：客户端异常退出处理

1. 启动一个客户端并认证成功
2. 强制关闭客户端（不按Ctrl+C）
3. 等待约1分钟，服务器将自动检测并释放许可证

### 测试5：服务器重启恢复

1. 启动服务器和客户端（认证成功）
2. 关闭服务器
3. 重新启动服务器
4. 新客户端可以正常连接认证

## 默认测试数据

系统内置两个测试许可证：

| 序列号 | 用户名 | 口令 | 最大人数 |
|--------|--------|------|----------|
| 1234567890 | admin | password123 | 10 |
| 0987654321 | testorg | testpass | 5 |

## 协议说明

### 应用层协议

服务器支持三种请求格式：

| 请求类型 | 格式 | 说明 |
|----------|------|------|
| 认证 | `AUTH:<序列号>` | 客户端首次连接时发送 |
| 心跳 | `HEARTBEAT:<客户端ID>` | 每30分钟发送一次 |
| 释放 | `RELEASE:<客户端ID>` | 客户端退出时发送 |

### 响应格式

| 响应类型 | 格式 | 说明 |
|----------|------|------|
| 成功 | `OK:<客户端ID>` | 认证成功，返回客户端ID |
| 成功 | `OK` | 心跳/释放成功 |
| 失败 | `DENY:<原因>` | 操作失败，返回原因 |

## PHP管理界面

将 `web/index.php` 放置到支持PHP的Web服务器中，即可通过浏览器管理许可证。

**功能：**
- 生成新许可证
- 查看所有许可证列表及使用状态
- 查看活跃客户端列表

## 技术实现

- **服务端**: C++ + Winsock2 + 文件存储
- **客户端**: C++ + Winsock2
- **管理界面**: PHP + SQLite（可选）

## 注意事项

1. 服务器需要先启动，客户端才能连接
2. 客户端需要网络访问服务器的8888端口
3. 心跳超时时间为30分钟，超时后许可证自动释放
4. 许可证数据存储在 `database/licenses.txt` 文件中