#ifdef __TURBOC__
#include <mem.h>
#endif

#include "aatypes.h"
#include "aascreen.h"

void aa_copy_screen(Vscreen *s, Vscreen *d)
{
	memcpy(d->pmap, s->pmap, d->psize);
	memcpy(d->cmap, s->cmap, AA_COLORS*3);
}

void aa_clear_screen(Vscreen *vs)
{
	memset(vs->pmap, 0, vs->psize);
}
