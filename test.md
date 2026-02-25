# WizzMania — Résumé des modifications backend

## Feature : MARK_AS_READ (v1 — fire-and-forget)
**Branche** : `feature/mark-as-read`

Le client envoie `{type: 16, id_channel, last_id_message}` via WebSocket. Le serveur met à jour `last_read_id_message` en BDD pour persister le dernier message lu. Cela corrige le bug où les messages repassaient en "non lu" après déconnexion/reconnexion.

### Fichiers modifiés (8 fichiers)

| Fichier | Modification |
|---------|-------------|
| `common/message_structure.hpp` | Ajout struct `MarkAsRead` (type, id_channel, last_id_message, unread_count) |   ajout count pour que le client
//Parce que le serveur doit renvoyer le nombre de messages non lus au client après le mark as read. Sans le unread_count dans la struct, le client ne saurait pas combien de messages non lus restent dans le channel.
C'est ce qui permet au front de mettre à jour le badge : il reçoit unread_count: 0 → il efface le badge. Et sur tous les devices en même temps.
| `common/messages.hpp` | `MARK_AS_READ = 16` déjà existant, vérifié |
| `server/headers/json_helpers.hpp` | Ajout `parse_mark_as_read()` pour parser le JSON client |
| `server/headers/database.hpp` | Déclaration `update_last_read_message()` |
| `server/sources/database/database_messages.cpp` | Implémentation SQL UPDATE avec condition anti-recul (`last_read_id_message < ?`) |
| `server/headers/message/message_service.hpp` | Déclaration `mark_as_read()` |
| `server/sources/message/message_service.cpp` | Appel DB + log warning si pas de mise à jour |
| `server/headers/message/message_controller.hpp` | Déclaration `mark_as_read()` |
| `server/sources/message/message_controller.cpp` | Parse → vérification accès → appel service |
| `server/sources/server.cpp` | Ajout `case MARK_AS_READ` dans le switch WebSocket |

### Points techniques
- La condition SQL `last_read_id_message IS NULL OR last_read_id_message < ?` empêche de reculer le curseur de lecture
- `std::lock_guard<std::mutex>` pour la thread-safety des accès DB
- Vérification d'accès au channel avant toute modification

---

## Feature : MARK_AS_READ (v2 — multi-device broadcast)

Après l'update en BDD, le serveur renvoie maintenant une réponse à **tous les appareils connectés** de l'utilisateur avec le `unread_count` recalculé. Cela permet la synchronisation multi-device.

### Fichiers modifiés (3 fichiers)

| Fichier | Modification |
|---------|-------------|
| `server/headers/json_helpers.hpp` | Ajout `to_json(MarkAsRead)` pour sérialiser la réponse serveur + ajout `to_json(Message)` manquant |
| `server/sources/message/message_controller.cpp` | Après l'update DB : calcul `get_unread_count()` puis `ws_manager.send_to_user()` pour broadcast |
| `common/message_structure.hpp` | Struct `MarkAsRead` sortie du namespace `ClientSend` → struct commune (utilisée par client et serveur) |

### Réponse envoyée aux clients
```json
{
    "type": 16,
    "id_channel": 1,
    "last_id_message": 74,
    "unread_count": 0
}
```

### Point technique
- `ws_manager.send_to_user(id_user, ...)` envoie à toutes les connexions WebSocket de cet utilisateur (multi-device)
- `db.get_unread_count()` existait déjà dans la codebase

---

## Bugfix : UTF-8 crash

Le serveur crashait avec "Invalid UTF-8 in text frame" lors de l'envoi d'erreurs WebSocket.

| Fichier | Modification |
|---------|-------------|
| `server/headers/exception.hpp` | `err.error_code = error.get_code()` → `err.error_code = std::to_string(error.get_code())` |

**Cause** : un `int` était assigné à un `std::string`, créant un caractère UTF-8 invalide.

---

## Nettoyage : gitignore

Suppression de fichiers parasites créés accidentellement par une commande Qt logging.

| Fichier | Modification |
|---------|-------------|
| `.gitignore` | Ajout `QT_LOGGING_RULES*` |
| Index Git | Suppression des fichiers `QT_LOGGING_RULES=*` via `git update-index --force-remove` |

---

## Test client web (`tests/draft/index.js`)

Ajout du mark-as-read automatique dans le client de test JavaScript :
- **À l'ouverture d'un channel** : envoie `MARK_AS_READ` avec le dernier message
- **À la réception d'un message** dans le channel actif : envoie `MARK_AS_READ` immédiatement

---

## TODO — Prochaines features backend
- **Création de compte** : route `POST /register` (validation, nettoyage inputs, insertion DB)
- **Suppression de compte** : à définir
- **Client Qt** : le dev front doit gérer la réception du `type: 16` pour mettre à jour les badges