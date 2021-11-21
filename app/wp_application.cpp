#include "wp_application.h"

#include "core/http/request.h"

#include <iostream>

#include "core/file_cache.h"

#include "core/http/handler_instance.h"

#include "core/database/database_manager.h"
#include "core/database/query_builder.h"
#include "core/database/query_result.h"

#include "core/html/html_builder.h"
#include "core/http/http_session.h"
#include "core/http/session_manager.h"

#include "core/utils.h"

void WPApplication::index_fun(Object *instance, Request *request) {
	WPApplication *app = Object::cast_to<WPApplication>(instance);

	app->index(request);
}

void WPApplication::blog_fun(Object *instance, Request *request) {
	WPApplication *app = Object::cast_to<WPApplication>(instance);

	app->blog(request);
}

void WPApplication::index(Request *request) {
	request->head += header;

	HTMLBuilder b;

	b.div("content");

	b.div("content_head")->f()->w("Saved blogs:")->cdiv();

	for (int i = 0; i < _blog_data.size(); ++i) {
		BlogData &bd = _blog_data[i];

		b.div("content_row")->f()->fa("/blog/" + bd.name + "/", bd.name, "blog_link")->cdiv();
	}

	b.cdiv();

	request->body += b.result;
	request->body += footer;
	request->compile_and_send_body();
}

void WPApplication::blog(Request *request) {
	request->head += header;

	String blog = request->get_current_path_segment();

	Database *db = nullptr;

	for (int i = 0; i < _blog_data.size(); ++i) {
		BlogData &bd = _blog_data[i];

		if (bd.name == blog) {
			db = bd.db;
			break;
		}
	}

	if (!db) {
		request->send_error(404);
		return;
	}

	request->push_path();

	HTMLBuilder b;

	int page = 1;

	String action_segment = request->get_current_path_segment();

	if (action_segment == "") {
		// nothign to do
	} else if (action_segment == "page") {
		request->push_path();

		page = request->get_current_path_segment().to_int();
	} else if (action_segment == "post") {
		request->push_path();

		int post_id = request->get_current_path_segment().to_int();

		PostData *p = get_post(db, post_id);

		if (!p) {
			request->send_error(404);
			return;
		}

		b.div("content");
		b.div("blog_content")->f()->w(p->data)->cdiv();
		b.cdiv();

		request->body += b.result;
		request->body += footer;
		request->compile_and_send_body();
		return;
	} else {
		request->send_error(404);
		return;
	}

	if (page <= 0) {
		page = 1;
	}

	--page;

	Vector<PostData *> posts = get_posts(db, page);
	int post_count = (get_post_count(db) / 5.0) + 0.9;

	b.div("content");

	b.w(Utils::get_pagination("/blog/" + blog + "/page/", post_count, page));

	if (posts.size() != 0) {
		for (int i = 0; i < posts.size(); ++i) {
			PostData *p = posts[i];

			b.div("blog_content_row");
			b.div("blog_entry_link")->f()->fa("/blog/" + blog + "/post/" + String::num(p->id), "Open")->cdiv();
			b.div("blog_content")->f()->w(p->data)->cdiv();
			b.cdiv();
		}
	} else {
		b.fdiv("No saved blog posts.", "blog_content_row");
	}

	b.w(Utils::get_pagination("/blog/" + blog + "/page/", post_count, page));

	b.cdiv();

	request->body += b.result;
	request->body += footer;
	request->compile_and_send_body();
}

Vector<WPApplication::PostData *> WPApplication::get_posts(Database *db, const int page, const int num_per_page) {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->select("id,url,extracted_data")->from("data")->order_by_desc("id")->corder_by()->limit(num_per_page)->offset(page * num_per_page)->end_command();
	Ref<QueryResult> res = qb->run();

	Vector<PostData *> r;

	while (res->next_row()) {
		PostData *p = new PostData();

		p->id = res->get_cell_int(0);
		p->url = res->get_cell(1);
		p->data = res->get_cell(2);

		r.push_back(p);
	}

	return r;
}

int WPApplication::get_post_count(Database *db) {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->select("COUNT(id)")->from("data")->end_command();
	Ref<QueryResult> res = qb->run();

	if (!res->next_row()) {
		return 0;
	}

	return res->get_cell_int(0);
}

WPApplication::PostData *WPApplication::get_post(Database *db, const int id) {
	Ref<QueryBuilder> qb = db->get_query_builder();

	qb->select("id,url,extracted_data")->from("data")->where()->wp("id", id)->end_command();
	Ref<QueryResult> res = qb->run();

	if (!res->next_row()) {
		return nullptr;
	}

	PostData *p = new PostData();

	p->id = res->get_cell_int(0);
	p->url = res->get_cell(1);
	p->data = res->get_cell(2);

	return p;
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

	bh.link()->rel_stylesheet()->href("/site.css");
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
