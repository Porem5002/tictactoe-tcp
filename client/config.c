#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <ctype.h>

#include "config.h"

static const char* skip_spaces(const char* p)
{
    while(isspace(*p)) p++;

    return p;
}

static bool load_file(const char* path, char** out_content)
{
    assert(out_content != NULL);

    *out_content = NULL;

    FILE* f = fopen(path, "rb");
    if(f == NULL) return false;

    fseek(f, 0, SEEK_END);
    size_t size = (size_t)ftell(f);
    fseek(f, 0, SEEK_SET);

    char* content = malloc(sizeof(char) * (size + 1));
    fread(content, 1, size, f);
    content[size] = '\0';
    fclose(f);

    *out_content = content;
    return true;
}

static bool cstr_equals_strview(const char* cstr, const char* strview_data, size_t strview_size)
{
    return strlen(cstr) == strview_size && (strncmp(cstr, strview_data, strview_size) == 0);
}

bool localhost_config_load_from_file(const char* path, bool* out_is_ai_player1, bool* out_is_ai_player2)
{
    assert(out_is_ai_player1 != NULL);
    assert(out_is_ai_player2 != NULL);

    *out_is_ai_player1 = false;
    *out_is_ai_player2 = false;

    char* content; 
    if(!load_file(path, &content))
        return false;

    bool success = localhost_config_load_from_text(content, out_is_ai_player1, out_is_ai_player2);
    free(content);
    return success;
}

bool localhost_config_load_from_text(const char* text, bool* out_is_ai_player1, bool* out_is_ai_player2)
{
    assert(out_is_ai_player1 != NULL);
    assert(out_is_ai_player2 != NULL);

    *out_is_ai_player1 = false;
    *out_is_ai_player2 = false;

    const char* p = text;
    p = skip_spaces(p);

    if(*p == '\0')
    {
        // Expected a property name
        goto failed;
    }

    while(*p != '\0')
    {
        p = skip_spaces(p);

        const char* property = p;
        int property_length = 0;

        while(!isspace(*p) && *p != '\0')
        {
            property_length++;
            p++;
        }

        if(cstr_equals_strview("PLAYER_X", property, property_length))
        {
            p = skip_spaces(p);
            
            if(*p == '\0')
            {
                // Expected a value after PLAYER_X property
                goto failed;
            }
            
            const char* value = p;

            while(!isspace(*p) && *p != '\0') p++;

            size_t length = (size_t)((uintptr_t)p - (uintptr_t)value);

            if(cstr_equals_strview("ai", value, length))
                *out_is_ai_player1 = true;
            else if(cstr_equals_strview("human", value, length))
                *out_is_ai_player1 = false;
            else
            {
                // Invalid value for PLAYER_X property
                goto failed;
            }
        }
        else if(cstr_equals_strview("PLAYER_O", property, property_length))
        {
            p = skip_spaces(p);
            
            if(*p == '\0')
            {
                // Expected a value after PLAYER_O property
                goto failed;
            }
            
            const char* value = p;

            while(!isspace(*p) && *p != '\0') p++;

            size_t length = (size_t)((uintptr_t)p - (uintptr_t)value);

            if(cstr_equals_strview("ai", value, length))
                *out_is_ai_player2 = true;
            else if(cstr_equals_strview("human", value, length))
                *out_is_ai_player2 = false;
            else
            {
                // Invalid value for PLAYER_O property
                goto failed;
            }
        }
        else
        {
            // Unknown property
            goto failed;
        }

        p = skip_spaces(p);
    }

    return true;
failed:
    *out_is_ai_player1 = false;
    *out_is_ai_player2 = false;
    return false;
}

bool online_config_load_from_file(const char* path, char** out_server_name, unsigned short* out_server_port)
{
    assert(out_server_name != NULL);
    assert(out_server_port != NULL);

    *out_server_name = NULL;
    *out_server_port = 0;

    char* content;
    if(!load_file(path, &content))
        return false;

    bool success = online_config_load_from_text(content, out_server_name, out_server_port);
    free(content);
    return success;
}

bool online_config_load_from_text(const char* text, char** out_server_name, unsigned short* out_server_port)
{
    assert(out_server_name != NULL);
    assert(out_server_port != NULL);

    *out_server_name = NULL;
    *out_server_port = 0;

    const char* p = text;
    p = skip_spaces(p);

    if(*p == '\0')
    {
        // Expected a property name
        goto failed;
    }

    while(*p != '\0')
    {
        p = skip_spaces(p);

        const char* property = p;
        int property_length = 0;

        while(!isspace(*p) && *p != '\0')
        {
            property_length++;
            p++;
        }

        if(cstr_equals_strview("IP", property, property_length))
        {
            p = skip_spaces(p);
            
            if(*p == '\0')
            {
                // Expected a name after IP property
                goto failed;
            }
            
            const char* start = p;

            while(!isspace(*p) && *p != '\0') p++;

            size_t length = (size_t)((uintptr_t)p - (uintptr_t)start);
            
            char* value = malloc(sizeof(char) * (length + 1));
            memcpy(value, start, length);
            value[length] = '\0';
            
            free(*out_server_name);
            *out_server_name = value;
        }
        else if(cstr_equals_strview("PORT", property, property_length))
        {
            p = skip_spaces(p);

            if(*p == '\0')
            {
                // Expected a number after PORT property
                goto failed;
            }

            char* end;
            long port = strtol(p, &end, 10);

            p = end;

            if(!isspace(*end) && *end != '\0')
            {
                // Expected white space or end of stream after number
                goto failed;
            }

            if(port < 0)
            {
                // Port number cannot be negative
                goto failed;
            }

            if(port > (long)USHRT_MAX)
            {
                // Port number exceeds maximum value allowed
                goto failed;
            }

            *out_server_port = port;
        }
        else
        {
            // Unknown property
            goto failed;
        }

        p = skip_spaces(p);
    }

    return true;
failed:
    free(*out_server_name);
    *out_server_name = NULL;
    return false;
}