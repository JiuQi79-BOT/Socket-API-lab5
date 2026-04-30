@echo off
echo 正在编译许可证服务器...
g++ -o server/license_server.exe server/license_server.cpp -lsqlite3 -lws2_32 -lpthread

if %errorlevel% equ 0 (
    echo 服务器编译成功!
) else (
    echo 服务器编译失败!
    pause
    exit /b 1
)

echo 正在编译序列号生成器...
g++ -o server/serial_generator.exe server/serial_generator.cpp -lsqlite3

if %errorlevel% equ 0 (
    echo 序列号生成器编译成功!
) else (
    echo 序列号生成器编译失败!
    pause
    exit /b 1
)

echo 正在编译客户端...
g++ -o client/license_client.exe client/license_client.cpp -lws2_32 -lpthread

if %errorlevel% equ 0 (
    echo 客户端编译成功!
) else (
    echo 客户端编译失败!
    pause
    exit /b 1
)

echo 所有程序编译完成!
pause