#ifndef CCMS_APPLICATION_H
#define CCMS_APPLICATION_H

//#include "core/http/web_application.h"
#include "core/object.h"
#include "modules/drogon/web_application.h"

#undef LOG_TRACE
#undef LOG_WARN

#include "modules/list_page/list_page.h"
#include "modules/message_page/message_page.h"
#include "modules/paged_article/paged_article.h"
#include "modules/paged_list/paged_list.h"

class AdminPanel;
class RBACController;
class RBACModel;

#define ENSURE_LOGIN(request)                  \
	if (!is_logged_in(request)) {              \
		request->send_redirect("/user/login"); \
		return;                                \
	}

class CCMSApplication : public DWebApplication {
public:
	enum MenuEntries {
		MENUENTRY_NEWS = 0,
		MENUENTRY_MAIL,
		MENUENTRY_HERO,
		MENUENTRY_VILLAGE,
		MENUENTRY_SELECT_VILLAGE,
		MENUENTRY_ALLIANCE,
		MENUENTRY_ALLIANCE_MENU,
		MENUENTRY_FORUM,
		MENUENTRY_CHANGELOG,
		MENUENTRY_SETTINGS,
		MENUENTRY_LOGOUT,

		MENUENTRY_MAX,
	};

public:
	static bool is_logged_in(Request *request);

	static void index(Object *instance, Request *request);

	static void session_middleware_func(Object *instance, Request *request);

	static void add_menu(Request *request, const MenuEntries index);

	static void village_page_func(Object *instance, Request *request);
	static void admin_page_func(Object *instance, Request *request);
	static void user_page_func(Object *instance, Request *request);

	virtual void setup_routes();
	virtual void setup_middleware();

	virtual void migrate();

	void compile_menu();

	CCMSApplication();
	~CCMSApplication();

	AdminPanel *_admin_panel; 
	RBACController *_rbac_controller;
	RBACModel *_rbac_model;

	static std::string menu_head;
	static std::string footer;
};

#endif