#include "messages.h"

TextString text_strings[] = { {"SVO", 							200, 	140, 	128, 	150},
							  {"#OLDFARTS", 					10,		140, 	256,	150},
							  {"HOORJ!!",						95,		10,		384,	250},
							  {"GBA PR0GR4MM1NG 15 T3H PHUN",	10, 	140, 	400, 	230},


#define TXT_HIS		800
#define TXT_NH		13
#define TXT_HDUR	96
							  {"SALUT !!!",						85, 	10,		TXT_HIS,				TXT_HDUR*12},
							  {"DESASTER",					 	85,		140,	TXT_HIS+TXT_HDUR,		TXT_HDUR},
							  {"GREEN",					 		100,	140,	TXT_HIS+TXT_HDUR*2,		TXT_HDUR},
							  {"JANO",				 			103,	140,	TXT_HIS+TXT_HDUR*3,		TXT_HDUR},
							  {"KIB_TPH",				 		90,		140,	TXT_HIS+TXT_HDUR*4,		TXT_HDUR},
							  {"MACKE",				 			100,	140,	TXT_HIS+TXT_HDUR*5,		TXT_HDUR},
							  {"MIKASAARI",				 		85,		140,	TXT_HIS+TXT_HDUR*6,		TXT_HDUR},
							  {"SGEFANT",				 		89,		140,	TXT_HIS+TXT_HDUR*7,		TXT_HDUR},
							  {"NISHIN",				 		96,		140,	TXT_HIS+TXT_HDUR*8,		TXT_HDUR},
							  {"POE-T",				 			100,	140,	TXT_HIS+TXT_HDUR*9,		TXT_HDUR},
							  {"TWINGY",				 		95,		140,	TXT_HIS+TXT_HDUR*10,	TXT_HDUR},
							  {"UNNAMED",				 		88,		140,	TXT_HIS+TXT_HDUR*11,	TXT_HDUR},
							  {"...",				 			108,	140,	TXT_HIS+TXT_HDUR*12,	TXT_HDUR},

#define TXT_MIS	(TXT_HIS+TXT_HDUR*TXT_NH)
							  {"MONTZ & KATI",					70, 	20,		TXT_MIS,				TXT_HDUR/2},
							  {"MONTZ & KATI",					70, 	20,		TXT_MIS+TXT_HDUR/2,		TXT_HDUR*3+TXT_HDUR/2},
							  {"MISSING YOU HEAPS",				50,		30,		TXT_MIS+TXT_HDUR*2,		TXT_HDUR},
							  {"COME TO US",					75,		40,		TXT_MIS+TXT_HDUR*3,		TXT_HDUR},

#define TXT_MOO	(TXT_MIS+TXT_HDUR*8)								  
#define TXT_MODUR	20
							  {"MOO!!",							80, 	100,	TXT_MOO+TXT_MODUR*1,	TXT_MODUR},
							  {"MOOO!!",						180, 	70,		TXT_MOO+TXT_MODUR*2,	TXT_MODUR+7},
							  {"MOO!!",							20, 	120,	TXT_MOO+TXT_MODUR*3,	TXT_MODUR+7},
							  {"MOOOO!!",						70, 	10,		TXT_MOO+TXT_MODUR*4,	TXT_MODUR+7},
							  {"MOO!!",							140, 	111,	TXT_MOO+TXT_MODUR*5,	TXT_MODUR+7},
							  {"MOO!!",							120, 	30,		TXT_MOO+TXT_MODUR*6,	TXT_MODUR+7},
							  {"MOOOO!!",						30, 	130,	TXT_MOO+TXT_MODUR*7,	TXT_MODUR+7},
							  {"MOO!!",							66, 	50,		TXT_MOO+TXT_MODUR*8,	TXT_MODUR+7},
							  {"MOO!!",							70, 	10,		TXT_MOO+TXT_MODUR*9,	TXT_MODUR+7},
							  {"MOOO!!",						140, 	111,	TXT_MOO+TXT_MODUR*10,	TXT_MODUR+7},
							  {"MOO!!",							120, 	30,		TXT_MOO+TXT_MODUR*11,	TXT_MODUR+7},
							  {"MOO!!",							99, 	99,		TXT_MOO+TXT_MODUR*12,	TXT_MODUR+7},
							  {"MOOOOO!!",						80, 	100,	TXT_MOO+TXT_MODUR*13,	TXT_MODUR+7},
							  {"MOO!!",							180, 	70,		TXT_MOO+TXT_MODUR*14,	TXT_MODUR+7},
							  {"MOMOOOHOO!!",					77, 	120,	TXT_MOO+TXT_MODUR*15,	TXT_MODUR+7},
//							  {"MOO!!",							99, 	10,		TXT_MOO+TXT_MODUR*16,	TXT_MODUR},


#define TXT_KUDOS	(TXT_MOO+TXT_MODUR*25)
#define TXT_KDUR	96
							  {"KUDOS TO",						 85, 	10,		TXT_KUDOS,				TXT_KDUR*7},
							  {"CLD - TONC",					 77,	140,	TXT_KUDOS+TXT_KDUR,		TXT_KDUR},
							  {"DANNYBOY - F.R.A.S.",			 52,	140,	TXT_KUDOS+TXT_KDUR*2,	TXT_KDUR},
							  {"GBADEV.NET",					 80,	140,	TXT_KUDOS+TXT_KDUR*3,	TXT_KDUR},
							  {"DEVKIT PPL",					 80,	140,	TXT_KUDOS+TXT_KDUR*4,	TXT_KDUR},
							  {"-ALL GBA DEVELOPERS-",			 42,	140,	TXT_KUDOS+TXT_KDUR*6,	TXT_KDUR},

							  {"MUSIC BY ERIC BERG",			 45,	25,		TXT_KUDOS+TXT_KDUR*8,	TXT_KDUR*2},
							  {"CODE & GFX BY SVO",			 	48,		25,		TXT_KUDOS+TXT_KDUR*10,	TXT_KDUR},

#define TXT_BYE		(TXT_KUDOS+TXT_KDUR*16)
#define TXT_BUR		128
							  {"WTF?!?",				 		97,		120,	TXT_BYE,				TXT_BUR},

							  {"BYE BYE",				 		93,		120,	TXT_BYE+TXT_BUR*2,		TXT_BUR},

							  {"WHAT ARE YOU LOOKING AT?",		25,		30,		TXT_BYE+TXT_BUR*10,		TXT_BUR},

							  {"BATTERY LOW",					75,		120,	TXT_BYE+TXT_BUR*20,		TXT_BUR*2},

							  {"",								100,	120,	TXT_BYE+TXT_BUR*24,		TXT_BUR},

							  {0, 0, 0, 0, 0}

                               };
