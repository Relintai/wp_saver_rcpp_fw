#include <string.h>

#include "core/file_cache.h"
#include "core/math/math.h"
#include "core/os/platform.h"
#include "core/settings.h"

#include "database/db_init.h"

// Backends
#include "backends/hash_hashlib/setup.h"

#include "core/database/database_manager.h"
#include "platform/platform_initializer.h"

#include "core/settings.h"

#include "app/wp_application.h"
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

	DatabaseManager *dbm = new DatabaseManager();

	bool download = Platform::get_singleton()->arg_parser.has_arg("-d");

	if (download) {
		Vector<Ref<WPDownloader> > downloaders;

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

				// todo
				// downloaders.push_back(d);

				d->run();
			}
		}

	} else {
		FileCache *file_cache = new FileCache(true);
		file_cache->wwwroot = "./www";
		file_cache->wwwroot_refresh_cache();

		WPApplication *app = new WPApplication();
		app->load_settings();
		app->setup_routes();
		app->setup_middleware();
		app->add_listener("127.0.0.1", 8080);

		String sites = settings->get_value("sites");

		int sc = sites.get_slice_count(',');
		for (int i = 0; i < sc; ++i) {
			String s = sites.get_slice(',', i);

			uint32_t index = dbm->create_database("sqlite");
			Database *db = dbm->databases[index];
			db->connect(database_folder + s + ".sqlite");

			app->add_blog(s, db);
		}

		LOG_INFO << "Server running on 127.0.0.1:8080";
		printf("Initialized!\n");
		app->run();
		delete app;

		delete file_cache;
	}

	delete dbm;
	delete settings;

	PlatformInitializer::free_all();

	return 0;
}