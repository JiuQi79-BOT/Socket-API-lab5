CREATE TABLE IF NOT EXISTS licenses (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    serial_number VARCHAR(10) UNIQUE NOT NULL,
    username VARCHAR(100) NOT NULL,
    password VARCHAR(255) NOT NULL,
    max_users INTEGER NOT NULL DEFAULT 10,
    current_users INTEGER NOT NULL DEFAULT 0,
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP
);

CREATE TABLE IF NOT EXISTS active_clients (
    id INTEGER PRIMARY KEY AUTOINCREMENT,
    serial_number VARCHAR(10) NOT NULL,
    client_ip VARCHAR(45) NOT NULL,
    client_id VARCHAR(36) NOT NULL,
    last_heartbeat TIMESTAMP DEFAULT CURRENT_TIMESTAMP,
    FOREIGN KEY (serial_number) REFERENCES licenses(serial_number)
);

INSERT OR IGNORE INTO licenses (serial_number, username, password, max_users) VALUES ('1234567890', 'admin', 'password123', 10);
INSERT OR IGNORE INTO licenses (serial_number, username, password, max_users) VALUES ('0987654321', 'testorg', 'testpass', 5);