//----------------------------------------------------------
//
//   SA:MP Multiplayer Modification For GTA:SA
//   Copyright 2004-2006 SA:MP Team
//
//----------------------------------------------------------

#include "plugincommon.h"
#include "amx.h"

#define amx_StrParam(amx,param,result)                                      \
    do {                                                                    \
      cell *amx_cstr_; int amx_length_;                                     \
      amx_GetAddr((amx), (param), &amx_cstr_);                              \
      amx_StrLen(amx_cstr_, &amx_length_);                                  \
      if (amx_length_ > 0 &&                                                \
          ((result) = (char*)alloca((amx_length_ + 1) * sizeof(*(result)))) != NULL) \
        amx_GetString((char*)(result), amx_cstr_, sizeof(*(result))>1, amx_length_ + 1); \
      else (result) = NULL;                                                 \
    } while (0)

extern char g_paramBuffer[256];
inline const char* string_param(AMX* amx, cell param) {
	cell* amx_address;
	int amx_length;

	amx_GetAddr(amx, param, &amx_address);
	amx_StrLen(amx_address, &amx_length);
	if (amx_length > 0) {
		amx_GetString(g_paramBuffer, amx_address, 0, amx_length + 1);
	}

	return g_paramBuffer;
}

#define CHECK_PARAMS(n) { if (params[0] != n * sizeof(cell)) { logprintf("SCRIPT: Bad parameter count (%d != %d): ", params[0], n); return 0; } }

typedef void (*logprintf_t)(char *format, ...);
extern void *pAMXFunctions;
