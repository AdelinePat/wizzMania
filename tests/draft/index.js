// Global state
let token = null;
let userId = null;
let username = null;
let ws = null;

const SERVER_URL = "http://localhost:8888";
const WS_URL = "ws://localhost:8888/ws";


const MessageType = {
  WS_AUTH: 0,          // client -> server
  LOGOUT: 1,
  SEND_MESSAGE: 10,
  CREATE_CHANNEL: 11,
  ACCEPT_INVITATION: 12,
  REJECT_INVITATION: 13,
  LEAVE_CHANNEL: 14,
  UPDATE_CHANNEL_TITLE: 15,
  MARK_AS_READ: 16,
  TYPING_START: 17,
  TYPING_STOP: 18,
  REQUEST_CHANNEL_HISTORY: 19,
  CHANNEL_OPEN: 20,

  // server -> client
  WS_AUTH_SUCCESS: 100,
  NEW_MESSAGE: 101,
  CHANNEL_CREATED: 102,
  CHANNEL_INVITATION: 103,
  INVITATION_ACCEPTED: 104,
  INVITATION_REJECTED: 105,
  USER_JOINED: 106,
  USER_LEFT: 107,
  CHANNEL_ACTIVATED: 108,
  CHANNEL_DELETED: 109,
  TITLE_UPDATED: 110,
  USER_STATUS: 111,
  USER_TYPING: 112,
  INITIAL_DATA: 113,
  CHANNEL_HISTORY: 114,
  ERROR: 255
};

// ==================== LOGIN ====================

async function login() {
  const usernameInput = document.getElementById("username").value;
  const passwordInput = document.getElementById("password").value;
  const statusDiv = document.getElementById("loginStatus");

  statusDiv.innerHTML = '<div class="status info">Logging in...</div>';

  try {
    const response = await fetch(`${SERVER_URL}/login`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
      },
      body: JSON.stringify({
        username: usernameInput,
        password: passwordInput,
      }),
    });

    const data = await response.json();

    if (data.success) {
      token = data.token;
      userId = data.user_id;
      username = data.username;

      statusDiv.innerHTML = `
        <div class="status success">
          ✅ Login successful!<br>
          User ID: ${userId}<br>
          Username: ${username}<br>
          Token: ${token.substring(0, 30)}...
        </div>
      `;

      document.getElementById("loginBtn").disabled = true;

      // Automatically connect WebSocket and send WSAuthRequest
      connectWebSocket();
    } else {
      statusDiv.innerHTML = `<div class="status error">❌ ${data.message}</div>`;
    }
  } catch (error) {
    statusDiv.innerHTML = `<div class="status error">❌ Error: ${error.message}</div>`;
  }
}

// ==================== WEBSOCKET ====================

function connectWebSocket() {
  if (!token) {
    alert("No token available. Login first!");
    return;
  }

  const statusDiv = document.getElementById("wsStatus");
  statusDiv.innerHTML = '<div class="status info">Connecting to WebSocket...</div>';

  ws = new WebSocket(WS_URL);

  ws.onopen = () => {
    console.log("WebSocket opened, sending WSAuthRequest...");
    addSystemMessage("WebSocket connection opened");

    // Send WSAuthRequest immediately
    ws.send(JSON.stringify({
      type: 0,        // WS_AUTH message type
      token: token,   // authentication token
    }));
  };

  ws.onmessage = (event) => {
    console.log("Received:", event.data);

    try {
      const data = JSON.parse(event.data);

      switch (data.type) {
        case MessageType.WS_AUTH_SUCCESS:
          statusDiv.innerHTML = `
          <div class="status success">
            ✅ WebSocket connected and authenticated!<br>
            ${data.message || "Welcome!"}
          </div>
        `;
          // Enable messaging
          document.getElementById("messageInput").disabled = false;
          document.getElementById("sendBtn").disabled = false;
          document.getElementById("connectBtn").disabled = true;
          document.getElementById("disconnectBtn").disabled = false;

          addSystemMessage(`Authenticated as ${username} (ID: ${userId})`);
          break;

        case MessageType.NEW_MESSAGE:
          if (data.message) {
            const sender = data.message.sender_username || "System";
            const body = data.message.body || "";
            const timestamp = data.message.timestamp
              ? ` (${data.message.timestamp})`
              : "";

            // addReceivedMessage(`${sender}:${timestamp} ${body}`);
            addReceivedMessage(body);
          } else {
            // fallback if message object is missing
            addReceivedMessage(JSON.stringify(data, null, 2));
          }
          break;

        case MessageType.ERROR:
          addSystemMessage(`Server Error: ${data.message}`);
          break;

        default:
          // fallback for unknown types
          addReceivedMessage(JSON.stringify(data, null, 2));
      }
    } catch (e) {
      addReceivedMessage(event.data);
    }
  };

  ws.onerror = (error) => {
    console.error("WebSocket error:", error);
    statusDiv.innerHTML = '<div class="status error">❌ WebSocket error</div>';
    addSystemMessage("WebSocket error occurred");
  };

  ws.onclose = (event) => {
    console.log("WebSocket closed:", event.reason);
    statusDiv.innerHTML = `<div class="status info">WebSocket disconnected: ${event.reason || "Unknown reason"}</div>`;

    // Disable messaging
    document.getElementById("messageInput").disabled = true;
    document.getElementById("sendBtn").disabled = true;
    document.getElementById("connectBtn").disabled = false;
    document.getElementById("disconnectBtn").disabled = true;

    addSystemMessage(`Connection closed: ${event.reason || "Unknown reason"}`);
  };
}

// ==================== MESSAGING ====================

function sendMessage() {
  const input = document.getElementById("messageInput");
  const message = input.value.trim();

  if (!message) return;

  if (!ws || ws.readyState !== WebSocket.OPEN) {
    alert("WebSocket not connected!");
    return;
  }

  // Send message in proper format
  const sendRequest = {
    type: MessageType.SEND_MESSAGE,               // SEND_MESSAGE
    channel_id: 1,          // replace with your active channel_id
    body: message
  };

  ws.send(JSON.stringify(sendRequest));

  // Display in UI immediately
  addSentMessage(message);

  input.value = "";
}


// Allow Enter key to send message
document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("messageInput").addEventListener("keypress", (e) => {
    if (e.key === "Enter") {
      sendMessage();
    }
  });
});

// ==================== UI HELPERS ====================

function addSentMessage(text) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message sent";
  messageDiv.innerHTML = `
        <div><strong>You:</strong> ${escapeHtml(text)}</div>
        <div class="timestamp">${new Date().toLocaleTimeString()}</div>
    `;
  messagesDiv.appendChild(messageDiv);
  messagesDiv.scrollTop = messagesDiv.scrollHeight;
}

function addReceivedMessage(text) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message received";
  messageDiv.innerHTML = `
        <div><strong>Server:</strong> ${escapeHtml(text)}</div>
        <div class="timestamp">${new Date().toLocaleTimeString()}</div>
    `;
  messagesDiv.appendChild(messageDiv);
  messagesDiv.scrollTop = messagesDiv.scrollHeight;
}

function addSystemMessage(text) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message system";
  messageDiv.innerHTML = `
        <div>ℹ️ ${escapeHtml(text)}</div>
        <div class="timestamp">${new Date().toLocaleTimeString()}</div>
    `;
  messagesDiv.appendChild(messageDiv);
  messagesDiv.scrollTop = messagesDiv.scrollHeight;
}

function escapeHtml(text) {
  const div = document.createElement("div");
  div.textContent = text;
  return div.innerHTML;
}
// ```

// ## How to Use

// 1. **Put both files in a folder** (e.g., `client/`)
// 2. **Open index.html in your browser** (just double-click it, or use a simple HTTP server)
// 3. **Make sure your server is running** on port 8888

// ### Testing Flow:

// **Step 1: Login**
// - Username: `alice`
// - Password: `hash`
// - Click "Login"
// - You should see success message with token

// **Step 2: Connect WebSocket**
// - Click "Connect WebSocket"
// - Should see "WebSocket connected!" message

// **Step 3: Send Messages**
// - Type anything in the message box
// - Click "Send" or press Enter
// - Server will echo it back

// ## What You'll See in Server Console:
// ```
// [LOGIN] Received login request
// [LOGIN] Attempting login for user: alice
// [LOGIN] Login successful! User ID: 1
// [WS] New WebSocket connection opened (waiting for auth)
// [WS] Received message: {"token":"eyJhbGci..."}
// [WS] Authenticating with token: eyJhbGciOiJIUzI1NiIs...
// [WS] ✅ User 1 authenticated and connected!
// [WS] Received message: Hello from browser!
// [WS] Message from authenticated user 1
