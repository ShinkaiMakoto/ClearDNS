#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include "loader.h"
#include "logger.h"
#include "sundry.h"
#include "system.h"
#include "assets.h"
#include "adguard.h"
#include "crontab.h"
#include "constant.h"
#include "dnsproxy.h"
#include "overture.h"
#include "structure.h"

struct {
    char *config;
    uint8_t debug;
    uint8_t verbose;
} settings;

void init(int argc, char *argv[]) { // return config file
    settings.config = strdup(CONFIG_FILE);
    settings.debug = FALSE;
    settings.verbose = FALSE;

    if (getenv("CONFIG") != NULL) {
        free(settings.config);
        settings.config = strdup(getenv("CONFIG"));
    }
    if (getenv("DEBUG") != NULL && !strcmp(getenv("DEBUG"), "TRUE")) {
        settings.debug = TRUE;
    }
    if (getenv("VERBOSE") != NULL && !strcmp(getenv("VERBOSE"), "TRUE")) {
        settings.verbose = TRUE;
    }

    for (int i = 0; i < argc; ++i) {
        if (!strcmp(argv[i], "--debug")) {
            settings.debug = TRUE;
        }
        if (!strcmp(argv[i], "--verbose")) {
            settings.verbose = TRUE;
        }
        if (!strcmp(argv[i], "--version")) {
            printf("ClearDNS version %s\n", VERSION); // show version
            exit(0);
        }
        if (!strcmp(argv[i], "--help")) {
            printf("\n%s\n", HELP_MSG); // show help message
            exit(0);
        }
        if (!strcmp(argv[i], "--config")) {
            if (i + 1 == argc) {
                log_error("Option `--config` missing value");
                exit(1);
            }
            free(settings.config);
            settings.config = strdup(argv[++i]); // use custom config file
        }
    }
    log_debug("Config file -> %s", settings.config);
}

void cleardns() { // cleardns service
    if (settings.verbose || settings.debug) {
        LOG_LEVEL = LOG_DEBUG; // enable debug log level
    }
    create_folder(EXPOSE_DIR);
    create_folder(WORK_DIR);
    chdir(EXPOSE_DIR);
    load_config(settings.config); // configure parser
    free(settings.config);
    if (settings.debug) { // debug mode enabled
        loader.diverter->debug = TRUE;
        loader.domestic->debug = TRUE;
        loader.foreign->debug = TRUE;
        if (loader.crond != NULL) {
            loader.crond->debug = TRUE;
        }
        if (loader.filter != NULL) {
            loader.filter->debug = TRUE;
        }
    }

    log_info("Start loading process");
    process_list_init();
//    assets_load(loader.resource);
    process_list_append(dnsproxy_load("Domestic", loader.domestic, "domestic.json"));
    process_list_append(dnsproxy_load("Foreign", loader.foreign, "foreign.json"));
    process_list_append(overture_load(loader.diverter, "overture.json"));
    overture_free(loader.diverter);
    dnsproxy_free(loader.domestic);
    dnsproxy_free(loader.foreign);
//    assets_free(loader.resource);
    if (loader.crond != NULL) {
        process_list_append(crontab_load(loader.crond)); // free crontab struct later
    }
    if (loader.filter != NULL) {
        process_list_append(adguard_load(loader.filter, ADGUARD_DIR));
        adguard_free(loader.filter);
    }

    for (char **script = loader.script; *script != NULL; ++script) { // running custom script
        log_info("Run custom script -> `%s`", *script);
        run_command(*script);
    }
    string_list_free(loader.script);

    process_list_run(); // start all process
    if (loader.crond != NULL) { // assets not disabled
        kill(getpid(), SIGALRM); // send alarm signal to cleardns
        crontab_free(loader.crond);
    }
    process_list_daemon(); // daemon all process
}

int main(int argc, char *argv[]) {

    LOG_LEVEL = LOG_DEBUG;

    asset *res_1 = asset_init("demo 1");
    asset *res_2 = asset_init("demo 2");
    asset *res_3 = asset_init("demo 3");

    string_list_append(&res_1->sources, "item a1");
    string_list_append(&res_1->sources, "item a2");
    string_list_append(&res_1->sources, "item a3");

//    asset_add_src(res_1, "item a1");
//    asset_add_src(res_1, "item a2");
//    asset_add_src(res_1, "item a3");

    string_list_append(&res_2->sources, "item b1");
    string_list_append(&res_2->sources, "item b2");

//    asset_add_src(res_2, "item b1");
//    asset_add_src(res_2, "item b2");

    string_list_append(&res_3->sources, "item c1");

//    asset_add_src(res_3, "item c1");

    asset **res = assets_init();
    log_info("res init size: %d", assets_size(res));

    assets_append(&res, res_1);
    assets_append(&res, res_2);
    assets_append(&res, res_3);

    log_info("res size: %d", assets_size(res));

    assets_dump(res);

//    asset_free(res_1);
//    asset_free(res_2);
//    asset_free(res_3);

    assets_free(res);

//    char **res_1 = string_list_init();
//    char **res_2 = string_list_init();
//    char **res_3 = string_list_init();
//
//    string_list_append(&res_1, "item a1");
//    string_list_append(&res_1, "item a2");
//    string_list_append(&res_1, "item a3");
//
//    string_list_append(&res_2, "item b1");
//    string_list_append(&res_2, "item b2");
//
//    string_list_append(&res_3, "item c");

//    char *tmp = string_list_dump(demo);
//    log_warn("dump -> %s", tmp);

//    assets_log_init(TRUE);
    assets_log_init(FALSE);

//    rust_assets_update("test.txt", demo, ASSETS_DIR);

//    string_list_free(demo);

    log_warn("test end");
    exit(0);

    init(argc, argv);
    log_info("ClearDNS server start (%s)", VERSION);
    cleardns();
    return 0;
}
