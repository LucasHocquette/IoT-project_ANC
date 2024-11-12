#include "mbed.h"
#include "cloud.h"
#include "wifi_helper.h"
#include <cstdlib>  // pour rand()
#include <ctime>    // pour time()

static constexpr size_t MAX_MESSAGE_RECEIVED_LENGTH = 100;
static constexpr size_t REMOTE_PORT = 80; // port HTTP standard

NetworkInterface *net;
SocketAddress address;
TCPSocket socket;

#define thingspeak_APIkey_write "JD7ZM7X1W6B6GFYO"  // Remplacez par votre clé d'écriture ThingSpeak
#define send_buffer_size 20000

char sbuffer[send_buffer_size];

bool resolve_hostname(const char *hostname) {
    printf("\nResolving %s\n", hostname);
    nsapi_size_or_error_t result = net->gethostbyname(hostname, &address);
    if (result != 0) {
        printf("Error! gethostbyname(%s) failed: %d\n", hostname, result);
        return false;
    }
    printf("Address of %s is %s\n", hostname, address.get_ip_address() ? address.get_ip_address() : "None");
    return true;
}

bool send_http_request(const char* buffer, int buffer_length) {
    nsapi_size_t bytes_to_send = buffer_length;
    nsapi_size_or_error_t bytes_sent = 0;

    while (bytes_to_send > 0) {
        bytes_sent = socket.send(buffer + (buffer_length - bytes_to_send), bytes_to_send);
        if (bytes_sent < 0) {
            printf("Erreur! socket.send() a retourné: %d\n", bytes_sent);
            return false;
        }
        bytes_to_send -= bytes_sent;
    }
    printf("Send successfully.\n");
    return true;
}

bool receive_http_response(void) {
    char buffer[MAX_MESSAGE_RECEIVED_LENGTH];
    int remaining_bytes = MAX_MESSAGE_RECEIVED_LENGTH;
    int received_bytes = 0;

    nsapi_size_or_error_t result = remaining_bytes;
    while (result > 0 && remaining_bytes > 0) {
        result = socket.recv(buffer + received_bytes, remaining_bytes);
        if (result < 0) {
            printf("Error! socket.recv() failed: %d\n", result);
            return false;
        }
        received_bytes += result;
        remaining_bytes -= result;
    }
    printf("Receive (%d bytes):\n%.*s\n\n", received_bytes, received_bytes, buffer);
    return true;
}

bool cloud_init(void) {
    net = NetworkInterface::get_default_instance();
    if (!net) {
        printf("Erreur! Aucune interface réseau trouvée.\n");
        return -1;
    }

    printf("Connection...\n");
    nsapi_size_or_error_t result = net->connect();
    if (result != 0) {
        printf("Error! net->connect() failed: %d\n", result);
        return false;
    }

    char host[] = "api.thingspeak.com";
    if (!resolve_hostname(host)) {
        printf("Resolve hostname failed!\r\n");
        return false;
    }

    return true;
}

char str_values[send_buffer_size] = "";
int cloud_send(uint16_t* sample_buffer, int buffer_size) {
    nsapi_size_or_error_t result;
    memset(str_values,0,send_buffer_size);

    // Ajouter les valeurs de A0 dans la chaîne
    for (int i = 0; i < buffer_size; i++) {
        char temp[6];
        snprintf(temp, 6, "%d", sample_buffer[i]);
        strcat(str_values, temp);
        if (i < buffer_size-1) {
            strcat(str_values, ","); // Séparer les valeurs par des virgules
        }
    }

    // Construire la requête HTTP
    snprintf(sbuffer, sizeof(sbuffer),
                "GET http://api.thingspeak.com/update.json?api_key=%s&field2=%s\r\n\r\n",
                thingspeak_APIkey_write,
                str_values);

    //printf("HTTP command : %s\n", sbuffer);

    result = socket.open(net);
    if (result != 0) {
        printf("Error! socket.open() failed: %d\n", result);
        return -1;
    }

    address.set_port(REMOTE_PORT);
    result = socket.connect(address);
    if (result != 0) {
        printf("Error! socket.connect() failed: %d\n", result);
        return -1;
    }

    if (!send_http_request(sbuffer, strlen(sbuffer))) {
        return -1;
    }
    if (!receive_http_response()) {
        return -1;
    }

    socket.close();
    return buffer_size;
}

void cloud_close(void) {
    net->disconnect();
}
