#!/bin/bash
pkill -f "./bin/server"
pkill -f bot_client

echo "Resetting Database..."
rm -f db/game.db
sqlite3 db/game.db < import_data.sql

echo "Starting Server..."
./bin/server 5500 > logs/server.log 2>&1 &
SERVER_PID=$!
sleep 2

echo "Starting Host Bot..."
./bin/bot_client HostBot HOST > logs/bot_host.log 2>&1 &
sleep 2

echo "Starting Player Bots..."
./bin/bot_client Player1 > logs/bot1.log 2>&1 &
./bin/bot_client Player2 > logs/bot2.log 2>&1 &
./bin/bot_client Player3 > logs/bot3.log 2>&1 &
./bin/bot_client Player4 > logs/bot4.log 2>&1 &
./bin/bot_client Player5 > logs/bot5.log 2>&1 &

echo "Simulation running... Monitoring Host Log..."
tail -f logs/bot_host.log | grep -E "Game Over|ELIMINATED|QUESTION"
