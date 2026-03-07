
## Docker Commands

### Launch the project

```bash
# Build and run server + database
docker compose up --build

# In the background
docker compose up --build -d
```

### Logs

```bash
# All logs
docker compose logs -f

# Server logs only
docker compose logs -f server

# Database logs only
docker compose logs -f mysql-db
```

### Rebuild after server code changes

```bash
# Rebuild only the server (faster)
docker compose up --build server

# Full rebuild
docker compose down && docker compose up --build
```

### Stop

```bash
# Stop containers (data preserved)
docker compose down

# Stop and delete MySQL volume (reset database)
docker compose down -v
```


### Troubleshooting

```bash
# Check container status
docker compose ps -a

# Check which ports are in use (Windows)
netstat -ano | findstr 3306
netstat -ano | findstr 8888

# Recreate containers from scratch
docker compose down -v
docker compose up --build

# If port 3306 is already in use (local MySQL)
# → Change DB_PORT in .env or stop the local MySQL service
```