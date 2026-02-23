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
  document.getElementById("leaveChannelBtn").classList.add("hidden");

  document.getElementById("chat-icon").textContent = "⌂";
  document.getElementById("chat-title").textContent = "Home";
  document.getElementById("chat-sub").textContent = "";

  document.querySelectorAll("#channels-list .sidebar-item").forEach(el => el.classList.remove("active"));
  document.getElementById("user-pill").classList.add("active");
}

function showChat(channelId) {
  document.getElementById("home-view").classList.add("hidden");
  document.getElementById("chat-view").classList.remove("hidden");
  document.getElementById("user-pill").classList.remove("active");
  document.getElementById("leaveChannelBtn").classList.remove("hidden");
}

// ==================== DM TITLE RESOLUTION ====================
function getChannelDisplayTitle(ch) {
  if (ch.is_group) return ch.title;
  const other = ch.participants?.find(p => p.id_user !== userId);
  return other?.username ?? ch.title;
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
  window.ws = ws; 

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
        case MessageType.USER_LEFT: handleUserLeft(data); break;
        case MessageType.CHANNEL_CREATED: handleChannelCreated(data); break;
        case MessageType.ERROR: handleServerError(data); break;
        case MessageType.CHANNEL_INVITATION: handleChannelInvitation(data); break;
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

// ==================== SERVER ERROR ====================
function handleServerError(data) {
  const modal = document.getElementById("createChannelModal");
  if (modal.classList.contains("open")) {
    setModalError(data.message ?? "An error occurred");
    document.getElementById("submitCreateChannelBtn").disabled = false;
  } else {
    addSystemMessage(`Server Error: ${data.message} (${data.error_code})`);
  }
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
      // only keep invitees, not self — so length === 0 works correctly on rejection
      ch.participants = (ch.participants ?? []).filter(p => p.id_user !== userId);
      outgoingCache.set(ch.id_channel, ch);
      ch.participants.forEach(p => userCache.set(p.id_user, p.username));
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
    const displayTitle = getChannelDisplayTitle(ch);
    const preview = ch.last_message?.body ?? "";
    const unread = ch.unread_count > 0 ? `<div class="unread-badge">${ch.unread_count}</div>` : "";

    item.innerHTML = `
      <div class="item-icon">${iconChar}</div>
      <div class="item-info">
        <div class="item-name">${escapeHtml(displayTitle)}</div>
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

  if (hasOutgoing) {
    const title = document.createElement("div");
    title.className = "inv-section-title";
    title.textContent = "Sent";
    container.appendChild(title);

    outgoing.forEach(ch => appendOutgoingCard(container, ch));
  }

  updateInvitationsBadge();
}

function appendOutgoingCard(container, ch) {
  let sentTitle = container.querySelector(".inv-section-title[data-section='sent']");
  if (!sentTitle) {
    sentTitle = document.createElement("div");
    sentTitle.className = "inv-section-title";
    sentTitle.dataset.section = "sent";
    sentTitle.textContent = "Sent";
    container.appendChild(sentTitle);
  }

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
}

// ==================== INVITATION ACTIONS ====================
async function acceptInvitation(id_channel) {
  try {
    const response = await fetch(`${SERVER_URL}/invitations/${id_channel}/accept`, {
      method: "PATCH",
      headers: { "X-Auth-Token": token }
    });
    if (!response.ok) {
      const err = await response.json();
      console.error("[ACCEPT] Error:", err.error);
      return;
    }
    const data = await response.json();
    handleInvitationAccepted(data);
  } catch (err) {
    console.error("[ACCEPT] Network error:", err.message);
  }
}

async function rejectInvitation(id_channel) {
  try {
    const response = await fetch(`${SERVER_URL}/invitations/${id_channel}/reject`, {
      method: "PATCH",
      headers: { "X-Auth-Token": token }
    });
    if (!response.ok) {
      const err = await response.json();
      console.error("[REJECT] Error:", err.error);
      return;
    }
    const data = await response.json();
    handleInvitationRejected(data);
  } catch (err) {
    console.error("[REJECT] Network error:", err.message);
  }
}

// ==================== CHANNEL SELECTION ====================
function selectChannel(channelId) {
  activeChannelId = channelId;
  const ch = channelCache.get(channelId);

  showChat(channelId);

  document.querySelectorAll("#channels-list .sidebar-item").forEach(el => {
    el.classList.toggle("active", el.dataset.id == channelId);
  });

  const displayTitle = ch ? getChannelDisplayTitle(ch) : `Channel ${channelId}`;
  document.getElementById("chat-icon").textContent = ch?.is_group ? "⊞" : "◈";
  document.getElementById("chat-title").textContent = displayTitle;
  const participantCount = ch?.participants?.length ?? 0;
  document.getElementById("chat-sub").textContent = participantCount > 0 ? `${participantCount} members` : "";

 const sidebarItem = document.querySelector(`#channels-list .sidebar-item[data-id="${channelId}"]`);
  if (sidebarItem) { const badge = sidebarItem.querySelector(".unread-badge"); if (badge) badge.remove(); }

  // Auto mark as read when opening channel
  if (ch && ch.last_message && ch.last_message.id_message > 0 && ws?.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ type: MessageType.MARK_AS_READ, id_channel: channelId, last_id_message: ch.last_message.id_message }));
    ch.unread_count = 0;
    ch.last_read_id_message = ch.last_message.id_message;
  }

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

  // Auto mark as read for incoming messages in active channel
  if (ws?.readyState === WebSocket.OPEN) {
    ws.send(JSON.stringify({ type: MessageType.MARK_AS_READ, id_channel: data.id_channel, last_id_message: msg.id_message }));
    if (chData) { chData.unread_count = 0; chData.last_read_id_message = msg.id_message; }
  }

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

  const card = document.querySelector(`#home-invitations-content .inv-card[data-id="${ch.id_channel}"]`);
  if (card) card.remove();
  updateInvitationsBadge();

  renderChannelsSidebar();
  selectChannel(ch.id_channel);
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
    // I rejected — remove my incoming card
    const card = document.querySelector(`#home-invitations-content .inv-card:not(.outgoing)[data-id="${id_channel}"]`);
    if (card) card.remove();
  } else {
    userCache.set(contact.id_user, contact.username);
    const ch = outgoingCache.get(id_channel);
    if (ch) {
      ch.participants = ch.participants.filter(p => p.id_user !== contact.id_user);
      if (ch.participants.length === 0) {
        // no more invitees pending — remove outgoing card entirely
        outgoingCache.delete(id_channel);
        const card = document.querySelector(`#home-invitations-content .inv-card.outgoing[data-id="${id_channel}"]`);
        if (card) card.remove();
      } else {
        // still waiting on others — update the card
        const card = document.querySelector(`#home-invitations-content .inv-card.outgoing[data-id="${id_channel}"]`);
        if (card) {
          const meta = card.querySelector(".inv-meta");
          if (meta) meta.textContent = `Waiting for: ${ch.participants.map(p => p.username).join(", ")}`;
        }
      }
    }
  }

  updateInvitationsBadge();
}

function handleChannelCreated(data) {
  const ch = data.channel;

  // only keep invitees, not self — so length === 0 works correctly on rejection
  ch.participants = (ch.participants ?? []).filter(p => p.id_user !== userId);
  outgoingCache.set(ch.id_channel, ch);
  ch.participants.forEach(p => userCache.set(p.id_user, p.username));

  const container = document.getElementById("home-invitations-content");
  const empty = container.querySelector(".empty-home");
  if (empty) empty.remove();

  appendOutgoingCard(container, ch);
  updateInvitationsBadge();

  switchTab("invitations");
  closeCreateChannelModal();
}

function handleChannelInvitation(data) {
  const inviterName = userCache.get(data.id_inviter) ?? `User ${data.id_inviter}`;
  data.other_participant_ids?.forEach(p => userCache.set(p.id_user, p.username));

  const container = document.getElementById("home-invitations-content");
  const empty = container.querySelector(".empty-home");
  if (empty) empty.remove();

  let incomingTitle = container.querySelector(".inv-section-title[data-section='incoming']");
  if (!incomingTitle) {
    incomingTitle = document.createElement("div");
    incomingTitle.className = "inv-section-title";
    incomingTitle.dataset.section = "incoming";
    incomingTitle.textContent = "Incoming";
    container.prepend(incomingTitle);
  }

  const memberNames = (data.other_participant_ids ?? []).map(p => p.username).join(", ");
  const card = document.createElement("div");
  card.className = "inv-card";
  card.dataset.id = data.id_channel;
  card.innerHTML = `
    <div class="inv-avatar">✉</div>
    <div class="inv-info">
      <div class="inv-title">${escapeHtml(data.title)}</div>
      <div class="inv-meta">From ${escapeHtml(inviterName)}${memberNames ? ` · ${escapeHtml(memberNames)}` : ""}</div>
    </div>
    <div class="inv-actions">
      <button class="inv-btn accept-btn" data-id="${data.id_channel}">Accept</button>
      <button class="inv-btn reject-btn" data-id="${data.id_channel}">Decline</button>
    </div>
  `;
  card.querySelector(".accept-btn").addEventListener("click", () => acceptInvitation(data.id_channel));
  card.querySelector(".reject-btn").addEventListener("click", () => rejectInvitation(data.id_channel));

  incomingTitle.insertAdjacentElement("afterend", card);
  updateInvitationsBadge();
  switchTab("invitations");
}

// ==================== LEAVE CHANNEL ====================
async function leaveChannel(id_channel) {
  try {
    const response = await fetch(`${SERVER_URL}/channels/${id_channel}/leave`, {
      method: "PATCH",
      headers: { "X-Auth-Token": token }
    });

    if (!response.ok) {
      const err = await response.json();
      console.error("[LEAVE] Error:", err.error);
      return;
    }

    channelCache.delete(id_channel);
    if (activeChannelId === id_channel) showHome();
    renderChannelsSidebar();
  } catch (err) {
    console.error("[LEAVE] Network error:", err.message);
  }
}

function handleUserLeft(data) {
  const ch = channelCache.get(data.id_channel);
  if (!ch) return;

  if (data.channel_deleted) {
    channelCache.delete(data.id_channel);
    if (activeChannelId === data.id_channel) showHome();
    renderChannelsSidebar();
    return;
  }

  ch.participants = ch.participants.filter(p => p.id_user !== data.id_user);
  ch.is_group = ch.participants.length > 2;

  if (activeChannelId === data.id_channel) {
    const name = userCache.get(data.id_user) ?? `User ${data.id_user}`;
    addSystemMessage(`${name} left the channel`);
    document.getElementById("chat-sub").textContent = `${ch.participants.length} members`;
  }

  renderChannelsSidebar();
}

// ==================== CREATE CHANNEL ====================
async function sendCreateChannel() {
  const usernames = [...tagState.tags];
  if (usernames.length === 0) return;

  const title = document.getElementById("channelTitleInput").value.trim();

  setModalError("");
  document.getElementById("submitCreateChannelBtn").disabled = true;

  try {
    const response = await fetch(`${SERVER_URL}/channels`, {
      method: "POST",
      headers: {
        "Content-Type": "application/json",
        "X-Auth-Token": token
      },
      body: JSON.stringify({ usernames, title })
    });

    if (response.status === 409) {
      const data = await response.json();
      if (channelCache.has(data.id_channel)) {
        selectChannel(data.id_channel);
      }
      closeCreateChannelModal();
      return;
    }

    if (!response.ok) {
      const err = await response.json();
      setModalError(err.error ?? "An error occurred");
      document.getElementById("submitCreateChannelBtn").disabled = false;
      return;
    }

    const data = await response.json();
    handleChannelCreated(data);
  } catch (err) {
    setModalError(`Connection error: ${err.message}`);
    document.getElementById("submitCreateChannelBtn").disabled = false;
  }
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
  const resolved = resolveAtMentions(text);
  row.innerHTML = `<div class="msg-bubble">${escapeHtml(resolved)}</div>`;
  container.appendChild(row);
  scrollToBottom();
}

function resolveAtMentions(text) {
  return text.replace(/@(\d+)/g, (match, rawId) => {
    const name = userCache.get(parseInt(rawId));
    return name ? `@${name}` : match;
  });
}

// ==================== MODAL ====================
const tagState = { tags: new Set() };

function openCreateChannelModal() {
  tagState.tags.clear();
  document.getElementById("tagInputWrap").querySelectorAll(".tag-pill").forEach(p => p.remove());
  document.getElementById("tagTextInput").value = "";
  document.getElementById("channelTitleInput").value = "";
  setModalError("");
  document.getElementById("submitCreateChannelBtn").disabled = true;
  document.getElementById("createChannelModal").classList.add("open");
  document.getElementById("tagTextInput").focus();
}

function closeCreateChannelModal() {
  document.getElementById("createChannelModal").classList.remove("open");
  document.getElementById("submitCreateChannelBtn").disabled = false;
}

function setModalError(msg) {
  document.getElementById("createChannelError").textContent = msg;
}

function commitTag() {
  const input = document.getElementById("tagTextInput");
  const value = input.value.trim().replace(/^@/, "").replace(/,$/, "");
  if (!value) return;

  if (value === username) {
    setModalError("You can't invite yourself.");
    input.value = "";
    return;
  }

  if (tagState.tags.has(value)) {
    input.value = "";
    return;
  }

  tagState.tags.add(value);

  const pill = document.createElement("div");
  pill.className = "tag-pill";
  pill.dataset.username = value;
  pill.innerHTML = `
    <span>${escapeHtml(value)}</span>
    <button class="tag-pill-remove" title="Remove">×</button>
  `;
  pill.querySelector(".tag-pill-remove").addEventListener("click", () => {
    tagState.tags.delete(value);
    pill.remove();
    updateSubmitButton();
  });

  const wrap = document.getElementById("tagInputWrap");
  wrap.insertBefore(pill, input);
  input.value = "";
  setModalError("");
  updateSubmitButton();
}

function updateSubmitButton() {
  document.getElementById("submitCreateChannelBtn").disabled = tagState.tags.size === 0;
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

  document.getElementById("user-pill").addEventListener("click", showHome);

  document.querySelectorAll(".home-tab").forEach(tab => {
    tab.addEventListener("click", () => switchTab(tab.dataset.tab));
  });

  document.getElementById("openCreateChannelBtn").addEventListener("click", openCreateChannelModal);
  document.getElementById("cancelCreateChannelBtn").addEventListener("click", closeCreateChannelModal);
  document.getElementById("submitCreateChannelBtn").addEventListener("click", sendCreateChannel);

  document.getElementById("createChannelModal").addEventListener("click", (e) => {
    if (e.target === document.getElementById("createChannelModal")) closeCreateChannelModal();
  });

  document.addEventListener("keydown", (e) => {
    if (e.key === "Escape") closeCreateChannelModal();
  });

  document.getElementById("tagTextInput").addEventListener("keydown", (e) => {
    if (e.key === " " || e.key === "," || e.key === "Enter") {
      e.preventDefault();
      commitTag();
    }
    if (e.key === "Backspace" && e.target.value === "") {
      const lastTag = [...tagState.tags].at(-1);
      if (lastTag) {
        tagState.tags.delete(lastTag);
        const wrap = document.getElementById("tagInputWrap");
        const pill = wrap.querySelector(`.tag-pill[data-username="${CSS.escape(lastTag)}"]`);
        if (pill) pill.remove();
        updateSubmitButton();
      }
    }
  });

  document.getElementById("tagInputWrap").addEventListener("click", () => {
    document.getElementById("tagTextInput").focus();
  });

  document.getElementById("leaveChannelBtn").addEventListener("click", () => {
    if (activeChannelId) leaveChannel(activeChannelId);
  });
});
