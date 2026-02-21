#include "database.hpp"

Database::Database() {
  std::string db_host =
      std::getenv("DB_HOST") ? std::getenv("DB_HOST") : "mysql-db";
  std::string db_user =
      std::getenv("DB_USER") ? std::getenv("DB_USER") : "root";
  std::string db_pass =
      std::getenv("DB_PASSWORD") ? std::getenv("DB_PASSWORD") : "root_password";
  std::string db_name =
      std::getenv("DB_NAME") ? std::getenv("DB_NAME") : "wizzmania";
  try {
    this->driver = get_driver_instance();
    this->start_connection(db_host, db_user, db_pass, db_name);
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] ERROR: " << e.what() << std::endl;
    std::cerr << "[DB] Error code: " << e.getErrorCode() << std::endl;
    throw;
  }
}

Database::Database(const std::string& host, const std::string& user,
                   const std::string& password, const std::string& database)
    : host(host), user(user), password(password), database(database) {
  try {
    this->driver = get_driver_instance();
    this->start_connection(host, user, password, database);
    // this->conn.reset(this->driver->connect(host, user, password));
    // this->conn->setSchema(database);
    std::cout << "[DB] Connected to MySQL database: " << database << std::endl;
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] ERROR: " << e.what() << std::endl;
    std::cerr << "[DB] Error code: " << e.getErrorCode() << std::endl;
    throw;
  }
}

Database::~Database() {
  if (conn) {
    this->conn->close();
  }
}

void Database::start_connection(const std::string& host,
                                const std::string& user,
                                const std::string& password,
                                const std::string& database) {
  conn.reset(driver->connect(host, user, password));
  // change smart pointer ownership driver->connect(host,
  // user, password) is a raw pointer
  conn->setSchema(database);
}

void Database::ensure_connection() {
  try {
    // Test connection with a simple query
    if (!this->conn || this->conn->isClosed()) {
      this->start_connection(this->host, this->user, this->password,
                             this->database);
      std::cout << "[DB] Reconnecting to database..." << std::endl;
    }
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] Reconnection failed: " << e.what() << std::endl;
    throw;
  }
}

// ===== ESSENTIALS =====

int64_t Database::verify_user(const std::string& username,
                              const std::string& password) {
  std::lock_guard<std::mutex> lock(db_mutex);

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT id_user "
                                     "FROM users "
                                     "WHERE username = ? AND password = ?"));
    prep_statement->setString(1, username);
    prep_statement->setString(2, password);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      return res->getInt64("id_user");
    }
    return -1;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] verify_user error: " << e.what() << std::endl;
    return -1;
  }
}

// [HELPER] Get set of participants id for a channel, allow websocket
// broadcasting to this set
std::unordered_set<int64_t> Database::get_channel_participants(
    int64_t id_channel, ChannelStatus membership) {
  std::unordered_set<int64_t> channel_participants;
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT id_user FROM userChannel "
                                     "WHERE id_channel = ? "
                                     "AND membership = ?;"));

    prep_statement->setInt64(1, id_channel);
    prep_statement->setInt(2, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      channel_participants.insert(res->getInt64("id_user"));
    }
    return channel_participants;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channel_participants error: " << e.what()
              << std::endl;
    return channel_participants;
  }
}

bool Database::has_channel_access(int64_t id_user, int64_t id_channel) {
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT 1 FROM userChannel "
                                     "WHERE id_user = ? "
                                     "AND id_channel = ? "
                                     "AND membership = 1;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt64(2, id_channel);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    return res->next();

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] has_channel_access error: " << e.what() << std::endl;
    return false;
  }
}

// ===== GET INITIAL DATA =====

ServerSend::InitialDataResponse Database::get_initial_data(
    const int64_t id_user) {
  ServerSend::InitialDataResponse init_data;
  init_data.contacts = this->get_contacts(id_user);
  init_data.channels = this->get_initial_channels(id_user);
  init_data.invitations = this->get_initial_invitations(id_user);
  init_data.outgoing_invitations = this->get_outgoing_invitations(id_user);
  return init_data;
}

// ===== GET INITIAL CONTACT =====

// Get contact list for a user
std::vector<ServerSend::Contact> Database::get_contacts(
    const int64_t id_user, ChannelStatus membership) {
  std::lock_guard<std::mutex> lock(db_mutex);

  std::vector<ServerSend::Contact> contacts;
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT DISTINCT "
            "u.id_user, u.username "
            "FROM userChannel uc1 "
            "JOIN "
            "userChannel uc2 ON uc1.id_channel = uc2.id_channel "
            "JOIN "
            "users u ON u.id_user = uc2.id_user "
            "WHERE "
            "uc1.id_user = ? "
            "AND uc1.membership = ? "
            "AND uc2.membership = ? "
            "AND uc2.id_user <> ? "
            "AND (SELECT COUNT(*) FROM userChannel uc3 "
            "WHERE uc3.id_channel = uc1.id_channel "
            "AND uc3.membership = ?) = 2;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));
    prep_statement->setInt(3, static_cast<int32_t>(membership));
    prep_statement->setInt64(4, id_user);
    prep_statement->setInt(5, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::Contact contact;
      contact.id_user = res->getInt64("id_user");
      contact.username = res->getString("username");
      contacts.push_back(contact);
    }
    return contacts;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] verify_user error: " << e.what() << std::endl;
    return contacts;
  }
}

// ===== GET INITIAL CHANNEL =====

// Create ChannelInfo list for initial_data
std::vector<ServerSend::ChannelInfo> Database::get_initial_channels(
    const int64_t id_user) {
  std::lock_guard<std::mutex> lock(db_mutex);

  std::vector<ServerSend::ChannelInfo> channels_info =
      this->get_channels(id_user);
  std::map<int64_t, std::vector<ServerSend::Contact>> channel_participants =
      this->get_participants_and_channel(id_user);
  std::map<int64_t, int64_t> channels_unread_count =
      this->get_unread_count(id_user);
  std::map<int64_t, ServerSend::Message> channels_last_message =
      this->get_last_messages(id_user);

  for (ServerSend::ChannelInfo& channel : channels_info) {
    auto it_participant = channel_participants.find(channel.id_channel);
    if (it_participant != channel_participants.end()) {
      channel.participants = it_participant->second;
      channel.is_group = channel.participants.size() > 2;
    } else {
      std::cerr << "[DB] Warning: no participants found for channel "
                << channel.id_channel << "\n";
    }

    auto it_unread_count = channels_unread_count.find(channel.id_channel);
    if (it_unread_count != channels_unread_count.end()) {
      channel.unread_count = it_unread_count->second;
    } else {
      channel.unread_count = 0;
    }

    auto it_last_message = channels_last_message.find(channel.id_channel);
    if (it_last_message != channels_last_message.end()) {
      channel.last_message = it_last_message->second;
    } else {
      std::cerr << "[DB] Warning: no last message found for channel "
                << channel.id_channel << "\n";
    }
  }
  return channels_info;
}

// Get all channels for a specific user , populate most of ChannelInfo struct,
// missing participants vector
std::vector<ServerSend::ChannelInfo> Database::get_channels(
    const int64_t id_user, ChannelStatus membership) {
  std::vector<ServerSend::ChannelInfo> channels_info;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT c.id_channel, c.title, c.created_by, "
            "uc1.last_read_id_message "
            "FROM channels c "
            "JOIN userChannel uc1 ON c.id_channel = uc1.id_channel "
            "WHERE uc1.id_user = ? "
            "AND uc1.membership = ? "
            "AND (SELECT COUNT(*) FROM userChannel uc2 "
            "WHERE uc2.id_channel = c.id_channel "
            "AND uc2.membership = ?) >= 2;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));
    prep_statement->setInt(3, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::ChannelInfo channel;
      channel.id_channel = res->getInt64("id_channel");
      channel.title = res->getString("title");
      channel.created_by = res->getInt64("created_by");
      channel.last_read_id_message = res->getInt64("last_read_id_message");
      channels_info.push_back(channel);
    }
    return channels_info;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channels error: " << e.what() << std::endl;
    return channels_info;
  }
}

// Get all the channel a user participates it and the set of every participants
// from this channel Returns a map<id_channel, vector<ServerSend::Contact>>
std::map<int64_t, std::vector<ServerSend::Contact>>
Database::get_participants_and_channel(const int64_t id_user,
                                       ChannelStatus membership,
                                       ChannelStatus other_membership) {
  std::map<int64_t, std::vector<ServerSend::Contact>> channel_participants;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT DISTINCT uc2.id_user, uc2.id_channel, u.username "
            "FROM userChannel uc1 "
            "JOIN userChannel uc2 ON uc1.id_channel = uc2.id_channel "
            "JOIN users u ON u.id_user = uc2.id_user "
            "WHERE uc1.id_user = ? "
            "AND uc1.membership = ? "
            "AND uc2.membership = ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));
    prep_statement->setInt(3, static_cast<int32_t>(other_membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      int64_t id_channel = res->getInt64("id_channel");
      // int64_t participant = res->getInt64("id_user");
      ServerSend::Contact participant;
      participant.id_user = res->getInt64("id_user");
      participant.username = res->getString("username");
      channel_participants[id_channel].push_back(participant);
    }
    return channel_participants;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_participants_and_channel error: " << e.what()
              << std::endl;
    return channel_participants;
  }
}

// Get unread_count for each channels a user participates in
// Returns map<id_channel, count>
std::map<int64_t, int64_t> Database::get_unread_count(const int64_t id_user) {
  std::map<int64_t, int64_t> unread_counts;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT uc1.id_channel, "
            "COUNT(m1.id_message) AS unread_count "
            "FROM userChannel "
            "uc1 LEFT JOIN messages m1 ON uc1.id_channel = m1.id_channel "
            "WHERE uc1.id_user = ? AND "
            "uc1.last_read_id_message < m1.id_message GROUP BY "
            "uc1.id_channel;"));

    prep_statement->setInt64(1, id_user);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      int64_t id_channel = res->getInt64("id_channel");
      int64_t count = res->getInt64("unread_count");
      unread_counts[id_channel] = count;
    }
    return unread_counts;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_unread_count error: " << e.what() << std::endl;
    return unread_counts;
  }
}

// Get last message of each channels a specific user participates in
// Returns map<id_channel, message>
std::map<int64_t, ServerSend::Message> Database::get_last_messages(
    const int64_t id_user) {
  std::map<int64_t, ServerSend::Message> last_messages;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT uc1.id_channel, m1.id_message, m1.id_user, "
            "COALESCE(m1.body, 'no message yet...') as body, "
            "m1.timestamp FROM userChannel uc1 "
            "LEFT JOIN messages m1 "
            "ON (uc1.id_channel = m1.id_channel) "
            "AND m1.id_message = ( "
            "SELECT MAX(id_message) FROM messages m2 "
            "WHERE uc1.id_channel = m2.id_channel) "
            "WHERE uc1.id_user = ?;"));

    prep_statement->setInt64(1, id_user);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::Message message;
      message.id_message = res->getInt64("id_message");
      message.id_sender = res->getInt64("id_user");
      message.body = res->getString("body");
      message.timestamp = res->getString("timestamp");
      message.is_system = (message.id_sender == 1);

      last_messages[res->getInt64("id_channel")] = message;
      // last_messages.push_back(message);
    }
    return last_messages;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_last_messages error: " << e.what() << std::endl;
    return last_messages;
  }
}

// ===== GET INITIAL INVITATION =====

// Populate ChannelInvitation structs for a user
std::vector<ServerSend::ChannelInvitation> Database::get_initial_invitations(
    const int64_t id_user) {
  std::lock_guard<std::mutex> lock(db_mutex);

  std::vector<ServerSend::ChannelInvitation> channels_invitations =
      this->get_invitations_base(id_user);
  std::map<int64_t, std::vector<ServerSend::Contact>> channel_participants =
      this->get_participants_and_channel(id_user, ChannelStatus::PENDING,
                                         ChannelStatus::ACCEPTED);

  for (ServerSend::ChannelInvitation& channel : channels_invitations) {
    auto it_participant = channel_participants.find(channel.id_channel);
    if (it_participant != channel_participants.end()) {
      channel.other_participant_ids = it_participant->second;
    } else {
      std::cerr << "[DB] Warning: no participants found for channel "
                << channel.id_channel << "\n";
    }
  }

  return channels_invitations;
}

// Populate part of ChannelInvitation structs for a user, missing participants
// vector
std::vector<ServerSend::ChannelInvitation> Database::get_invitations_base(
    const int64_t id_user, ChannelStatus membership) {
  std::vector<ServerSend::ChannelInvitation> channels_invitations;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT c.id_channel, c.created_by, c.title FROM channels c "
            "LEFT JOIN userChannel uc1 ON uc1.id_channel = c.id_channel "
            "WHERE uc1.id_user = ? AND uc1.membership = ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::ChannelInvitation channel;
      channel.id_channel = res->getInt64("id_channel");
      channel.title = res->getString("title");
      channel.id_inviter = res->getInt64("created_by");
      channels_invitations.push_back(channel);
    }
    return channels_invitations;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channels error: " << e.what() << std::endl;
    return channels_invitations;
  }
}

std::vector<ServerSend::ChannelInfo> Database::get_outgoing_invitations(
    int64_t id_user) {
  std::lock_guard<std::mutex> lock(db_mutex);

  std::vector<ServerSend::ChannelInfo> channels_info =
      this->get_outgoing_invitations_base(id_user, ChannelStatus::ACCEPTED);
  std::map<int64_t, std::vector<ServerSend::Contact>> channel_participants =
      this->get_participants_and_channel(id_user, ChannelStatus::ACCEPTED,
                                         ChannelStatus::PENDING);

  for (ServerSend::ChannelInfo& channel : channels_info) {
    auto it_participant = channel_participants.find(channel.id_channel);
    if (it_participant != channel_participants.end()) {
      channel.participants = it_participant->second;
      channel.is_group = channel.participants.size() > 2;
    } else {
      std::cerr << "[DB] Warning: no participants found for channel "
                << channel.id_channel << "\n";
    }
  }
  return channels_info;
}

// Get outgoing invitations, meaning the channel the user created but no one
// accepted yet
std::vector<ServerSend::ChannelInfo> Database::get_outgoing_invitations_base(
    int64_t id_user, ChannelStatus membership) {
  std::vector<ServerSend::ChannelInfo> channels_info;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT c.id_channel, c.title, c.created_by FROM userChannel uc1 "
            "JOIN channels c ON c.id_channel = uc1.id_channel "
            "WHERE  uc1.membership = ? AND c.created_by = ? "
            "AND (SELECT COUNT(*) FROM userChannel uc2 "
            "WHERE uc2.id_channel = c.id_channel "
            "AND uc2.membership = ?) = 1 "
            "AND (SELECT COUNT(*) FROM userChannel uc3 "
            "WHERE uc3.id_channel = c.id_channel "
            "AND uc3.membership = ?) >= 1;"));

    prep_statement->setInt(1, static_cast<int32_t>(membership));
    prep_statement->setInt64(2, id_user);
    prep_statement->setInt(3, static_cast<int32_t>(membership));
    prep_statement->setInt(4, static_cast<int32_t>(ChannelStatus::PENDING));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::ChannelInfo channel;
      channel.id_channel = res->getInt64("id_channel");
      channel.title = res->getString("title");
      channel.created_by = res->getInt64("created_by");
      channel.last_read_id_message = 0;  // nothing read yet!
      channel.unread_count = 0;          // no notification yet either
      channels_info.push_back(channel);
    }
    return channels_info;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channels error: " << e.what() << std::endl;
    return channels_info;
  }
}

// ===== SAVE MESSAGE =====

// Save message in db for the right channel and returns id_message
std::optional<int64_t> Database::save_message(int64_t id_user,
                                              int64_t id_channel,
                                              const std::string& body,
                                              const std::string& timestamp) {
  std::lock_guard<std::mutex> lock(db_mutex);

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "INSERT INTO messages (id_user, id_channel, body, timestamp) "
            "VALUES (?, ?, ?, ?);"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt64(2, id_channel);
    prep_statement->setString(3, body);
    prep_statement->setString(4, timestamp);
    prep_statement->executeUpdate();

    // std::unique_ptr<sql::ResultSet>
    // res(prep_statement->getGeneratedKeys());
    std::unique_ptr<sql::Statement> stmt(this->conn->createStatement());
    std::unique_ptr<sql::ResultSet> res(
        stmt->executeQuery("SELECT LAST_INSERT_ID()"));

    if (res->next()) {
      return res->getInt64(1);
    }
    return std::nullopt;
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] save_message error: " << e.what() << std::endl;
    return std::nullopt;
  }
}

// ===== CHANNELS ESSENTIALS =====

std::vector<ServerSend::Message> Database::get_channel_history(
    int64_t id_channel, int64_t before_id_message, int limit) {
  std::lock_guard<std::mutex> lock(db_mutex);
  std::vector<ServerSend::Message> messages;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT * FROM ( "
            "SELECT id_message, id_user, body, timestamp FROM messages "
            "WHERE id_channel = ? AND (0 = ? OR id_message < ?) "
            "ORDER BY id_message DESC "
            "LIMIT ?) AS recent "
            "ORDER BY id_message ASC;"));

    prep_statement->setInt64(1, id_channel);
    prep_statement->setInt64(2, before_id_message);
    prep_statement->setInt64(3, before_id_message);
    prep_statement->setInt(4, limit);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::Message message =
          create_message(res->getInt64("id_message"), res->getInt64("id_user"),
                         res->getString("body"), res->getString("timestamp"));
      messages.push_back(message);
    }
    return messages;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channel_history error: " << e.what() << std::endl;
    return messages;
  }
}

// Help me rename the final get_channel and get_channel_info please :')
ServerSend::ChannelInfo Database::get_channel(int64_t id_user,
                                              int64_t id_channel,
                                              ChannelStatus membership,
                                              ChannelStatus other_membership) {
  ServerSend::ChannelInfo channel =
      this->get_channel_info(id_user, id_channel, membership);
  channel.participants =
      this->get_participants(id_user, id_channel, membership, other_membership);
  channel.unread_count = this->get_unread_count(id_user, id_channel);
  channel.last_message = this->get_last_message(id_user, id_channel);
  channel.is_group = channel.participants.size() > 2;
  return channel;
}

// Populate most of channel info for a specific user and channel
ServerSend::ChannelInfo Database::get_channel_info(int64_t id_user,
                                                   int64_t id_channel,
                                                   ChannelStatus membership) {
  ServerSend::ChannelInfo channel_info;
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT c.id_channel, c.title, c.created_by, "
            "COALESCE(uc1.last_read_id_message, 0) "
            "FROM channels c "
            "JOIN userChannel uc1 ON c.id_channel = uc1.id_channel "
            "WHERE uc1.id_user = ? "
            "AND uc1.membership = ? "
            "AND uc1.id_channel = ? "
            "AND (SELECT COUNT(*) FROM userChannel uc2 "
            "WHERE uc2.id_channel = c.id_channel "
            "AND uc2.membership = ?) >= 2;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));
    prep_statement->setInt64(3, id_channel);
    prep_statement->setInt(4, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      channel_info.id_channel = id_channel;
      channel_info.title = res->getString("title");
      channel_info.created_by = res->getInt64("created_by");
      channel_info.last_read_id_message = res->getInt("last_read_id_message");
    }
    return channel_info;
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channel_info error: " << e.what() << std::endl;
    return channel_info;
  }
}

// Get all participant for a channel for a user
// Returns a vector<ServerSend::Contact>
std::vector<ServerSend::Contact> Database::get_participants(
    const int64_t id_user, const int64_t id_channel, ChannelStatus membership,
    ChannelStatus other_membership) {
  std::vector<ServerSend::Contact> channel_participants;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT DISTINCT uc2.id_user, u.username "
            "FROM userChannel uc1 "
            "JOIN userChannel uc2 ON uc1.id_channel = uc2.id_channel "
            "JOIN users u ON u.id_user = uc2.id_user "
            "WHERE uc1.id_user = ? "
            "AND uc1.membership = ? "
            "AND uc2.membership = ? "
            "AND uc1.id_channel = ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));
    prep_statement->setInt(3, static_cast<int32_t>(other_membership));
    prep_statement->setInt64(4, id_channel);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::Contact participant;
      participant.id_user = res->getInt64("id_user");
      participant.username = res->getString("username");
      channel_participants.push_back(participant);
    }
    return channel_participants;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_participants error: " << e.what() << std::endl;
    return channel_participants;
  }
}

// Get unread_count for a channel for a user
// Returns count>
int64_t Database::get_unread_count(const int64_t id_user,
                                   const int64_t id_channel) {
  int64_t unread_count = 0;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT COUNT(m1.id_message) AS unread_count "
            "FROM userChannel "
            "uc1 LEFT JOIN messages m1 ON uc1.id_channel = m1.id_channel "
            "WHERE uc1.id_user = ? AND uc1.id_channel = ? AND "
            "uc1.last_read_id_message < m1.id_message GROUP BY "
            "uc1.id_channel;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt64(2, id_channel);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      unread_count = res->getInt64("unread_count");
    }
    return unread_count;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_unread_count error: " << e.what() << std::endl;
    return unread_count;
  }
}

// Get last message of a channel a specific user participates in
// Returns ServerSend::Message
ServerSend::Message Database::get_last_message(const int64_t id_user,
                                               const int64_t id_channel) {
  ServerSend::Message last_message;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT uc1.id_channel, m1.id_message, m1.id_user, "
            "COALESCE(m1.body, 'no message yet...') as body, "
            "m1.timestamp FROM userChannel uc1 "
            "LEFT JOIN messages m1 "
            "ON (uc1.id_channel = m1.id_channel) "
            "AND m1.id_message = ( "
            "SELECT MAX(id_message) FROM messages m2 "
            "WHERE uc1.id_channel = m2.id_channel) "
            "WHERE uc1.id_user = ? AND uc1.id_channel = ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt64(2, id_channel);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      last_message.id_message = res->getInt64("id_message");
      last_message.id_sender = res->getInt64("id_user");
      last_message.body = res->getString("body");
      last_message.timestamp = res->getString("timestamp");
      last_message.is_system = (last_message.id_sender == 1);
      // last_messages.push_back(message);
    }
    return last_message;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_last_messages error: " << e.what() << std::endl;
    return last_message;
  }
}

// ===== INVITATION ACCEPTED/REJECTED =====

bool Database::update_invitation(int64_t id_user, int64_t id_channel,
                                 const std::string& responded_at,
                                 ChannelStatus membership) {
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("UPDATE userChannel "
                                     "SET membership = ?, responded_at = ? "
                                     "WHERE id_user = ? AND id_channel = ?;"));
    prep_statement->setInt(1, static_cast<int32_t>(membership));
    prep_statement->setString(2, responded_at);
    prep_statement->setInt64(3, id_user);
    prep_statement->setInt64(4, id_channel);

    int affected_rows = prep_statement->executeUpdate();
    return affected_rows > 0;
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] update_invitation error: " << e.what() << std::endl;
    return false;
  }
};

bool Database::accept_invitation(int64_t id_user, int64_t id_channel,
                                 const std::string& responded_at) {
  return this->update_invitation(id_user, id_channel, responded_at,
                                 ChannelStatus::ACCEPTED);
};

bool Database::reject_invitation(int64_t id_user, int64_t id_channel,
                                 const std::string& responded_at) {
  return this->update_invitation(id_user, id_channel, responded_at,
                                 ChannelStatus::REJECTED);
};

std::optional<int64_t> Database::get_channel_creator(int64_t id_channel) {
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT created_by FROM channels "
                                     "WHERE id_channel = ?;"));

    prep_statement->setInt64(1, id_channel);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      return res->getInt64("created_by");
    }
    return std::nullopt;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channel_creator error: " << e.what() << std::endl;
    return std::nullopt;
  }
}

// ===== CONTACT =====

// Get contact list for a user
std::optional<ServerSend::Contact> Database::get_contact(
    const int64_t id_user) {
  std::lock_guard<std::mutex> lock(db_mutex);

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT id_user, username FROM users "
                                     "WHERE id_user = ?;"));

    prep_statement->setInt64(1, id_user);
    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      ServerSend::Contact contact;
      contact.id_user = res->getInt64("id_user");
      contact.username = res->getString("username");
      return contact;
    }
    return std::nullopt;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_contact error: " << e.what() << std::endl;
    return std::nullopt;
  }
}

// Get user id
std::optional<int64_t> Database::get_id_user(const std::string& username) {
  // std::lock_guard<std::mutex> lock(db_mutex);

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT id_user FROM users "
                                     "WHERE username = ?;"));

    prep_statement->setString(1, username);
    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      return res->getInt64("id_user");
    }
    return std::nullopt;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_id_user error: " << e.what() << std::endl;
    return std::nullopt;
  }
}

// Returns created channel id
std::optional<int64_t> Database::create_channel(const int64_t created_by,
                                                const std::string& title,
                                                const std::string& created_at) {
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "INSERT INTO channels (title, created_at, created_by) "
            "VALUES (?, ?, ?);"));

    prep_statement->setString(1, title);
    prep_statement->setString(2, created_at);
    prep_statement->setInt64(3, created_by);
    prep_statement->executeUpdate();

    std::unique_ptr<sql::Statement> stmt(this->conn->createStatement());
    std::unique_ptr<sql::ResultSet> res(
        stmt->executeQuery("SELECT LAST_INSERT_ID()"));

    if (res->next()) {
      return res->getInt64(1);
    }
    return std::nullopt;
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] create_channel error: " << e.what() << std::endl;
    return std::nullopt;
  }
}

// Returns false if an error occured
bool Database::create_user_channels(
    const int64_t id_creator, const int64_t id_channel,
    const std::unordered_set<int64_t>& participants) {
  try {
    this->ensure_connection();
    std::string query =
        "INSERT INTO userChannel (id_user, id_channel, membership) VALUES ";

    std::string placeholder =
        "(?, ?, " + std::to_string(static_cast<int>(ChannelStatus::PENDING)) +
        "), ";
    for (const int64_t& participant : participants) {
      query += placeholder;
    }
    query += "(?, ?, " +
             std::to_string(static_cast<int>(ChannelStatus::ACCEPTED)) + "); ";

    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(query));
    int param = 0;
    for (const int64_t id_participant : participants) {
      prep_statement->setInt64(++param, id_participant);
      prep_statement->setInt64(++param, id_channel);
    }
    prep_statement->setInt64(++param, id_creator);
    prep_statement->setInt64(++param, id_channel);

    prep_statement->executeUpdate();
    return true;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] create_user_channels error: " << e.what() << std::endl;
    return false;
  }
}

// returns new id_channel "transaction" for create channel + userchannel ! all
// or nothing
std::optional<int64_t> Database::create_channel_with_participants(
    const int64_t id_creator, const std::string& title,
    const std::string& created_at,
    const std::unordered_set<int64_t>& participants) {
  std::lock_guard<std::mutex> lock(db_mutex);

  try {
    this->ensure_connection();
    this->conn->setAutoCommit(false);
    std::optional<int64_t> id_channel_opt =
        this->create_channel(id_creator, title, created_at);

    if (!id_channel_opt.has_value()) {
      this->conn->rollback();
      this->conn->setAutoCommit(true);
      return std::nullopt;
    }

    bool success = this->create_user_channels(
        id_creator, id_channel_opt.value(), participants);
    if (!success) {
      this->conn->rollback();
      this->conn->setAutoCommit(true);
      return std::nullopt;
    }

    this->conn->commit();
    this->conn->setAutoCommit(true);
    return id_channel_opt;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] create_channel_with_participants error: " << e.what()
              << std::endl;

    this->conn->rollback();
    this->conn->setAutoCommit(true);

    return std::nullopt;
  }
}

std::vector<ServerSend::Contact> Database::get_channel_contacts(
    int64_t id_channel, ChannelStatus membership) {
  std::vector<ServerSend::Contact> contacts;
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT u.id_user, u.username FROM userChannel uc "
            "JOIN users u ON u.id_user = uc.id_user "
            "WHERE uc.id_channel = ? AND uc.membership = ?;"));

    prep_statement->setInt64(1, id_channel);
    prep_statement->setInt(2, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::Contact contact;
      contact.id_user = res->getInt64("id_user");
      contact.username = res->getString("username");
      contacts.push_back(contact);
    }
    return contacts;
  } catch (sql::SQLException& e) {
    try {
      this->conn->rollback();
      this->conn->setAutoCommit(true);
    } catch (...) {
    }
    return contacts;
  }
}