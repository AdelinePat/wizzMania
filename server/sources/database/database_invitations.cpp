#include "database.hpp"

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

// ===== INVITATION ACCEPTED/REJECTED =====

void Database::update_invitation(int64_t id_user, int64_t id_channel,
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
    if (affected_rows == 0) {
      throw NotFoundError("Invitation not found for this user/channel");
    }
    // return affected_rows > 0;
    // return true;
  } catch (sql::SQLException& e) {
    // std::cerr << "[DB] update_invitation error: " << e.what() << std::endl;
    throw InternalError(std::string("DB error: ") + e.what());
    // return false;
  }
};

void Database::accept_invitation(int64_t id_user, int64_t id_channel,
                                 const std::string& responded_at) {
  std::lock_guard<std::mutex> lock(db_mutex);
  return this->update_invitation(id_user, id_channel, responded_at,
                                 ChannelStatus::ACCEPTED);
};

void Database::reject_invitation(int64_t id_user, int64_t id_channel,
                                 const std::string& responded_at) {
  std::lock_guard<std::mutex> lock(db_mutex);
  try {
    this->ensure_connection();
    this->conn->setAutoCommit(false);

    this->update_invitation(id_user, id_channel, responded_at,
                            ChannelStatus::REJECTED);

    int64_t remaining = get_number_invited_users_in_channel(
        id_channel, ChannelStatus::PENDING, ChannelStatus::ACCEPTED);

    // only creator remains, delete channel!
    if (remaining <= 1) {
      std::unique_ptr<sql::PreparedStatement> del(this->conn->prepareStatement(
          "DELETE FROM channels WHERE id_channel = ?;"));
      del->setInt64(1, id_channel);
      del->executeUpdate();
    }

    this->conn->commit();
    this->conn->setAutoCommit(true);

  } catch (const NotFoundError&) {
    this->conn->rollback();
    this->conn->setAutoCommit(true);
    throw;  // rethrow as-is
  } catch (const std::exception& e) {
    this->conn->rollback();
    this->conn->setAutoCommit(true);
    throw InternalError(std::string("DB error: ") + e.what());
  }
}