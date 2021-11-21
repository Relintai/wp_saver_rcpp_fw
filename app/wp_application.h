#ifndef WP_APPLICATION_H
#define WP_APPLICATION_H

//#include "core/http/web_application.h"
#include "core/object.h"
#include "core/string.h"
#include "modules/drogon/web_application.h"

#undef LOG_TRACE
#undef LOG_WARN

class WPApplication : public DWebApplication {
	RCPP_OBJECT(WPApplication, DWebApplication);

public:
	static void index_fun(Object *instance, Request *request);
	static void blog_fun(Object *instance, Request *request);

	void index(Request *request);
	void blog(Request *request);

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