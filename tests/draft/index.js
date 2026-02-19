import { SERVER_IP, SERVER_PORT } from "./secret.js";

// ==================== GLOBAL STATE ====================
let token = null;
let userId = null;
let username = null;
let ws = null;
let activeChannelId = null;

const userCache = new Map();      // id_user -> username
const channelCache = new Map();   // id_channel -> ChannelInfo
const outgoingCache = new Map();  // id_channel -> ChannelInfo (pending sent)

const SERVER_URL = `http://${SERVER_IP ?? "192.168.0.117"}:${SERVER_PORT ?? "8080"}`;
const WS_URL = `ws://${SERVER_IP}:${SERVER_PORT}/ws`;

// ==================== MESSAGE TYPES ====================
const MessageType = {
  WS_AUTH: 0, LOGOUT: 1,
  SEND_MESSAGE: 10, CREATE_CHANNEL: 11, ACCEPT_INVITATION: 12, REJECT_INVITATION: 13,
  LEAVE_CHANNEL: 14, UPDATE_CHANNEL_TITLE: 15, MARK_AS_READ: 16,
  TYPING_START: 17, TYPING_STOP: 18, REQUEST_CHANNEL_HISTORY: 19, CHANNEL_OPEN: 20,
  WS_AUTH_SUCCESS: 100, NEW_MESSAGE: 101, CHANNEL_CREATED: 102, CHANNEL_INVITATION: 103,
  INVITATION_ACCEPTED: 104, INVITATION_REJECTED: 105, USER_JOINED: 106, USER_LEFT: 107,
  CHANNEL_ACTIVATED: 108, CHANNEL_DELETED: 109, TITLE_UPDATED: 110, USER_STATUS: 111,
  USER_TYPING: 112, INITIAL_DATA: 113, CHANNEL_HISTORY: 114, ERROR: 255
};

// ==================== LOGIN ====================
async function login() {
  const usernameInput = document.getElementById("username").value.trim();
  const passwordInput = document.getElementById("password").value;
  const errorDiv = document.getElementById("login-error");
  const loginBtn = document.getElementById("loginBtn");

  if (!usernameInput || !passwordInput) { errorDiv.textContent = "Please fill in all fields."; return; }

  loginBtn.disabled = true;
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
      token = data.token;
      userId = data.id_user;
      username = data.username;
      showApp();
      connectWebSocket();
    } else {
      errorDiv.textContent = data.message ?? "Login failed.";
      loginBtn.disabled = false;
      loginBtn.textContent = "Connect";
    }
  } catch (err) {
    errorDiv.textContent = `Connection error: ${err.message}`;
    loginBtn.disabled = false;
    loginBtn.textContent = "Connect";
  }
}

// ==================== SHOW APP =====================
function showApp() {
  document.getElementById("login-screen").style.display = "none";
  document.getElementById("app-screen").classList.add("visible");

  const initials = username?.slice(0, 2).toUpperCase() ?? "??";
  document.getElementById("user-avatar-initials").textContent = initials;
  document.getElementById("sidebar-username").textContent = username;
}

// ==================== HOME / CHAT NAVIGATION ====================
function showHome() {
  activeChannelId = null;

  document.getElementById("home-view").classList.remove("hidden");
  document.getElementById("chat-view").classList.add("hidden");

  document.getElementById("chat-icon").textContent = "⌂";
  document.getElementById("chat-title").textContent = "Home";
  document.getElementById("chat-sub").textContent = "";

  // Deactivate all sidebar channel items
  document.querySelectorAll("#channels-list .sidebar-item").forEach(el => el.classList.remove("active"));

  // Highlight user pill
  document.getElementById("user-pill").classList.add("active");
}

function showChat(channelId) {
  document.getElementById("home-view").classList.add("hidden");
  document.getElementById("chat-view").classList.remove("hidden");

  // De-highlight user pill
  document.getElementById("user-pill").classList.remove("active");
}

// ==================== HOME TABS ====================
function switchTab(tabName) {
  document.querySelectorAll(".home-tab").forEach(t => t.classList.remove("active"));
  document.querySelectorAll(".tab-content").forEach(c => c.classList.remove("active"));
  document.getElementById(`tab-${tabName}`).classList.add("active");
  document.getElementById(`tab-content-${tabName}`).classList.add("active");
}

function updateInvitationsBadge() {
  const badge = document.getElementById("invitations-badge");
  const invList = document.getElementById("home-invitations-content");
  const count = invList.querySelectorAll(".inv-card:not(.outgoing)").length;
  if (count > 0) {
    badge.textContent = count;
    badge.style.display = "inline-block";
  } else {
    badge.style.display = "none";
  }
}

// ==================== WEBSOCKET ====================
function connectWebSocket() {
  ws = new WebSocket(WS_URL);

  ws.onopen = () => {
    setWsDot("connecting");
    setTimeout(() => ws.send(JSON.stringify({ type: MessageType.WS_AUTH, token })), 50);
  };

  ws.onmessage = (event) => {
    try {
      const data = JSON.parse(event.data);
      switch (data.type) {
        case MessageType.WS_AUTH_SUCCESS: onAuthSuccess(); break;
        case MessageType.INITIAL_DATA: handleInitialData(data); break;
        case MessageType.NEW_MESSAGE: handleNewMessage(data); break;
        case MessageType.CHANNEL_HISTORY: handleChannelHistory(data); break;
        case MessageType.USER_TYPING: handleTypingNotification(data); break;
        case MessageType.INVITATION_ACCEPTED: handleInvitationAccepted(data); break;
        case MessageType.INVITATION_REJECTED: handleInvitationRejected(data); break;
        case MessageType.USER_JOINED: handleUserJoined(data); break;
        case MessageType.ERROR: addSystemMessage(`Server Error: ${data.message} (${data.error_code})`); break;
        default: console.log("Unhandled message type:", data.type, data);
      }
    } catch (e) { console.error("Error parsing message:", e); }
  };

  ws.onerror = () => setWsDot("error");
  ws.onclose = (event) => {
    setWsDot("disconnected");
    document.getElementById("messageInput").disabled = true;
    document.getElementById("sendBtn").disabled = true;
    addSystemMessage(`Disconnected: ${event.reason || "connection closed"}`);
  };
}

function onAuthSuccess() {
  setWsDot("connected");
  document.getElementById("messageInput").disabled = false;
  document.getElementById("sendBtn").disabled = false;
}

function disconnectWebSocket() {
  if (ws) {
    if (ws.readyState === WebSocket.OPEN) ws.send(JSON.stringify({ type: MessageType.LOGOUT, reason: "User logged out" }));
    ws.close();
    ws = null;
  }
}

function setWsDot(state) {
  const dot = document.getElementById("ws-dot");
  dot.className = "ws-status-dot";
  if (state === "connected") { dot.classList.add("connected"); dot.title = "Connected"; }
  else if (state === "error") { dot.classList.add("error"); dot.title = "Error"; }
  else { dot.title = "Disconnected"; }
}

// ==================== INITIAL DATA ====================
function handleInitialData(data) {
  console.log("[INIT] raw data:", JSON.stringify(data));

  if (Array.isArray(data.contacts)) data.contacts.forEach(c => userCache.set(c.id_user, c.username));
  if (userId && username) userCache.set(userId, username);

  if (Array.isArray(data.channels)) {
    data.channels.forEach(ch => {
      channelCache.set(ch.id_channel, ch);
      ch.participants?.forEach(p => userCache.set(p.id_user, p.username));
    });
  }

  if (Array.isArray(data.outgoing_invitations)) {
    data.outgoing_invitations.forEach(ch => {
      outgoingCache.set(ch.id_channel, ch);
      ch.participants?.forEach(p => userCache.set(p.id_user, p.username));
    });
  }

  renderChannelsSidebar();
  renderContactsSidebar(data.contacts ?? []);
  renderHomeContacts(data.contacts ?? []);
  renderHomeInvitations(data.invitations ?? [], data.outgoing_invitations ?? []);

  console.log(`[INIT] ${userCache.size} users, ${channelCache.size} channels, ${(data.invitations ?? []).length} incoming, ${outgoingCache.size} outgoing`);
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
    item.className = "sidebar-item";
    item.dataset.id = ch.id_channel;

    const iconChar = ch.is_group ? "⊞" : "◈";
    const preview = ch.last_message?.body ?? "";
    const unread = ch.unread_count > 0 ? `<div class="unread-badge">${ch.unread_count}</div>` : "";

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
      <div class="item-info"><div class="item-name">${escapeHtml(c.username)}</div></div>
    `;
    list.appendChild(item);
  });
}

// ==================== HOME VIEW RENDERING ====================
function renderHomeContacts(contacts) {
  const grid = document.getElementById("home-contacts-list");
  grid.innerHTML = "";

  if (contacts.length === 0) {
    grid.innerHTML = `<div class="empty-home"><div class="empty-home-icon">◈</div><div class="empty-home-text">No contacts yet</div></div>`;
    return;
  }

  contacts.forEach(c => {
    const card = document.createElement("div");
    card.className = "contact-card";
    const initials = c.username.slice(0, 2).toUpperCase();
    card.innerHTML = `
      <div class="contact-avatar">${initials}</div>
      <div>
        <div class="contact-name">${escapeHtml(c.username)}</div>
        <div class="contact-sub">Click to open chat</div>
      </div>
    `;
    // Find their shared DM channel and open it
    card.addEventListener("click", () => {
      const dmChannel = [...channelCache.values()].find(ch =>
        !ch.is_group && ch.participants?.some(p => p.id_user === c.id_user)
      );
      if (dmChannel) selectChannel(dmChannel.id_channel);
    });
    grid.appendChild(card);
  });
}

function renderHomeInvitations(incoming, outgoing) {
  const container = document.getElementById("home-invitations-content");
  container.innerHTML = "";

  const hasIncoming = incoming.length > 0;
  const hasOutgoing = outgoing.length > 0;

  if (!hasIncoming && !hasOutgoing) {
    container.innerHTML = `<div class="empty-home"><div class="empty-home-icon">✦</div><div class="empty-home-text">No pending invitations</div></div>`;
    updateInvitationsBadge();
    return;
  }

  // Incoming
  if (hasIncoming) {
    const title = document.createElement("div");
    title.className = "inv-section-title";
    title.textContent = "Incoming";
    container.appendChild(title);

    incoming.forEach(inv => {
      const inviterName = userCache.get(inv.id_inviter) ?? `User ${inv.id_inviter}`;
      const memberNames = (inv.other_participant_ids ?? []).map(p => p.username).join(", ");

      const card = document.createElement("div");
      card.className = "inv-card";
      card.dataset.id = inv.id_channel;
      card.innerHTML = `
        <div class="inv-avatar">✉</div>
        <div class="inv-info">
          <div class="inv-title">${escapeHtml(inv.title)}</div>
          <div class="inv-meta">From ${escapeHtml(inviterName)}${memberNames ? ` · ${escapeHtml(memberNames)}` : ""}</div>
        </div>
        <div class="inv-actions">
          <button class="inv-btn accept-btn" data-id="${inv.id_channel}">Accept</button>
          <button class="inv-btn reject-btn" data-id="${inv.id_channel}">Decline</button>
        </div>
      `;
      card.querySelector(".accept-btn").addEventListener("click", () => acceptInvitation(inv.id_channel));
      card.querySelector(".reject-btn").addEventListener("click", () => rejectInvitation(inv.id_channel));
      container.appendChild(card);
    });
  }

  // Outgoing
  if (hasOutgoing) {
    const title = document.createElement("div");
    title.className = "inv-section-title";
    title.textContent = "Sent";
    container.appendChild(title);

    outgoing.forEach(ch => {
      const waitingFor = (ch.participants ?? []).map(p => p.username).join(", ");
      const card = document.createElement("div");
      card.className = "inv-card outgoing";
      card.dataset.id = ch.id_channel;
      card.innerHTML = `
        <div class="inv-avatar">⏳</div>
        <div class="inv-info">
          <div class="inv-title">${escapeHtml(ch.title)}</div>
          <div class="inv-meta">Waiting for: ${escapeHtml(waitingFor || "...")}</div>
        </div>
        <span class="outgoing-tag">Pending</span>
      `;
      container.appendChild(card);
    });
  }

  updateInvitationsBadge();
}

function acceptInvitation(id_channel) {
  if (!ws || ws.readyState !== WebSocket.OPEN) return;
  ws.send(JSON.stringify({ type: MessageType.ACCEPT_INVITATION, id_channel }));
}

function rejectInvitation(id_channel) {
  if (!ws || ws.readyState !== WebSocket.OPEN) return;
  ws.send(JSON.stringify({ type: MessageType.REJECT_INVITATION, id_channel }));
}

// ==================== CHANNEL SELECTION ====================
function selectChannel(channelId) {
  activeChannelId = channelId;
  const ch = channelCache.get(channelId);

  showChat(channelId);

  document.querySelectorAll("#channels-list .sidebar-item").forEach(el => {
    el.classList.toggle("active", el.dataset.id == channelId);
  });

  document.getElementById("chat-icon").textContent = ch?.is_group ? "⊞" : "◈";
  document.getElementById("chat-title").textContent = ch?.title ?? `Channel ${channelId}`;
  const participantCount = ch?.participants?.length ?? 0;
  document.getElementById("chat-sub").textContent = participantCount > 0 ? `${participantCount} members` : "";

  const sidebarItem = document.querySelector(`#channels-list .sidebar-item[data-id="${channelId}"]`);
  if (sidebarItem) { const badge = sidebarItem.querySelector(".unread-badge"); if (badge) badge.remove(); }

  document.getElementById("messages-container").innerHTML = "";
  requestChannelHistory(channelId, 0);
}

function requestChannelHistory(channelId, beforeMessageId) {
  if (!ws || ws.readyState !== WebSocket.OPEN) return;
  ws.send(JSON.stringify({ type: MessageType.REQUEST_CHANNEL_HISTORY, id_channel: channelId, before_id_message: beforeMessageId, limit: 50 }));
}

// ==================== NEW MESSAGE ====================
function handleNewMessage(data) {
  if (!data.message) return;

  const msg = data.message;
  const isMe = msg.id_sender === userId;
  const isSystem = msg.is_system;
  const sender = isSystem ? "System" : (userCache.get(msg.id_sender) ?? `User ${msg.id_sender}`);

  const chData = channelCache.get(data.id_channel);
  if (chData) {
    chData.last_message = msg;
    const item = document.querySelector(`#channels-list .sidebar-item[data-id="${data.id_channel}"]`);
    const preview = item?.querySelector(".item-preview");
    if (preview) preview.textContent = msg.body;

    if (data.id_channel !== activeChannelId) {
      if (item) {
        let badge = item.querySelector(".unread-badge");
        if (!badge) { badge = document.createElement("div"); badge.className = "unread-badge"; badge.textContent = "1"; item.appendChild(badge); }
        else badge.textContent = parseInt(badge.textContent) + 1;
      }
      return;
    }
  }

  if (data.id_channel !== activeChannelId) return;

  if (isSystem) addSystemMessage(msg.body);
  else if (isMe) addSentMessage(msg.body, sender, msg.timestamp);
  else addReceivedMessage(msg.body, sender, msg.timestamp);
}

function handleTypingNotification(data) {
  if (data.id_user === userId) return;
  const name = userCache.get(data.id_user) ?? `User ${data.id_user}`;
  console.log(data.is_typing ? `${name} is typing...` : `${name} stopped typing`);
}

// ==================== CHANNEL HISTORY ====================
function handleChannelHistory(data) {
  if (data.id_channel !== activeChannelId) return;
  const container = document.getElementById("messages-container");
  container.innerHTML = "";

  if (!data.messages || data.messages.length === 0) { addSystemMessage("No messages yet. Say hello!"); return; }

  data.messages.forEach(msg => {
    const isMe = msg.id_sender === userId;
    const isSystem = msg.is_system;
    const sender = isSystem ? "System" : (userCache.get(msg.id_sender) ?? `User ${msg.id_sender}`);
    if (isSystem) addSystemMessage(msg.body);
    else if (isMe) addSentMessage(msg.body, sender, msg.timestamp);
    else addReceivedMessage(msg.body, sender, msg.timestamp);
  });

  if (data.has_more) {
    const loadMore = document.createElement("div");
    loadMore.className = "date-sep";
    loadMore.style.cursor = "pointer";
    loadMore.style.color = "var(--accent)";
    loadMore.textContent = "↑ Load older messages";
    loadMore.addEventListener("click", () => requestChannelHistory(activeChannelId, data.messages[0]?.id_message ?? 0));
    container.prepend(loadMore);
  }
}

// ==================== INVITATION HANDLERS ====================
function handleInvitationAccepted(data) {
  const ch = data.channel;
  channelCache.set(ch.id_channel, ch);
  ch.participants?.forEach(p => userCache.set(p.id_user, p.username));

  // Remove from home invitations UI
  const card = document.querySelector(`#home-invitations-content .inv-card[data-id="${ch.id_channel}"]`);
  if (card) card.remove();
  updateInvitationsBadge();

  renderChannelsSidebar();
  selectChannel(ch.id_channel); // go straight to the channel
}



function handleUserJoined(data) {
  const contact = data.contact;
  userCache.set(contact.id_user, contact.username);

  if (outgoingCache.has(data.id_channel)) {
    const ch = outgoingCache.get(data.id_channel);
    ch.participants = ch.participants.filter(p => p.id_user !== contact.id_user);
    ch.participants.push(contact);
    ch.is_group = ch.participants.length > 2;
    channelCache.set(data.id_channel, ch);
    outgoingCache.delete(data.id_channel);

    // Remove outgoing card from home
    const card = document.querySelector(`#home-invitations-content .inv-card[data-id="${data.id_channel}"]`);
    if (card) card.remove();
    updateInvitationsBadge();

    renderChannelsSidebar();
  } else {
    const ch = channelCache.get(data.id_channel);
    if (ch) {
      ch.participants = ch.participants.filter(p => p.id_user !== contact.id_user);
      ch.participants.push(contact);
      ch.is_group = ch.participants.length > 2;
    }
  }
}

function handleInvitationRejected(data) {
  const { id_channel, contact } = data;
  const iAmRejecter = contact.id_user === userId;

  if (iAmRejecter) {
    // Remove from incoming invitations list
    const card = document.querySelector(`#home-invitations-content .inv-card:not(.outgoing)[data-id="${id_channel}"]`);
    if (card) card.remove();
  } else {
    // I'm the creator — contact tells me who rejected
    userCache.set(contact.id_user, contact.username);
    outgoingCache.delete(id_channel);

    const card = document.querySelector(`#home-invitations-content .inv-card.outgoing[data-id="${id_channel}"]`);
    if (card) card.remove();
  }

  updateInvitationsBadge();
}

// ==================== MESSAGING ====================
function sendMessage() {
  const input = document.getElementById("messageInput");
  const message = input.value.trim();
  if (!message || !ws || ws.readyState !== WebSocket.OPEN) return;
  ws.send(JSON.stringify({ type: MessageType.SEND_MESSAGE, id_channel: activeChannelId, body: message }));
  input.value = "";
}

// ==================== MESSAGE RENDERING ====================
function addSentMessage(text, senderName, timestamp) {
  const container = document.getElementById("messages-container");
  const row = document.createElement("div");
  row.className = "msg-row sent";
  row.innerHTML = `
    <div class="msg-avatar">${senderName.slice(0, 2).toUpperCase()}</div>
    <div class="msg-bubble-wrap">
      <div class="msg-sender">${escapeHtml(senderName)}</div>
      <div class="msg-bubble">${escapeHtml(text)}</div>
      <div class="msg-time">${formatTime(timestamp)}</div>
    </div>
  `;
  container.appendChild(row);
  scrollToBottom();
}

function addReceivedMessage(text, senderName, timestamp) {
  const container = document.getElementById("messages-container");
  const row = document.createElement("div");
  row.className = "msg-row received";
  row.innerHTML = `
    <div class="msg-avatar">${senderName.slice(0, 2).toUpperCase()}</div>
    <div class="msg-bubble-wrap">
      <div class="msg-sender">${escapeHtml(senderName)}</div>
      <div class="msg-bubble">${escapeHtml(text)}</div>
      <div class="msg-time">${formatTime(timestamp)}</div>
    </div>
  `;
  container.appendChild(row);
  scrollToBottom();
}

function addSystemMessage(text) {
  const container = document.getElementById("messages-container");
  const row = document.createElement("div");
  row.className = "msg-row system";
  row.style.alignSelf = "center";
  const resolved = resolveAtMentions(text);  // resolve before escaping
  row.innerHTML = `<div class="msg-bubble">${escapeHtml(resolved)}</div>`;
  container.appendChild(row);
  scrollToBottom();
}


function resolveAtMentions(text) {
  return text.replace(/@(\d+)/g, (match, rawId) => {
    const name = userCache.get(parseInt(rawId));
    return name ? `@${name}` : match; // fallback to @id if unknown
  });
}

// ==================== UTILS ====================
function scrollToBottom() { const c = document.getElementById("messages-container"); c.scrollTop = c.scrollHeight; }
function formatTime(ts) { if (!ts) return new Date().toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" }); return new Date(ts).toLocaleTimeString([], { hour: "2-digit", minute: "2-digit" }); }
function escapeHtml(text) { const d = document.createElement("div"); d.textContent = text ?? ""; return d.innerHTML; }

// ==================== EVENT LISTENERS ====================
document.addEventListener("DOMContentLoaded", () => {
  document.getElementById("loginBtn").addEventListener("click", login);
  document.getElementById("sendBtn").addEventListener("click", sendMessage);
  document.getElementById("logoutBtn").addEventListener("click", disconnectWebSocket);
  document.getElementById("password").addEventListener("keypress", e => { if (e.key === "Enter") login(); });
  document.getElementById("messageInput").addEventListener("keypress", e => { if (e.key === "Enter") sendMessage(); });

  // User pill → go home
  document.getElementById("user-pill").addEventListener("click", showHome);

  // Tab switching
  document.querySelectorAll(".home-tab").forEach(tab => {
    tab.addEventListener("click", () => switchTab(tab.dataset.tab));
  });
});