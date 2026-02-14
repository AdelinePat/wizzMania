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

// Get contact list for a user
std::vector<ServerSend::Contact> Database::get_contact(
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
            "AND uc2.id_user <> ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));
    prep_statement->setInt(3, static_cast<int32_t>(membership));
    prep_statement->setInt64(4, id_user);

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

// Get all the channel a user participates it and the set of every participants from this channel
// Returns a map<id_channel, set<id_users>>
std::map<int64_t, std::set<int64_t>> Database::get_participants_and_channel(
    const int64_t id_user, ChannelStatus membership) {
  std::map<int64_t, std::set<int64_t>> channel_participants;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT DISTINCT uc2.id_user, uc2.id_channel "
            "FROM userChannel uc1 "
            "JOIN userChannel uc2 ON uc1.id_channel = uc2.id_channel "
            "WHERE uc1.id_user = ? "
            "AND uc1.membership = ? "
            "AND uc2.membership = ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));
    prep_statement->setInt(3, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      int64_t id_channel = res->getInt64("id_channel");
      int64_t participant = res->getInt64("id_user");
      channel_participants[id_channel].insert(participant);
    }
    return channel_participants;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_participants_and_channel error: " << e.what()
              << std::endl;
    return channel_participants;
  }
}

std::vector<ServerSend::ChannelInfo> Database::get_channels(
    const int64_t id_user, ChannelStatus membership) {
  std::vector<ServerSend::ChannelInfo> channels_info;

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT c.id_channel, c.title, c.created_by, "
            "uc1.last_read_id_message "
            "FROM channels c JOIN userChannel uc1 ON c.id_channel = "
            "uc1.id_channel "
            "WHERE uc1.id_user = ? AND uc1.membership = ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt(2, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    while (res->next()) {
      ServerSend::ChannelInfo channel;
      channel.channel_id = res->getInt64("id_channel");
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

// Create ChannelInfo list for initial_data
std::vector<ServerSend::ChannelInfo> Database::get_initial_channels(
    const int64_t id_user) {
  std::lock_guard<std::mutex> lock(db_mutex);

  std::vector<ServerSend::ChannelInfo> channels_info =
      this->get_channels(id_user);
  std::map<int64_t, std::set<int64_t>> channel_participants =
      this->get_participants_and_channel(id_user);
  std::map<int64_t, int64_t> channels_unread_count =
      this->get_unread_count(id_user);
  std::map<int64_t, ServerSend::Message> channels_last_message =
      this->get_last_messages(id_user);

  for (ServerSend::ChannelInfo& channel : channels_info) {
    auto it_participant = channel_participants.find(channel.channel_id);
    if (it_participant != channel_participants.end()) {
      channel.participants = it_participant->second;
      channel.is_group = channel.participants.size() > 2;
    } else {
      std::cerr << "[DB] Warning: no participants found for channel "
                << channel.channel_id << "\n";
    }

    auto it_unread_count = channels_unread_count.find(channel.channel_id);
    if (it_unread_count != channels_unread_count.end()) {
      channel.unread_count = it_unread_count->second;
    } else {
      channel.unread_count = 0;
    }

    auto it_last_message = channels_last_message.find(channel.channel_id);
    if (it_last_message != channels_last_message.end()) {
      channel.last_message = it_last_message->second;
    } else {
      std::cerr << "[DB] Warning: no last message found for channel "
                << channel.channel_id << "\n";
    }
  }
  return channels_info;
}

ServerSend::InitialDataResponse Database::get_initial_data(
    const int64_t id_user) {
  ServerSend::InitialDataResponse init_data;
  init_data.contacts = this->get_contact(id_user);
  init_data.channels = this->get_initial_channels(id_user);
  init_data.invitations = {};
  return init_data;
}
