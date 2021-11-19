#include <string.h>
#include <iostream>
#include <string>

#include "core/file_cache.h"
#include "core/http/web_application.h"

#include "app/ccms_application.h"

#include "database/db_init.h"

#include "core/settings.h"

#include "modules/drogon/web_application.h"

//Backends
#include "backends/hash_hashlib/setup.h"

#include "core/database/database_manager.h"
#include "platform/platform_initializer.h"

#include "core/os/platform.h"

#include "modules/rbac/rbac_rank.h"

#include "core/html/html_parser.h"

#include "core/settings.h"

#include "modules/drogon/trantor/net/EventLoop.h"
#include "modules/drogon/trantor/net/TcpClient.h"

void initialize_backends() {
	initialize_database_backends();
	backend_hash_hashlib_install_providers();
}

int main(int argc, char **argv, char **envp) {
	PlatformInitializer::allocate_all();
	PlatformInitializer::arg_setup(argc, argv, envp);

	initialize_backends();

	Settings *settings = new Settings(true);
	settings->parse_ini_file("settings.ini");

	bool download = Platform::get_singleton()->arg_parser.has_arg("-d");

	if (download) {
		DatabaseManager *dbm = new DatabaseManager();

		uint32_t index = dbm->create_database("sqlite");
		Database *db = dbm->databases[index];
		db->connect("database.sqlite");

		delete dbm;
	} else {
		/*
		FileCache *file_cache = new FileCache(true);
		file_cache->wwwroot = "./www";
		file_cache->wwwroot_refresh_cache();

		DWebApplication *app = new DWebApplication();
		app->load_settings();
		app->setup_routes();
		app->setup_middleware();
		app->add_listener("127.0.0.1", 8080);
		LOG_INFO << "Server running on 127.0.0.1:8080";
		printf("Initialized!\n");
		app->run();
		delete app;

		delete file_cache;
		*/
	}

	delete settings;

	PlatformInitializer::free_all();

	return 0;
}