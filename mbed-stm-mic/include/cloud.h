#ifndef CLOUD_HPP
#define CLOUD_HPP

#include "mbed.h"

bool resolve_hostname(const char *hostname);
bool send_http_request(const char* buffer, int buffer_length);
bool receive_http_response(void);
bool cloud_init(void);
int cloud_send(uint16_t* sample_buffer, int buffer_size);
void cloud_close(void);

#endif