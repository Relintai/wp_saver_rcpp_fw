#ifndef WP_DOWNLOADER_H
#define WP_DOWNLOADER_H

#include "core/reference.h"

#include "core/database/database.h"
#include "modules/drogon/trantor/net/EventLoop.h"

class WPDownloader : public Reference {
	RCPP_OBJECT(WPDownloader, Reference);

public:
	void setup_database();
	String get_last_url();
	void save_page(const String &url, const String &full_data, const String &extracted_data);
	void query_page(trantor::EventLoop *loop, const String &url, const String &path);
	void download_posts(const String &site);

	void setup(const String &site, Database *p_db);

	void run();
	void stop();

	void _thread_func();

	WPDownloader();
	~WPDownloader();

	bool done;
	bool save_original_data;
	String site_name;
	trantor::EventLoop *main_event_loop;
	String *ss;
	std::thread *t;
	Database *db;
};

#endif