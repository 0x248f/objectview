#include <sys/queue.h>
#include <stdlib.h>

#include "objectview.h"

typedef struct ovw_list {
	ovwindow *ovw;
	struct ovw_list *next;
} ovw_list;

ovw_list *window_list = NULL;
int action_bar_height = 140;

ovwindow *ovwindow_from_windowID(uint32_t windowID) {
	ovw_list *entry;
	for (entry = window_list; entry != NULL; entry = entry->next)
		if (windowID == SDL_GetWindowID(entry->ovw->window))
			return entry->ovw;

	return NULL;
}

// Only one thread needed
void *ovwindow_sdl_loop(void *nullp) {
	SDL_Event event;
	ovwindow *ovw;

	while (true) {
		SDL_PollEvent(&event); // TODO: check sdl window id against ovwindows (store them in a linked list)

		ovw = ovwindow_from_windowID(event.window.windowID);
		if (ovw == NULL)
			continue;

		if (event.type == SDL_QUIT) {
			ovwindow_destroy(ovw);
			if (window_list == NULL) {
				SDL_Quit();
				return NULL;
			}
		}
		if (event.type == SDL_KEYDOWN) {
			switch (event.key.keysym.sym) {
			case SDLK_SPACE:
				ovcontext_toggle_running(ovw->context);
				ovwindow_update(ovw);
				break;
			case SDLK_UP:
				ov_slice_up(ovw->context->ov);
				ovwindow_update(ovw);
				break;
			case SDLK_DOWN:
				ov_slice_down(ovw->context->ov);
				ovwindow_update(ovw);
				break;
			case SDLK_RIGHT:
				ovcontext_step(ovw->context);
				ovwindow_update(ovw);
				break;
			case SDLK_LEFT:
				ovcontext_step_back(ovw->context);
				ovwindow_update(ovw);
				break;
			default:
				break;
			}
		}
	}

	return NULL;
}

// One thread per window
void *ovwindow_action_loop(void *ovw_voidp) {
	ovwindow *ovw = (ovwindow *)ovw_voidp;
	ovcontext *ctx = ovw->context;
	while (true) {
		if (ctx->running) {
			ovcontext_step(ctx);
			ovwindow_update(ovw);
			SDL_RenderPresent(ovw->renderer);
			SDL_Delay(ctx->delay);
		} else {
			SDL_Delay(250);
		}

		ovwindow_update(ovw);
	}
}

void draw_action_bar(ovwindow *ovw) {
	// TODO: Draw buttons for the action bar and keep their position in a struct placed in ovw
}

ovwindow *ovwindow_create(const char *name, objectview *ov) {
	if (ov == NULL || ov->object == NULL)
		return NULL;

	ovwindow *ovw = malloc(sizeof(ovwindow));
	memset(ovw, 0, sizeof(ovwindow));
	ovw->context = ovcontext_create(name);
	ovw->context->ov = ov;
	ov->point_size = 4;

	int X = ov->width*ov->point_size, Y = ov->height*ov->point_size + action_bar_height;
	ovw->window = SDL_CreateWindow(ovw->context->name, SDL_WINDOWPOS_UNDEFINED, SDL_WINDOWPOS_UNDEFINED, X, Y, SDL_WINDOW_SHOWN);
	ovw->renderer = SDL_CreateRenderer(ovw->window, -1, SDL_RENDERER_ACCELERATED);
	// TODO: Check for error creating window or renderer

	pthread_create(&ovw->thread, NULL, ovwindow_action_loop, ovw);

	SDL_Rect rect = {0, 0, X, Y};
	SDL_SetRenderTarget(ovw->renderer, NULL);
	SDL_SetRenderDrawColor(ovw->renderer, 0, 0, 0, 0);
	SDL_RenderFillRect(ovw->renderer, &rect);

	// Add to window list
	ovw_list *temp, *entry = malloc(sizeof(window_list));
	entry->ovw = ovw;
	entry->next = NULL;
	if (window_list == NULL) {
		window_list = entry;
	} else {
		for (temp = window_list; temp->next != NULL; temp = temp->next)
			;
		temp->next = entry;
	}

	ovwindow_update(ovw);
	return ovw;
}

void ovwindow_destroy(ovwindow *ovw) {
	SDL_DestroyRenderer(ovw->renderer);
	SDL_DestroyWindow(ovw->window);
	free(ovw);

	// Remove from the window list
	if (window_list != NULL && ovw == window_list->ovw) {
		window_list = window_list->next;
		return;
	}
	
	ovw_list *temp;
	for (temp = window_list; temp->next != NULL; temp = temp->next)
		if (temp->next->ovw == ovw) {
			if (temp->next != NULL)
				temp->next = temp->next->next;
			
			break;
		}
}

void ovwindow_update(ovwindow *ovw) {
	objectview *ov = ovw->context->ov;
	ov_update(ov);

	int X = ov->width*ov->point_size, Y = ov->height*ov->point_size + action_bar_height;
	ovw->width = X;
	ovw->height = Y;
	SDL_SetWindowSize(ovw->window, X, Y);

	SDL_Rect rect = {0, 0, ov->point_size, ov->point_size};
	SDL_Colour colour = {0, 0, 0, 0xff};
	double val;
	for (uint i = 0; i < ov->width; i++) {
		for (uint j = 0; j < ov->height; j++) {
		        val = ov_get_value(ov, i, j, ov->current_slice);
			ov_lut_colour(ov, val, &colour);

			rect.x = i*ov->point_size;
			rect.y = j*ov->point_size;
			SDL_SetRenderDrawColor(ovw->renderer, colour.r, colour.g, colour.b, 0xff);
			SDL_RenderFillRect(ovw->renderer, &rect);
		}
	}

	SDL_RenderPresent(ovw->renderer);
}
