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
    title VARCHAR(60) NOT NULL,
    created_at VARCHAR(24) NOT NULL DEFAULT '1970-01-01T00:00:00Z',
    created_by BIGINT NULL,
    CONSTRAINT kf_channel_creator FOREIGN KEY (created_by) REFERENCES users(id_user) ON DELETE SET NULL ON UPDATE CASCADE
);

-- User-Channel link table
CREATE TABLE userChannel (
    id_userChannel BIGINT AUTO_INCREMENT PRIMARY KEY,
    id_user BIGINT NULL,
    id_channel BIGINT NOT NULL,
    membership TINYINT NOT NULL DEFAULT 0,  -- 0=pending, 1=accepted, 2=rejected, 3=left
    responded_at VARCHAR(24) NULL,
    last_read_id_message BIGINT,
    CONSTRAINT fk_userChannel_user FOREIGN KEY (id_user)
        REFERENCES users(id_user)
        ON DELETE CASCADE
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
    body TEXT,
    timestamp VARCHAR(24) NOT NULL DEFAULT '1970-01-01T00:00:00Z',
    CONSTRAINT fk_messages_user FOREIGN KEY (id_user)
        REFERENCES users(id_user)
        ON DELETE SET NULL
        ON UPDATE CASCADE,
    CONSTRAINT fk_messages_channel FOREIGN KEY (id_channel)
        REFERENCES channels(id_channel)
        ON DELETE CASCADE
        ON UPDATE CASCADE
);


-- INIT FALSE DATA
-- USERS
INSERT INTO users (username, email, password) 
VALUES ('System', 'system@wizzmania.internal', 'SYSTEM_NO_LOGIN');

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
INSERT INTO channels (id_channel, title, created_by) VALUES
(1, 'DM alice-bob', 2),      -- alice (was 1, now 2)
(2, 'DM alice-carol', 2),    -- alice
(3, 'DM bob-dave', 3),       -- bob (was 2, now 3)
(4, 'DM carol-eve', 4),      -- carol (was 3, now 4)
(5, 'DM dave-frank', 5),     -- dave (was 4, now 5)
(6, 'DM eve-grace', 6),      -- eve (was 5, now 6)
(7, 'DM frank-heidi', 7),    -- frank (was 6, now 7)
(8, 'DM grace-ivan', 8),     -- grace (was 7, now 8)
(9, 'DM heidi-judy', 9),     -- heidi (was 8, now 9)
(10, 'DM ivan-alice', 10),   -- ivan (was 9, now 10)
(11, 'Group Alpha', 2),      -- alice
(12, 'Group Beta', 3),       -- bob
(13, 'Group Gamma', 4),      -- carol
(14, 'Group Delta', 5),      -- dave
(15, 'Group Omega', 6),      -- eve
(16, 'Group Sigma', 7),      -- frank
(17, 'Group Lambda', 8),     -- grace
(18, 'Group Zeta', 9);       -- heidi


-- USER_CHANNEL memberships
-- Remember: membership: 0=pending, 1=accepted, 2=rejected, 3=left

-- Channel 1: DM alice-bob (both accepted)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(2, 1, 1, '2026-02-14 10:26:00', 10),  -- alice accepted, read up to message 10
(3, 1, 1, '2026-02-14 10:26:00', 10);  -- bob accepted, read up to message 10

-- Channel 2: DM alice-carol
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(2, 2, 1, '2026-02-14 10:26:00', NULL),  -- alice
(4, 2, 1, '2026-02-14 10:26:00', NULL);  -- carol

-- Channel 3: DM bob-dave
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(3, 3, 1, '2026-02-14 10:26:00', NULL),  -- bob
(5, 3, 0, '2026-02-14 10:26:00', NULL);  -- dave PENDING (invited but not accepted yet)

-- Channel 11: Group Alpha (alice created it)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(2, 11, 1, '2026-02-14 10:26:00', 10),   -- alice (creator)
(3, 11, 1, '2026-02-14 10:26:00', 10),   -- bob
(4, 11, 1, '2026-02-14 10:26:00', 10),   -- carol
(5, 11, 1, '2026-02-14 10:26:00', 10);   -- dave

-- MESSAGES
-- System logs
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 1, '@2 has joined the chat', '2026-02-14T10:26:00Z'),
(1, 1, '@3 has joined the chat', '2026-02-14T10:26:00Z');

-- DM messages
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(2, 1, 'Hey Bob', '2026-02-14T10:26:00Z'),
(3, 1, 'Hey Alice', '2026-02-14T10:26:00Z'),
(2, 1, 'How are you?', '2026-02-14T10:26:00Z'),
(3, 1, 'Good, you?', '2026-02-14T10:26:00Z'),
(2, 1, 'All good', '2026-02-14T10:26:00Z'),
(3, 1, 'Nice', '2026-02-14T10:26:00Z'),
(2, 1, 'What are you doing?', '2026-02-14T10:26:00Z'),
(3, 1, 'Working', '2026-02-14T10:26:00Z'),
(2, 1, 'Same here', '2026-02-14T10:26:00Z'),
(3, 1, 'Coffee later?', '2026-02-14T10:26:00Z');

-- Group chat messages
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(2, 11, 'Welcome everyone', '2026-02-14T10:26:00Z'),
(3, 11, 'Hi!', '2026-02-14T10:26:00Z'),
(4, 11, 'Hello', '2026-02-14T10:26:00Z'),
(5, 11, 'Hey', '2026-02-14T10:26:00Z'),
(2, 11, 'Let s start', '2026-02-14T10:26:00Z'),
(3, 11, 'Sure', '2026-02-14T10:26:00Z'),
(4, 11, 'Ready', '2026-02-14T10:26:00Z'),
(5, 11, 'Yep', '2026-02-14T10:26:00Z'),
(2, 11, 'Cool', '2026-02-14T10:26:00Z'),
(3, 11, 'Go', '2026-02-14T10:26:00Z');



-- ===============================
-- ADDITIONAL USER_CHANNEL DATA
-- ===============================

-- Channel 4: DM carol-eve
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(4, 4, 1, '2026-02-14 11:00:00', NULL),   -- carol accepted
(6, 4, 1, '2026-02-14 11:00:00', NULL);   -- eve accepted

-- Channel 5: DM dave-frank
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(5, 5, 1, '2026-02-14 11:05:00', NULL),
(7, 5, 0, '2026-02-14 11:05:00', NULL);   -- frank pending

-- Channel 6: DM eve-grace
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(6, 6, 1, '2026-02-14 11:10:00', NULL),
(8, 6, 2, '2026-02-14 11:10:00', NULL);   -- grace rejected

-- Channel 7: DM frank-heidi
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(7, 7, 1, '2026-02-14 11:15:00', NULL),
(9, 7, 1, '2026-02-14 11:15:00', NULL);

-- Channel 8: DM grace-ivan
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(8, 8, 1, '2026-02-14 11:20:00', NULL),
(10, 8, 0, '2026-02-14 11:20:00', NULL);  -- ivan pending

-- Channel 9: DM heidi-judy
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(9, 9, 1, '2026-02-14 11:25:00', NULL),
(11, 9, 1, '2026-02-14 11:25:00', NULL);

-- Channel 10: DM ivan-alice
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(10, 10, 1, '2026-02-14 11:30:00', NULL),
(2, 10, 1, '2026-02-14 11:30:00', NULL);

-- Add rejected & pending into group channels

-- Channel 12: Group Beta
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(3, 12, 1, '2026-02-14 11:35:00', NULL),
(4, 12, 0, '2026-02-14 11:35:00', NULL),  -- pending
(5, 12, 2, '2026-02-14 11:35:00', NULL);  -- rejected

-- Channel 13: Group Gamma
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(4, 13, 1, '2026-02-14 11:40:00', NULL),
(6, 13, 0, '2026-02-14 11:40:00', NULL),
(7, 13, 2, '2026-02-14 11:40:00', NULL);

-- Channel 14: Group Delta
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(5, 14, 1, '2026-02-14 11:45:00', NULL),
(8, 14, 1, '2026-02-14 11:45:00', NULL),
(9, 14, 0, '2026-02-14 11:45:00', NULL);

-- Channel 15: Group Omega
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(6, 15, 1, '2026-02-14 11:50:00', NULL),
(10, 15, 2, '2026-02-14 11:50:00', NULL);

-- Channel 16: Group Sigma
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(7, 16, 1, '2026-02-14 11:55:00', NULL),
(11, 16, 0, '2026-02-14 11:55:00', NULL);

-- Channel 17: Group Lambda
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(8, 17, 1, '2026-02-14 12:00:00', NULL),
(2, 17, 2, '2026-02-14 12:00:00', NULL);

-- Channel 18: Group Zeta
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(9, 18, 1, '2026-02-14 12:05:00', NULL),
(3, 18, 0, '2026-02-14 12:05:00', NULL);

-- ===============================
-- FILL MESSAGES FOR CHANNELS 2–18
-- Each gets 10 messages
-- ===============================

-- Channel 2
INSERT INTO messages (id_user, id_channel, body, timestamp)
SELECT 2, 2, CONCAT('Message ', n, ' in channel 2'), '2026-02-14T12:10:00Z'
FROM (
    SELECT 1 n UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
    UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9 UNION SELECT 10
) numbers;

-- Repeat same pattern for channels 3–10 and 12–18

INSERT INTO messages (id_user, id_channel, body, timestamp)
SELECT 3, 3, CONCAT('Message ', n, ' in channel 3'), '2026-02-14T12:15:00Z'
FROM (
    SELECT 1 n UNION SELECT 2 UNION SELECT 3 UNION SELECT 4 UNION SELECT 5
    UNION SELECT 6 UNION SELECT 7 UNION SELECT 8 UNION SELECT 9 UNION SELECT 10
) numbers;

-- You can duplicate and just change:
--  id_user
--  id_channel
--  timestamp
-- until channel 18
