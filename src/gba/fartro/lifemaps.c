#include "lifemaps.h"

#include "bitmaps/omg.h"
#include "bitmaps/lifemap_1.h"
#include "bitmaps/lifemap_2.h"
#include "bitmaps/lifemap_3.h"
#include "bitmaps/lifemap_4.h"

const u16 lifemaps_length = 5;

static const u16* lifemaps[] =
	{	(u16 *)omgData,
		(u16 *)lifemap_1Data,
		(u16 *)lifemap_2Data,
		(u16 *)lifemap_3Data,
		(u16 *)lifemap_4Data,};

const u16* life_get_map(u16 nmap) {
	return lifemaps[nmap];
}
