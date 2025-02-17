#ifndef CONFIG_H
#define CONFIG_H

#include <stdbool.h>

bool localhost_config_load_from_file(const char* file, bool* out_is_ai_player1, bool* out_is_ai_player2);
bool localhost_config_load_from_text(const char* text, bool* out_is_ai_player1, bool* out_is_ai_player2);

bool online_config_load_from_file(const char* file, char** out_server_name, unsigned short* out_server_port);
bool online_config_load_from_text(const char* text, char** out_server_name, unsigned short* out_server_port);

#endif