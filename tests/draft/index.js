// ==================== GLOBAL STATE ====================
let token = null;
let userId = null;
let username = null;
let ws = null;

// Username cache (hardcoded for testing - matches your 3 fake users)
const userCache = new Map([
  [1, "alice"],
  [2, "bob"],
  [3, "charlie"]
]);

const SERVER_URL = "http://localhost:8888";
const WS_URL = "ws://localhost:8888/ws";

// Message type constants (matching your C++ enum)
const MessageType = {
  // Client -> Server
  WS_AUTH: 0,
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

  // Server -> Client
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
          User: ${username} (ID: ${userId})
        </div>
      `;

      document.getElementById("loginBtn").disabled = true;

      addSystemMessage(`Logged in as ${username} (ID: ${userId})`);
      
      // ✅ Automatically connect WebSocket after successful login
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
    console.log("WebSocket opened, sending authentication...");
    addSystemMessage("Connecting to chat server...");

    // Send authentication message
    setTimeout(() => {
      ws.send(JSON.stringify({
        type: MessageType.WS_AUTH,
        token: token
      }));
      console.log("Auth message sent");
    }, 50);
  };

  ws.onmessage = (event) => {
    console.log("Received:", event.data);

    try {
      const data = JSON.parse(event.data);

      switch (data.type) {
        case MessageType.WS_AUTH_SUCCESS:
          statusDiv.innerHTML = `
            <div class="status success">
              ✅ Connected to chat server!
            </div>
          `;

          // Enable messaging
          document.getElementById("messageInput").disabled = false;
          document.getElementById("sendBtn").disabled = false;
          document.getElementById("disconnectBtn").disabled = false;

          addSystemMessage(`Ready to chat!`);
          break;

        case MessageType.NEW_MESSAGE:
          // ✅ Handle message echo from server
          handleNewMessage(data);
          break;

        case MessageType.USER_TYPING:
          handleTypingNotification(data);
          break;

        case MessageType.ERROR:
          addSystemMessage(`Server Error: ${data.message} (${data.error_code})`);
          break;

        default:
          console.log("Unhandled message type:", data.type);
          addSystemMessage(`Received message type ${data.type}`);
      }
    } catch (e) {
      console.error("Error parsing message:", e);
      addReceivedMessage(event.data);
    }
  };

  ws.onerror = (error) => {
    console.error("WebSocket error:", error);
    statusDiv.innerHTML = '<div class="status error">❌ WebSocket error</div>';
    addSystemMessage("Connection error");
  };

  ws.onclose = (event) => {
    console.log("WebSocket closed:", event.reason);
    statusDiv.innerHTML = `<div class="status info">Disconnected from chat server</div>`;

    // Disable messaging
    document.getElementById("messageInput").disabled = true;
    document.getElementById("sendBtn").disabled = true;
    document.getElementById("disconnectBtn").disabled = true;

    addSystemMessage(`Disconnected from server`);
  };
}

function disconnectWebSocket() {
  if (ws) {
    // Send logout message
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({
        type: MessageType.LOGOUT,
        reason: "User logged out"
      }));
    }
    ws.close();
    ws = null;
    
    addSystemMessage("Logged out");
  }
}

// ==================== MESSAGE HANDLERS ====================

function handleNewMessage(data) {
  if (!data.message) {
    console.error("NEW_MESSAGE missing message field:", data);
    return;
  }

  const msg = data.message;
  const isMe = msg.sender_id === userId;
  
  // Get username from cache
  const senderUsername = userCache.get(msg.sender_id) || `User${msg.sender_id}`;
  
  // Display message with proper styling
  if (isMe) {
    addSentMessage(msg.body, senderUsername, msg.timestamp);
  } else {
    addReceivedMessage(msg.body, senderUsername, msg.timestamp);
  }
}

function handleTypingNotification(data) {
  // Don't show our own typing
  if (data.user_id === userId) return;

  const typingUsername = userCache.get(data.user_id) || `User${data.user_id}`;
  
  if (data.is_typing) {
    console.log(`${typingUsername} is typing...`);
    // TODO: Show typing indicator in UI
  } else {
    console.log(`${typingUsername} stopped typing`);
    // TODO: Hide typing indicator
  }
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

  // Send message to server
  ws.send(JSON.stringify({
    type: MessageType.SEND_MESSAGE,
    channel_id: 1,  // Global chat
    body: message
  }));

  console.log("Message sent:", message);

  // ❌ DON'T display locally - wait for server echo
  // The server will broadcast it back and handleNewMessage() will display it

  // Clear input
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

function addSentMessage(text, senderUsername, timestamp) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message sent";
  
  const time = timestamp ? new Date(timestamp).toLocaleTimeString() : new Date().toLocaleTimeString();
  
  messageDiv.innerHTML = `
    <div><strong>${escapeHtml(senderUsername)} (You):</strong> ${escapeHtml(text)}</div>
    <div class="timestamp">${time}</div>
  `;
  
  messagesDiv.appendChild(messageDiv);
  messagesDiv.scrollTop = messagesDiv.scrollHeight;
}

function addReceivedMessage(text, senderUsername, timestamp) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message received";
  
  const time = timestamp ? new Date(timestamp).toLocaleTimeString() : new Date().toLocaleTimeString();
  
  messageDiv.innerHTML = `
    <div><strong>${escapeHtml(senderUsername)}:</strong> ${escapeHtml(text)}</div>
    <div class="timestamp">${time}</div>
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