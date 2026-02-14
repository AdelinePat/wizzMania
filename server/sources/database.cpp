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

std::vector<ServerSend::Contact> Database::get_contact(const int64_t id_user) {
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
    prep_statement->setInt(2, static_cast<int32_t>(ChannelStatus::ACCEPTED));
    prep_statement->setInt(3, static_cast<int32_t>(ChannelStatus::ACCEPTED));
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