#ifndef CAT_MACRO_HPP
#define CAT_MACRO_HPP

#include <cstdio>

#define VERSION 0x01000002

// maybe replace these values with your files

#define HTTPS_CERT "/opt/crt/iarba.xyz/cert.pem"
#define HTTPS_KEY "/opt/crt/iarba.xyz/key.pem"

#define PROD_HTTP_HOST "https://d.iarba.xyz"
#define DEV_HTTP_HOST "https://d.iarba.xyz:8080"
#define PROD_DISCORD_BOT_PERSISTENT_MEMFILE "/opt/catbot/memfile"
#define DEV_DISCORD_BOT_PERSISTENT_MEMFILE "./memfile"
#define PROD_DISCORD_BOT_API_KEY_FILE "/opt/crt/meoliver/apikey"
#define DEV_DISCORD_BOT_API_KEY_FILE "/opt/crt/iarbot/apikey"

#define HTTP_HOST (pmode ? (PROD_HTTP_HOST) : (DEV_HTTP_HOST))
#define DISCORD_BOT_PERSISTENT_MEMFILE (pmode ? (PROD_DISCORD_BOT_PERSISTENT_MEMFILE) : (DEV_DISCORD_BOT_PERSISTENT_MEMFILE))
#define DISCORD_BOT_API_KEY_FILE (pmode ? (PROD_DISCORD_BOT_API_KEY_FILE) : (DEV_DISCORD_BOT_API_KEY_FILE))

extern bool pmode;

#define GENERIC(...) {printf(MODULE " " __VA_ARGS__); fflush(stdout);}
#define ERROR(...) GENERIC("ERROR: " __VA_ARGS__)
#define WARN(...) GENERIC("WARNING: " __VA_ARGS__)
#define INFO(...) GENERIC("INFO: " __VA_ARGS__)
#define TRACE(...) {if(!pmode)GENERIC("TRC: " __VA_ARGS__);}

#endif // CAT_MACRO_HPP
