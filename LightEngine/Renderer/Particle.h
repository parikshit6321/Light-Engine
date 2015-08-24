#pragma once

/*
	Structure that defines a particle in the particle system.
	active	-	boolean variable which tells whether the particle
	is dead or still visible.
	life	-	duration for which the particle will remain active.
	fade	-	factor by which the particle's life will decrease.
	x		-	particle's x co-ordinate.
	y		-	particle's y co-ordinate.
	z		-	particle's z co-ordinate.
	xi		-	particle's x velocity.
	yi		-	particle's y velocity.
	zi		-	particle's z velocity.
	xg		-	affect of gravity on particle in x direction.
	yg		-	affect of gravity on particle in y direction.
	zg		-	affect of gravity on particle in z direction.
*/

struct Particle
{
	bool	active;
	float	life;
	float	fade;
	float	x;
	float	y;
	float	z;
	float	xi;
	float	yi;
	float	zi;
	float	xg;
	float	yg;
	float	zg;
};