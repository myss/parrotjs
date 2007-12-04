#ifndef __STR_ESCAPING_H__
#define __STR_ESCAPING_H__

char* unescape_js_string(const char* str, int len, int* error);
char* escape_pir_string(const char* raw_str);

#endif
