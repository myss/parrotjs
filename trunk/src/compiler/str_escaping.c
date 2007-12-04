#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#include "str_escaping.h"

static int hexchar2int(const char hex_char, int* err) {
    *err = 0;
    if (hex_char >= '0' && hex_char <= '9') {
        return hex_char - '0';
    } else if (hex_char >= 'A' && hex_char <= 'F') {
        return 10 + (hex_char - 'A');
    } else if (hex_char >= 'a' && hex_char <= 'f') {
        return 10 + (hex_char - 'a');
    } else {
        *err = 1;
        return 0;
    }
}

static int hex2int(const char* hex_str, int len, int* err) {
    int sum = 0;
    int i = len-1;
    int k = 1;
    
    *err = 0;

    while (i >= 0) {
        sum += hexchar2int(hex_str[i], err) * k;
        if (*err)
            return 0;
        i--;
        k *= 16;
    }
    return sum;
}

/*static char escape_x(const char* str, int* err) {
    return (char) hex2int(str, 2, err);
}*/

static void escape_x(const char* str, char* result, int* nResultChars, int* err) {
    int n = hex2int(str, 2, err);
    if (*err) 
        return;
    
    if (n < 128) {
        result[0] = (char) n;
        *nResultChars = 1;
    } else {
        /* TODO: encode UNICODE */
        result[0] = '?';
        *nResultChars = 1;
    }
}

static void escape_u(const char* str, char* result, int* nResultChars, int* err) {
    int n = hex2int(str, 4, err);
    if (*err) 
        return;
    
    if (n < 128) {
        result[0] = (char) n;
        *nResultChars = 1;
    } else {
        /* TODO: encode UNICODE */
        result[0] = '?';
        *nResultChars = 1;
    }
}

static void escape( const char* str,    /* contains something to escape at the start */
                    int len,            /* characters in str until '\0' */
                    int* parsed_len,    /* set it to the number of used chars in str after parsing */
                    char* result,       /* store here the result chars */
                    int* result_len,    /* how many result chars are there */
                    int* error       ) {
    if (len <= 0) {
        *error = 1;
        return;
    }
    *error = 0;
    switch(str[0]) {
        case 'n':
            *parsed_len = 1;
            *result_len = 1;
            result[0] = '\n';
            return;
            
        case 'r':
            *parsed_len = 1;
            *result_len = 1;
            result[0] = '\r';
            return;
            
        case 't':
            *parsed_len = 1;
            *result_len = 1;
            result[0] = '\t';
            return;
            
        case 'v':
            *parsed_len = 1;
            *result_len = 1;
            result[0] = 0xb;
            return;
            
        case 'b':
            *parsed_len = 1;
            *result_len = 1;
            result[0] = 0x8;
            return;
            
        case 'f':
            *parsed_len = 1;
            *result_len = 1;
            result[0] = 0xC;
            return;
            
        case '0':
            *parsed_len = 1;
            *result_len = 1;
            result[0] = '\0';
            return;
            
        case 'x':
            if (len < 2) {
                *error = 1;
                return;
            }
            *parsed_len = 2 + 1;
            
            escape_x(str+1, result, result_len, error);
            return;
            
        case 'u':
            if (len < 4) {
                *error = 1;
                return;
            }
            *parsed_len = 4 + 1;
            
            escape_u(str+1, result, result_len, error);
            return;
            
        default:
            *parsed_len = 1;
            *result_len = 1;
            result[0] = str[0];
            return;
    }
}

char* unescape_js_string(const char* str, int len, int* error) {
    char* unescaped;
    int i, j;
    
    *error = 0;
    
    if (! (len > 1 && str[0] == str[len-1] && 
                       (str[0] == '"' || str[0] == '\''))) {
        *error = 1;
        return unescaped;
    };
    
    unescaped = (char*) malloc((len - 2 + 1) * sizeof(char));
    
    i = 1;
    j = 0;
    while(i < len-1) {
        if (str[i] == '\\') {
            int err = 0;
            int parsed_len = 0;
            char result[4];
            int result_len;
            
            i++;
            escape(str+i,
                   len-i,
                   &parsed_len,
                   result,
                   &result_len,
                   &err);
            
            if (err) {
                *error = 1;
                return NULL;
            } else {
                int k;
                for(k = 0; k<result_len; k++) {
                    unescaped[j++] = result[k];
                }
                i += parsed_len;
            }
        } else {
            unescaped[j++] = str[i++];
        }
    }
    assert(j <= len - 2);
    unescaped[j] = '\0';
    return unescaped;
}

char* escape_pir_string(const char* raw_str) {
    char* pir_escaped;
    const char* c;
    int pir_escaped_len;
    int i;
    
    /* find the length of escaped string  */
    pir_escaped_len = 0;
    for (c = raw_str; *c; c++) {
        char ch = *c;
        if (ch == '"' || ch == '\r' || ch == '\n' || ch == '\\') {
            pir_escaped_len++;
        }
        pir_escaped_len++;
    }
    pir_escaped = (char*) malloc((pir_escaped_len+1) * sizeof(char));
    
    i = 0;
    for (c = raw_str; *c; c++) {
        char ch = *c;
        if (ch == '"') {
            pir_escaped[i] = '\\';
            pir_escaped[i+1] = '"';
            i += 2;
        } else if (ch == '\r') {
            pir_escaped[i] = '\\';
            pir_escaped[i+1] = 'r';
            i += 2;
        } else if (ch == '\n') {
            pir_escaped[i] = '\\';
            pir_escaped[i+1] = 'n';
            i += 2;
        } else if (ch == '\\') {
            pir_escaped[i] = '\\';
            pir_escaped[i+1] = '\\';
            i += 2;
        } else {
            pir_escaped[i] = ch;
            i += 1;
        }
    }
    assert(i == pir_escaped_len);
    pir_escaped[i] = '\0';
    
    return pir_escaped;
}

/*int main(int argc, const char* argv[]) {
    int err;
    char* unescaped_js;
    
    if (argc < 2) 
        return 1;
        
    unescaped_js = unescape_js_string(argv[1], strlen(argv[1]), &err);
    if (err) {
        printf("error!\n");
    } else {
        char* escaped_pir = escape_pir_string(unescaped_js);
        printf("js2pir(%s) = utf8:unicode:\"%s\"\n", argv[1], escaped_pir);
        free(escaped_pir);
    }
    free(unescaped_js);
    return 0;
}*/
