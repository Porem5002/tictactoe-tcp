#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

bool online_config_load_from_file(const char* file, char** out_server_name, unsigned short* out_server_port);
bool online_config_load_from_text(const char* text, char** out_server_name, unsigned short* out_server_port);

#endif