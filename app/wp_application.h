#ifndef WP_APPLICATION_H
#define WP_APPLICATION_H

//#include "core/http/web_application.h"
#include "core/object.h"
#include "core/string.h"
#include "modules/drogon/web_application.h"

class Database;

class WPApplication : public DWebApplication {
	RCPP_OBJECT(WPApplication, DWebApplication);

public:
	static void index_fun(Object *instance, Request *request);
	static void blog_fun(Object *instance, Request *request);

	void index(Request *request);
	void blog(Request *request);

	struct PostData {
		int id;
		String url;
		String data;
	};

	Vector<PostData *> get_posts(Database *db, const int page, const int num_per_page = 5);
	PostData * get_post(Database *db, const int id);
	int get_post_count(Database *db);

	static void routing_middleware(Object *instance, Request *request);

	virtual void setup_routes();
	virtual void setup_middleware();

	virtual void migrate();

	void add_blog(const String &name, Database *db);

	void compile_menu();

	WPApplication();
	~WPApplication();

	HandlerInstance blog_func;

	String header;
	String footer;

	struct BlogData {
		String name;
		Database *db;
	};

	Vector<BlogData> _blog_data;
};

#endif