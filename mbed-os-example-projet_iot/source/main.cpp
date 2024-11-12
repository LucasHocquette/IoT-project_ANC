#include "mbed.h"
#include "wifi_helper.h"
#include <cstdlib>  // pour rand()
#include <ctime>    // pour time()

static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;
static constexpr size_t REMOTE_PORT = 80; // port HTTP standard

NetworkInterface *net;
SocketAddress address;
TCPSocket socket;

#define thingspeak_APIkey_write "1QJWVUMIK0AD2K5W"  // Remplacez par votre clé d'écriture ThingSpeak

char sbuffer[512];
int valuesA0[7];  // Tableau pour stocker 7 valeurs de A0
int valuesA1[7];  // Tableau pour stocker 7 valeurs de A1

bool resolve_hostname(const char *hostname) {
    printf("\nRésolution de l'adresse de %s\n", hostname);
    nsapi_size_or_error_t result = net->gethostbyname(hostname, &address);
    if (result != 0) {
        printf("Erreur! gethostbyname(%s) a retourné: %d\n", hostname, result);
        return false;
    }
    printf("Adresse de %s est %s\n", hostname, address.get_ip_address() ? address.get_ip_address() : "None");
    return true;
}

bool send_http_request(const char* buffer, int buffer_length) {
    nsapi_size_t bytes_to_send = buffer_length;
    nsapi_size_or_error_t bytes_sent = 0;

    printf("\nEnvoi de la requête HTTP:\n%s", buffer);

    while (bytes_to_send > 0) {
        bytes_sent = socket.send(buffer + (buffer_length - bytes_to_send), bytes_to_send);
        if (bytes_sent < 0) {
            printf("Erreur! socket.send() a retourné: %d\n", bytes_sent);
            return false;
        }
        bytes_to_send -= bytes_sent;
    }
    printf("Message envoyé avec succès.\n");
    return true;
}

bool receive_http_response() {
    char buffer[MAX_MESSAGE_RECEIVED_LENGTH];
    int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
    int received_bytes = 0;

    nsapi_size_or_error_t result = remaining_bytes;
    while (result > 0 && remaining_bytes > 0) {
        result = socket.recv(buffer + received_bytes, remaining_bytes);
        if (result < 0) {
            printf("Erreur! socket.recv() a retourné: %d\n", result);
            return false;
        }
        received_bytes += result;
        remaining_bytes -= result;
    }
    printf("Réponse reçue (%d octets):\n%.*s\n\n", received_bytes, received_bytes, buffer);
    return true;
}

int main() {
    printf("\nDémarrage de la transmission de données multiples des broches A0 et A1 vers ThingSpeak\n\n");

    net = NetworkInterface::get_default_instance();
    if (!net) {
        printf("Erreur! Aucune interface réseau trouvée.\n");
        return -1;
    }

    printf("Connexion au réseau...\n");
    nsapi_size_or_error_t result = net->connect();
    if (result != 0) {
        printf("Erreur! net->connect() a retourné: %d\n", result);
        return -1;
    }

    char host[] = "api.thingspeak.com";
    if (!resolve_hostname(host)) {
        return -1;
    }

    srand(time(0));

    while (true) {
        // Remplissage des tableaux avec des valeurs aléatoires pour simuler les lectures des capteurs
        for (int i = 0; i < 7; i++) {
            valuesA0[i] = rand() % 100; // Simulation pour A0
            valuesA1[i] = rand() % 100; // Simulation pour A1
        }

        // Construction des chaînes avec les 7 valeurs de A0 et A1
        char A0_values[100] = "";
        char A1_values[100] = "";

        // Ajouter les valeurs de A0 dans la chaîne
        for (int i = 0; i < 7; i++) {
            char temp[10];
            sprintf(temp, "%d", valuesA0[i]);
            strcat(A0_values, temp);
            if (i < 6) {
                strcat(A0_values, ","); // Séparer les valeurs par des virgules
            }
        }

        // Ajouter les valeurs de A1 dans la chaîne
        for (int i = 0; i < 7; i++) {
            char temp[10];
            sprintf(temp, "%d", valuesA1[i]);
            strcat(A1_values, temp);
            if (i < 6) {
                strcat(A1_values, ","); // Séparer les valeurs par des virgules
            }
        }

        // Construire la requête HTTP avec les valeurs de A0 et A1
        snprintf(sbuffer, sizeof(sbuffer),
                 "GET http://api.thingspeak.com/update.json?api_key=%s&field1=%s&field2=%s\r\n\r\n",
                 thingspeak_APIkey_write,
                 A0_values, A1_values);

        printf("Commande HTTP : %s\n", sbuffer);

        result = socket.open(net);
        if (result != 0) {
            printf("Erreur! socket.open() a retourné: %d\n", result);
            return -1;
        }

        address.set_port(REMOTE_PORT);
        result = socket.connect(address);
        if (result != 0) {
            printf("Erreur! socket.connect() a retourné: %d\n", result);
            return -1;
        }

        if (!send_http_request(sbuffer, strlen(sbuffer))) {
            return -1;
        }
        if (!receive_http_response()) {
            return -1;
        }

        socket.close();

        ThisThread::sleep_for(15000ms);  // Attente de 15 secondes avant la prochaine lecture
    }

    net->disconnect();
    printf("Transmission terminée.\n");
    return 0;
}
