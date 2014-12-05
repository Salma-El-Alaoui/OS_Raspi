#define PAGE_SIZE			= 4*1024*8;// 4KB
#define SECON_LVL_TT_COUN 	= 256; // 2⁸=256 entrees au lvl 2
#define SECON_LVL_TT_SIZE 	= (SECON_LVL_TT_COUN * 20)/8; // taille du lvl 2 en octets
#define FIRST_LVL_TT_COUN 	= 4096; // 2^12=4096 entrees au lvl 1
#define FIRST_LVL_TT_SIZE 	= (FIRST_LVL_TT_COUN * 22)/8 // taille du lvl 1 en octets
#define TOTAL_TT_SIZE 		= SECON_LVL_TT_SIZE * FIRST_LVL_TT_COUN; 

