// Global state
let token = null;
let userId = null;
let username = null;
let ws = null;

const SERVER_URL = "http://localhost:8888";
const WS_URL = "ws://localhost:8888/ws";

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
      console.log(token);
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

      // Enable WebSocket connect button
      document.getElementById("connectBtn").disabled = false;
      document.getElementById("loginBtn").disabled = true;
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
    alert("Please login first!");
    return;
  }

  const statusDiv = document.getElementById("wsStatus");
  statusDiv.innerHTML =
    '<div class="status info">Connecting to WebSocket...</div>';

  ws = new WebSocket(WS_URL);

  ws.onopen = () => {
    // console.log("WebSocket opened, sending token...");
    // addSystemMessage("WebSocket connection opened");

    // // Send token as first message
    // console.log("first message :') ???");
    // ws.send(
    //   JSON.stringify({
    //     token: token,
    //   }),
    // );
    console.log("WebSocket opened, sending token...");
    addSystemMessage("WebSocket connection opened");

    // Wait 50ms just to be sure handshake is fully done
    setTimeout(() => {
        ws.send(JSON.stringify({ token: token }));
        console.log("Token sent");
    }, 50);
  };

  ws.onmessage = (event) => {
    console.log("Received:", event.data);

    try {
      const data = JSON.parse(event.data);

      if (data.type === "connected") {
        statusDiv.innerHTML = `
                    <div class="status success">
                        ✅ WebSocket connected!<br>
                        ${data.message}
                    </div>
                `;

        // Enable messaging
        document.getElementById("messageInput").disabled = false;
        document.getElementById("sendBtn").disabled = false;
        document.getElementById("connectBtn").disabled = true;
        document.getElementById("disconnectBtn").disabled = false;

        addSystemMessage(`Authenticated as ${username} (ID: ${userId})`);
      } else if (data.type === "echo") {
        addReceivedMessage(data.message);
      } else {
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

function disconnectWebSocket() {
  if (ws) {
    ws.close();
    ws = null;
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

  // Send message
  ws.send(message);

  // Display in UI
  addSentMessage(message);

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
