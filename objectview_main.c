#include <errno.h>
#include <err.h>

#include "objectview.h"

int main(int argc, char **argv) {
	if (SDL_Init(SDL_INIT_EVERYTHING) != 0)
		err(errno, "%s", SDL_GetError());

	char *path;
	if (argc > 1)
		path = argv[1];

	ovobject *obj = ovobject_load(path);
	objectview *ov = ov_create(obj);
	ovwindow *ovw = ovwindow_create(path, ov);
	ovwindow_sdl_loop(NULL);
}
