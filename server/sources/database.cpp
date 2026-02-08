#include "database.hpp"

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
      // AuthMessages::LoginResponse login_response;
      return res->getInt64("id_user");
      // login_response.user_id = res->getInt64("id_user");
      // login_response.username = res->getString("username");
      // return login_response;
    }
    return -1;

  } catch (sql::SQLException& e) {
    std::cerr << "[DB] verify_user error: " << e.what() << std::endl;
    return -1;
  }
}