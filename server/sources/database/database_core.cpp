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
