#include "database.hpp"

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

// ==== CHANNEL INFO

// get entire channel info for a channel id
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

// ==== ??

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

