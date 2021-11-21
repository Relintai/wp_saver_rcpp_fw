#include "wp_application.h"

#include "core/http/request.h"

#include <iostream>

#include "core/file_cache.h"

#include "core/http/handler_instance.h"

#include "core/database/database_manager.h"

#include "core/html/html_builder.h"
#include "core/http/http_session.h"
#include "core/http/session_manager.h"

void WPApplication::index(Object *instance, Request *request) {

	request->body += "test";
	request->compile_and_send_body();
}

void WPApplication::blog(Object *instance, Request *request) {
	request->body += "test";
	request->compile_and_send_body();
}

void WPApplication::routing_middleware(Object *instance, Request *request) {
	std::string path = request->get_path_full();

	WPApplication *app = Object::cast_to<WPApplication>(instance);

	if (FileCache::get_singleton()->wwwroot_has_file(path)) {
		app->send_file(path, request);

		return;
	}

	if (request->get_path_segment_count() == 0) {
		request->handler_instance = app->index_func;
	} else {
		const std::string main_route = request->get_current_path_segment();

		request->push_path();

		if (main_route == "blog") {
			request->handler_instance = app->blog_func;
		}
	}

	if (!request->handler_instance.handler_func) {
		app->send_error(404, request);

		return;
	}

	request->next_stage();
}

void WPApplication::setup_routes() {
	DWebApplication::setup_routes();

	index_func = HandlerInstance(index, this);
	blog_func = HandlerInstance(blog, this);
}

void WPApplication::setup_middleware() {
	middlewares.push_back(HandlerInstance(routing_middleware, this));
}

void WPApplication::migrate() {
}

void WPApplication::compile_menu() {
	HTMLBuilder bh;

	bh.meta()->charset_utf_8();
	bh.title();
	bh.w("WPSaver");
	bh.ctitle();

	bh.link()->rel_stylesheet()->href("site.css");
	bh.write_tag();

	header = bh.result;

	HTMLBuilder bf;

	bf.cdiv();
	bf.footer();
	bf.cfooter();

	footer = bf.result;
}

WPApplication::WPApplication() :
		DWebApplication() {

	compile_menu();
}

WPApplication::~WPApplication() {
}

