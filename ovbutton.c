#include "objectview.h"

ovbutton *ovbutton_create(ovwindow *parent, const char *label, ovbutton_function function) {
	ovbutton *button = malloc(sizeof(ovbutton));
	memset(button, 0, sizeof(ovbutton));
	button->parent = parent;
	button->label = strdup(label);
	button->function = function;
	return button;
}

void ovbutton_destroy(ovbutton *button) {
	free(button);
}

void ovbutton_draw(ovbutton *button) {
	// printf("%s %d %d %d %d\n", button->label, button->x, button->y, button->width, button->height);
	SDL_Rect rect = {button->x, button->y, button->width, button->height};
	SDL_SetRenderDrawColor(button->parent->renderer, 0xff, 0xff, 0xff, 0xff);
	SDL_RenderFillRect(button->parent->renderer, &rect);
}

void ovbutton_process_event(ovbutton *button, SDL_Event *event) {
	if (event->type != SDL_MOUSEBUTTONDOWN)
		return;

	SDL_MouseButtonEvent *be = &event->button;
	if ((be->x >= button->x && be->x <= button->x + button->width) &&
	    (be->y >= button->y && be->y <= button->y + button->height)) {
		pthread_mutex_lock(&button->parent->mutex);
		button->function(button);
		pthread_mutex_unlock(&button->parent->mutex);
	}
}
