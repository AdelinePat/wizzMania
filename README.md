# wizzMania
Second year IT Bachelor group project : create a chat application with C++ and Qt


## 🔁 Flow : Sequence diagram
Flow of login to sending a message. Only one server, one DB and possible multiple clients
```mermaid
sequenceDiagram
        participant Client@{"type": "collections"}
        participant Server@{"type": "control"}
        participant DB@{"type": "database"}
        Note over Client: Get input + hash password
        Client->>Server: login
        Note over Server: treat/clean input
        Server<<->>DB: checks
        Note over DB: checks if user id and password exists and matchs
        Note over Server: accept login
        Server->>Client: Send dashboard data
        Note over Client: display dashboard
        Note left of Client: User click on a channel
        Client->>Server: Ask for a channel
        Server<<->>DB: retrieve channel and messages
        Server->>Client: send all messages from channel
        Note over Client: display channel messages 
        Note left of Client: User write and send message
        Note over Client: user id, channel id, timestamp, text
        Client->>Server: Send message 
        Server->>DB: Save message in db
        DB->>Server: Send all message from channel id
        Server->>Client: send all messages from channel
        Note over Client: display channel's messages
```

##  🗄️ Database : Physical Data Model

```mermaid
erDiagram
    USERS {
        bigint id_user PK
        varchar username
        varchar email
        varchar password
    }

    CHANNELS {
        bigint id_channel PK
        varchar title
    }

    USER_CHANNEL {
        bigint id_userChannel PK
        bigint id_user FK
        bigint id_channel FK
    }

    MESSAGES {
        bigint id_message PK
        bigint id_user FK
        bigint id_channel FK
        datetime timestamp
        text body
    }

    %% Users -> User_Channel (0..* per user)
    USERS ||--o{ USER_CHANNEL : "accesses"

    %% Channels -> User_Channel (1..* per channel)
    CHANNELS ||--|{ USER_CHANNEL : "lists"

    %% Users -> Messages (0..* per user)
    USERS ||--o{ MESSAGES : "writes"

    %% Channels -> Messages (0..* per channel)
    CHANNELS ||--o{ MESSAGES : "contains"
```
