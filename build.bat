@echo off
chcp 65001 >nul
echo ========================================
echo    许可证认证软件编译脚本
echo ========================================
echo.

echo 正在编译许可证服务器...
g++ -o server/license_server.exe server/license_server.cpp -lws2_32

if %errorlevel% equ 0 (
    echo [OK] 服务器编译成功!
) else (
    echo [ERROR] 服务器编译失败!
    pause
    exit /b 1
)

echo.
echo 正在编译序列号生成器...
g++ -o server/serial_generator.exe server/serial_generator.cpp

if %errorlevel% equ 0 (
    echo [OK] 序列号生成器编译成功!
) else (
    echo [ERROR] 序列号生成器编译失败!
    pause
    exit /b 1
)

echo.
echo 正在编译客户端...
g++ -o client/license_client.exe client/license_client.cpp -lws2_32

if %errorlevel% equ 0 (
    echo [OK] 客户端编译成功!
) else (
    echo [ERROR] 客户端编译失败!
    pause
    exit /b 1
)

echo.
echo ========================================
echo    所有程序编译完成!
echo ========================================
echo.
echo 生成的文件:
echo   - server/license_server.exe
echo   - server/serial_generator.exe
echo   - client/license_client.exe
echo.
pause