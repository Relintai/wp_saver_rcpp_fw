#ifndef CCMS_USER_CONTROLLER_H
#define CCMS_USER_CONTROLLER_H

#include "modules/rbac_users/rbac_user_controller.h"

#include <string>
#include "modules/users/user.h"

class Request;
class FormValidator;

class CCMSUserController : public RBACUserController {
	RCPP_OBJECT(CCMSUserController, RBACUserController);
public:
	void render_login_request_default(Request *request, LoginRequestData *data);
	void render_register_request_default(Request *request, RegisterRequestData *data);
	void render_login_success(Request *request);
	void render_already_logged_in_error(Request *request);
	void render_settings_request(Ref<User> &user, Request *request, SettingsRequestData *data);

	CCMSUserController();
	~CCMSUserController();
};

#endif