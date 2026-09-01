//
// expat_config.h (Windows) for Hardklor's in-tree compile of pwiz's bundled expat.
//
// Copyright 2026 ProteoWizard contributors
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
// http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.
//
// This is an original pwiz-side configuration header (feature macros + winconfig.h);
// it is not copied from the Expat project. Expat's own sources retain their MIT license.
//

/* Windows expat_config.h for Hardklor's in-tree compile of pwiz's bundled expat.
 *
 * pwiz upgraded libraries/expat 2.0.1 -> 2.8.2 (CVE remediation). expat >= 2.6
 * *unconditionally* does `#include "expat_config.h"` from xmlparse.c (expat 2.0.1
 * selected winconfig.h on Windows instead, so no config header was needed). Hardklor
 * compiles pwiz's expat sources but also carries MSToolKit's include dir on its search
 * path, which ships a stale expat 2.2.9 UNIX expat_config.h: it lacks XML_GE (required
 * by expat >= 2.6) and defines HAVE_GETRANDOM / HAVE_SYSCALL_GETRANDOM, which would
 * wrongly drag random_getrandom.h into a Windows build. This header is placed earlier
 * on Hardklor's include path so it shadows that stale config and gives the expat 2.8.2
 * sources a correct Windows configuration (matching pwiz's ext-expat build).
 */
#ifndef EXPAT_CONFIG_H
#define EXPAT_CONFIG_H 1

#define XML_GE 1
#define XML_CONTEXT_BYTES 1024
#define XML_DTD 1
#define XML_NS 1

/* Windows entropy source for expat's hash-salt (rand_s); winconfig.h also arranges
 * _CRT_RAND_S to be defined before <stdlib.h>. Resolves to pwiz's expat-2.8.2 winconfig.h. */
#if defined(_WIN32)
#  include "winconfig.h"
#endif

#endif /* EXPAT_CONFIG_H */
