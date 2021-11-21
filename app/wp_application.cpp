#include "wp_application.h"

#include "core/http/request.h"

#include <iostream>

#include "core/file_cache.h"

#include "core/http/handler_instance.h"

#include "core/database/database_manager.h"

#include "core/html/html_builder.h"
#include "core/http/http_session.h"
#include "core/http/session_manager.h"

void WPApplication::index_fun(Object *instance, Request *request) {
	WPApplication *app = Object::cast_to<WPApplication>(instance);

	app->index(request);
}

void WPApplication::blog_fun(Object *instance, Request *request) {
	WPApplication *app = Object::cast_to<WPApplication>(instance);

	app->blog(request);
}

void WPApplication::index(Request *request) {
	HTMLBuilder b;

	b.div("content");

	b.div("content_head")->f()->w("Saved blogs:")->cdiv();

	for (int i = 0; i < _blog_data.size(); ++i) {
		BlogData &bd = _blog_data[i];

		b.div("content_row")->f()->fa("/blog/" + bd.name + "/", bd.name, "blog_link")->cdiv();
	}

	b.cdiv();

	request->body += b.result;
	request->compile_and_send_body();
}

void WPApplication::blog(Request *request) {
	request->body += "test blog";
	request->compile_and_send_body();
}

void WPApplication::routing_middleware(Object *instance, Request *request) {
	String path = request->get_path_full();

	WPApplication *app = Object::cast_to<WPApplication>(instance);

	if (FileCache::get_singleton()->wwwroot_has_file(path)) {
		app->send_file(path, request);

		return;
	}

	bool handled = false;

	if (request->get_path_segment_count() == 0) {
		handled = true;

		request->handler_instance = app->index_func;
	} else {
		const String main_route = request->get_current_path_segment();

		request->push_path();

		if (main_route == "blog") {
			handled = true;
			request->handler_instance = app->blog_func;
		}
	}

	if (!handled) {
		app->send_error(404, request);

		return;
	}

	request->next_stage();
}

void WPApplication::setup_routes() {
	DWebApplication::setup_routes();

	index_func = HandlerInstance(index_fun, this);
	blog_func = HandlerInstance(blog_fun, this);
}

void WPApplication::setup_middleware() {
	middlewares.push_back(HandlerInstance(routing_middleware, this));
}

void WPApplication::migrate() {
}

void WPApplication::add_blog(const String &name, Database *db) {
	BlogData bd;

	bd.name = name;
	bd.db = db;

	_blog_data.push_back(bd);
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
