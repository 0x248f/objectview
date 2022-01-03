#include <SDL2/SDL.h>

#include <pthread.h>
#include <stdbool.h>
#include <stdint.h>

typedef struct ovobject {
	double *data;
	uint64_t data_size, x, y, z; // data_size = x*y*z*sizeof(double)
	struct ovobject *previous, *next; // This is the time dimension
	uint32_t time_index;
} ovobject;

// TODO: Create from various formats
void ovobject_save(ovobject *obj, const char *path);
ovobject *ovobject_load(const char *path);
ovobject *ovobject_copy(ovobject *obj);
void ovobject_destroy(ovobject *obj);
void ovobject_destroy_all(ovobject *obj);
double ovobject_index(ovobject *obj, uint x, uint y, uint z);
double ovobject_index_time(ovobject *obj, uint x, uint y, uint z, uint t);
void ovobject_set(ovobject *obj, double value, uint x, uint y, uint z);
void ovobject_set_time(ovobject *obj, double value, uint t, uint x, uint y, uint z);
uint64_t ovobject_max_time(ovobject *obj);

typedef void (*ovobject_function)(ovobject *);

enum {
	OV_LUT_GREY_SCALE,
	OV_LUT_RGB_SCALE
};

enum {
	OV_VIEW_XY,
	OV_VIEW_XZ,
	OV_VIEW_YZ
};

typedef struct objectview {
	ovobject *object;
	int lut_type, view_type;
	uint64_t height, width, depth, current_slice;
        uint32_t t_max;
	uint8_t point_size;
	double max_value;
} objectview;

objectview *ov_create(ovobject *obj);
void ov_destroy(objectview *ov);
void ov_slice_up(objectview *ov);
void ov_slice_down(objectview *ov);
double ov_get_value(objectview *ov, int i, int j, int k);
void ov_lut_colour(objectview *ov, double val, SDL_Colour *colour);
void ov_update(objectview *ov);

typedef struct {
	char name[256];
	objectview *ov;
	bool running;
	uint32_t delay;

	ovobject_function step_function;
} ovcontext;

ovcontext *ovcontext_create(const char *name);
void ovcontext_destroy(ovcontext *ctx);
void ovcontext_toggle_running(ovcontext *ctx);
void ovcontext_step(ovcontext *ctx);
void ovcontext_step_back(ovcontext *ctx);

typedef struct {
	ovcontext *context;

	SDL_Window *window;
	uint16_t width, height;
	SDL_Renderer *renderer;
	pthread_t thread;
	pthread_mutex_t mutex;
} ovwindow;

ovwindow *ovwindow_create(const char *name, objectview *ov);
void ovwindow_destroy(ovwindow *ovw); // Frees the argument
void ovwindow_update(ovwindow *ovw);
void *ovwindow_sdl_loop(void *nullp);
