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

void initialize_backends() {
	initialize_database_backends();
	backend_hash_hashlib_install_providers();
}

trantor::EventLoop *main_event_loop;

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
	tb->text("full_data")->not_null()->next_row();
	tb->text("extracted_data")->not_null()->next_row();
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

void save_page(Database *db, const String &url, const String &full_data, const String &extracted_data) {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->begin_transaction();
	qb->insert("data", "url,full_data,extracted_data")->values()->val(url)->val(full_data)->val(extracted_data)->cvalues()->end_command();
	qb->update("settings")->set()->setp("last_url", url)->cset()->end_command();
	qb->commit();
	qb->run();
}

String *ss = nullptr;

void query_page(trantor::EventLoop *loop, const String &url, const String &path) {
	HttpClientPtr http_client = drogon::HttpClient::newHttpClient(url, loop, false, false);
	http_client->setUserAgent("Mozilla/5.0 (Windows NT 10.0; rv:78.0) Gecko/20100101 Firefox/78.0");

	HttpRequestPtr request = drogon::HttpRequest::newHttpRequest();
	request->setMethod(drogon::HttpMethod::Get);
	request->setPath(path);
	// request->setCustomContentTypeString("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\n");

	bool done = false;
	http_client->sendRequest(request, [&done](ReqResult res, const HttpResponsePtr &resptr) {
		if (res != ReqResult::Ok) {
			RLOG_ERR("sendRequest: res != ReqResult::Ok!");
			done = true;
			return;
		}

		RLOG_MSG("Got response. Saving.");

		ss = new String(resptr->getBody().data());

		done = true;
	});

	while (!done) {
		// todo remove
	}
}

void download_posts(Database *db, const String &site) {
	setup_database(db);

	String url = Settings::get_singleton()->get_value(site + ".url");
	String port = Settings::get_singleton()->get_value(site + ".port");
	String first_url = Settings::get_singleton()->get_value(site + ".first_url");
	String last_url = get_last_url(db);

	// String url = "127.0.0.1";
	// String port = "http";
	// String first_url = "/";
	// String last_url = get_last_url(db);

	String full_site_url = port + "://" + url;
	String full_site_url_repl = full_site_url;

	if (full_site_url_repl.ends_with("/")) {
		full_site_url_repl.pop_back();
	}

	int wait_seconds_min = Settings::get_singleton()->get_value_int(site + ".wait_seconds_min", 10);
	int wait_seconds_max = Settings::get_singleton()->get_value_int(site + ".wait_seconds_max", 20);

	bool should_skip = true;

	if (last_url == "") {
		should_skip = false;

		last_url = first_url;
	}

	RLOG_MSG("Post downloading started for " + site + " | last url: " + last_url);

	bool done = false;

	while (!done) {
		RLOG_MSG("Sending query to: " + full_site_url + last_url);

		query_page(main_event_loop, full_site_url, last_url);

		if (ss) {
			HTMLParser p;
			p.parse(*ss);

			HTMLParserTag *article_tag = p.root->get_first("article");

			String extracted_data = "";
			String next_link;

			if (article_tag) {
				extracted_data = article_tag->to_string();
			} else {
				RLOG_WARN("Couldn't extract data!");
			}

			HTMLParserTag *n_link_tag = p.root->get_first("a", "rel", "next");

			if (n_link_tag) {
				next_link = n_link_tag->get_attribute_value("href");

				if (next_link == "") {
					RLOG_WARN("Couldn't extract link!");
				}
			} else {
				next_link = "";
				RLOG_WARN("Couldn't extract link tag!");
			}

			if (should_skip) {
				should_skip = false;
				RLOG_MSG("Continuing from last session, this page is already saved, skipping.");
			} else {
				save_page(db, last_url, *ss, extracted_data);
			}

			if (next_link == "") {
				done = true;
			} else {
				int wait_seconds = Math::rand(wait_seconds_min, wait_seconds_max);

				RLOG_MSG("Waiting for " + String::num(wait_seconds) + " seconds!");
				std::this_thread::sleep_for(std::chrono::duration<double, std::milli>(wait_seconds * 1000));

				last_url = next_link;

				if (last_url.starts_with(full_site_url_repl)) {
					last_url.replace(full_site_url_repl, "", 1);
				}
			}
		} else {
			done = true;
		}

		delete ss;
		ss = nullptr;
	}
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
		std::thread t([]() { main_event_loop = new trantor::EventLoop(); main_event_loop->loop(); delete main_event_loop; main_event_loop = nullptr; });

		while (main_event_loop == nullptr) {
			// todo sleep
		}

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

		main_event_loop->quit();
		t.join();
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