# Client Qt - Architecture

## 🏗️ Vue d'ensemble

L'application cliente Qt suit une architecture en couches avec séparation claire des responsabilités.

### **Point d'entrée : `client.cpp`**
```cpp
main() → crée QApplication → affiche MainWindow → lance event loop
```

---

## **📦 Séparation des responsabilités**

### **1. Core / Orchestration**

**`MainWindow`** (le chef d'orchestre)
- **Rôle** : Coordonne tout, gère l'état de l'application
- **Contient** :
  - LoginWidget (écran de connexion)
  - WebSocketClient (communication temps réel)
  - ChannelPanelWidget (liste des canaux)
  - RightPanelWidget (chat + input)
- **État** : `currentUserId`, `currentChannelId`, `authToken`, maps de users/channels
- **Quand utilisé** : Toujours — c'est la fenêtre principale active pendant toute la session

---

### **2. Services (`services/`)**

**Couche métier** : logique réutilisable, pas d'UI

#### **`AuthManager`**
- **Rôle** : Gère le login HTTP
- **Utilise** : `ApiClient` pour faire la requête `POST /login`
- **Émet** : `loginSucceeded(username, token)` ou `loginFailed()`
- **Quand** : Appelé quand l'utilisateur clique "Login" dans `LoginWidget`

#### **`ApiClient`**
- **Rôle** : Wrapper HTTP bas niveau (GET/POST avec QNetworkAccessManager)
- **Quand** : Utilisé par `AuthManager` pour les requêtes REST

#### **`ServerConfig`**
- **Rôle** : Config centralisée (URL serveur depuis `settings.ini`)
- **Quand** : Lu au démarrage par `WebSocketClient` et `ApiClient`

---

### **3. WebSocket (`ws/`)**

#### **`WebSocketClient`**
- **Rôle** : Communication temps réel avec le serveur
- **Responsabilités** :
  - Connexion WS avec token
  - Envoi de messages (`SEND_MESSAGE`, `REQUEST_CHANNEL_HISTORY`)
  - Réception et parsing des broadcasts (`NEW_MESSAGE`, `INITIAL_DATA`)
  - Reconnexion automatique
- **Signaux émis** :
  - `authenticated(idUser)`
  - `initialDataReceived(data)`
  - `channelHistoryReceived(history)`
  - `newMessageReceived(msg)`
- **Quand** : Créé après login réussi, reste actif toute la session

---

### **4. Utils (`utils/`)**

**Outils purs** : pas d'état, juste des transformations

#### **`message_json.hpp/.cpp`**
- **Rôle** : Sérialisation/désérialisation JSON ↔ structs C++
- **Fonctions** :
  - `to_json()` : struct → QJsonObject (pour envoyer)
  - `from_json()` : QJsonObject → struct (pour recevoir)
- **Quand** : Utilisé par `WebSocketClient` à chaque message envoyé/reçu

#### **`message_qt_types.hpp`**
- **Rôle** : Enregistre les types pour signaux Qt (`qRegisterMetaType`)
- **Quand** : Appelé une fois au démarrage dans `main()`

---

### **5. Widgets (`widgets/`)**

**Composants UI** réutilisables

#### **`ChannelPanelWidget`**
- **Rôle** : Panneau gauche avec liste des canaux
- **Contient** : QListWidget de `ChannelRowWidget`
- **Émet** : `channelSelected(channelId, title)`
- **Quand** : Créé à l'entrée dans la vue chat, populé avec `INITIAL_DATA`

#### **`ChannelRowWidget`**
- **Rôle** : Une ligne dans la liste (titre canal + preview + badge unread)
- **Quand** : Créé pour chaque canal dans `INITIAL_DATA`

#### **`RightPanelWidget`**
- **Rôle** : Panneau droit (titre + messages + input)
- **Contient** : QScrollArea + QLineEdit
- **Émet** : `sendRequested(message)`
- **Quand** : Créé à l'entrée dans la vue chat, mis à jour à chaque sélection de canal

#### **`MessageItemWidget`**
- **Rôle** : Bulle de message individuel (style, timestamp, username)
- **Quand** : Créé pour chaque message dans l'historique ou nouveau message reçu

---

### **6. Models (`models/`)**

**Structures de données** (actuellement peu utilisées)

#### **`MessageModel` / `ChannelModel`**
- **Rôle prévu** : Encapsuler logique métier des messages/canaux
- **État actuel** : Existent mais MainWindow utilise surtout des `QHash` directs
- **Utilisation future** : Pourraient gérer cache, tri, filtrage

---

## **🔄 Flow d'exécution**

### **1️⃣ Démarrage**
```
main() 
  → Enregistre types Qt
  → Crée MainWindow
    → Affiche LoginWidget
```

### **2️⃣ Login**
```
User entre credentials → LoginWidget::onLoginClicked()
  → AuthManager::login()
    → ApiClient::post("/login")
      ← Serveur répond {token, username, id_user}
    → emit loginSucceeded(username, token)
  → MainWindow::onLoginSuccessful()
    → Crée WebSocketClient
    → WebSocketClient::connectWithToken(token)
```

### **3️⃣ Authentification WS**
```
WebSocketClient::onConnected()
  → Envoie WS_AUTH avec token
    ← Serveur répond WS_AUTH_SUCCESS {id_user}
  → emit authenticated(idUser)
    ← Serveur envoie INITIAL_DATA {channels, contacts}
  → emit initialDataReceived(data)
    → MainWindow::onInitialDataReceived()
      → Cache users dans userNamesById
      → Populate ChannelPanelWidget
      → Switch vers vue chat
```

### **4️⃣ Ouvrir un canal**
```
User clique canal → ChannelPanelWidget emit channelSelected(channelId)
  → MainWindow::onChannelSelected()
    → Met à jour currentChannelId
    → WebSocketClient::openChannel(channelId)
      → Envoie REQUEST_CHANNEL_HISTORY
        ← Serveur répond CHANNEL_HISTORY {messages[]}
      → emit channelHistoryReceived(history)
        → MainWindow::onChannelHistoryReceived()
          → Clear rightPanel
          → Pour chaque message : createMessageWidget() et affiche
```

### **5️⃣ Envoyer un message**
```
User tape message + Enter → RightPanelWidget emit sendRequested(text)
  → MainWindow::onSendMessageRequested()
    → WebSocketClient::sendMessage(channelId, text)
      → Envoie SEND_MESSAGE {id_channel, body}
        ← Serveur broadcast NEW_MESSAGE à tous participants
      → emit newMessageReceived(msg)
        → MainWindow::onNewMessageReceived()
          → appendMessageToView() — ajoute bulle en bas
```

---

## **🎯 Pourquoi cette séparation ?**

| Couche | Responsabilité | Testable solo | Réutilisable |
|--------|----------------|---------------|--------------|
| **Services** | Logique métier HTTP/config | ✅ | ✅ |
| **WS** | Protocole temps réel | ✅ | ✅ |
| **Utils** | Parsing/sérialisation pure | ✅ | ✅ |
| **Widgets** | Composants UI isolés | ✅ | ✅ |
| **MainWindow** | Orchestration globale | ❌ (intégration) | ❌ |
| **Models** | État/logique métier données | ✅ | ✅ |

**Avantages :**
- **Maintenabilité** : bug dans le parsing → `message_json.cpp` uniquement
- **Testabilité** : tester `AuthManager` sans lancer toute l'app
- **Réutilisabilité** : `MessageItemWidget` peut servir ailleurs
- **Clarté** : chacun sa job, évite le "god object"

---

## **📝 Conventions C++**

### **Références (`&`)**

```cpp
void foo(const QString& title)  // Passage par référence constante
```

- `&` = référence (pas de copie, utilise l'original)
- `const` = lecture seule (ne peut pas modifier)
- **Best practice** : utiliser `const &` pour tout objet non-trivial (QString, std::string, vector, etc.)

### **Paramètres de sortie**

```cpp
bool from_json(const QJsonObject& obj, AuthMessages::WSAuthResponse& out);
```

- `obj` (const &) = **entrée** : JSON à parser (lecture seule)
- `out` (&) = **sortie** : structure à remplir (modifiable)
- Pattern "output parameter" classique en C++

### **Signaux et Slots Qt**

- **Signaux** : émis par un objet pour notifier un événement
- **Slots** : méthodes appelées en réponse aux signaux
- Connexion : `connect(emetteur, &Class::signal, recepteur, &Class::slot)`

---

**TL;DR :** `MainWindow` = chef d'orchestre ; `services/` = logique métier ; `ws/` = temps réel ; `utils/` = outils purs ; `widgets/` = composants UI ; `models/` = structures données.
