#include <string.h>
#include <iostream>
#include <string>

#include "core/bry_http/http_server.h"
#include "core/file_cache.h"
#include "core/http/web_application.h"

#include "app/ccms_application.h"

#include "database/db_init.h"

#include "core/settings.h"

#include "core/http/session_manager.h"

#define MAIN_CLASS CCMSApplication

#include "modules/drogon/web_application.h"

//Backends
#include "backends/hash_hashlib/setup.h"

#include "app/ccms_user_controller.h"
#include "modules/users/user.h"
#include "modules/rbac_users/rbac_user_model.h"

#include "core/database/database_manager.h"
#include "platform/platform_initializer.h"

#include "core/os/platform.h"

#include "modules/rbac/rbac_rank.h"

#include "core/html/html_parser.h"

void initialize_backends() {
	initialize_database_backends();
	backend_hash_hashlib_install_providers();
}

void create_databases() {
	DatabaseManager *dbm = DatabaseManager::get_singleton();

	uint32_t index = dbm->create_database("sqlite");
	Database *db = dbm->databases[index];
	db->connect("database.sqlite");
}

int main(int argc, char **argv, char **envp) {
	PlatformInitializer::allocate_all();
	PlatformInitializer::arg_setup(argc, argv, envp);

	initialize_backends();

	bool migrate = false;

	for (int i = 1; i < argc; ++i) {
		const char *a = argv[i];

		if (a[0] == 'm') {
			migrate = true;
		}
	}

	String thtml = "<!doctype html><html><head><title>Title</title></head><body><h1 align=\"center\">Hello</h1><p>Hello world!</p></body></html>";

	String thtml2 = "<!doctype html><html><head><link rel=\"stylesheet\" type=\"text/css\" href=\"/css/base.css\"><link rel=\"stylesheet\" type=\"text/css\" href=\"/css/admin.css\"></head><body><div class=\"back\"><a href=\"/admin/buildings/\"><--- Back</a></div><br/><div class=\"top_menu\">Building Editor</div><br/><form method=\"post\" for=\"/admin/buildings/edit/\"><div class=\"row_edit\"><div class=\"edit_name\">Name:</div><div class=\"edit_input\"><input type=\"text\" name=\"name\" value=\"Build in Progress\" class=\"input\"></div></div><div class=\"row_edit_textbox\"><div class=\"edit_name\">Description:</div><div class=\"edit_input\"><textarea name=\"description\" class=\"textarea\"></textarea></div></div><div class=\"row_edit\"><div class=\"edit_name\">Icon:</div><div class=\"edit_input\">TODO</div></div><div class=\"row_edit\"><div class=\"edit_name\">Rank:</div><div class=\"edit_input\"><input type=\"text\" name=\"rank\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Next Rank:</div><div class=\"edit_input\"><select name=\"next_rank\" class=\"drop\"><option value=\"0\" selected>- None -</option><option value=\"1\">empty R0</option><option value=\"3\">Corn Field R1</option><option value=\"4\">Lumber Mill R1</option><option value=\"5\">Stone Mine R1</option><option value=\"6\">House R1</option><option value=\"7\">Corn Field R2</option><option value=\"8\">Farm R1</option><option value=\"9\">Iron Mine R1</option><option value=\"10\">School R1</option></select></div></div><div class=\"row_edit\"><div class=\"edit_name\">Time to Build:</div><div class=\"edit_input\"><input type=\"text\" name=\"time_to_build\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Score:</div><div class=\"edit_input\"><input type=\"text\" name=\"score\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Defense:</div><div class=\"edit_input\"><input type=\"text\" name=\"defense\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Ability:</div><div class=\"edit_input\">TODO</div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Cost Food:</div><div class=\"edit_input\"><input type=\"text\" name=\"cost_food\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Cost Wood:</div><div class=\"edit_input\"><input type=\"text\" name=\"cost_wood\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Cost Stone:</div><div class=\"edit_input\"><input type=\"text\" name=\"cost_stone\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Cost Iron:</div><div class=\"edit_input\"><input type=\"text\" name=\"cost_iron\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Cost Mana:</div><div class=\"edit_input\"><input type=\"text\" name=\"cost_mana\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Creates:</div><div class=\"edit_input\">TODO</div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Max Food:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_max_food\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Max Wood:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_max_wood\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Max Stone:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_max_stone\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Max Iron:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_max_iron\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Max Mana:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_max_mana\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Rate Food:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_rate_food\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Rate Wood:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_rate_wood\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Rate Stone:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_rate_stone\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Rate Iron:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_rate_iron\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Rate Mana:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_rate_mana\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Percent Food:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_percent_food\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Percent Wood:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_percent_wood\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Percent Stone:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_percent_stone\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Percent Iron:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_percent_iron\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Mod Percent Mana:</div><div class=\"edit_input\"><input type=\"text\" name=\"mod_percent_mana\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Assignment 1:</div><div class=\"edit_input\"><input type=\"text\" name=\"assignment1\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Assignment 2:</div><div class=\"edit_input\"><input type=\"text\" name=\"assignment2\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Assignment 3:</div><div class=\"edit_input\"><input type=\"text\" name=\"assignment3\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Assignment 4:</div><div class=\"edit_input\"><input type=\"text\" name=\"assignment4\" value=\"0\" class=\"input\"></div></div><div class=\"row_edit\"><div class=\"edit_name\">Assignment 5:</div><div class=\"edit_input\"><input type=\"text\" name=\"assignment5\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Required Technology:</div><div class=\"edit_input\"><input type=\"text\" name=\"req_tech\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Technology Group:</div><div class=\"edit_input\"><input type=\"text\" name=\"tech_group\" value=\"0\" class=\"input\"></div></div><div class=\"edit_spacer\"></div><div class=\"row_edit\"><div class=\"edit_name\">Secondary Technology Group:</div><div class=\"edit_input\"><input type=\"text\" name=\"tech_secondary_group\" value=\"0\" class=\"input\"></div></div><div class=\"edit_submit\"><input type=\"submit\" value=\"Save\" class=\"submit\"></div></form></body></html>";

	HTMLParser p;
	p.parse(thtml2);

	::SessionManager *session_manager = new ::SessionManager();

	//todo init these in the module automatically
	UserController *user_controller = new CCMSUserController();
	RBACUserModel *user_model = new RBACUserModel();
	//user_manager->set_path("./users/");

	Settings *settings = new Settings(true);
	//settings->parse_file("settings.json");

	FileCache *file_cache = new FileCache(true);
	file_cache->wwwroot = "./www";
	file_cache->wwwroot_refresh_cache();

	DatabaseManager *dbm = new DatabaseManager();

	create_databases();

	DWebApplication *app = new MAIN_CLASS();

	//Ref<RBACRank> r = RBACRank::db_get(1);

	//if (r.is_valid()) {
	//	RLOG_ERR("asdasdasd\n");
	//}

	app->load_settings();
	app->setup_routes();
	app->setup_middleware();

	app->add_listener("127.0.0.1", 8080);
	LOG_INFO << "Server running on 127.0.0.1:8080";
	
	if (!migrate) {
		session_manager->load_sessions();

		printf("Initialized!\n");
		app->run();
	} else {
		printf("Running migrations.\n");

		session_manager->migrate();
		user_model->migrate();

		app->migrate();
	}

	delete app;
	delete dbm;
	delete file_cache;
	delete settings;
	delete user_controller;
	delete user_model;
	delete session_manager;

	PlatformInitializer::free_all();

	return 0;
}