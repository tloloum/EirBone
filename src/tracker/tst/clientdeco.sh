#!/bin/bash

# Fonction pour simuler un client qui se connecte, s'annonce, puis se déconnecte
function simulate_client {
    local port="$1"
    local announce_message="$2"

    # Connectez-vous au serveur en utilisant netcat, en envoyant un message d'annonce
    {
        sleep 1
        echo -e "$announce_message"
        sleep 2  # Gardez la connexion ouverte pendant 2 secondes avant de fermer
    } | nc localhost "$port"
}

# Assurez-vous d'exécuter le tracker en premier et de vérifier le bon port
TRACKER_PORT=8000

# Message d'annonce simulé pour chaque client
ANNOUNCE_MESSAGE_1="announce listen 1000 seed [file1.txt 100 10 abcdef123456]"
ANNOUNCE_MESSAGE_2="announce listen 2000 seed [file2.txt 200 20 bcdefa654321]"
ANNOUNCE_MESSAGE_3="announce listen 3000 seed [file3.txt 300 30 cdefab123456]"

# Lancement des clients en parallèle
simulate_client "$TRACKER_PORT" "$ANNOUNCE_MESSAGE_1" &
CLIENT1_PID=$!

simulate_client "$TRACKER_PORT" "$ANNOUNCE_MESSAGE_2" &
CLIENT2_PID=$!

simulate_client "$TRACKER_PORT" "$ANNOUNCE_MESSAGE_3" &
CLIENT3_PID=$!

# Attendre quelques secondes, puis arrêter les clients 1 et 2
sleep 4
kill "$CLIENT1_PID"
kill "$CLIENT2_PID"

# Attendre la fin du client 3
wait "$CLIENT3_PID"

echo "Test complet. Clients 1 et 2 ont été déconnectés."

