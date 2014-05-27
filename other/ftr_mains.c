// cc99 -Ofast ftr_mains.c ftr_x11.c iio.o -o fm -ltiff -lpng -lm -lX11 -lrt

#include "seconds.c"
#include <assert.h>
#include <ctype.h>
#include <math.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "ftr.h"

#include "iio.h"


static unsigned char *read_image_uint8_rgb(char *fname, int *w, int *h)
{
	int pd;
	unsigned char *x = iio_read_image_uint8_vec(fname, w, h, &pd);
	if (pd == 3) return x;
	unsigned char *y = malloc(3**w**h);
	for (int i = 0; i < *w**h; i++) {
		switch(pd) {
		case 1:
			y[3*i+0] = y[3*i+1] = y[3*i+2] = x[i];
			break;
		case 2:
			y[3*i+0] = x[2*i+0];
			y[3*i+1] = y[3*i+2] = x[2*i+1];
			break;
		default:
			assert(pd > 3);
			for (int l = 0; l < 3; l++)
				y[3*i+l] = x[pd*i+l];
			break;
		}
	}
	free(x);
	return y;
}

static void write_image_uint8_rgb(char *fname, unsigned char *x, int w, int h)
{
	iio_save_image_uint8_vec(fname, x, w, h, 3);
}


// simple image viewer
int main_viewimage(int c, char *v[])
{
	// process input arguments
	if (c != 2 && c != 1) {
		fprintf(stderr, "usage:\n\t%s [image]\n", *v);
		//                          0  1
		return 1;
	}
	char *filename_in = c > 1 ? v[1] : "-";

	// read image
	int w, h;
	unsigned char *x = read_image_uint8_rgb(filename_in, &w, &h);

	// show image in window
	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);
	int r = ftr_loop_run(&f);

	// cleanup and exit (optional)
	free(x);
	ftr_close(&f);
	return r;
}

// even simpler
int main_viewimage0(int c, char *v[])
{
	// process input arguments
	if (c != 2 && c != 1) {
		fprintf(stderr, "usage:\n\t%s [image]\n", *v);
		//                          0  1
		return 1;
	}
	char *filename_in = c > 1 ? v[1] : "-";

	// read image
	int w, h;
	unsigned char *x = read_image_uint8_rgb(filename_in, &w, &h);

	// show image in window
	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);

	sleep(4);
	return 42;
}

// forking image viewer
int main_viewimage2(int c, char *v[])
{
	// process input arguments
	if (c != 2 && c != 1) {
		fprintf(stderr, "usage:\n\t%s [image]\n", *v);
		//                          0  1
		return 1;
	}
	char *filename_in = c > 1 ? v[1] : "-";

	// read image
	int w, h;
	unsigned char *x = read_image_uint8_rgb(filename_in, &w, &h);

	// show image in window
	ftr_fork_window_with_image_uint8_rgb(x, w, h);

	// cleanup and exit (optional)
	free(x);
	return 0;
}

// fancier image viewer
int main_display(int c, char *v[])
{
	// process input arguments
	if (c != 2 && c != 1) {
		fprintf(stderr, "usage:\n\t%s [image]\n", *v);
		//                          0  1
		return 1;
	}
	char *filename_in = c > 1 ? v[1] : "-";

	// read image
	int w, h;
	unsigned char *x = read_image_uint8_rgb(filename_in, &w, &h);

	// show image in window
	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);
	ftr_set_handler(&f, "key", ftr_handler_exit_on_ESC_or_q);
	ftr_set_handler(&f, "button", ftr_handler_stop_loop);
	int r = ftr_loop_run(&f);

	// cleanup and exit (optional)
	free(x);
	ftr_close(&f);
	return r;
}

// simple image viewer (does not actually work)
int main_twoimages(int c, char *v[])
{
	// process input arguments
	if (c != 3) {
		fprintf(stderr, "usage:\n\t%s image1 image2\n", *v);
		//                          0 1      2
		return 1;
	}
	char *filename1 = v[1];
	char *filename2 = v[2];

	// read images
	int w[2], h[2];
	unsigned char *x[2];
	x[0] = read_image_uint8_rgb(filename1, w+0, h+0);
	x[1] = read_image_uint8_rgb(filename2, w+1, h+1);

	// show image in window
	struct FTR f[2];
	f[0] = ftr_new_window_with_image_uint8_rgb(x[0], w[0], h[0]);
	f[1] = ftr_new_window_with_image_uint8_rgb(x[1], w[1], h[1]);

	sleep(4);

	// cleanup and exit (optional)
	return 0;
}

// forking image viewer
int main_twoimages2(int c, char *v[])
{
	// process input arguments
	if (c != 3) {
		fprintf(stderr, "usage:\n\t%s image1 image2\n", *v);
		//                          0 1      2
		return 1;
	}
	char *filename1 = v[1];
	char *filename2 = v[2];

	// read images
	int w[2], h[2];
	unsigned char *x1 = read_image_uint8_rgb(filename1, w+0, h+0);
	unsigned char *x2 = read_image_uint8_rgb(filename2, w+1, h+1);

	// show image in window
	ftr_fork_window_with_image_uint8_rgb(x1, w[0], h[0]);
	ftr_fork_window_with_image_uint8_rgb(x2, w[1], h[1]);

	// cleanup and exit (optional)
	free(x1);
	free(x2);
	return 0;
}



#define BAD_MIN(a,b) a<b?a:b
#define BAD_MAX(a,b) a>b?a:b

static void do_inline_crop_rgb(unsigned char *x, int *w, int *h, int crop[4])
{
	int from[2], to[2];
	from[0] = BAD_MIN(crop[0], crop[2]);
	from[1] = BAD_MIN(crop[1], crop[3]);
	to[0] = BAD_MAX(crop[0], crop[2]);
	to[1] = BAD_MAX(crop[1], crop[3]);
	int ow = to[0] - from[0]; assert(ow > 0); assert(ow <= *w);
	int oh = to[1] - from[1]; assert(oh > 0); assert(oh <= *h);
	for (int j = 0; j < oh; j++)
	for (int i = 0; i < ow; i++)
	{
		int o_idx = j*ow + i;
		int i_idx = (j+from[1])* *w + i+from[0];
		for (int l = 0; l < 3; l++)
			x[3*o_idx+l] = x[3*i_idx+l];
	}
	*w = ow;
	*h = oh;
}

// interactive crop
int main_icrop(int c, char *v[])
{
	// process input arguments
	if (c != 2 && c != 1 && c != 3) {
		fprintf(stderr, "usage:\n\t%s [in [out]]\n", *v);
		//                          0  1   2
		return 1;
	}
	char *filename_in = c > 1 ? v[1] : "-";
	char *filename_out = c > 2 ? v[2] : "-";

	// read image
	int w, h;
	unsigned char *x = read_image_uint8_rgb(filename_in, &w, &h);

	// show image in window
	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);

	// read the two corners of the crop rectangle
	int crop_param[4];
	ftr_wait_for_mouse_click(&f, crop_param+0, crop_param+1, NULL, NULL);
	ftr_wait_for_mouse_click(&f, crop_param+2, crop_param+3, NULL, NULL);

	// perform the crop on the image data
	do_inline_crop_rgb(x, &w, &h, crop_param);

	// write outpuf file
	write_image_uint8_rgb(filename_out, x, w, h);

	// cleanup and exit (optional)
	ftr_close(&f);
	free(x);
	return 0;
}

struct icrop2_state {
	unsigned char *original_image;
	int step, ox, oy;
};

static int inbetween(int a, int b, int x)
{
	return (a <= x && x <= b) || (b <= x && x <= a);
}

void icrop2_motion(struct FTR *f, int b, int m, int x, int y)
{
	//if (ftr_num_pending(f)) return;

	struct icrop2_state *e = f->userdata;
	unsigned char *o = e->original_image;

	if (e->step == 0) {
		e->ox = x;
		e->oy = y;
		for (int j = 0; j < f->h; j++)
		for (int i = 0; i < f->w; i++)
		for (int l = 0; l < 3; l++)
		{
			int idx = 3*(j * f->w + i) + l;
			f->rgb[idx] = (i>=x && j>=y) ? o[idx] : o[idx]/2;
		}
	} else {
		fprintf(stderr, "step2 (%d %d) (%d %d)\n", e->ox, e->oy, x, y);
		for (int j = 0; j < f->h; j++)
		for (int i = 0; i < f->w; i++)
		for (int l = 0; l < 3; l++)
		{
			int idx = 3*(j * f->w + i) + l;
			f->rgb[idx] = inbetween(e->ox, x, i)
				&& inbetween(e->oy, y, j)
			//f->rgb[idx] = (i>=e->ox && j>=e->oy
			//		&& i <= x && j <= y)
				? o[idx] : o[idx]/2;
		}
	}
	f->changed = 1;
}

// interactive crop, just fancier
int main_icrop2(int c, char *v[])
{
	// process input arguments
	if (c != 2 && c != 1 && c != 3) {
		fprintf(stderr, "usage:\n\t%s [in [out]]\n", *v);
		//                          0  1   2
		return 1;
	}
	char *filename_in = c > 1 ? v[1] : "-";
	char *filename_out = c > 2 ? v[2] : "-";

	// read image
	int w, h;
	unsigned char *x = read_image_uint8_rgb(filename_in, &w, &h);

	// show image in window
	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);
	struct icrop2_state e = {x, 0, 0, 0};
	f.userdata = &e;


	// set handlers
	ftr_set_handler(&f, "motion", icrop2_motion);

	// read the two corners of the crop rectangle
	int crop_param[4];
	ftr_wait_for_mouse_click(&f, crop_param+0, crop_param+1, NULL, NULL);
	e.step = 1;
	ftr_wait_for_mouse_click(&f, crop_param+2, crop_param+3, NULL, NULL);

	// perform the crop on the image data
	do_inline_crop_rgb(x, &w, &h, crop_param);

	// write outpuf file
	write_image_uint8_rgb(filename_out, x, w, h);

	// cleanup and exit (optional)
	ftr_close(&f);
	free(x);
	return 0;
}

// print to stdout the coordinates of the first mouse click
int main_pclick(int c, char *v[])
{
	// process input arguments
	if (c != 2 && c != 1) {
		fprintf(stderr, "usage:\n\t%s [in] > pos.txt\n", *v);
		//                          0  1
		return 1;
	}
	char *filename_in = c > 1 ? v[1] : "-";

	// read image
	int w, h;
	unsigned char *x = read_image_uint8_rgb(filename_in, &w, &h);

	// show image on window
	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);

	// get the first mouse click
	int pos[2];
	ftr_wait_for_mouse_click(&f, pos+0, pos+1, NULL, NULL);

	// close the window
	ftr_close(&f);

	// print the coordinates to stdout
	printf("%d %d\n", pos[0], pos[1]);

	// cleanup and exit (optional)
	free(x);
	return 0;
}


static void draw_random(struct FTR *f, int x, int y, int k, int m)
{
	// benchmarking control
	static int cx = 0;
	cx += 1;
	if (0 == cx % 100) {
		static double os = 0;
		double s = seconds();
		double dif = s - os;
		double fps = 100/dif;
		fprintf(stdout, "CX = %d\t%g FPS\n", cx++, fps);
		os = s;
	}

	// actual drawing
	f->rgb[0] = (512341*cx)%256;
	for (int i = 1; i < f->w * f->h * 3; i++)
		f->rgb[i] = (223+f->rgb[i-1]*912359+1234567*i+54321*cx)%256;
	f->changed = 1;
}

// display an animation
int main_random(int c, char *v[])
{
	int w = 800;
	int h = 600;
	unsigned char *x = malloc(3*w*h);

	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);
	ftr_set_handler(&f, "idle", draw_random);
	ftr_loop_run(&f);

	ftr_close(&f);
	free(x);
	return 0;
}


static float lstep(float a, float b, float t, float x)
{
	if (x < a) return 0;
	if (x > b) return t;
	return t*(x-a)/(b-a);
}


static void draw_fire(struct FTR *f, int x, int y, int k, int m)
{
	// measure time
	static int cx = 0;
	cx += 1;
	if (0 == cx % 100) {
		static double os = 0;
		double s = seconds();
		double dif = s - os;
		double fps = 100/dif;
		fprintf(stdout, "CX = %d\t%g FPS\n", cx++, fps);
		os = s;
	}

	// build palette
	static unsigned char *pal = NULL;
	if (!pal) {
		pal = malloc(3*256);
		for (int i = 0; i < 256; i++) {
			pal[3*i+0] = lstep(0,105,255,i);
			pal[3*i+1] = lstep(60,120,255,i);
			pal[3*i+2] = lstep(150,160,255,i);
		}
	}

	int num_lines_bottom = 5;
	int num_lines_hidden = 25;

	// build buffer
	static float *t = NULL;
	static int w = 0;
	static int h = 0;
	if (!f || w != f->w || h != f->h + num_lines_hidden) { 
		w = f->w;
		h = f->h + num_lines_hidden;
		if (t) free(t);
		t = malloc(w * h * sizeof*f); 
		for (int i = 0; i < w*h; i++)
			t[i] = 104;
		cx = 0;
	}

	// draw random values at the bottom
	int p = 0;
	int rfac = cx < 75 ? 200 : 10;
	for (int j = 0; j < num_lines_bottom; j++)
	for (int i = 0; i < w; i++) {
		t[p] = fmod(t[p] + rfac*(rand()/(1.0+RAND_MAX)),256);
		p++;
	}

	// paint pixels by combining lower rows
	for (int j = h-1; j >= num_lines_bottom; j--)
	for (int i = 0; i < w; i++) {
		p = j*w+i;
		t[p+2*w+1] = (1.5*t[p-3*w] + 1.7 * t[p-2*w+1] 
				+ 1.5 * t[p-4*w] + 1.9 * t[p-3*w-1]
				+ 1.0 * t[p-1*w-2]
				+1.9 * t[p-4*w+1]
			) / 9.51;
	}

	// render with palette
	for (int j = 0; j < h-num_lines_hidden; j++)
	for (int i = 0; i < w; i++)
	{
		int iidx = w*(h-j-1) + i;
		int idx = (unsigned char)(lstep(105,145,255,t[iidx]));
		int pos = w*j + i;
		f->rgb[3*pos+0] = pal[3*idx+0];
		f->rgb[3*pos+1] = pal[3*idx+1];
		f->rgb[3*pos+2] = pal[3*idx+2];
	}
}

static void draw_minifire(struct FTR *f, int x, int y, int k, int m)
{
	// build palette
	unsigned char pal[3*256];
	for (int i = 0; i < 256; i++) {
		pal[3*i+0] = -7*i;
		pal[3*i+1] = 5*i;;
		pal[3*i+2] = 3*i;
	}

	int num_lines_bottom = 5;

	// build buffer
	static float *t = NULL;
	static int w = 0;
	static int h = 0;
	if (!f || w != f->w || h != f->h) { 
		w = f->w;
		h = f->h;
		if (t) free(t);
		t = malloc(w * h * sizeof*t); 
		for (int i = 0; i < w*h; i++)
			t[i] = 104;
		f->changed = 1;
	}

	// draw random values at the bottom
	int p = 0;
	for (int j = 0; j < num_lines_bottom; j++)
	for (int i = 0; i < w; i++) {
		t[p] = fmod(t[p] + 15*(rand()/(1.0+RAND_MAX)),256);
		p++;
	}

	// paint pixels by combining lower rows
	for (int j = h-3; j >= num_lines_bottom; j--)
	for (int i = 0; i < w; i++) {
		p = j*w+i;
		t[p+2*w+1] = (1.5*t[p-3*w] + 1.7 * t[p-2*w+1] 
				+ 1.5 * t[p-4*w] + 1.9 * t[p-3*w-1]
				+ 1.0 * t[p-1*w-2]
				+1.9 * t[p-4*w+1]
			) / 9.51;
	}

	// render with palette
	for (int j = 0; j < h; j++)
	for (int i = 0; i < w; i++)
	{
		int idx = (unsigned char)(t[w*(h-j-1) + i]);
		f->rgb[3*(w*j+i)+0] = pal[3*idx+0];
		f->rgb[3*(w*j+i)+1] = pal[3*idx+1];
		f->rgb[3*(w*j+i)+2] = pal[3*idx+2];
	}
}

static void toggle_idle2(struct FTR *f, int b, int m, int x, int y)
{
	static ftr_event_handler_t prev = NULL;
	ftr_event_handler_t new = ftr_get_handler(f, "idle");
	ftr_set_handler(f, "idle", prev);
	prev = new;
}

static void fire_resize(struct FTR *f, int b, int m, int x, int y)
{
	fprintf(stderr, "resize %d %d\n", x, y);
}

// display another animation
int main_fire(int c, char *v[])
{
	struct FTR f = ftr_new_window(800, 600);
	ftr_set_handler(&f, "idle", draw_fire);
	//ftr_set_handler(&f, "button", toggle_idle2);
	ftr_set_handler(&f, "button", ftr_handler_toggle_idle);
	ftr_set_handler(&f, "resize", fire_resize);
	ftr_loop_run(&f);
	ftr_close(&f);
	return 0;
}

// display another animation
int main_minifire(int c, char *v[])
{
	int w = 800;
	int h = 600;
	unsigned char *x = malloc(3*w*h);

	struct FTR f = ftr_new_window_with_image_uint8_rgb(x, w, h);
	ftr_set_handler(&f, "idle", draw_minifire);
	//ftr_set_handler(&f, "button", ftr_handler_toggle_idle);
	ftr_set_handler(&f, "resize", fire_resize);
	ftr_loop_run(&f);

	ftr_close(&f);
	free(x);
	return 0;
}

static void print_event_key(struct FTR *f, int k, int m, int x, int y)
{
	printf("event KEY     k=%d '%c'\tm=%d (%d %d)\n", k,
			(isprint(k))?k:' ', m, x, y);
}
static void print_event_button(struct FTR *f, int k, int m, int x, int y)
{
	printf("event BUTTON  b=%d\tm=%d (%d %d)\n", k, m, x, y);
}
static void print_event_motion(struct FTR *f, int b, int m, int x, int y)
{
	printf("event MOTION  b=%d\tm=%d (%d %d)\n", b, m, x, y);
}
static void print_event_resize(struct FTR *f, int b, int m, int x, int y)
{
	printf("event RESIZE  b=%d\tm=%d (%d %d)\n", b, m, x, y);
}

int main_events(int c, char *v[])
{
	struct FTR f = ftr_new_window(320, 200);
	ftr_set_handler(&f, "key", print_event_key);
	ftr_set_handler(&f, "button", print_event_button);
	ftr_set_handler(&f, "motion", print_event_motion);
	ftr_set_handler(&f, "resize", print_event_resize);
	return ftr_loop_run(&f);
}

int main(int c, char *v[])
{
	int (*f)(int,char*[]);
	if (c < 2) return fprintf(stderr, "name a main\n");
	else if (0 == strcmp(v[1], "viewimage")) f = main_viewimage;
	else if (0 == strcmp(v[1], "viewimage0")) f = main_viewimage0;
	else if (0 == strcmp(v[1], "viewimage2")) f = main_viewimage2;
	else if (0 == strcmp(v[1], "display")) f = main_display;
	else if (0 == strcmp(v[1], "twoimages"))     f = main_twoimages;
	else if (0 == strcmp(v[1], "twoimages2"))     f = main_twoimages2;
	else if (0 == strcmp(v[1], "icrop"))     f = main_icrop;
	else if (0 == strcmp(v[1], "icrop2"))     f = main_icrop2;
	else if (0 == strcmp(v[1], "pclick"))    f = main_pclick;
	else if (0 == strcmp(v[1], "random"))    f = main_random;
	else if (0 == strcmp(v[1], "fire"))      f = main_fire;
	else if (0 == strcmp(v[1], "minifire"))      f = main_minifire;
	else if (0 == strcmp(v[1], "events")) f = main_events;
	//else if (0 == strcmp(*v, "simplest"))  f = main_simplest;
	//else if (0 == strcmp(*v, "simplest2")) f = main_simplest2;
	else return fprintf(stderr, "bad main \"%s\"\n", v[1]);
	return f(c-1, v+1);
}