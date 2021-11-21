#include "wp_downloader.h"

#include <string.h>
#include <chrono>
#include <iostream>
#include <string>

#include "core/math/math.h"

#include "core/database/database_manager.h"
#include "core/database/query_builder.h"
#include "core/database/query_result.h"
#include "core/database/table_builder.h"

#include "core/html/html_parser.h"

#include "core/settings.h"

#include "modules/drogon/drogon/lib/inc/drogon/HttpClient.h"
#include "modules/drogon/drogon/lib/inc/http/HttpRequest.h"

void WPDownloader::setup_database() {
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

String WPDownloader::get_last_url() {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->select("last_url")->from("settings")->where()->wp("id", 1)->end_command();

	Ref<QueryResult> res = qb->run();

	if (!res->next_row()) {
		RLOG_ERR("get_last_url: !res->next_row()");
		return "";
	}

	return String(res->get_cell(0));
}

void WPDownloader::save_page(const String &url, const String &full_data, const String &extracted_data) {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->begin_transaction();
	qb->insert("data", "url,full_data,extracted_data")->values()->val(url)->val(full_data)->val(extracted_data)->cvalues()->end_command();
	qb->update("settings")->set()->setp("last_url", url)->cset()->end_command();
	qb->commit();
	qb->run();
}

void WPDownloader::query_page(trantor::EventLoop *loop, const String &url, const String &path) {
	drogon::HttpClientPtr http_client = drogon::HttpClient::newHttpClient(url, loop, false, false);
	http_client->setUserAgent("Mozilla/5.0 (Windows NT 10.0; rv:78.0) Gecko/20100101 Firefox/78.0");

	drogon::HttpRequestPtr request = drogon::HttpRequest::newHttpRequest();
	request->setMethod(drogon::HttpMethod::Get);
	request->setPath(path);
	// request->setCustomContentTypeString("Accept: text/html,application/xhtml+xml,application/xml;q=0.9,image/webp,*/*;q=0.8\nAccept-Language: en-US,en;q=0.5\n");

	bool done = false;
	http_client->sendRequest(request, [this, &done](drogon::ReqResult res, const drogon::HttpResponsePtr &resptr) {
		if (res != drogon::ReqResult::Ok) {
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

void WPDownloader::download_posts(const String &site) {
	setup_database();

	String url = Settings::get_singleton()->get_value(site + ".url");
	String port = Settings::get_singleton()->get_value(site + ".port");
	String first_url = Settings::get_singleton()->get_value(site + ".first_url");
	String last_url = get_last_url();

	// String url = "127.0.0.1";
	// String port = "http";
	// String first_url = "/";
	// String last_url = get_last_url();

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
				save_page(last_url, *ss, extracted_data);
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

void WPDownloader::setup(const String &site, Database *p_db) {
	site_name = site;
	db = p_db;
}

void WPDownloader::run() {
	download_posts(site_name);
}

void WPDownloader::stop() {
	done = true;
}

void WPDownloader::_thread_func() {
	main_event_loop = new trantor::EventLoop();
	main_event_loop->loop();

	delete main_event_loop;
	main_event_loop = nullptr;
}

WPDownloader::WPDownloader() {
	done = false;
	save_original_data = true;
	main_event_loop = nullptr;
	ss = nullptr;
	t = nullptr;
	db = nullptr;

	t = new std::thread(&WPDownloader::_thread_func, this);

	while (main_event_loop == nullptr) {
		// todo sleep
	}
}
WPDownloader::~WPDownloader() {
	main_event_loop->quit();
	t->join();

	delete t;
	t = nullptr;
}