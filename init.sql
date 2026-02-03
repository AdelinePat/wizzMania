-- =====================================
--   wizzMania Database Initialization
-- =====================================

-- Users table
CREATE TABLE users (
    id_user BIGINT AUTO_INCREMENT PRIMARY KEY,
    username VARCHAR(100) NOT NULL UNIQUE,
    email VARCHAR(255) NOT NULL UNIQUE,
    password VARCHAR(255)
    -- last_seen DATETIME
);

-- Channels table
CREATE TABLE channels (
    id_channel BIGINT AUTO_INCREMENT PRIMARY KEY,
    title VARCHAR(100) NOT NULL
    created_at DATETIME DEFAULT CURRENT_TIMESTAMP
    created_by BIGINT NOT NULL
    CONSTRAINT kf_channel_creator FOREIGN KEY (created_by) REFERENCES users(id_user) ON DELETE NO ACTION ON UPDATE CASCADE
);

-- User-Channel link table
CREATE TABLE userChannel (
    id_userChannel BIGINT AUTO_INCREMENT PRIMARY KEY,
    id_user BIGINT NULL,
    id_channel BIGINT NOT NULL,
    accepted BOOLEAN NOT NULL DEFAULT FALSE,
    -- status ENUM('pending', 'accepted', 'rejected') NOT NULL DEFAULT 'pending',
    -- ORRR FOR LESS ISSUES !! 
    status TINYINT NOT NULL DEFAULT 0,  -- 0=pending, 1=accepted, 2=rejected, 3=left
    joined_at DATETIME, 
    last_read_message_id BIGINT,
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
    timestamp DATETIME DEFAULT CURRENT_TIMESTAMP,
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


-- INIT FALSE DATA
-- USERS
-- Add system user during initialization
INSERT INTO users (id_user, username, email, password) 
VALUES (0, 'System', 'system@wizzmania.internal', 'SYSTEM_NO_LOGIN');

-- FAKE USERS 
INSERT INTO users (username, email, password) VALUES
('alice', 'alice@mail.com', 'hash'),
('bob', 'bob@mail.com', 'hash'),
('carol', 'carol@mail.com', 'hash'),
('dave', 'dave@mail.com', 'hash'),
('eve', 'eve@mail.com', 'hash'),
('frank', 'frank@mail.com', 'hash'),
('grace', 'grace@mail.com', 'hash'),
('heidi', 'heidi@mail.com', 'hash'),
('ivan', 'ivan@mail.com', 'hash'),
('judy', 'judy@mail.com', 'hash');

-- CHANNELS
INSERT INTO channels (title) VALUES
('Alice,  Bob'),
('Alice, Carol, Dave'),
('Bob, Carol'),
('Dave, Eve, Frank'),
('Grace, Heidi'),
('Ivan, Judy, Alice');

-- USER_CHANNEL
INSERT INTO userChannel (id_user, id_channel, accepted, last_read_message_id) VALUES
-- Chat 1 (A-B)
(1, 1, TRUE, 15),
(2, 1, TRUE, 18),

-- Chat 2 (A-C-D) → D has not accepted yet
(1, 2, TRUE, 10),
(3, 2, TRUE, 12),
(4, 2, TRUE, 12),

-- Chat 3 (B-C)
(2, 3, TRUE, 20),
(3, 3, TRUE, 20),

-- Chat 4 (D-E-F) → F pending
(4, 4, TRUE, 8),
(5, 4, TRUE, 10),
(6, 4, FALSE, NULL),

-- Chat 5 (G-H)
(7, 5, TRUE, 20),
(8, 5, TRUE, 19),

-- Chat 6 (I-J-A) → J pending
(9, 6, TRUE, 14),
(10, 6, FALSE, NULL),
(1, 6, TRUE, 16);

-- MESSAGES
INSERT INTO messages (id_user, id_channel, timestamp, body) VALUES
(1, 1, NOW() - INTERVAL 20 MINUTE, 'Hey'),
(2, 1, NOW() - INTERVAL 19 MINUTE, 'Hi'),
(1, 1, NOW() - INTERVAL 18 MINUTE, 'How are you?'),
(2, 1, NOW() - INTERVAL 17 MINUTE, 'Good'),
(1, 1, NOW() - INTERVAL 16 MINUTE, 'Nice'),
(2, 1, NOW() - INTERVAL 15 MINUTE, 'Yep'),
(1, 1, NOW() - INTERVAL 14 MINUTE, 'Any news?'),
(2, 1, NOW() - INTERVAL 13 MINUTE, 'Not really'),
(1, 1, NOW() - INTERVAL 12 MINUTE, 'Ok'),
(2, 1, NOW() - INTERVAL 11 MINUTE, 'See you'),
(1, 1, NOW() - INTERVAL 10 MINUTE, 'Later'),
(2, 1, NOW() - INTERVAL 9 MINUTE, 'Bye'),
(1, 1, NOW() - INTERVAL 8 MINUTE, '👍'),
(2, 1, NOW() - INTERVAL 7 MINUTE, '👍'),
(1, 1, NOW() - INTERVAL 6 MINUTE, 'Done'),
(2, 1, NOW() - INTERVAL 5 MINUTE, 'Yep'),
(1, 1, NOW() - INTERVAL 4 MINUTE, 'End'),
(2, 1, NOW() - INTERVAL 3 MINUTE, 'Ok'),
(1, 1, NOW() - INTERVAL 2 MINUTE, 'Final'),
(2, 1, NOW() - INTERVAL 1 MINUTE, '👍');
