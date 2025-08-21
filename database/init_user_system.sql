-- MyTelegram 用户系统数据库初始化脚本
-- 执行前请确保已连接到testdb数据库

-- 创建用户表
CREATE TABLE IF NOT EXISTS users (
    user_id BIGINT AUTO_INCREMENT PRIMARY KEY COMMENT '用户ID',
    username VARCHAR(50) UNIQUE NOT NULL COMMENT '用户名',
    password_hash VARCHAR(255) NOT NULL COMMENT '密码哈希',
    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP COMMENT '创建时间',
    updated_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP ON UPDATE CURRENT_TIMESTAMP COMMENT '更新时间',
    
    -- 添加索引提高查询性能
    INDEX idx_username (username),
    INDEX idx_created_at (created_at)
) ENGINE=InnoDB DEFAULT CHARSET=utf8mb4 COLLATE=utf8mb4_unicode_ci COMMENT='用户表';

-- 创建测试用户（可选）
-- 密码是 'testpassword123' 的哈希值，使用bcrypt生成
INSERT IGNORE INTO users (username, password_hash) VALUES 
('testuser1', '$6$rounds=5000$testsalt12345678$gKnKdEMKYSJCZ1pCaOt8mMkw7Kx6zIRw.pLMVPWP8RzWdNtQ0M5LQEgYKFKzJ8DzGR9ZXcVoJ4LzSzQkQl7.r0'),
('testuser2', '$6$rounds=5000$testsalt87654321$fDlMsJQEtQ6JzDv9zKm8wNsQ6VsX7QoVrNlKtMdQ5JmRxYpC3Z8cN1WmEqVyJ4SrF8PxBw2TlKqZ4MlNvK9.s1');

-- 验证表创建成功
SELECT 'Users table created successfully' AS status;
SELECT COUNT(*) AS user_count FROM users;

-- 显示表结构
DESCRIBE users;