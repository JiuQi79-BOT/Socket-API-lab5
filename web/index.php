<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>许可证管理系统</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; }
        .container { max-width: 1000px; margin: 0 auto; }
        table { border-collapse: collapse; width: 100%; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        .btn { padding: 8px 16px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }
        .btn:hover { background-color: #45a049; }
        .form-group { margin: 10px 0; }
        input { padding: 8px; width: 200px; }
    </style>
</head>
<body>
    <div class="container">
        <h1>许可证管理系统</h1>
        
        <h2>生成新许可证</h2>
        <form method="post" action="index.php">
            <input type="hidden" name="action" value="generate">
            <div class="form-group">
                <label>用户名:</label>
                <input type="text" name="username" required>
            </div>
            <div class="form-group">
                <label>口令:</label>
                <input type="password" name="password" required>
            </div>
            <div class="form-group">
                <label>许可人数:</label>
                <input type="number" name="max_users" min="1" max="1000" value="10">
            </div>
            <div class="form-group">
                <input type="submit" class="btn" value="生成序列号">
            </div>
        </form>
        
        <?php
        if ($_SERVER['REQUEST_METHOD'] == 'POST' && $_POST['action'] == 'generate') {
            $username = $_POST['username'];
            $password = $_POST['password'];
            $max_users = intval($_POST['max_users']);
            
            $db = new SQLite3('../database/licenses.db');
            
            function generate_serial() {
                $serial = '';
                for ($i = 0; $i < 10; $i++) {
                    $serial .= rand(0, 9);
                }
                return $serial;
            }
            
            $serial = generate_serial();
            while (true) {
                $result = $db->querySingle("SELECT COUNT(*) FROM licenses WHERE serial_number = '$serial'");
                if ($result == 0) break;
                $serial = generate_serial();
            }
            
            $db->exec("INSERT INTO licenses (serial_number, username, password, max_users) 
                      VALUES ('$serial', '$username', '$password', $max_users)");
            
            echo "<div style='background-color: #d4edda; padding: 10px; margin: 10px 0;'>";
            echo "生成成功! 序列号: <strong>$serial</strong>";
            echo "</div>";
            
            $db->close();
        }
        ?>
        
        <h2>许可证列表</h2>
        <table>
            <tr>
                <th>序列号</th>
                <th>用户名</th>
                <th>许可人数</th>
                <th>当前使用</th>
                <th>状态</th>
            </tr>
            <?php
            $db = new SQLite3('../database/licenses.db');
            $result = $db->query("SELECT * FROM licenses");
            
            while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
                $status = $row['current_users'] >= $row['max_users'] ? '已满' : '正常';
                echo "<tr>";
                echo "<td>" . $row['serial_number'] . "</td>";
                echo "<td>" . $row['username'] . "</td>";
                echo "<td>" . $row['max_users'] . "</td>";
                echo "<td>" . $row['current_users'] . "</td>";
                echo "<td>" . $status . "</td>";
                echo "</tr>";
            }
            
            $db->close();
            ?>
        </table>
        
        <h2>活跃客户端</h2>
        <table>
            <tr>
                <th>序列号</th>
                <th>客户端ID</th>
                <th>客户端IP</th>
                <th>最后心跳</th>
            </tr>
            <?php
            $db = new SQLite3('../database/licenses.db');
            $result = $db->query("SELECT * FROM active_clients");
            
            while ($row = $result->fetchArray(SQLITE3_ASSOC)) {
                echo "<tr>";
                echo "<td>" . $row['serial_number'] . "</td>";
                echo "<td>" . $row['client_id'] . "</td>";
                echo "<td>" . $row['client_ip'] . "</td>";
                echo "<td>" . $row['last_heartbeat'] . "</td>";
                echo "</tr>";
            }
            
            $db->close();
            ?>
        </table>
    </div>
</body>
</html>