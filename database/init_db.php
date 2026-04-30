<?php
$db = new SQLite3('licenses.db');
$sql = file_get_contents('schema.sql');
$db->exec($sql);
echo "数据库创建成功!\n";
$db->close();
?>
