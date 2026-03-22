#include "database.hpp"

// Get unread_count for each channels a user participates in
// Returns map<id_channel, count>
// Must gard lock before call !
std::map<int64_t, int64_t> Database::get_unread_count(const int64_t id_user) {
  std::map<int64_t, int64_t> unread_counts;
  std::lock_guard<std::recursive_mutex> lock(db_mutex);

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
// Must be guard lock outside method!
std::map<int64_t, ServerSend::Message> Database::get_last_messages(
    const int64_t id_user) {
  std::map<int64_t, ServerSend::Message> last_messages;
  std::lock_guard<std::recursive_mutex> lock(db_mutex);

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

// ===== SAVE MESSAGE =====

// Save message in db for the right channel and returns id_message
std::optional<int64_t> Database::save_message(int64_t id_user,
                                              int64_t id_channel,
                                              const std::string& body,
                                              const std::string& timestamp) {
  // std::lock_guard<std::mutex> lock(db_mutex);
  std::lock_guard<std::recursive_mutex> lock(db_mutex);

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

    // TODO THROW ERROR !!!
    if (res->next()) {
      return res->getInt64(1);
    }
    return std::nullopt;
  } catch (sql::SQLException& e) {
    std::cerr << "[DB] save_message error: " << e.what() << std::endl;
    return std::nullopt;
  }
}
// ==== MARK AS READ === //
bool Database::update_last_read_message(int64_t id_user, int64_t id_channel,
                                        int64_t last_read_id_message) {
  // std::lock_guard<std::mutex> lock(db_mutex);
  std::lock_guard<std::recursive_mutex> lock(db_mutex);

  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "UPDATE userChannel SET last_read_id_message = ? "
            "WHERE id_user = ? AND id_channel = ? "
            "AND (last_read_id_message IS NULL OR last_read_id_message < ?);"));

    prep_statement->setInt64(1, last_read_id_message);
    prep_statement->setInt64(2, id_user);
    prep_statement->setInt64(3, id_channel);
    prep_statement->setInt64(4, last_read_id_message);

    int rows_affected = prep_statement->executeUpdate();
    return rows_affected > 0;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] update_last_read_message error: " << e.what()
              << std::endl;
    return false;
  }
}

// ===== CHANNELS ESSENTIALS =====

std::vector<ServerSend::Message> Database::get_channel_history(
    int64_t id_channel, int64_t before_id_message, int limit) {
  // std::lock_guard<std::mutex> lock(db_mutex);
  std::lock_guard<std::recursive_mutex> lock(db_mutex);
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
      ServerSend::Message message = Structure::create_message_struct(
          res->getInt64("id_message"), res->getInt64("id_user"),
          res->getString("body"), res->getString("timestamp"));
      messages.push_back(message);
    }
    return messages;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channel_history error: " << e.what() << std::endl;
    return messages;
  }
}

/// == UNREAD COUNT & LAST MESSAGE

// Get unread_count for a channel for a user
// Returns count>
// Lock inside
int64_t Database::get_unread_count(const int64_t id_user,
                                   const int64_t id_channel) {
  int64_t unread_count = 0;
  // std::lock_guard<std::mutex> lock(db_mutex);
  std::lock_guard<std::recursive_mutex> lock(db_mutex);

  try {
    this->ensure_connection();
    // std::unique_ptr<sql::PreparedStatement> prep_statement(
    //     this->conn->prepareStatement(
    //         "SELECT COUNT(m1.id_message) AS unread_count "
    //         "FROM userChannel "
    //         "uc1 LEFT JOIN messages m1 ON uc1.id_channel = m1.id_channel "
    //         "WHERE uc1.id_user = ? AND uc1.id_channel = ? AND "
    //         "uc1.last_read_id_message < m1.id_message GROUP BY "
    //         "uc1.id_channel;"));

    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement(
            "SELECT COUNT(m1.id_message) AS unread_count "
            "FROM userChannel uc1 "
            "LEFT JOIN messages m1 ON uc1.id_channel = m1.id_channel "
            "WHERE uc1.id_user = ? AND uc1.id_channel = ? "
            "AND COALESCE(uc1.last_read_id_message, 0) < m1.id_message "
            "GROUP BY uc1.id_channel;"));

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
  // std::lock_guard<std::mutex> lock(db_mutex);
  std::lock_guard<std::recursive_mutex> lock(db_mutex);

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
    std::cerr << "[DB] get_last_message error: " << e.what() << std::endl;
    return last_message;
  }
}