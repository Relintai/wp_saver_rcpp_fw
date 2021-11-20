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
#include "core/database/query_builder.h"
#include "core/database/query_result.h"
#include "core/database/table_builder.h"
#include "platform/platform_initializer.h"

#include "core/os/platform.h"

#include "modules/rbac/rbac_rank.h"

#include "core/html/html_parser.h"

#include "core/settings.h"

#include "modules/drogon/trantor/net/EventLoop.h"
#include "modules/drogon/drogon/lib/inc/drogon/HttpClient.h"
#include "modules/drogon/drogon/lib/inc/http/HttpRequest.h"
//#include "modules/drogon/trantor/net/Resolver.h"
//#include "modules/drogon/trantor/net/TcpClient.h"

void initialize_backends() {
	initialize_database_backends();
	backend_hash_hashlib_install_providers();
}

void setup_database(Database *db) {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->select("name")->from("sqlite_master")->where()->wp("type", "table")->land()->wp("name", "settings")->end_command();

	Ref<QueryResult> res = qb->run();

	if (res->next_row()) {
		return;
	}

	qb->reset();

	Ref<TableBuilder> tb = db->get_table_builder();

	tb->create_table("settings");
	tb->integer("id")->auto_increment()->next_row();
	tb->varchar("last_url", 1000)->not_null()->next_row();
	tb->integer("table_version")->not_null()->next_row();

	tb->primary_key("id");
	tb->ccreate_table();

	tb->create_table("data");
	tb->integer("id")->auto_increment()->next_row();
	tb->varchar("url", 1000)->not_null()->next_row();
	tb->text("data")->not_null()->next_row();
	tb->primary_key("id");
	tb->ccreate_table();

	tb->run_query();

	qb->insert("settings", "id,last_url,table_version")->values()->val(1)->val("")->val(1)->cvalues()->end_command();
	qb->run_query();
}

String get_last_url(Database *db) {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->select("last_url")->from("settings")->where()->wp("id", 1)->end_command();

	Ref<QueryResult> res = qb->run();

	if (!res->next_row()) {
		RLOG_ERR("get_last_url: !res->next_row()");
		return "";
	}

	return String(res->get_cell(0));
}

void save_page(Database *db, const String &url, const String &data) {
}

void download_posts(Database *db, const String &site) {
	setup_database(db);

	String url = Settings::get_singleton()->get_value(site + ".url");
	String port = Settings::get_singleton()->get_value(site + ".port");
	String first_url = Settings::get_singleton()->get_value(site + ".first_url");
	String last_url = get_last_url(db);

	if (last_url == "") {
		last_url = first_url;
	}

	RLOG_MSG("Post downloading started for " + site + " | last url: " + last_url);

	trantor::EventLoop *loop;
	std::thread t([&loop]() { loop = new trantor::EventLoop(); loop->loop(); delete loop; loop = nullptr; });

	while (loop == nullptr) {
		//todo sleep
	}

	HttpClientPtr http_client = drogon::HttpClient::newHttpClient("http://127.0.0.1:8080/", loop);

	HttpRequestPtr request = drogon::HttpRequest::newHttpRequest();
	request->setMethod(drogon::HttpMethod::Get);
	request->setPath(last_url);

	http_client->sendRequest(request, [](ReqResult res, const HttpResponsePtr &resptr){ RLOG_ERR("test\n");  });

	while (true) {
		//todo remove
	}

	loop->quit();

	t.join();
}


/*
trantor::InetAddress site_address;

void download_posts(Database *db, const String &site) {
	setup_database(db);

	String url = Settings::get_singleton()->get_value(site + ".url");
	String port = Settings::get_singleton()->get_value(site + ".port");
	String first_url = Settings::get_singleton()->get_value(site + ".first_url");
	String last_url = get_last_url(db);

	if (last_url == "") {
		last_url = first_url;
	}

	RLOG_MSG("Post downloading started for " + site + " | last url: " + last_url);

	trantor::EventLoop *loop;
	std::thread t([&loop]() { loop = new trantor::EventLoop(); loop->loop(); delete loop; loop = nullptr; });

	std::shared_ptr<trantor::Resolver> resolver = trantor::Resolver::newResolver(loop);

	bool resolved = false;
	resolver->resolve(url, [&resolved](const trantor::InetAddress& addr) { site_address = addr; resolved = true; });

	while (!resolved) {
		//todo sleep
	}

	if (port == "http") {
		site_address = trantor::InetAddress(site_address.toIp(), 80);
	} else if (port == "https") {
		site_address = trantor::InetAddress(site_address.toIp(), 443);
	} else {
		site_address = trantor::InetAddress(site_address.toIp(), port.to_uint());
	}

	RLOG_MSG("Resolved ip:");
	RLOG_MSG(site_address.toIpPort().c_str());

	loop->quit();

	t.join();
}
*/

int main(int argc, char **argv, char **envp) {
	PlatformInitializer::allocate_all();
	PlatformInitializer::arg_setup(argc, argv, envp);

	initialize_backends();
	String database_folder = "./data/";

	Settings *settings = new Settings(true);
	settings->parse_ini_file("settings.ini");

	bool download = Platform::get_singleton()->arg_parser.has_arg("-d");

	if (download) {
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

				download_posts(db, s);
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