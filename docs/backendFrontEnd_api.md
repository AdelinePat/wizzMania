# WizzMania — Backend API Documentation

> Technical documentation for the C++ server (Crow) for developers.

---

## Table of Contents

- [Docker Commands](#docker-commands)
- [Server Architecture](#server-architecture)
- [Authentication](#authentication)
- [HTTP Endpoints](#http-endpoints)
- [WebSocket Messages](#websocket-messages)
- [Database](#database)

---

## Docker Commands

### Launch the project

```bash
# Build and run server + database
docker compose up --build

# In the background
docker compose up --build -d
```

### Logs

```bash
# All logs
docker compose logs -f

# Server logs only
docker compose logs -f server

# Database logs only
docker compose logs -f mysql-db
```

### Rebuild after server code changes

```bash
# Rebuild only the server (faster)
docker compose up --build server

# Full rebuild
docker compose down && docker compose up --build
```

### Stop

```bash
# Stop containers (data preserved)
docker compose down

# Stop and delete MySQL volume (reset database)
docker compose down -v
```

### Database access

```bash
# Interactive MySQL shell
docker compose exec mysql-db sh -c 'mysql -h localhost -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" "$MYSQL_DATABASE"'

# Execute a query directly
docker compose exec mysql-db sh -c 'mysql -h localhost -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" "$MYSQL_DATABASE" -e "SELECT * FROM users;"'

# Execute an SQL file
docker compose exec mysql-db sh -c 'mysql -h localhost -u "$MYSQL_USER" -p"$MYSQL_PASSWORD" "$MYSQL_DATABASE" < /queries.sql'
```

### Troubleshooting

```bash
# Check container status
docker compose ps -a

# Check which ports are in use (Windows)
netstat -ano | findstr 3306
netstat -ano | findstr 8888

# Recreate containers from scratch
docker compose down -v
docker compose up --build

# If port 3306 is already in use (local MySQL)
# → Change DB_PORT in .env or stop the local MySQL service
```

### Environment Variables (.env)

The `.env` file in the root directory configures the server and database:

```dotenv
MYSQL_ROOT_PASSWORD=...
MYSQL_DATABASE=wizzmania
MYSQL_USER=wizzuser
MYSQL_PASSWORD=...
SERVER_PORT=8888
DB_PORT=3306
SERVER_IP=127.0.0.1
SECRET_KEY=...
```

> Copy `.env.template` to `.env` and fill in the values before the first launch.

---

## Server Architecture

```
server/
├── headers/
│   ├── auth/            → JWT authentication (controller + service)
│   ├── channel/         → Channel management (controller + service)
│   ├── invitation/      → Channel invitations (controller + service)
│   ├── message/         → Messages + WIZZ (controller + service)
│   ├── user/            → Users: login, register, delete (controller + service)
│   ├── database.hpp     → Database interface
│   ├── exception.hpp    → HTTP exceptions (400, 401, 403, 404, 409, 500)
│   ├── json_helpers.hpp → JSON parsing/serialization for all message types
│   ├── websocket_manager.hpp → WebSocket connection management
│   └── server.hpp
├── sources/
│   ├── database/        → Database implementation by domain (users, channels, messages, invitations)
│   └── ...              → Controller and service implementations
└── CMakeLists.txt
```

**Pattern:** Route (server.cpp) → Controller → Service → Database

Each controller handles errors using `try/catch` and returns the appropriate exception via `WizzManiaError::send_ws_error()` (WebSocket) or `WizzManiaError::send_http_error()` (HTTP).

---

## Authentication

The server uses **JWT** (JSON Web Token) for authentication.

**Flow:**

1. The client sends a `POST /login` request with username + password
2. The server returns a JWT token
3. For protected routes: `X-Auth-Token: <token>` header
4. For WebSocket: the first message after connection must be `WS_AUTH` with the token

---

## HTTP Endpoints

### `POST /login`

User authentication.

| Field | Type | Required |
|-------|------|----------|
| `username` | string | ✓ |
| `password` | string | ✓ |

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

**Error 401:** `{"message": "Invalid username or password", "success": false}`

---

### `POST /register`

Creates a new account. No authentication required.

| Field | Type | Required | Validation |
|-------|------|----------|------------|
| `username` | string | ✓ | Allowed characters, automatic cleanup |
| `email` | string | ✓ | Valid email format |
| `password` | string | ✓ | Min 8 chars, uppercase, lowercase, special character |

**Response 201:**
```json
{
  "success": true,
  "message": "Account created successfully",
  "id_user": 13
}
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

### `DELETE /account`

Permanent account deletion. **Authentication required** (`X-Auth-Token`).

**Cascade deletion:**
1. `UPDATE messages SET id_user = NULL` — preserves message history
2. `DELETE FROM userChannel` — removes channel memberships
3. `UPDATE channels SET created_by = 1` — transfers ownership to the System user
4. `DELETE FROM users` — deletes the account

The server automatically disconnects all of the user's active WebSocket sessions.

**Response 200:**
```json
{
  "success": true,
  "message": "Account deleted successfully"
}
```

---

### `GET /channels/:id/messages`

Retrieves the message history of a channel. **Authentication required.**

**Query params:** `?before_id=X&limit=50`

---

### `POST /channels`

Creates a new channel. **Authentication required.**

---

### `POST /channels/:id/invite`

Invites a user to a channel. **Authentication required.**

---

### `POST /channels/:id/leave`

Leaves a channel. **Authentication required.**

---

## WebSocket Messages

Connection: `ws://localhost:8888/ws`

All messages are JSON and include a `type` field (integer) that identifies the message type.

### Message Types

#### Client → Server

| Type | Name | Description |
|------|------|-------------|
| 0 | `WS_AUTH` | WebSocket authentication with JWT token |
| 1 | `LOGOUT` | Graceful disconnect |
| 10 | `SEND_MESSAGE` | Send a message to a channel |
| 11 | `CREATE_CHANNEL` | Create a channel |
| 12 | `ACCEPT_INVITATION` | Accept a channel invitation |
| 13 | `REJECT_INVITATION` | Reject a channel invitation |
| 14 | `LEAVE_CHANNEL` | Leave a channel |
| 15 | `UPDATE_CHANNEL_TITLE` | Update channel title |
| 16 | `MARK_AS_READ` | Mark messages as read |
| 19 | `REQUEST_CHANNEL_HISTORY` | Request channel message history |
| 20 | `CHANNEL_OPEN` | Open a specific channel |
| 21 | `WIZZ` | Send a WIZZ (screen shake) |

#### Server → Client

| Type | Name | Description |
|------|------|-------------|
| 100 | `WS_AUTH_SUCCESS` | Authentication successful |
| 101 | `NEW_MESSAGE` | New message in a channel |
| 102 | `CHANNEL_CREATED` | Channel created successfully |
| 103 | `CHANNEL_INVITATION` | Invitation notification |
| 104 | `INVITATION_ACCEPTED` | Someone accepted the invitation |
| 105 | `INVITATION_REJECTED` | Someone rejected the invitation |
| 106 | `USER_JOINED` | A user joined the channel |
| 107 | `USER_LEFT` | A user left the channel |
| 110 | `TITLE_UPDATED` | Channel title changed |
| 111 | `USER_STATUS` | User online/offline status |
| 113 | `INITIAL_DATA` | Initial data (all channels) |
| 114 | `CHANNEL_HISTORY` | Message history |
| 255 | `ERROR` | Error response |

---

### Key Message Details

#### WS_AUTH (type 0)

Mandatory first message after WebSocket connection.

```json
{ "type": 0, "token": "eyJhbG..." }
```

**Response (type 100):**
```json
{ "type": 100, "id_user": 3 }
```

---

#### SEND_MESSAGE (type 10)

```json
{ "type": 10, "id_channel": 1, "body": "Hello!" }
```

**Broadcast (type 101):**
```json
{
  "type": 101,
  "id_message": 42,
  "id_channel": 1,
  "id_user": 3,
  "body": "Hello!",
  "timestamp": "2026-02-26 10:30:00"
}
```

---

#### MARK_AS_READ (type 16)

```json
{ "type": 16, "id_channel": 1, "last_id_message": 42 }
```

**Broadcast (type 16):** sent back to all of the user's sessions with updated `unread_count`.

```json
{
  "type": 16,
  "id_channel": 1,
  "last_id_message": 42,
  "unread_count": 0
}
```

---

#### WIZZ (type 21)

Feature inspired by MSN Messenger. Ephemeral — no database storage.

**Send:**
```json
{ "type": 21, "id_channel": 1 }
```

**Broadcast to all participants:**
```json
{ "type": 21, "id_channel": 1, "id_user": 3 }
```

The client triggers a window shake animation (QPropertyAnimation) upon reception.

---

#### ERROR (type 255)

```json
{
  "type": 255,
  "error_code": "403",
  "message": "No access to this channel"
}
```

---

## Database

### Tables

**users**

| Column | Type | Description |
|--------|------|-------------|
| id_user | BIGINT PK AUTO_INCREMENT | Unique identifier |
| username | VARCHAR | Unique username |
| email | VARCHAR | Unique email |
| password | VARCHAR | Hashed password |

**channels**

| Column | Type | Description |
|--------|------|-------------|
| id_channel | BIGINT PK AUTO_INCREMENT | Unique identifier |
| title | VARCHAR | Channel name |
| created_by | BIGINT FK → users | Channel creator |

**userChannel**

| Column | Type | Description |
|--------|------|-------------|
| id_userChannel | BIGINT PK AUTO_INCREMENT | Unique identifier |
| id_user | BIGINT FK → users | User |
| id_channel | BIGINT FK → channels | Channel |
| accepted | BOOLEAN | Invitation status |
| last_read_id_message | INT | Last read message (for unread count) |

**messages**

| Column | Type | Description |
|--------|------|-------------|
| id_message | BIGINT PK AUTO_INCREMENT | Unique identifier |
| id_user | BIGINT FK → users (nullable) | Author (NULL if account deleted) |
| id_channel | BIGINT FK → channels | Channel |
| timestamp | DATETIME | Send date |
| body | TEXT | Message content |

### Constraints

- `users.username` and `users.email` are unique
- `messages.id_user` is nullable (preserves message history after account deletion)
- `channels.created_by` references `users.id_user` — transferred to System user (id=1) on deletion
- FK constraints are `ON DELETE NO ACTION` — cascade deletion is handled manually in `database_users.cpp`

