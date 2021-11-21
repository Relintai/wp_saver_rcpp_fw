#include <string.h>
#include <chrono>
#include <iostream>
#include <string>

#include "core/file_cache.h"
#include "core/http/web_application.h"
#include "core/math/math.h"

#include "app/ccms_application.h"

#include "database/db_init.h"

#include "core/settings.h"

#include "modules/drogon/web_application.h"

// Backends
#include "backends/hash_hashlib/setup.h"

#include "core/database/database_manager.h"
#include "core/database/query_builder.h"
#include "core/database/query_result.h"
#include "core/database/table_builder.h"
#include "platform/platform_initializer.h"

#include "core/os/platform.h"

#include "modules/rbac/rbac_rank.h"

#include "core/html/html_parser.h"

#include "core/settings.h"

#include "modules/drogon/drogon/lib/inc/drogon/HttpClient.h"
#include "modules/drogon/drogon/lib/inc/http/HttpRequest.h"
#include "modules/drogon/trantor/net/EventLoop.h"
//#include "modules/drogon/trantor/net/Resolver.h"
//#include "modules/drogon/trantor/net/TcpClient.h"

#include <string>

#include "app/wp_downloader.h"

void initialize_backends() {
	initialize_database_backends();
	backend_hash_hashlib_install_providers();
}

int main(int argc, char **argv, char **envp) {
	PlatformInitializer::allocate_all();
	PlatformInitializer::arg_setup(argc, argv, envp);

	initialize_backends();
	String database_folder = "./data/";

	Settings *settings = new Settings(true);
	settings->parse_ini_file("settings.ini");

	bool download = Platform::get_singleton()->arg_parser.has_arg("-d");

	if (download) {
		Vector<Ref<WPDownloader> > downloaders;
		DatabaseManager *dbm = new DatabaseManager();

		bool save_original_data = settings->get_value_bool("save_original_data");

		if (save_original_data) {
			String sites = settings->get_value("sites");

			int sc = sites.get_slice_count(',');
			for (int i = 0; i < sc; ++i) {
				String s = sites.get_slice(',', i);

				uint32_t index = dbm->create_database("sqlite");
				Database *db = dbm->databases[index];
				db->connect(database_folder + s + ".sqlite");

				Ref<WPDownloader> d;
				d.instance();

				d->setup(s, db);

				//todo
				//downloaders.push_back(d);

				d->run();
			}
		}

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