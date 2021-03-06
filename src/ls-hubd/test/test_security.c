/* @@@LICENSE
*
*      Copyright (c) 2008-2013 LG Electronics, Inc.
*
* Licensed under the Apache License, Version 2.0 (the "License");
* you may not use this file except in compliance with the License.
* You may obtain a copy of the License at
*
* http://www.apache.org/licenses/LICENSE-2.0
*
* Unless required by applicable law or agreed to in writing, software
* distributed under the License is distributed on an "AS IS" BASIS,
* WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
* See the License for the specific language governing permissions and
* limitations under the License.
*
* LICENSE@@@ */

#include <stdlib.h>
#include <string.h>
#include <glib.h>
#include <unistd.h>
#include "../security.h"
#include "../conf.h"
#include "../../libluna-service2/transport.h"
#include "../../libluna-service2/transport_client.h"
#include "../../libluna-service2/transport_security.h"


void _ConfigSetDefaults(void);
void _ConfigFreeSettings(void);
bool LSHubActiveRoleMapUnref(pid_t pid);

// Define mock functions.
bool
ServiceInitMap(LSError *lserror)
{
    return true;
}

bool
ParseServiceDirectory(const char *path, LSError *lserror)
{
    return true;
}

bool
LSHubSendConfScanCompleteSignal(void)
{
    return true;
}


static void
test_LSHubPermissionMapLookup(void *fixture, gconstpointer user_data)
{
    _ConfigSetDefaults();

    char const *dirs[] = { TEST_ROLES_DIRECTORY, NULL };
    LSError error;
    LSErrorInit(&error);
    g_assert(ProcessRoleDirectories(dirs, NULL, &error));
    g_assert(!LSErrorIsSet(&error));

    _LSTransportCred *cred = _LSTransportCredNew();
    g_assert(cred);
    pid_t pid = getpid();
    _LSTransportCredSetPid(cred, pid);

    struct LSTransportHandlers test_handlers = {};
    _LSTransport *transport = NULL;
    g_assert(_LSTransportInit(&transport, "com.webos.foo", &test_handlers, NULL));
    _LSTransportSetTransportType(transport, _LSTransportTypeLocal);
    g_assert(transport);

    _LSTransportClient test_client =
    {
        .service_name = "com.webos.foo",
        .cred = cred,
        .transport = transport,
    };

    _LSTransportCredSetExePath(cred, "/bin/foo");
    test_client.service_name = "com.webos.foo";
    g_assert(LSHubIsClientAllowedToRequestName(&test_client, "com.webos.foo"));
    LSHubActiveRoleMapUnref(pid);
    g_assert(!LSHubIsClientAllowedToRequestName(&test_client, "com.webos.foo2"));
    LSHubActiveRoleMapUnref(pid);

    _LSTransportCredSetExePath(cred, "/bin/bar");
    test_client.service_name = "com.webos.bar";
    g_assert(LSHubIsClientAllowedToRequestName(&test_client, "com.webos.bar"));
    LSHubActiveRoleMapUnref(pid);
    g_assert(LSHubIsClientAllowedToRequestName(&test_client, "com.webos.bar2"));
    LSHubActiveRoleMapUnref(pid);

    _LSTransportCredSetExePath(cred, "/bin/foo");
    test_client.service_name = "com.webos.foo";
    g_assert(LSHubIsClientAllowedToQueryName(&test_client, "com.webos.bar", "asdf"));
    _LSTransportCredSetExePath(cred, "/bin/bar");
    test_client.service_name = "com.webos.bar";
    g_assert(LSHubIsClientAllowedToQueryName(&test_client, "com.webos.foo", "asdf"));
    _LSTransportCredSetExePath(cred, "/bin/bar");
    test_client.service_name = "com.webos.bar2";
    g_assert(LSHubIsClientAllowedToQueryName(&test_client, "com.webos.foo", "asdf"));

    _LSTransportDeinit(transport);
    _LSTransportCredFree(cred);
    _ConfigFreeSettings();
}

int
main(int argc, char *argv[])
{
    g_test_init(&argc, &argv, NULL);

    g_log_set_always_fatal (G_LOG_LEVEL_ERROR);
    g_log_set_fatal_mask ("LunaServiceHub", G_LOG_LEVEL_ERROR);

    g_test_add("/hub/LSHubPermissionMapLookup", void, NULL, NULL, test_LSHubPermissionMapLookup, NULL);

    return g_test_run();
}
