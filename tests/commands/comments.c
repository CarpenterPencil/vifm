#include <stic.h>

#include <test-utils.h>

#include "../../src/cfg/config.h"
#include "../../src/engine/cmds.h"
#include "../../src/ui/ui.h"
#include "../../src/utils/env.h"
#include "../../src/cmd_core.h"

TEST(whole_line_comments)
{
	assert_success(cmds_dispatch1("\"", NULL, CIT_COMMAND));
	assert_success(cmds_dispatch1(" \"", NULL, CIT_COMMAND));
	assert_success(cmds_dispatch1("  \"", NULL, CIT_COMMAND));
}

TEST(trailing_comments)
{
	view_setup(&lwin);
	view_setup(&rwin);

	curr_view = &lwin;
	other_view = &rwin;

	cmds_init();

	opt_handlers_setup();

	assert_success(cmds_dispatch1("let $a = 4 \"", &lwin, CIT_COMMAND));
	assert_string_equal("4", env_get("a"));
	assert_success(cmds_dispatch1("let $a = \" 4 \"", &lwin, CIT_COMMAND));
	assert_string_equal(" 4 ", env_get("a"));
	assert_failure(cmds_dispatch1("echo \" 4 \"", &lwin, CIT_COMMAND));
	assert_success(cmds_dispatch1("exe \" 4 \"", &lwin, CIT_COMMAND));
	assert_success(cmds_dispatch1("unlet $a \"comment", &lwin, CIT_COMMAND));
	assert_string_equal(NULL, env_get("a"));

	assert_success(cmds_dispatch1("set statusline=\"  %t%= %A %15E %20d  \"",
				&lwin, CIT_COMMAND));
	assert_string_equal("  %t%= %A %15E %20d  ", cfg.status_line);

	opt_handlers_teardown();

	vle_cmds_reset();

	view_teardown(&lwin);
	view_teardown(&rwin);
}

/* vim: set tabstop=2 softtabstop=2 shiftwidth=2 noexpandtab cinoptions-=(0 : */
/* vim: set cinoptions+=t0 filetype=c : */
