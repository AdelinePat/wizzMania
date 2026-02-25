#include "database.hpp"

// ===== ESSENTIALS =====
// rename : get_id_user ?
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

bool Database::has_channel_access(int64_t id_user, int64_t id_channel,   ChannelStatus membership) {
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT 1 FROM userChannel "
                                     "WHERE id_user = ? "
                                     "AND id_channel = ? "
                                     "AND membership = ?;"));

    prep_statement->setInt64(1, id_user);
    prep_statement->setInt64(2, id_channel);
     prep_statement->setInt(3, static_cast<int32_t>(membership));

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      return true;
    }
    return false;
    // return res->next();

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] has_channel_access error: " << e.what() << std::endl;
    throw InternalError(std::string("DB error: ") + e.what());
    // return false;
  }
}

// Get contact struct for one id_user
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

// ===== GET INITIAL CONTACT =====

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

// Get contact list for a user
std::vector<ServerSend::Contact> Database::get_user_contacts(
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