import { SERVER_IP, SERVER_PORT } from "./secret.js";

// ==================== GLOBAL STATE ====================
let token = null;
let userId = null;
let username = null;
let ws = null;

// Username cache (hardcoded for testing - matches your 3 fake users)
const userCache = new Map([
  [2, "alice"],
  [3, "bob"],
  [4, "carol"],
  [5, 'dave'],
  [6, 'eve'],
  [7, 'frank'],
  [8, 'grace'],
  [9, 'heidi'],
  [10, 'ivan'],
  [11, 'judy'],
]);

const SERVER_URL = `http://${SERVER_IP ? SERVER_IP : "192.168.0.117"}:${SERVER_PORT ? SERVER_PORT : "8080"}`;
const WS_URL = `ws://${SERVER_IP}:${SERVER_PORT}/ws`;

// Message type constants
const MessageType = {
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
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ username: usernameInput, password: passwordInput })
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

    setTimeout(() => {
      ws.send(JSON.stringify({ type: MessageType.WS_AUTH, token }));
      console.log("Auth message sent");
    }, 50);
  };

  ws.onmessage = (event) => {
    console.log("Received:", event.data);
    try {
      const data = JSON.parse(event.data);
      switch (data.type) {
        case MessageType.WS_AUTH_SUCCESS:
          statusDiv.innerHTML = '<div class="status success">✅ Connected to chat server!</div>';
          document.getElementById("messageInput").disabled = false;
          document.getElementById("sendBtn").disabled = false;
          document.getElementById("disconnectBtn").disabled = false;
          addSystemMessage(`Ready to chat!`);
          break;
        case MessageType.NEW_MESSAGE:
          handleNewMessage(data);
          break;
        case MessageType.USER_TYPING:
          handleTypingNotification(data);
          break;
        case MessageType.ERROR:
          addSystemMessage(`Server Error: ${data.message} (${data.error_code})`);
          break;
        default:
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
    statusDiv.innerHTML = '<div class="status info">Disconnected from chat server</div>';
    document.getElementById("messageInput").disabled = true;
    document.getElementById("sendBtn").disabled = true;
    document.getElementById("disconnectBtn").disabled = true;
    addSystemMessage(`Disconnected from server`);
  };
}

function disconnectWebSocket() {
  if (ws) {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ type: MessageType.LOGOUT, reason: "User logged out" }));
    }
    ws.close();
    ws = null;
    addSystemMessage("Logged out");
  }
}

// ==================== MESSAGE HANDLERS ====================
function handleNewMessage(data) {
  if (!data.message) return console.error("NEW_MESSAGE missing message field:", data);

  const msg = data.message;
  const isMe = msg.sender_id === userId;
  const senderUsername = userCache.get(msg.sender_id) || `User${msg.sender_id}`;

  if (isMe) addSentMessage(msg.body, senderUsername, msg.timestamp);
  else addReceivedMessage(msg.body, senderUsername, msg.timestamp);
}

function handleTypingNotification(data) {
  if (data.user_id === userId) return;
  const typingUsername = userCache.get(data.user_id) || `User${data.user_id}`;
  console.log(data.is_typing ? `${typingUsername} is typing...` : `${typingUsername} stopped typing`);
}

// ==================== MESSAGING ====================
function sendMessage() {
  const input = document.getElementById("messageInput");
  const message = input.value.trim();
  if (!message || !ws || ws.readyState !== WebSocket.OPEN) return;

  ws.send(JSON.stringify({ type: MessageType.SEND_MESSAGE, channel_id: 1, body: message }));
  console.log("Message sent:", message);
  input.value = "";
}

// ==================== UI HELPERS ====================
function addSentMessage(text, senderUsername, timestamp) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message sent";
  const time = timestamp ? new Date(timestamp).toLocaleTimeString() : new Date().toLocaleTimeString();
  messageDiv.innerHTML = `<div><strong>${escapeHtml(senderUsername)} (You):</strong> ${escapeHtml(text)}</div>
                          <div class="timestamp">${time}</div>`;
  messagesDiv.appendChild(messageDiv);
  messagesDiv.scrollTop = messagesDiv.scrollHeight;
}

function addReceivedMessage(text, senderUsername, timestamp) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message received";
  const time = timestamp ? new Date(timestamp).toLocaleTimeString() : new Date().toLocaleTimeString();
  messageDiv.innerHTML = `<div><strong>${escapeHtml(senderUsername)}:</strong> ${escapeHtml(text)}</div>
                          <div class="timestamp">${time}</div>`;
  messagesDiv.appendChild(messageDiv);
  messagesDiv.scrollTop = messagesDiv.scrollHeight;
}

function addSystemMessage(text) {
  const messagesDiv = document.getElementById("messages");
  const messageDiv = document.createElement("div");
  messageDiv.className = "message system";
  messageDiv.innerHTML = `<div>ℹ️ ${escapeHtml(text)}</div>
                          <div class="timestamp">${new Date().toLocaleTimeString()}</div>`;
  messagesDiv.appendChild(messageDiv);
  messagesDiv.scrollTop = messagesDiv.scrollHeight;
}

function escapeHtml(text) {
  const div = document.createElement("div");
  div.textContent = text;
  return div.innerHTML;
}

// ==================== EVENT LISTENERS ====================
document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("loginBtn").addEventListener("click", login);
  document.getElementById("sendBtn").addEventListener("click", sendMessage);
  document.getElementById("disconnectBtn").addEventListener("click", disconnectWebSocket);

  document.getElementById("messageInput").addEventListener("keypress", (e) => {
    if (e.key === "Enter") sendMessage();
  });
});
