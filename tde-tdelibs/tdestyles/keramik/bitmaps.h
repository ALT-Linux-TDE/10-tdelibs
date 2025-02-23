#ifndef __BITMAPS_H
#define __BITMAPS_H

/* Image bits processed by KPixmap2Bitmaps */
// Arrow bitmaps
static const TQCOORD u_arrow[]={-1,-3, 0,-3, -2,-2, 1,-2, -3,-1, 2,-1, -4,0, 3,0, -4,1, 3,1};
static const TQCOORD d_arrow[]={-4,-2, 3,-2, -4,-1, 3,-1, -3,0, 2,0, -2,1, 1,1, -1,2, 0,2};
static const TQCOORD l_arrow[]={-3,-1, -3,0, -2,-2, -2,1, -1,-3, -1,2, 0,-4, 0,3, 1,-4, 1,3};
static const TQCOORD r_arrow[]={-2,-4, -2,3, -1,-4, -1,3, 0,-3, 0,2, 1,-2, 1,1, 2,-1, 2,0};

static const TQCOORD keramik_combo_arrow[] =
	{-4,-5, 4, -5,
	 -2 ,-2, 2, -2,
	  -2 ,-1, 2, -1,
	   -2 ,0, 2, 0,
	   -4, 1, 4, 1,
	   -3, 2, 3, 2,
	   -2 , 3, 2, 3,
	   -1 , 4, 1, 4,
	   0 , 5, 0, 5
	    };
	    

static const TQCOORD keramik_up_arrow[] = 
	{
		0, -4, 0, -4,
		-1, -3, 1, -3,
		-2, -2, 2, -2,
		-3, -1, 3, -1,
		-4, 0, 4, 0,
		-2, 1, 2, 1,
		-2, 2, 2, 2,
		-2, 3, 2, 3,
		-2, 4, 2, 4
	};
	
static const TQCOORD keramik_down_arrow[] = 
	{
		0, 4, 0, 4,
		-1, 3, 1, 3,
		-2, 2, 2, 2,
		-3, 1, 3, 1,
		-4, 0, 4, 0,
		-2, -1, 2, -1,
		-2, -2, 2, -2,
		-2, -3, 2, -3,
		-2, -4, 2, -4
	};


	static const TQCOORD keramik_right_arrow[] = 
	{
		4, 0, 4, 0,
		3, -1, 3, 1,
		2, -2, 2, 2,
		1, -3, 1, 3,
		0, -4, 0, 4,
		-1, -2, -1, 2,
		-2, -2, -2, 2,
		-3, -2, -3, 2,
		-4, -2, -4, 2
	};
	
	static const TQCOORD keramik_left_arrow[] = 
	{
		-4, 0, -4, 0,
		-3, -1, -3, 1,
		-2, -2, -2, 2,
		-1, -3, -1, 3,
		0, -4, 0, 4,
		1, -2, 1, 2,
		2, -2, 2, 2,
		3, -2, 3, 2,
		4, -2, 4, 2
	};

	
		
#define TQCOORDARRLEN(x) sizeof(x)/(sizeof(TQCOORD)*2)



#endif
