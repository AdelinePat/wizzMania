#ifndef MOCK_WEBSOCKET_MANAGER_H
#define MOCK_WEBSOCKET_MANAGER_H

#include <gmock/gmock.h>

#include "iwebsocket_manager.hpp"

// MockDatabase implements every pure virtual method from IDatabase using
// GMock's MOCK_METHOD macro. In each test you only set expectations on the
// methods that the code-under-test actually calls. All others return GMock's
// safe defaults (0, false, empty vector, nullopt, etc.) and will emit a
// warning if unexpectedly called.

class MockWebSocketManager : public IWebSocketManager {
 public:
  // ── Users ──────────────────────────────────────────────────────────────────

  MOCK_METHOD(void, add_user, (int64_t, WSConn, const std::string&),
              (override));

  MOCK_METHOD(void, remove_connection, (WSConn), (override));

  MOCK_METHOD(void, disconnect_token, (const std::string&), (override));

  MOCK_METHOD(bool, is_authenticated, (WSConn), (override));

  MOCK_METHOD((std::optional<int64_t>), get_id_user, (WSConn), (override));

  MOCK_METHOD(std::vector<WSConn>, get_user_connections, (int64_t), (override));

  MOCK_METHOD(void, broadcast_to_all, (const std::string&), (override));

  MOCK_METHOD(void, broadcast_to_users,
              (const std::unordered_set<int64_t>&, const std::string&),
              (override));

  MOCK_METHOD(bool, is_user_online, (int64_t), (override));

  MOCK_METHOD((std::optional<int64_t>), get_user_id, (WSConn), (override));

  MOCK_METHOD(void, send_to_user, (int64_t, const std::string&), (override));

  MOCK_METHOD(void, send_to_user_except,
              (int64_t, const std::string&, const std::string&), (override));

  MOCK_METHOD((std::string), get_token_for_connection, (WSConn), (override));

  MOCK_METHOD(void, disconnect_user, (int64_t, const std::string&), (override));
};

#endif