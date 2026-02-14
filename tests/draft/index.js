import { SERVER_IP, SERVER_PORT } from "./secret.js";

// ==================== GLOBAL STATE ====================
let token      = null;
let userId     = null;
let username   = null;
let ws         = null;
let activeChannelId = null;

const userCache    = new Map();  // id_user -> username
const channelCache = new Map();  // id_channel -> ChannelInfo

const SERVER_URL = `http://${SERVER_IP ?? "192.168.0.117"}:${SERVER_PORT ?? "8080"}`;
const WS_URL     = `ws://${SERVER_IP}:${SERVER_PORT}/ws`;

// ==================== MESSAGE TYPES ====================
const MessageType = {
  WS_AUTH:                  0,
  LOGOUT:                   1,
  SEND_MESSAGE:            10,
  CREATE_CHANNEL:          11,
  ACCEPT_INVITATION:       12,
  REJECT_INVITATION:       13,
  LEAVE_CHANNEL:           14,
  UPDATE_CHANNEL_TITLE:    15,
  MARK_AS_READ:            16,
  TYPING_START:            17,
  TYPING_STOP:             18,
  REQUEST_CHANNEL_HISTORY: 19,
  CHANNEL_OPEN:            20,
  WS_AUTH_SUCCESS:        100,
  NEW_MESSAGE:            101,
  CHANNEL_CREATED:        102,
  CHANNEL_INVITATION:     103,
  INVITATION_ACCEPTED:    104,
  INVITATION_REJECTED:    105,
  USER_JOINED:            106,
  USER_LEFT:              107,
  CHANNEL_ACTIVATED:      108,
  CHANNEL_DELETED:        109,
  TITLE_UPDATED:          110,
  USER_STATUS:            111,
  USER_TYPING:            112,
  INITIAL_DATA:           113,
  CHANNEL_HISTORY:        114,
  ERROR:                  255
};

// ==================== LOGIN ====================
async function login() {
  const usernameInput = document.getElementById("username").value.trim();
  const passwordInput = document.getElementById("password").value;
  const errorDiv      = document.getElementById("login-error");
  const loginBtn      = document.getElementById("loginBtn");

  if (!usernameInput || !passwordInput) {
    errorDiv.textContent = "Please fill in all fields.";
    return;
  }

  loginBtn.disabled    = true;
  loginBtn.textContent = "Connecting...";
  errorDiv.textContent = "";

  try {
    const response = await fetch(`${SERVER_URL}/login`, {
      method: "POST",
      headers: { "Content-Type": "application/json" },
      body: JSON.stringify({ username: usernameInput, password: passwordInput })
    });

    const data = await response.json();

    if (data.success) {
      token    = data.token;
      userId   = data.id_user;
      username = data.username;
      showApp();
      connectWebSocket();
    } else {
      errorDiv.textContent = data.message ?? "Login failed.";
      loginBtn.disabled    = false;
      loginBtn.textContent = "Connect";
    }
  } catch (err) {
    errorDiv.textContent = `Connection error: ${err.message}`;
    loginBtn.disabled    = false;
    loginBtn.textContent = "Connect";
  }
}

// ==================== SHOW APP =====================
function showApp() {
  document.getElementById("login-screen").style.display = "none";
  document.getElementById("app-screen").classList.add("visible");

  // Populate user info in sidebar
  const initials = username?.slice(0, 2).toUpperCase() ?? "??";
  document.getElementById("user-avatar-initials").textContent = initials;
  document.getElementById("sidebar-username").textContent     = username;
}

// ==================== WEBSOCKET ====================
function connectWebSocket() {
  ws = new WebSocket(WS_URL);

  ws.onopen = () => {
    setWsDot("connecting");
    setTimeout(() => {
      ws.send(JSON.stringify({ type: MessageType.WS_AUTH, token }));
    }, 50);
  };

  ws.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      switch (data.type) {
        case MessageType.WS_AUTH_SUCCESS:
          onAuthSuccess();
          break;
        case MessageType.INITIAL_DATA:
          handleInitialData(data);
          break;
        case MessageType.NEW_MESSAGE:
          handleNewMessage(data);
          break;
        case MessageType.CHANNEL_HISTORY:
          handleChannelHistory(data);
          break;
        case MessageType.USER_TYPING:
          handleTypingNotification(data);
          break;
        case MessageType.ERROR:
          addSystemMessage(`Server Error: ${data.message} (${data.error_code})`);
          break;
        default:
          console.log("Unhandled message type:", data.type, data);
      }
    } catch (e) {
      console.error("Error parsing message:", e);
    }
  };

  ws.onerror = () => setWsDot("error");

  ws.onclose = (event) => {
    setWsDot("disconnected");
    document.getElementById("messageInput").disabled = true;
    document.getElementById("sendBtn").disabled      = true;
    addSystemMessage(`Disconnected: ${event.reason || "connection closed"}`);
  };
}

function onAuthSuccess() {
  setWsDot("connected");
  document.getElementById("messageInput").disabled = false;
  document.getElementById("sendBtn").disabled      = false;
  addSystemMessage("Connected to WizzMania ✦");
}

function disconnectWebSocket() {
  if (ws) {
    if (ws.readyState === WebSocket.OPEN) {
      ws.send(JSON.stringify({ type: MessageType.LOGOUT, reason: "User logged out" }));
    }
    ws.close();
    ws = null;
  }
}

// ==================== WS STATUS DOT ====================
function setWsDot(state) {
  const dot = document.getElementById("ws-dot");
  dot.className = "ws-status-dot";
  if (state === "connected")   { dot.classList.add("connected"); dot.title = "Connected"; }
  else if (state === "error")  { dot.classList.add("error");     dot.title = "Error"; }
  else                         {                                  dot.title = "Disconnected"; }
}

// ==================== INITIAL DATA ====================
function handleInitialData(data) {
  // Populate contact cache
  if (Array.isArray(data.contacts)) {
    data.contacts.forEach(c => userCache.set(c.id_user, c.username));
  }
  // Always add self
  if (userId && username) userCache.set(userId, username);

  // Populate channel cache and sidebar
  if (Array.isArray(data.channels)) {
    data.channels.forEach(ch => {
      channelCache.set(ch.id_channel, ch);
      ch.participants?.forEach(p => userCache.set(p.id_user, p.username));
    });
  }

  renderChannelsSidebar();
  renderContactsSidebar(data.contacts ?? []);

  console.log(`[INIT] ${userCache.size} contacts, ${channelCache.size} channels`);
}

// ==================== SIDEBAR RENDERING ====================
function renderChannelsSidebar() {
  const list = document.getElementById("channels-list");
  list.innerHTML = "";

  if (channelCache.size === 0) {
    list.innerHTML = '<div class="empty-sidebar">No channels yet</div>';
    return;
  }

  channelCache.forEach(ch => {
    const item = document.createElement("div");
    item.className  = "sidebar-item";
    item.dataset.id = ch.id_channel;

    const isGroup   = ch.is_group;
    const iconChar  = isGroup ? "⊞" : "◈";
    const preview   = ch.last_message?.body ?? "";
    const unread    = ch.unread_count > 0
      ? `<div class="unread-badge">${ch.unread_count}</div>`
      : "";

    item.innerHTML = `
      <div class="item-icon">${iconChar}</div>
      <div class="item-info">
        <div class="item-name">${escapeHtml(ch.title)}</div>
        <div class="item-preview">${escapeHtml(preview)}</div>
      </div>
      ${unread}
    `;

    item.addEventListener("click", () => selectChannel(ch.id_channel));
    list.appendChild(item);
  });
}

function renderContactsSidebar(contacts) {
  const list = document.getElementById("contacts-list");
  list.innerHTML = "";

  if (contacts.length === 0) {
    list.innerHTML = '<div class="empty-sidebar">No contacts yet</div>';
    return;
  }

  contacts.forEach(c => {
    const item = document.createElement("div");
    item.className = "sidebar-item";

    const initials = c.username.slice(0, 2).toUpperCase();
    item.innerHTML = `
      <div class="item-icon" style="border-radius:50%; font-size:11px; font-weight:700; color:var(--accent-bright)">${initials}</div>
      <div class="item-info">
        <div class="item-name">${escapeHtml(c.username)}</div>
      </div>
    `;

    list.appendChild(item);
  });
}

// ==================== CHANNEL SELECTION ====================
function selectChannel(channelId) {
  activeChannelId = channelId;
  const ch = channelCache.get(channelId);

  // Update active state in sidebar
  document.querySelectorAll(".sidebar-item").forEach(el => {
    el.classList.toggle("active", el.dataset.id == channelId);
  });

  // Update header
  document.getElementById("chat-icon").textContent  = ch?.is_group ? "⊞" : "◈";
  document.getElementById("chat-title").textContent = ch?.title ?? `Channel ${channelId}`;

  // Show participant count
  const participantCount = ch?.participants?.length ?? 0;
  document.getElementById("chat-sub").textContent =
    participantCount > 0 ? `${participantCount} members` : "";

  // Clear unread badge
  const sidebarItem = document.querySelector(`.sidebar-item[data-id="${channelId}"]`);
  if (sidebarItem) {
    const badge = sidebarItem.querySelector(".unread-badge");
    if (badge) badge.remove();
  }

  // Clear messages and request history from server
  document.getElementById("messages-container").innerHTML = "";
  requestChannelHistory(channelId, 0);
}

function requestChannelHistory(channelId, beforeMessageId) {
  if (!ws || ws.readyState !== WebSocket.OPEN) return;
  ws.send(JSON.stringify({
    type: MessageType.REQUEST_CHANNEL_HISTORY,
    id_channel: channelId,
    before_id_message: beforeMessageId,
    limit: 50
  }));
}

// ==================== NEW MESSAGE ====================
function handleNewMessage(data) {
  if (!data.message) return;

  const msg      = data.message;
  const isMe     = msg.id_sender === userId;
  const isSystem = msg.is_system;
  const sender   = isSystem
    ? "System"
    : (userCache.get(msg.id_sender) ?? `User ${msg.id_sender}`);

  // Always update sidebar preview + unread badge
  const chData = channelCache.get(data.id_channel);
  if (chData) {
    chData.last_message = msg;
    const item    = document.querySelector(`.sidebar-item[data-id="${data.id_channel}"]`);
    const preview = item?.querySelector(".item-preview");
    if (preview) preview.textContent = msg.body;

    // Only bump unread if not the active channel
    if (data.id_channel !== activeChannelId) {
      if (item) {
        let badge = item.querySelector(".unread-badge");
        if (!badge) {
          badge = document.createElement("div");
          badge.className = "unread-badge";
          badge.textContent = "1";
          item.appendChild(badge);
        } else {
          badge.textContent = parseInt(badge.textContent) + 1;
        }
      }
      return; // Don't display message - channel not open
    }
  }

  // Only display if this is the active channel
  if (data.id_channel !== activeChannelId) return;

  if (isSystem)     addSystemMessage(msg.body);
  else if (isMe)    addSentMessage(msg.body, sender, msg.timestamp);
  else              addReceivedMessage(msg.body, sender, msg.timestamp);
}

function handleTypingNotification(data) {
  if (data.id_user === userId) return;
  const name = userCache.get(data.id_user) ?? `User ${data.id_user}`;
  console.log(data.is_typing ? `${name} is typing...` : `${name} stopped typing`);
}

// ==================== CHANNEL HISTORY ====================
function handleChannelHistory(data) {
  // Ignore if user switched channels while waiting for response
  if (data.id_channel !== activeChannelId) return;

  const container = document.getElementById("messages-container");
  container.innerHTML = "";

  if (!data.messages || data.messages.length === 0) {
    addSystemMessage("No messages yet. Say hello!");
    return;
  }

  data.messages.forEach(msg => {
    const isMe     = msg.id_sender === userId;
    const isSystem = msg.is_system;
    const sender   = isSystem
      ? "System"
      : (userCache.get(msg.id_sender) ?? `User ${msg.id_sender}`);

    if (isSystem)     addSystemMessage(msg.body);
    else if (isMe)    addSentMessage(msg.body, sender, msg.timestamp);
    else              addReceivedMessage(msg.body, sender, msg.timestamp);
  });

  // TODO: if data.has_more, show "load older messages" button at top
  if (data.has_more) {
    const loadMore = document.createElement("div");
    loadMore.className = "date-sep";
    loadMore.style.cursor = "pointer";
    loadMore.style.color = "var(--accent)";
    loadMore.textContent = "↑ Load older messages";
    loadMore.addEventListener("click", () => {
      const oldest = data.messages[0]?.id_message ?? 0;
      requestChannelHistory(activeChannelId, oldest);
    });
    container.prepend(loadMore);
  }
}

// ==================== MESSAGING ====================
function sendMessage() {
  const input   = document.getElementById("messageInput");
  const message = input.value.trim();
  if (!message || !ws || ws.readyState !== WebSocket.OPEN) return;

  const channelId = activeChannelId ?? 1;
  ws.send(JSON.stringify({ type: MessageType.SEND_MESSAGE, id_channel: channelId, body: message }));
  input.value = "";
}

// ==================== MESSAGE RENDERING ====================
function addSentMessage(text, senderName, timestamp) {
  const container = document.getElementById("messages-container");
  const row       = document.createElement("div");
  row.className   = "msg-row sent";
  const initials  = senderName.slice(0, 2).toUpperCase();
  const time      = formatTime(timestamp);

  row.innerHTML = `
    <div class="msg-avatar">${initials}</div>
    <div class="msg-bubble-wrap">
      <div class="msg-sender">${escapeHtml(senderName)}</div>
      <div class="msg-bubble">${escapeHtml(text)}</div>
      <div class="msg-time">${time}</div>
    </div>
  `;

  container.appendChild(row);
  scrollToBottom();
}

function addReceivedMessage(text, senderName, timestamp) {
  const container = document.getElementById("messages-container");
  const row       = document.createElement("div");
  row.className   = "msg-row received";
  const initials  = senderName.slice(0, 2).toUpperCase();
  const time      = formatTime(timestamp);

  row.innerHTML = `
    <div class="msg-avatar">${initials}</div>
    <div class="msg-bubble-wrap">
      <div class="msg-sender">${escapeHtml(senderName)}</div>
      <div class="msg-bubble">${escapeHtml(text)}</div>
      <div class="msg-time">${time}</div>
    </div>
  `;

  container.appendChild(row);
  scrollToBottom();
}

function addSystemMessage(text) {
  const container = document.getElementById("messages-container");
  const row       = document.createElement("div");
  row.className   = "msg-row system";
  row.style.alignSelf = "center";

  row.innerHTML = `<div class="msg-bubble">${escapeHtml(text)}</div>`;
  container.appendChild(row);
  scrollToBottom();
}

// ==================== UTILS ====================
function scrollToBottom() {
  const c = document.getElementById("messages-container");
  c.scrollTop = c.scrollHeight;
}

function formatTime(timestamp) {
  if (!timestamp) return new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
  return new Date(timestamp).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" });
}

function escapeHtml(text) {
  const d = document.createElement("div");
  d.textContent = text ?? "";
  return d.innerHTML;
}

// ==================== EVENT LISTENERS ====================
document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("loginBtn").addEventListener("click", login);
  document.getElementById("sendBtn").addEventListener("click", sendMessage);
  document.getElementById("logoutBtn").addEventListener("click", disconnectWebSocket);

  document.getElementById("password").addEventListener("keypress", e => {
    if (e.key === "Enter") login();
  });

  document.getElementById("messageInput").addEventListener("keypress", e => {
    if (e.key === "Enter") sendMessage();
  });
});