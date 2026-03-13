# WizzMania — API Documentation

> Protocol reference for the WizzMania client ↔ server communication.  
> For setup, architecture, and how to run the project, see the [README](../README.md).

---

## Table of Contents

- [Authentication](#authentication)
- [HTTP Endpoints](#http-endpoints)
- [WebSocket](#websocket)
- [Data Structures](#data-structures)
- [Database Schema](#database-schema)

---

## Authentication

The server uses **JWT** tokens valid for **7 days**.

1. `POST /login` → server returns a token
2. All protected HTTP routes require the header: `X-Auth-Token: <token>`
3. WebSocket: the first message after connecting must be `WS_AUTH (type 0)` containing the token — all other messages are rejected until this succeeds

---

## HTTP Endpoints

### `POST /login`

No authentication required.

**Request body:**
```json
{ "username": "alice", "password": "hunter2" }
```

**Response 200:**
```json
{
  "success": true,
  "message": "Login successful",
  "id_user": 3,
  "username": "alice",
  "token": "eyJhbG..."
}
```

**Response 401:**
```json
{ "success": false, "message": "Invalid username or password" }
```

---

### `POST /register`

No authentication required.

**Request body:**
```json
{ "username": "alice", "email": "alice@example.com", "password": "S3cur3!" }
```

Password rules: min 8 characters, at least one uppercase, one lowercase, one digit, one special character.

**Response 201:**
```json
{ "success": true, "message": "Account created successfully", "id_user": 13 }
```

**Possible errors:**

| Code | Message |
|------|---------|
| 400 | Missing username, email or password |
| 400 | Invalid email format |
| 400 | Password must be at least 8 characters with uppercase, lowercase and special character |
| 400 | Password contains invalid character |
| 409 | Username already taken |
| 409 | Email already in use |

---

### `POST /logout`

🔒 Authentication required.

Closes the WebSocket connection associated with the token used in this request.

**Response 200:** *(empty body)*

---

### `DELETE /account`

🔒 Authentication required.

Permanently deletes the account. Cascade effects handled server-side:
- Messages are preserved but `id_user` is set to `NULL`
- Channel memberships are removed
- **Group channels with other accepted members** → channel survives, `created_by` transfers to the earliest-responding remaining accepted member (falls back to System user `id_user = 1`)
- All active WebSocket sessions for this user are closed

Other channel members receive a `USER_LEFT (107)` WS notification for each affected channel. Pending invitation recipients receive a `CANCEL_INVITATION (22)` WS notification.

**Response 200:**
```json
{ "success": true, "message": "Account deleted successfully" }
```

**Response 500:** if the database transaction fails — all changes are rolled back, the account is not deleted.

---

### `POST /channels`

🔒 Authentication required.

Creates a new channel. The creator is automatically a participant. All other listed users receive a `CHANNEL_INVITATION (103)` WS notification. The creator's other connected devices receive a `CHANNEL_CREATED (102)` WS notification.

If a channel already exists with exactly the same set of participants, no new channel is created.

**Request body:**
```json
{
  "usernames": ["bob", "carol"],
  "title": "optional title"
}
```

If `title` is omitted, one is generated automatically from participant usernames.

**Response 201** — channel created:
```json
{
  "type": 102,
  "id_channel": 7,
  "already_existed": false,
  "channel": { ...ChannelInfo }
}
```

**Response 409** — channel already existed:
```json
{
  "type": 102,
  "id_channel": 4,
  "already_existed": true
}
```

**Response 400:** `"Invalid username"` if any username is not found.

---

### `POST /channels/:id_channel/messages`

🔒 Authentication required.

Sends a message to a channel. The message is broadcast to all channel members via WebSocket (`NEW_MESSAGE (101)`). The sender's other connected devices also receive the broadcast; the requesting session receives only the HTTP response.

**Request body:**
```json
{ "body": "Hello!" }
```

**Response 201:**
```json
{
  "type": 101,
  "id_channel": 1,
  "message": {
    "id_message": 42,
    "id_sender": 3,
    "body": "Hello!",
    "timestamp": "2026-02-26T10:30:00Z",
    "is_system": false
  }
}
```

**Possible errors:**

| Code | Message |
|------|---------|
| 400 | Invalid SEND_MESSAGE format |
| 401 | User has no permission to SEND_MESSAGE to this channel |

---

### `PATCH /channels/:id_channel/read`

🔒 Authentication required.

Marks messages as read up to and including the given message ID. Updates `last_read_id_message` in the database and recalculates `unread_count`. The user's other connected devices receive the result via WebSocket.

**Request body:**
```json
{ "last_id_message": 42 }
```

**Response 200:**
```json
{ "type": 16, "id_channel": 1, "last_id_message": 42, "unread_count": 0 }
```

**Possible errors:**

| Code | Message |
|------|---------|
| 400 | Invalid MARK_AS_READ format |
| 401 | User has no permission to MARK_AS_READ in this channel |

---

### `POST /channels/:id_channel/wizz`

🔒 Authentication required.

Sends a Wizz to a channel. Ephemeral — not stored in the database. Triggers a window shake animation on the client. All other channel members and the sender's other connected devices receive a `WIZZ (21)` WS notification. The requesting session receives only the HTTP response.

**No request body required.**

**Response 200:**
```json
{ "type": 21, "id_channel": 1, "id_user": 3 }
```

**Response 403:** `"No access to this channel"`

---

### `GET /channels/:id_channel/history`

🔒 Authentication required.

Returns paginated message history for a channel.

**Query parameters:**

| Parameter | Type | Default | Description |
|-----------|------|---------|-------------|
| `before_id` | integer | — | Fetch messages older than this message ID (for pagination) |
| `limit` | integer | 50 | Number of messages to return (max 100) |

**Response 200:**
```json
{
  "type": 114,
  "id_channel": 1,
  "messages": [ ...Message ],
  "has_more": true
}
```

**Response 401:** `"User has no permission to GET_HISTORY for this channel"`

---

### `PATCH /invitations/:id_channel/accept`

🔒 Authentication required.

Accepts a pending invitation. The user is added to the channel. Existing channel members receive a `USER_JOINED (106)` WS notification. The accepting user's other devices receive the full `INVITATION_ACCEPTED (104)` payload. A system message is posted in the channel.

**Response 200:**
```json
{
  "type": 104,
  "channel": { ...ChannelInfo }
}
```

**Response 403:** `"No pending invitation for this channel"`

---

### `PATCH /invitations/:id_channel/reject`

🔒 Authentication required.

Rejects a pending invitation. The inviter receives an `INVITATION_REJECTED (105)` WS notification. The rejecting user's other devices also receive it. If the channel still exists after the rejection, a system message is posted.

**Response 200:**
```json
{
  "type": 105,
  "id_channel": 7,
  "contact": { "id_user": 3, "username": "alice" }
}
```

**Response 403:** `"No pending invitation for this channel"`

---

### `PATCH /invitations/:id_channel/cancel`

🔒 Authentication required.

Cancels an invitation. Only the channel creator can do this, and only if no other user has already accepted. The channel is deleted as a result. All pending invitees receive a `CANCEL_INVITATION (22)` WS notification.

**Response 200:**
```json
{
  "type": 22,
  "id_channel": 7,
  "contact": { "id_user": 3, "username": "alice" }
}
```

**Possible errors:**

| Code | Message |
|------|---------|
| 403 | Can't cancel an invitation you didn't create |
| 403 | Can't access this channel |
| 403 | Can't cancel an invitation that was already accepted |

---

### `PATCH /channels/:id_channel/leave`

🔒 Authentication required.

Leaves a channel. Remaining participants receive a `USER_LEFT (107)` WS notification. The leaving user's other devices also receive it. If the leaving user was the last active member (pending + accepted ≤ 1 after leaving), the channel is deleted entirely.

**Response 204:** *(empty body)*

**Response 403:** `"You are not a member of this channel"`

---

## WebSocket

**Connection URL:** `ws://localhost:8888/ws`

All messages are JSON with an integer `type` field. The first message after connecting **must** be `WS_AUTH`, otherwise the connection is closed.

### Message types

#### Client → Server

| Type | Name | Description |
|------|------|-------------|
| 0 | `WS_AUTH` | Authenticate the WebSocket connection |

> All other client-initiated actions (`SEND_MESSAGE`, `MARK_AS_READ`, `WIZZ`, `CREATE_CHANNEL`, `ACCEPT_INVITATION`, `REJECT_INVITATION`, `LEAVE_CHANNEL`, `CANCEL_INVITATION`) are defined in the message type enum but are handled via **HTTP**, not WebSocket.

#### Server → Client

| Type | Name | Description |
|------|------|-------------|
| 100 | `WS_AUTH_SUCCESS` | Authentication successful |
| 101 | `NEW_MESSAGE` | New message broadcast |
| 102 | `CHANNEL_CREATED` | Sent to creator's other devices after channel creation |
| 103 | `CHANNEL_INVITATION` | Incoming invitation |
| 104 | `INVITATION_ACCEPTED` | Sent to accepting user's other devices + existing channel members |
| 105 | `INVITATION_REJECTED` | Someone rejected an invitation |
| 106 | `USER_JOINED` | A user joined a channel |
| 107 | `USER_LEFT` | A user left (or deleted their account) |
| 113 | `INITIAL_DATA` | Sent automatically right after successful `WS_AUTH` |
| 114 | `CHANNEL_HISTORY` | Message history (HTTP only, kept for reference) |
| 22 | `CANCEL_INVITATION` | Invitation cancelled by creator |
| 255 | `ERROR` | Error response |

---

### WS_AUTH (type 0)

**Send:**
```json
{ "type": 0, "token": "eyJhbG..." }
```

**Response — WS_AUTH_SUCCESS (type 100):**
```json
{ "type": 100, "message": "Authentication successful", "id_user": 3 }
```

Immediately followed by `INITIAL_DATA (113)`. On failure, the connection is closed.

---

### INITIAL_DATA (type 113)

Sent automatically after successful authentication. Contains everything the client needs to bootstrap the UI.

```json
{
  "type": 113,
  "contacts": [ ...Contact ],
  "channels": [ ...ChannelInfo ],
  "invitations": [ ...ChannelInvitation ],
  "outgoing_invitations": [ ...ChannelInfo ]
}
```

| Field | Description |
|-------|-------------|
| `contacts` | Users from DM channels (2 accepted members) only — not group channel participants |
| `channels` | Channels where the invitation is accepted |
| `invitations` | Pending incoming invitations |
| `outgoing_invitations` | Channels created by this user where only they have accepted and at least one invitee is still pending |

---

### ERROR (type 255)

```json
{ "type": 255, "error_code": "403", "message": "No access to this channel" }
```

---

## Data Structures

Reused across multiple endpoints and WebSocket messages.

### Contact
```json
{ "id_user": 3, "username": "alice" }
```

### Message
```json
{
  "id_message": 42,
  "id_sender": 3,
  "body": "Hello!",
  "timestamp": "2026-02-26T10:30:00Z",
  "is_system": false
}
```

`id_sender` is `null` if the author's account has been deleted. `is_system` is `true` when `id_sender == 1` (the System user), which is set for automatically generated messages (e.g. *"User @3 joined the chat!"*). It is not stored in the database — the client or server derives it from the sender id.

### ChannelInfo
```json
{
  "id_channel": 7,
  "title": "alice, bob",
  "is_group": true,
  "created_by": 3,
  "participants": [ ...Contact ],
  "last_message": { ...Message },
  "unread_count": 5,
  "last_read_id_message": 37
}
```

### ChannelInvitation
```json
{
  "type": 103,
  "id_channel": 7,
  "id_inviter": 3,
  "other_participant_ids": [ ...Contact ],
  "title": "alice, bob, carol"
}
```

---

## Database Schema

### `users`

| Column | Type | Notes |
|--------|------|-------|
| `id_user` | BIGINT PK AUTO_INCREMENT | `id_user = 1` is the System user (non-login) |
| `username` | VARCHAR(100) | Unique |
| `email` | VARCHAR(255) | Unique |
| `password` | VARCHAR(255) | Hashed |

### `channels`

| Column | Type | Notes |
|--------|------|-------|
| `id_channel` | BIGINT PK AUTO_INCREMENT | |
| `title` | VARCHAR(60) | |
| `created_at` | VARCHAR(24) | ISO 8601 timestamp |
| `created_by` | BIGINT FK → users (nullable) | On user deletion: transferred to the earliest-responding remaining accepted member, or System user (`id_user = 1`) if none exists |

### `userChannel`

| Column | Type | Notes |
|--------|------|-------|
| `id_userChannel` | BIGINT PK AUTO_INCREMENT | |
| `id_user` | BIGINT FK → users (nullable) | `CASCADE` on user deletion |
| `id_channel` | BIGINT FK → channels | `CASCADE` on channel deletion |
| `membership` | TINYINT | 0=pending, 1=accepted, 2=rejected, 3=left |
| `responded_at` | VARCHAR(24) | ISO 8601 timestamp, NULL if not yet responded |
| `last_read_id_message` | BIGINT | Used to compute `unread_count`, NULL if never read |

### `messages`

| Column | Type | Notes |
|--------|------|-------|
| `id_message` | BIGINT PK AUTO_INCREMENT | |
| `id_user` | BIGINT FK → users (nullable) | `SET NULL` on user deletion — preserves history |
| `id_channel` | BIGINT FK → channels | `CASCADE` on channel deletion |
| `body` | TEXT | |
| `timestamp` | VARCHAR(24) | ISO 8601 timestamp |