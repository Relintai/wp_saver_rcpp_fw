#ifndef WP_APPLICATION_H
#define WP_APPLICATION_H

//#include "core/http/web_application.h"
#include "core/object.h"
#include "modules/drogon/web_application.h"

#undef LOG_TRACE
#undef LOG_WARN

class WPApplication : public DWebApplication {
	RCPP_OBJECT(WPApplication, DWebApplication);

public:
	static void index(Object *instance, Request *request);
	static void blog(Object *instance, Request *request);

	static void routing_middleware(Object *instance, Request *request);

	virtual void setup_routes();
	virtual void setup_middleware();

	virtual void migrate();

	void compile_menu();

	WPApplication();
	~WPApplication();

	static std::string menu_head;
	static std::string footer;
};

#endif