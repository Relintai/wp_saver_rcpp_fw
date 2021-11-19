#include "ccms_application.h"

#include "core/http/request.h"

#include <iostream>

#include "core/file_cache.h"

#include "core/http/handler_instance.h"

#include "core/database/database_manager.h"

#include "core/html/html_builder.h"
#include "core/http/http_session.h"
#include "core/http/session_manager.h"

#include "modules/users/user.h"
//#include "modules/users/user_controller.h"
#include "modules/rbac_users/rbac_user_controller.h"

#include "modules/admin_panel/admin_panel.h"
#include "modules/rbac/rbac_controller.h"
#include "modules/rbac/rbac_model.h"

bool CCMSApplication::is_logged_in(Request *request) {
	if (!request->session) {
		return false;
	}

	Ref<User> u = request->reference_data["user"];

	return u.is_valid();
}

void CCMSApplication::index(Object *instance, Request *request) {
	ENSURE_LOGIN(request);

	add_menu(request, MENUENTRY_NEWS);

	/*
	<?php if (isset($hero)): ?>
	<link rel="stylesheet" type="text/css" href="<?=base_url('css/hero.css'); ?>">
	<?php endif; ?>

	<?php if ($userlevel > 2): ?>
		<link rel="stylesheet" type="text/css" href="<?=base_url('css/admin.css'); ?>">
	<?php endif; ?>

	<?php if ($page == 'mail'): ?>
		<link rel="stylesheet" type="text/css" href="<?=base_url('css/mail.css'); ?>">
	<?php endif; ?>

	<?php if ($resources): ?>
		<script src="<?=base_url('js/resource.js'); ?>"></script>
	<?php endif; ?>
	*/

	//dynamic_cast<ListPage *>(instance)->index(request);
	request->body += "test";
	request->compile_and_send_body();
}

void CCMSApplication::session_middleware_func(Object *instance, Request *request) {
}

void CCMSApplication::add_menu(Request *request, const MenuEntries index) {
	request->head += menu_head;

	HTMLBuilder b;

	HTMLTag *t;

	/*
	<?php if ($weather): ?>
	<div class="menu_base <?=$weather['css']; ?>">
	<?php else: ?>
	<div class="menu_base">
	<?php endif; ?>
*/

	b.div()->cls("menu_base");
	{
		b.div()->cls("left");
		{
			b.div()->cls("menu_news");
			{
				b.a()->href("/news/index");
				b.w("News");
				b.ca();
			}
			b.cdiv();

			b.div()->cls("menu_mail");
			{
				b.a()->href("/mail/inbox");
				b.w("Mails");
				//if ($newmail) echo '!';
				b.ca();
			}
			b.cdiv();

			b.div()->cls("menu_hero");
			{
				b.a()->href("/hero/selected");
				b.w("Hero");
				b.ca();
			}
			b.cdiv();

			b.div()->cls("menu_village");
			{
				b.a()->href("/village/selected");
				b.w("Village"); //villagename
				b.ca();
			}
			b.cdiv();

			b.div()->cls("menu_sel_village");
			{
				b.a()->href("/village/select");
				b.w("v");
				b.ca();
			}
			b.cdiv();

			/*
			<?php if ($alliancename): ?>
			<div class="menu_alliance">
			<a href="<?=site_url($menu_alliance); ?>">[<?=$alliancename; ?>]</a>
			</div>
			<?php endif; ?>
			*/

			/*
			<?php if ($weather): ?>
			<div class="weather">
			<abbr title="<?=$weather['description']; ?>"><?=$weather['name']; ?></abbr>
			</div>
			<?php endif; ?>
			*/
		}
		b.cdiv();

		b.div()->cls("right");
		{
			/*
			<?php if ($userlevel > 4): ?>
			<div class="menu_gm">
			<a href="<?=site_url($link_gm); ?>">GM</a>
			</div>
			<?php endif; ?>
			*/

			/*
			<?php if ($userlevel > 5): //dev+?>
			<div class="menu_admin">
			<a href="<?=site_url($link_admin); ?>">Admin</a>
			</div>
			<?php endif; ?>
			*/

			b.div()->cls("menu_alliance_menu");
			{
				b.a()->href("/alliance/alliance_menu");
				b.w("Alliances");
				b.ca();
			}
			b.cdiv();

			b.div()->cls("menu_forum");
			{
				b.a()->href("/forum/index");
				b.w("Forum");
				b.ca();
			}
			b.cdiv();

			b.div()->cls("menu_settings");
			{
				b.a()->href("/user/settings");
				b.w("Settings");
				b.ca();
			}
			b.cdiv();

			b.div()->cls("menu_logout");
			{
				b.a()->href("/user/logout");
				b.w("Logout");
				b.ca();
			}
			b.cdiv();
		}
		b.cdiv();

		b.div()->cls("nofloat");
		b.cdiv();
	}

	b.cdiv();
	b.div()->cls("main");
	b.write_tag();

	request->body += b.result;

	request->footer = footer;
}

void CCMSApplication::village_page_func(Object *instance, Request *request) {
	add_menu(request, MENUENTRY_VILLAGE);

	//dynamic_cast<ListPage *>(instance)->index(request);
	request->body += "test";
	request->compile_and_send_body();
}

void CCMSApplication::admin_page_func(Object *instance, Request *request) {
	AdminPanel::get_singleton()->handle_request_main(request);
}

void CCMSApplication::user_page_func(Object *instance, Request *request) {
	if (is_logged_in(request)) {
		add_menu(request, MENUENTRY_SETTINGS);
	}

	UserController::get_singleton()->handle_request_default(request);
}

void CCMSApplication::setup_routes() {
	DWebApplication::setup_routes();

	index_func = HandlerInstance(index);
	main_route_map["admin"] = HandlerInstance(admin_page_func);
	main_route_map["user"] = HandlerInstance(user_page_func);
}

void CCMSApplication::setup_middleware() {
	middlewares.push_back(HandlerInstance(::SessionManager::session_setup_middleware));
	//middlewares.push_back(HandlerInstance(::UserController::user_session_setup_middleware));
	//middlewares.push_back(HandlerInstance(::RBACUserController::rbac_user_session_setup_middleware));
	middlewares.push_back(HandlerInstance(::RBACUserController::rbac_default_user_session_middleware));

	DWebApplication::setup_middleware();
}

void CCMSApplication::migrate() {
	_rbac_model->migrate();
}

void CCMSApplication::compile_menu() {
	HTMLBuilder bh;

	bh.meta()->charset_utf_8();
	bh.meta()->name("description")->content("RPG browsergame");
	bh.meta()->name("keywords")->content("RPG,browsergame,Mourne,game,play");
	bh.title();
	bh.w("Mourne");
	bh.ctitle();

	bh.link()->rel_stylesheet()->href("/css/base.css");
	bh.link()->rel_stylesheet()->href("/css/menu.css");
	bh.write_tag();

	menu_head = bh.result;

	HTMLBuilder bf;

	bf.cdiv();
	bf.footer();
	bf.cfooter();

	footer = bf.result;
}

CCMSApplication::CCMSApplication() :
		DWebApplication() {

	_rbac_model = new RBACModel();
	_rbac_controller = new RBACController();
	_rbac_controller->initialize();

	_admin_panel = new AdminPanel();
	_admin_panel->register_admin_controller("rbac", _rbac_controller);
	
	compile_menu();
}

CCMSApplication::~CCMSApplication() {
	delete _admin_panel;
	delete _rbac_controller;
	delete _rbac_model;
}

std::string CCMSApplication::menu_head = "";
std::string CCMSApplication::footer = "";
