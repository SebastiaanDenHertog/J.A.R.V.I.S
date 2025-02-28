CREATE TABLE IF NOT EXISTS client_device (
        id SERIAL PRIMARY KEY, 
        ip_address VARCHAR(15) NOT NULL,
        device_name VARCHAR(50) NOT NULL,
        device_type VARCHAR(50) NOT NULL,
        device_os VARCHAR(50) NOT NULL,
        device_os_version VARCHAR(50) NOT NULL,
    )

CREATE TABLE IF NOT EXISTS user (
        id SERIAL PRIMARY KEY, 
        client_device_id INT NOT NULL,
        user_id VARCHAR(50) NOT NULL,
        user_name VARCHAR(50) NOT NULL,
        user_email VARCHAR(50),
        user_phone VARCHAR(50),
        user_created_at TIMESTAMP NOT NULL,
        user_updated_at TIMESTAMP NOT NULL,
        FOREIGN KEY (client_device_id) REFERENCES client_devices(id)
    )

CREATE TABLE IF NOT EXISTS message (
        id SERIAL PRIMARY KEY, 
        conversation_id INT NOT NULL,
        message_id VARCHAR(50) NOT NULL,
        message_type VARCHAR(50) NOT NULL,
        message_content TEXT NOT NULL,
        message_created_at TIMESTAMP NOT NULL,
        message_updated_at TIMESTAMP NOT NULL,
        FOREIGN KEY (conversation_id) REFERENCES conversation(id)
    )

CREATE TABLE IF NOT EXISTS conversation (
        id SERIAL PRIMARY KEY, 
        client_device_id INT NOT NULL,
        conversation_id VARCHAR(50) NOT NULL,
        conversation_type VARCHAR(50) NOT NULL,
        conversation_status VARCHAR(50) NOT NULL,
        conversation_created_at TIMESTAMP NOT NULL,
        conversation_updated_at TIMESTAMP NOT NULL,
        FOREIGN KEY (client_device_id) REFERENCES client_devices(id)
    )

CREATE TABLE IF NOT EXISTS conversation_user (
        id SERIAL PRIMARY KEY, 
        conversation_id INT NOT NULL,
        user_id INT NOT NULL,
        FOREIGN KEY (conversation_id) REFERENCES conversation(id),
        FOREIGN KEY (user_id) REFERENCES user(id)
    )

CREATE TABLE IF NOT EXISTS conversation_message (
        id SERIAL PRIMARY KEY, 
        conversation_id INT NOT NULL,
        message_id INT NOT NULL,
        FOREIGN KEY (conversation_id) REFERENCES conversation(id),
        FOREIGN KEY (message_id) REFERENCES message(id)
    )

CREATE TABLE IF NOT EXISTS conversation_user_message (
        id SERIAL PRIMARY KEY, 
        conversation_user_id INT NOT NULL,
        conversation_message_id INT NOT NULL,
        FOREIGN KEY (conversation_user_id) REFERENCES conversation_user(id),
        FOREIGN KEY (conversation_message_id) REFERENCES conversation_message(id)
    )

