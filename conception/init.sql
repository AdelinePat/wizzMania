-- =====================================
--   wizzMania Database Initialization
-- =====================================

-- Users table
CREATE TABLE users (
    id_user BIGINT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(100) NOT NULL,
    email VARCHAR(255) NOT NULL UNIQUE,
    password VARCHAR(255)
);

-- Channels table
CREATE TABLE channels (
    id_channel BIGINT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(100) NOT NULL
);

-- User-Channel link table
CREATE TABLE userChannel (
    id_userChannel BIGINT AUTO_INCREMENT PRIMARY KEY,
    id_user BIGINT NULL,
    id_channel BIGINT NOT NULL,
    accepted BOOLEAN,
    CONSTRAINT fk_userChannel_user FOREIGN KEY (id_user)
        REFERENCES users(id_user)
        ON DELETE NO ACTION
        ON UPDATE CASCADE,
    CONSTRAINT fk_userChannel_channel FOREIGN KEY (id_channel)
        REFERENCES channels(id_channel)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);

-- Messages table
CREATE TABLE messages (
    id_message BIGINT AUTO_INCREMENT PRIMARY KEY,
    id_user BIGINT NULL,
    id_channel BIGINT NOT NULL,
    timestamp DATETIME,
    body TEXT,
    CONSTRAINT fk_messages_user FOREIGN KEY (id_user)
        REFERENCES users(id_user)
        ON DELETE NO ACTION
        ON UPDATE CASCADE,
    CONSTRAINT fk_messages_channel FOREIGN KEY (id_channel)
        REFERENCES channels(id_channel)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);
