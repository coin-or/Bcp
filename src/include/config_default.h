/* include the COIN-OR-wide system specific configure header */
#include "configall_system.h"

/* this needs to come before the include of config_bcp_default.h */
#ifndef BCPLIB_EXPORT
#if defined(_WIN32) && defined(DLL_EXPORT)
#define BCPLIB_EXPORT __declspec(dllexport)
#else
#define BCPLIB_EXPORT
#endif
#endif

/* include the public project specific macros */
#include "config_bcp_default.h"

/***************************************************************************/
/*             HERE DEFINE THE PROJECT SPECIFIC MACROS                     */
/*    These are only in effect in a setting that doesn't use configure     */
/***************************************************************************/

/* If defined, debug sanity checks are performed during runtime */
/* #define COIN_DEBUG 1 */

/* a few things that exists under unix, but not under windows */
#include <Windows.h>
#define gethostname(c, l) { DWORD cnlen = l; GetComputerName(c, &cnlen); }
#include <process.h>

