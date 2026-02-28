#include "database.hpp"

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

// === CREATE CHANNEL

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
  //   std::unordered_set<int64_t> invitees = participants;
  // invitees.erase(id_creator);
  try {
    this->ensure_connection();
    std::string query =
        "INSERT INTO userChannel (id_user, id_channel, membership) VALUES ";

    std::string placeholder =
        "(?, ?, " + std::to_string(static_cast<int>(ChannelStatus::PENDING)) +
        "), ";

    for (size_t i = 0; i < participants.size(); i++) {
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

// ==== INVITATION

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

int64_t Database::get_number_invited_users_in_channel(
    int64_t id_channel, ChannelStatus membership,
    ChannelStatus other_membership) {
  // count PENDING + ACCEPTED
  this->ensure_connection();
  std::unique_ptr<sql::PreparedStatement> prep_statement(
      this->conn->prepareStatement("SELECT COUNT(*) as total FROM userChannel "
                                   "WHERE id_channel = ? "
                                   "AND membership IN (?, ?);"));
  prep_statement->setInt64(1, id_channel);
  prep_statement->setInt(2, static_cast<int>(membership));
  prep_statement->setInt(3, static_cast<int>(other_membership));
  std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
  if (!res->next()) {
    throw InternalError("Couldn't get pending and accepted participants count");
  }
  return res->getInt64("total");
}

void Database::leave_channel(int64_t id_user, int64_t id_channel) {
  std::lock_guard<std::mutex> lock(db_mutex);
  try {
    this->ensure_connection();
    this->conn->setAutoCommit(false);

    int64_t remaining = get_number_invited_users_in_channel(
        id_channel, ChannelStatus::PENDING, ChannelStatus::ACCEPTED);

    this->leave_channel(id_user, id_channel, ChannelStatus::LEFT);

    // bool creator_left = (id_user == id_creator);
    if (remaining <= 1) {
      std::unique_ptr<sql::PreparedStatement> del(this->conn->prepareStatement(
          "DELETE FROM channels WHERE id_channel = ?;"));
      del->setInt64(1, id_channel);
      del->executeUpdate();
      // CASCADE handles userChannel and messages automatically
    }
    conn->commit(); 
    conn->setAutoCommit(true);

  } catch (sql::SQLException& e) {
    this->conn->rollback();
    this->conn->setAutoCommit(true);
    throw InternalError(std::string("DB error: ") + e.what());
  }
}

int Database::leave_channel(int64_t id_user, int64_t id_channel,
                            ChannelStatus membership) {
  try {
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("UPDATE userChannel "
                                     "SET membership = ? "
                                     "WHERE id_user = ? AND id_channel = ?;"));
    prep_statement->setInt(1, static_cast<int32_t>(membership));
    prep_statement->setInt64(2, id_user);
    prep_statement->setInt64(3, id_channel);

    int affected_rows = prep_statement->executeUpdate();
    if (affected_rows == 0) {
      throw NotFoundError("User is not a member of this channel");
    }
    return affected_rows;
    // return affected_rows > 0;
    // return true;
  } catch (sql::SQLException& e) {
    // std::cerr << "[DB] update_invitation error: " << e.what() << std::endl;
    throw InternalError(std::string("DB error: ") + e.what());
    // return false;
  }
}

std::optional<int64_t> Database::find_existing_channel(
    const std::unordered_set<int64_t>& all_participants) {
  std::lock_guard<std::mutex> lock(db_mutex);
  try {
    this->ensure_connection();

    std::string placeholders;
    for (size_t i = 0; i < all_participants.size(); i++) {
      placeholders += (i == 0) ? "?" : ", ?";
    }

    std::string query =
        "SELECT uc.id_channel FROM userChannel uc "
        "WHERE uc.membership IN (0, 1) "
        "AND uc.id_user IN (" +
        placeholders +
        ") "
        "GROUP BY uc.id_channel "
        "HAVING COUNT(DISTINCT uc.id_user) = ? "
        "AND ("
        "  SELECT COUNT(*) FROM userChannel uc2 "
        "  WHERE uc2.id_channel = uc.id_channel "
        "  AND uc2.membership IN (0, 1)"
        ") = ?;";

    std::unique_ptr<sql::PreparedStatement> stmt(
        this->conn->prepareStatement(query));

    int param = 0;
    for (int64_t id : all_participants) {
      stmt->setInt64(++param, id);
    }
    int64_t count = static_cast<int64_t>(all_participants.size());
    stmt->setInt64(++param, count);  // HAVING COUNT(DISTINCT) = ?
    stmt->setInt64(++param, count);  // subquery COUNT(*) = ?

    std::unique_ptr<sql::ResultSet> res(stmt->executeQuery());
    if (res->next()) {
      return res->getInt64("id_channel");
    }
    return std::nullopt;
  } catch (sql::SQLException& e) {
    throw InternalError(std::string("DB error: ") + e.what());
  }
}

std::optional<bool> Database::does_channel_exist(int64_t id_channel) {
  try {
    this->ensure_connection();
    std::unique_ptr<sql::PreparedStatement> prep_statement(
        this->conn->prepareStatement("SELECT 1 FROM channels "
                                     "WHERE id_channel = ?;"));

    prep_statement->setInt64(1, id_channel);

    std::unique_ptr<sql::ResultSet> res(prep_statement->executeQuery());
    if (res->next()) {
      return true;
    }
    return false;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] get_channel_creator error: " << e.what() << std::endl;
    return std::nullopt;
  }
}