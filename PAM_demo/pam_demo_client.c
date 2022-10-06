#include <security/pam_appl.h>
#include <security/pam_misc.h>
#include <security/pam_modules.h>
#include <stdio.h>
#include <string.h>

int pam_demo_conv(int num_msg, struct pam_message **mess, struct pam_response **resp, void *my_data)
{
    struct pam_message *m = *mess;
    struct pam_response *r;
    char passwd[256];

    if (num_msg != 1)
    {
        printf("bad number of messages %d\n", num_msg);
        *resp = NULL;
        return PAM_CONV_ERR;
    }

    if ((*resp = r = calloc(num_msg, sizeof(struct pam_response))) == NULL)
        return PAM_BUF_ERR;

    if (m->msg_style == PAM_PROMPT_ECHO_ON)
    {
        printf("input passwd for %s:\n", m->msg);
        scanf("%255s", passwd);
        r->resp = malloc(strlen(passwd)+1);
        strcpy(r->resp, passwd);
        r->resp_retcode = 0;
        return PAM_SUCCESS;
    }
    return PAM_CONV_ERR;
}

// define a conv
static struct pam_conv conv = {
    pam_demo_conv,
    NULL};

// main function
int main(int argc, char *argv[])
{
    pam_handle_t *pamh = NULL;
    int retval;
    const char *user = NULL;
    const char *err_ptr = NULL;

    if (argc != 2)
    {
        fprintf(stderr, "Usage: pam_demo_client [username]\n");
        exit(1);
    }

    user = argv[1];

    printf("user: %s\n", user);
    retval = 0;

    // go with pam_demo conf in /etc/pam.d
    retval = pam_start("pam_demo", user, &conv, &pamh);

    if (retval == PAM_SUCCESS)
    {
        // start success, continue to anthenticate
        retval = pam_authenticate(pamh, 0);
    }
    else
    {
        // start failed, report the error
        err_ptr = pam_strerror(pamh, retval);
        printf("pam_start failed\n");
        printf("\t err code: %d\n", retval);
        printf("\t err str: %s\n", err_ptr);
        return retval;
    }
    if (retval == PAM_SUCCESS)
    {
        // authenticate successfully, continue to check the permit.
        retval = pam_acct_mgmt(pamh, 0); /* permitted access? */
    }
    else
    {
        // authenticate failed, report the error.
        err_ptr = pam_strerror(pamh, retval);
        printf("pam_authenticate failed\n");
        printf("\t err code: %d\n", retval);
        printf("\t err str: %s\n", err_ptr);
        return retval;
    }
    /* This is where we have been authorized or not. */

    if (retval == PAM_SUCCESS)
    {
        printf("authenticate successfully\n");
    }
    else
    {
        err_ptr = pam_strerror(pamh, retval);
        printf("pam_acct_mgmt failed\n");
        printf("\t err code: %d\n", retval);
        printf("\t err str: %s\n", err_ptr);
        return retval;
    }

    if (pam_end(pamh, retval) != PAM_SUCCESS)
    { /* close Linux-PAM */
        pamh = NULL;
        fprintf(stderr, "pam_demo_client: failed to release the resource\n");
        exit(1);
    }

    return (retval == PAM_SUCCESS ? 0 : 1); /* indicate success */
}
