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


-- =====================================
--   USERS
-- =====================================
INSERT INTO users (username, email, password)
VALUES ('System', 'system@wizzmania.internal', 'SYSTEM_NO_LOGIN');  -- id=1

-- clear password is "hash"
INSERT INTO users (username, email, password) VALUES
('alice', 'alice@mail.com', '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=2
('bob',   'bob@mail.com',   '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=3
('carol', 'carol@mail.com', '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=4
('dave',  'dave@mail.com',  '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=5
('eve',   'eve@mail.com',   '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=6
('frank', 'frank@mail.com', '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=7
('grace', 'grace@mail.com', '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=8
('heidi', 'heidi@mail.com', '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=9
('ivan',  'ivan@mail.com',  '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq'),   -- id=10
('judy',  'judy@mail.com',  '$2a$12$1hBdPF7YNmYHXnL8pGFpVOdblgnqQJEdf9plJQdyuel8oarnOnRJq');   -- id=11


-- =====================================
--   CHANNELS
-- =====================================
-- DM channels (1-10):
--   1:  alice  <-> bob    both accepted
--   2:  alice  <-> carol  both accepted
--   3:  bob    <-> dave   bob accepted, dave PENDING   → outgoing for bob
--   4:  carol  <-> eve    both accepted
--   5:  dave   <-> frank  dave accepted, frank PENDING → outgoing for dave
--   6:  eve    <-> grace  eve accepted, grace REJECTED → dead (1 accepted, 0 pending, won't show anywhere - this is the reject-leaves-creator-alone edge case)
--   7:  frank  <-> heidi  both accepted
--   8:  grace  <-> ivan   grace accepted, ivan PENDING → outgoing for grace
--   9:  heidi  <-> judy   both accepted
--   10: ivan   <-> alice  both accepted
--
-- Group channels (11-18):
--   11: Group Alpha  — alice created, alice+bob+carol+dave accepted
--   12: Group Beta   — bob created, bob accepted, carol PENDING, dave REJECTED → outgoing for bob
--   13: Group Gamma  — carol created, carol accepted, eve PENDING, frank REJECTED → outgoing for carol
--   14: Group Delta  — dave created, dave+grace accepted, heidi PENDING
--   15: Group Omega  — eve created, eve accepted, ivan REJECTED → dead
--   16: Group Sigma  — frank created, frank accepted, judy PENDING → outgoing for frank
--   17: Group Lambda — grace created, grace accepted, alice REJECTED → dead
--   18: Group Zeta   — heidi created, heidi accepted, bob PENDING → outgoing for heidi

INSERT INTO channels (id_channel, title, created_at, created_by) VALUES
(1,  'DM alice-bob',   '2026-02-14T10:00:00Z', 2),
(2,  'DM alice-carol', '2026-02-14T10:05:00Z', 2),
(3,  'DM bob-dave',    '2026-02-14T10:10:00Z', 3),
(4,  'DM carol-eve',   '2026-02-14T10:15:00Z', 4),
(5,  'DM dave-frank',  '2026-02-14T10:20:00Z', 5),
(6,  'DM eve-grace',   '2026-02-14T10:25:00Z', 6),
(7,  'DM frank-heidi', '2026-02-14T10:30:00Z', 7),
(8,  'DM grace-ivan',  '2026-02-14T10:35:00Z', 8),
(9,  'DM heidi-judy',  '2026-02-14T10:40:00Z', 9),
(10, 'DM ivan-alice',  '2026-02-14T10:45:00Z', 10),
(11, 'Group Alpha',    '2026-02-14T11:00:00Z', 2),
(12, 'Group Beta',     '2026-02-14T11:05:00Z', 3),
(13, 'Group Gamma',    '2026-02-14T11:10:00Z', 4),
(14, 'Group Delta',    '2026-02-14T11:15:00Z', 5),
(15, 'Group Omega',    '2026-02-14T11:20:00Z', 6),
(16, 'Group Sigma',    '2026-02-14T11:25:00Z', 7),
(17, 'Group Lambda',   '2026-02-14T11:30:00Z', 8),
(18, 'Group Zeta',     '2026-02-14T11:35:00Z', 9);


-- =====================================
--   MESSAGES (inserted before userChannel so IDs are known)
-- =====================================
-- Only channels with COUNT(accepted) >= 2 get messages:
--   ch1(alice+bob), ch2(alice+carol), ch4(carol+eve), ch7(frank+heidi),
--   ch9(heidi+judy), ch10(ivan+alice), ch11(group, 4 accepted), ch14(dave+grace)
--
-- Channels with < 2 accepted get no messages:
--   ch3(dave pending), ch5(frank pending), ch6(grace rejected),
--   ch8(ivan pending), ch12(carol pending+dave rejected), ch13(eve pending+frank rejected),
--   ch15(ivan rejected), ch16(judy pending), ch17(alice rejected), ch18(bob pending)

-- ── Channel 1: DM alice-bob ── IDs 1-10 ──────────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 1, 'alice joined the chat',      '2026-02-14T10:00:00Z'),  -- id=1
(1, 1, 'bob joined the chat',        '2026-02-14T10:00:00Z'),  -- id=2
(2, 1, 'Hey Bob!',                   '2026-02-14T10:01:00Z'),  -- id=3
(3, 1, 'Hey Alice, what''s up?',     '2026-02-14T10:02:00Z'),  -- id=4
(2, 1, 'Not much, working',          '2026-02-14T10:03:00Z'),  -- id=5
(3, 1, 'Same here',                  '2026-02-14T10:04:00Z'),  -- id=6
(2, 1, 'Coffee later?',              '2026-02-14T10:05:00Z'),  -- id=7
(3, 1, 'Sure, 3pm works',            '2026-02-14T10:06:00Z'),  -- id=8
(2, 1, 'Perfect, see you then',      '2026-02-14T10:07:00Z'),  -- id=9
(3, 1, 'See you!',                   '2026-02-14T10:08:00Z');  -- id=10

-- ── Channel 2: DM alice-carol ── IDs 11-18 ───────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 2, 'alice joined the chat',      '2026-02-14T10:05:00Z'),  -- id=11
(1, 2, 'carol joined the chat',      '2026-02-14T10:05:00Z'),  -- id=12
(2, 2, 'Hi Carol!',                  '2026-02-14T10:06:00Z'),  -- id=13
(4, 2, 'Hey Alice!',                 '2026-02-14T10:07:00Z'),  -- id=14
(2, 2, 'Did you see the report?',    '2026-02-14T10:08:00Z'),  -- id=15
(4, 2, 'Not yet, I''ll check now',   '2026-02-14T10:09:00Z'),  -- id=16
(2, 2, 'Thanks!',                    '2026-02-14T10:10:00Z'),  -- id=17
(4, 2, 'On it',                      '2026-02-14T10:11:00Z');  -- id=18

-- ── Channel 4: DM carol-eve ── IDs 19-26 ─────────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 4, 'carol joined the chat',      '2026-02-14T10:15:00Z'),  -- id=19
(1, 4, 'eve joined the chat',        '2026-02-14T10:15:00Z'),  -- id=20
(4, 4, 'Hey Eve!',                   '2026-02-14T10:16:00Z'),  -- id=21
(6, 4, 'Hi Carol!',                  '2026-02-14T10:17:00Z'),  -- id=22
(4, 4, 'Can you review my PR?',      '2026-02-14T10:18:00Z'),  -- id=23
(6, 4, 'Sure, link?',                '2026-02-14T10:19:00Z'),  -- id=24
(4, 4, 'github.com/pr/42',           '2026-02-14T10:20:00Z'),  -- id=25
(6, 4, 'On it!',                     '2026-02-14T10:21:00Z');  -- id=26

-- ── Channel 7: DM frank-heidi ── IDs 27-34 ───────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 7, 'frank joined the chat',      '2026-02-14T10:30:00Z'),  -- id=27
(1, 7, 'heidi joined the chat',      '2026-02-14T10:30:00Z'),  -- id=28
(7, 7, 'Hey Heidi',                  '2026-02-14T10:31:00Z'),  -- id=29
(9, 7, 'Hi Frank!',                  '2026-02-14T10:32:00Z'),  -- id=30
(7, 7, 'Meeting tomorrow still on?', '2026-02-14T10:33:00Z'),  -- id=31
(9, 7, 'Yes, 10am',                  '2026-02-14T10:34:00Z'),  -- id=32
(7, 7, 'Great, I''ll prepare slides','2026-02-14T10:35:00Z'),  -- id=33
(9, 7, 'Perfect, see you then',      '2026-02-14T10:36:00Z');  -- id=34

-- ── Channel 9: DM heidi-judy ── IDs 35-40 ────────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 9, 'heidi joined the chat',      '2026-02-14T10:40:00Z'),  -- id=35
(1, 9, 'judy joined the chat',       '2026-02-14T10:40:00Z'),  -- id=36
(9, 9, 'Hey Judy!',                  '2026-02-14T10:41:00Z'),  -- id=37
(11,9, 'Hey Heidi!',                 '2026-02-14T10:42:00Z'),  -- id=38
(9, 9, 'Lunch today?',               '2026-02-14T10:43:00Z'),  -- id=39
(11,9, 'Sure, noon works!',          '2026-02-14T10:44:00Z');  -- id=40

-- ── Channel 10: DM ivan-alice ── IDs 41-46 ───────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 10, 'ivan joined the chat',      '2026-02-14T10:45:00Z'),  -- id=41
(1, 10, 'alice joined the chat',     '2026-02-14T10:45:00Z'),  -- id=42
(10,10, 'Hi Alice!',                 '2026-02-14T10:46:00Z'),  -- id=43
(2, 10, 'Hey Ivan!',                 '2026-02-14T10:47:00Z'),  -- id=44
(10,10, 'Can we sync tomorrow?',     '2026-02-14T10:48:00Z'),  -- id=45
(2, 10, 'Sure, morning works',       '2026-02-14T10:49:00Z');  -- id=46

-- ── Channel 11: Group Alpha ── IDs 47-58 ─────────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 11, 'alice joined the chat',     '2026-02-14T11:00:00Z'),  -- id=47
(1, 11, 'bob joined the chat',       '2026-02-14T11:00:00Z'),  -- id=48
(1, 11, 'carol joined the chat',     '2026-02-14T11:00:00Z'),  -- id=49
(1, 11, 'dave joined the chat',      '2026-02-14T11:00:00Z'),  -- id=50
(2, 11, 'Welcome everyone!',         '2026-02-14T11:01:00Z'),  -- id=51
(3, 11, 'Thanks for the invite!',    '2026-02-14T11:02:00Z'),  -- id=52
(4, 11, 'Happy to be here',          '2026-02-14T11:03:00Z'),  -- id=53
(5, 11, 'Hey all',                   '2026-02-14T11:04:00Z'),  -- id=54
(2, 11, 'Let''s get started',        '2026-02-14T11:05:00Z'),  -- id=55
(3, 11, 'What''s on the agenda?',    '2026-02-14T11:06:00Z'),  -- id=56
(4, 11, 'I''ll share the doc',       '2026-02-14T11:07:00Z'),  -- id=57
(5, 11, 'Looking forward to it',     '2026-02-14T11:08:00Z');  -- id=58

-- ── Channel 14: Group Delta ── IDs 59-64 ─────────────────────────────────────
INSERT INTO messages (id_user, id_channel, body, timestamp) VALUES
(1, 14, 'dave joined the chat',      '2026-02-14T11:15:00Z'),  -- id=59
(1, 14, 'grace joined the chat',     '2026-02-14T11:15:00Z'),  -- id=60
(5, 14, 'Hey Grace, welcome!',       '2026-02-14T11:16:00Z'),  -- id=61
(8, 14, 'Thanks Dave!',              '2026-02-14T11:17:00Z'),  -- id=62
(5, 14, 'Heidi should join soon',    '2026-02-14T11:18:00Z'),  -- id=63
(8, 14, 'Great, looking forward',    '2026-02-14T11:19:00Z');  -- id=64


-- =====================================
--   USER-CHANNEL MEMBERSHIPS
-- =====================================
-- Inserted AFTER messages so last_read_id_message references real IDs.
-- NULL = user never opened/has not read anything in this channel yet.
-- Unread counts are noted in comments for easy verification.

-- ── Channel 1: DM alice-bob ── last msg=10 ───────────────────────────────────
-- alice caught up (0 unread) | bob has 2 unread (id 9,10)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(2, 1, 1, '2026-02-14T10:00:00Z', 10),  -- alice: read all
(3, 1, 1, '2026-02-14T10:00:00Z',  8);  -- bob: 2 unread (9,10)

-- ── Channel 2: DM alice-carol ── last msg=18 ─────────────────────────────────
-- alice has 2 unread (17,18) | carol caught up (0 unread)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(2, 2, 1, '2026-02-14T10:05:00Z', 15),  -- alice: 3 unread (16,17,18)
(4, 2, 1, '2026-02-14T10:05:00Z', 18);  -- carol: read all

-- ── Channel 3: DM bob-dave ── no messages (dave pending) ─────────────────────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(3, 3, 1, '2026-02-14T10:10:00Z', NULL),  -- bob (creator, waiting)
(5, 3, 0, NULL,                   NULL);  -- dave pending

-- ── Channel 4: DM carol-eve ── last msg=26 ───────────────────────────────────
-- carol caught up (0 unread) | eve has 2 unread (25,26)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(4, 4, 1, '2026-02-14T10:15:00Z', 26),  -- carol: read all
(6, 4, 1, '2026-02-14T10:15:00Z', 23);  -- eve: 3 unread (24,25,26)

-- ── Channel 5: DM dave-frank ── no messages (frank pending) ──────────────────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(5, 5, 1, '2026-02-14T10:20:00Z', NULL),  -- dave (creator, waiting)
(7, 5, 0, NULL,                   NULL);  -- frank pending

-- ── Channel 6: DM eve-grace ── no messages (grace rejected) ──────────────────
-- grace rejected so count(accepted)=1, count(pending)=0
-- doesn't appear in channels OR outgoing invitations for eve
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(6, 6, 1, '2026-02-14T10:25:00Z', NULL),  -- eve (creator)
(8, 6, 2, '2026-02-14T10:26:00Z', NULL);  -- grace rejected

-- ── Channel 7: DM frank-heidi ── last msg=34 ─────────────────────────────────
-- frank has 2 unread (33,34) | heidi caught up (0 unread)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(7, 7, 1, '2026-02-14T10:30:00Z', 31),  -- frank: 3 unread (32,33,34)
(9, 7, 1, '2026-02-14T10:30:00Z', 34);  -- heidi: read all

-- ── Channel 8: DM grace-ivan ── no messages (ivan pending) ───────────────────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(8, 8, 1, '2026-02-14T10:35:00Z', NULL),  -- grace (creator, waiting)
(10,8, 0, NULL,                   NULL);  -- ivan pending

-- ── Channel 9: DM heidi-judy ── last msg=40 ──────────────────────────────────
-- heidi caught up (0 unread) | judy has 2 unread (39,40)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(9,  9, 1, '2026-02-14T10:40:00Z', 40),  -- heidi: read all
(11, 9, 1, '2026-02-14T10:40:00Z', 37);  -- judy: 3 unread (38,39,40)

-- ── Channel 10: DM ivan-alice ── last msg=46 ─────────────────────────────────
-- ivan has 2 unread (45,46) | alice caught up (0 unread)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(10,10, 1, '2026-02-14T10:45:00Z', 43),  -- ivan: 3 unread (44,45,46)
(2, 10, 1, '2026-02-14T10:45:00Z', 46);  -- alice: read all

-- ── Channel 11: Group Alpha ── last msg=58 ───────────────────────────────────
-- alice caught up (0 unread) | bob has 2 unread (57,58) | carol has 1 unread (58) | dave caught up (0 unread)
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(2, 11, 1, '2026-02-14T11:00:00Z', 58),  -- alice: read all
(3, 11, 1, '2026-02-14T11:00:00Z', 55),  -- bob: 3 unread (56,57,58)
(4, 11, 1, '2026-02-14T11:00:00Z', 57),  -- carol: 1 unread (58)
(5, 11, 1, '2026-02-14T11:00:00Z', 58);  -- dave: read all

-- ── Channel 12: Group Beta ── no messages (carol pending, dave rejected) ──────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(3, 12, 1, '2026-02-14T11:05:00Z', NULL),  -- bob (creator)
(4, 12, 0, NULL,                   NULL),   -- carol pending
(5, 12, 2, '2026-02-14T11:06:00Z', NULL);  -- dave rejected

-- ── Channel 13: Group Gamma ── no messages (eve pending, frank rejected) ──────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(4, 13, 1, '2026-02-14T11:10:00Z', NULL),  -- carol (creator)
(6, 13, 0, NULL,                   NULL),   -- eve pending
(7, 13, 2, '2026-02-14T11:11:00Z', NULL);  -- frank rejected

-- ── Channel 14: Group Delta ── last msg=64 ───────────────────────────────────
-- dave caught up (0 unread) | grace has 2 unread (63,64) | heidi pending NULL
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(5, 14, 1, '2026-02-14T11:15:00Z', 64),  -- dave: read all
(8, 14, 1, '2026-02-14T11:15:00Z', 61),  -- grace: 3 unread (62,63,64)
(9, 14, 0, NULL,                   NULL); -- heidi pending

-- ── Channel 15: Group Omega ── no messages (ivan rejected) ───────────────────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(6,  15, 1, '2026-02-14T11:20:00Z', NULL),  -- eve (creator)
(10, 15, 2, '2026-02-14T11:21:00Z', NULL);  -- ivan rejected

-- ── Channel 16: Group Sigma ── no messages (judy pending) ────────────────────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(7,  16, 1, '2026-02-14T11:25:00Z', NULL),  -- frank (creator)
(11, 16, 0, NULL,                   NULL);  -- judy pending

-- ── Channel 17: Group Lambda ── no messages (alice rejected) ─────────────────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(8, 17, 1, '2026-02-14T11:30:00Z', NULL),  -- grace (creator)
(2, 17, 2, '2026-02-14T11:31:00Z', NULL);  -- alice rejected

-- ── Channel 18: Group Zeta ── no messages (bob pending) ──────────────────────
INSERT INTO userChannel (id_user, id_channel, membership, responded_at, last_read_id_message) VALUES
(9, 18, 1, '2026-02-14T11:35:00Z', NULL),  -- heidi (creator)
(3, 18, 0, NULL,                   NULL);  -- bob pending