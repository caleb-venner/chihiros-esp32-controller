#pragma once

/* ── Wi-Fi Configuration ────────────────────────────────────────────────────
 *
 * Credentials live in secrets.h, which is not tracked by git.
 * Copy secrets.example.h to secrets.h and edit it before building.
 */
#include "secrets.h"

/* ── Timezone Configuration ─────────────────────────────────────────────────
 *
 * POSIX TZ string used once SNTP has set the clock.
 * Default is Sydney (AEST/AEDT).
 */
#define TIMEZONE "AEST-10AEDT,M10.1.0,M4.1.0/3"
