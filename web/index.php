<!DOCTYPE html>
<html>
<head>
    <meta charset="UTF-8">
    <title>许可证管理系统</title>
    <style>
        body { font-family: Arial, sans-serif; margin: 20px; background-color: #ffffff; }
        .container { max-width: 1000px; margin: 0 auto; }
        table { border-collapse: collapse; width: 100%; margin-top: 20px; }
        th, td { border: 1px solid #ddd; padding: 8px; text-align: left; }
        th { background-color: #f2f2f2; }
        .btn { padding: 8px 16px; background-color: #4CAF50; color: white; border: none; cursor: pointer; }
        .btn:hover { background-color: #45a049; }
        .form-group { margin: 10px 0; }
        input { padding: 8px; width: 200px; border: 1px solid #ccc; border-radius: 4px; }
        label { display: inline-block; width: 80px; }
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

            $db_path = '../database/licenses.txt';
            $licenses = [];

            if (file_exists($db_path)) {
                $lines = file($db_path, FILE_IGNORE_NEW_LINES);
                foreach ($lines as $line) {
                    $parts = explode('|', $line);
                    if (count($parts) >= 5) {
                        $licenses[$parts[0]] = $line;
                    }
                }
            }

            do {
                $serial = '';
                for ($i = 0; $i < 10; $i++) {
                    $serial .= rand(0, 9);
                }
            } while (isset($licenses[$serial]));

            $new_line = "$serial|$username|$password|$max_users|0";
            $licenses[$serial] = $new_line;

            $content = implode("\n", $licenses) . "\n";
            file_put_contents($db_path, $content);

            echo "<div style='background-color: #d4edda; padding: 10px; margin: 10px 0;'>";
            echo "生成成功! 序列号: <strong>$serial</strong>";
            echo "</div>";
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
            $db_path = '../database/licenses.txt';
            if (file_exists($db_path)) {
                $lines = file($db_path, FILE_IGNORE_NEW_LINES);
                foreach ($lines as $line) {
                    $parts = explode('|', $line);
                    if (count($parts) >= 5) {
                        $serial = htmlspecialchars($parts[0]);
                        $username = htmlspecialchars($parts[1]);
                        $max_users = intval($parts[3]);
                        $current_users = intval($parts[4]);
                        $status = $current_users >= $max_users ? '已满' : '正常';
                        echo "<tr>";
                        echo "<td>$serial</td>";
                        echo "<td>$username</td>";
                        echo "<td>$max_users</td>";
                        echo "<td>$current_users</td>";
                        echo "<td>$status</td>";
                        echo "</tr>";
                    }
                }
            }
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
            $active_clients_file = '../database/active_clients.txt';
            if (file_exists($active_clients_file) && filesize($active_clients_file) > 0) {
                $lines = file($active_clients_file, FILE_IGNORE_NEW_LINES);
                foreach ($lines as $line) {
                    $parts = explode('|', $line);
                    if (count($parts) >= 4) {
                        $serial = htmlspecialchars($parts[0]);
                        $client_id = htmlspecialchars($parts[1]);
                        $client_ip = htmlspecialchars($parts[2]);
                        $heartbeat = date('Y-m-d H:i:s', intval($parts[3]));
                        echo "<tr>";
                        echo "<td>$serial</td>";
                        echo "<td>$client_id</td>";
                        echo "<td>$client_ip</td>";
                        echo "<td>$heartbeat</td>";
                        echo "</tr>";
                    }
                }
            } else {
                echo "<tr><td colspan='4' style='text-align: center; color: #666;'>暂无活跃客户端</td></tr>";
            }
            ?>
        </table>
    </div>
</body>
</html>
