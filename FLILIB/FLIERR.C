
#include "aatypes.h"
#include "aaerr.h"

static char *err_msgs[] = {
	"Everything seems ok",
	"Unknown error",
	"Out of memory",
	"Can't find file",
	"Can't make file (disk write-protected?)",
	"Not correct type of file",
	"File internally damaged",
	"Wrong resolution, not 320x200",
	"File too short",
	"Unable to write all of file",
	"Error during seek",
	};

char *fli_error_message(Errval err)
{
if (err >= 0)
	err = 0;
else
	err = -err;
if (err > Array_els(err_msgs))
	err = AA_ERR_MISC;
return(err_msgs[err]);
}
