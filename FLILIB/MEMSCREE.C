#ifdef __TURBOC__
#include <mem.h>
#endif
#include <stdio.h>

#include "aatypes.h"
#include "aascreen.h"

Vscreen *aa_alloc_mem_cel(int x, int y, int w, int h)
{
	Vscreen *vs;

	if ((vs = (Vscreen *)malloc(sizeof(Vscreen))) != NULL) {
		memset(vs, 0, sizeof(Vscreen));
		vs->x = x;
		vs->y = y;
		vs->bpr = vs->w = w;
		vs->h = h;
		vs->psize = w*h;
		if ((vs->pmap = (Pixel *)malloc(vs->psize)) == NULL) {
			free(vs);
			return NULL;
		}
		if ((vs->cmap = (Cmap *)malloc(AA_COLORS*3)) == NULL) {
			aa_free_mem_screen(vs);
			return NULL;
		}
	}
	return vs;
}

Vscreen *aa_alloc_mem_screen(void)
{
return(aa_alloc_mem_cel(0, 0, AA_XMAX, AA_YMAX));
}

void aa_free_mem_screen(Vscreen *vs)
{
	if (vs) {
		if (vs->cmap)		free(vs->cmap);
		if (vs->pmap)		free(vs->pmap);
		free(vs);
	}
}
