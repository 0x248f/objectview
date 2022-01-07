#include "objectview.h"

char *play_button_label(ovcontext *ctx) {
	if (ctx->ov->object->next == NULL &&
	    ctx->ov->object->previous != NULL &&
	    ctx->step_function == NULL)
		return "rewind";

	if (ctx->running)
		return "pause";
	else
		return "play";
}

void playf(ovbutton *b) {
	bool toggle = true;

	objectview *ov = b->parent->context->ov;
	if (ov->object->next == NULL &&
	    ov->object->previous != NULL &&
	    b->parent->context->step_function == NULL) { // rewind
		toggle = false;
		while (ov->object->previous != NULL)
			ovcontext_step_back(b->parent->context); // TODO: Return bool?
	}

	if (toggle == true)
		ovcontext_toggle_running(b->parent->context);

	b->label = play_button_label(b->parent->context);
	ovwindow_update(b->parent);
}

char *pov_button_label(objectview *ov) {
	switch (ov->view_type) {
	case OV_VIEW_XY:
	        return "xy";
	case OV_VIEW_XZ:
	        return "xz";
	case OV_VIEW_YZ:
	        return "yz";
	default:
		return "?";
	}
}

void povf(ovbutton *b) {
	objectview *ov = b->parent->context->ov;
	ov->view_type = (ov->view_type + 1)%3;
	b->label = pov_button_label(ov);
	ovwindow_update(b->parent);
}

void stepf(ovbutton *b) {
	ovcontext_step(b->parent->context);
	ovwindow_update(b->parent);
}

void stepbackf(ovbutton *b) {
	ovcontext_step_back(b->parent->context);
	ovwindow_update(b->parent);
}

void sliceupf(ovbutton *b) {
	ov_slice_up(b->parent->context->ov);
	ovwindow_update(b->parent);
}

void slicedownf(ovbutton *b) {
	ov_slice_down(b->parent->context->ov);
	ovwindow_update(b->parent);
}

ovbar *ovbar_create(ovwindow *parent, uint64_t height) {
	ovbar *bar = malloc(sizeof(ovbar));
	bar->parent = parent;
	bar->height = height;

	bar->play_button = bar->buttons[0] = ovbutton_create(parent, "play", playf);
	bar->pov_button = bar->buttons[1] =
		ovbutton_create(parent, pov_button_label(parent->context->ov), povf);
	bar->step_button = bar->buttons[2] = ovbutton_create(parent, "step", stepf);
	bar->step_back_button = bar->buttons[3] = ovbutton_create(parent, "step back", stepbackf);
	bar->slice_up_button = bar->buttons[4] = ovbutton_create(parent, "in", sliceupf);
	bar->slice_down_button = bar->buttons[5] = ovbutton_create(parent, "out", slicedownf);

	ovbar_update(bar);

	return bar;
}

void ovbar_destroy(ovbar *bar) {
	for (uint i = 0; i < sizeof(bar->buttons)/8; i++)
		ovbutton_destroy(bar->buttons[i]);

	free(bar);
}

void ovbar_update(ovbar *bar) {
	uint32_t x_offset = 0, y_offset = 0.24*bar->height;
	bar->play_button->label = play_button_label(bar->parent->context);

	objectview *ov = bar->parent->context->ov;
	bar->x = 0;
	bar->y = ov->height*ov->point_size;
	bar->width = ov->width*ov->point_size;

	for (uint i = 0; i < 6; i++) {
		x_offset += 10;
		bar->buttons[i]->x = bar->x + x_offset;
		bar->buttons[i]->y = bar->y + y_offset;
		bar->buttons[i]->width = 100;
		bar->buttons[i]->height = 0.4*bar->height;
		x_offset += bar->buttons[i]->width;
	}

}

void ovbar_draw(ovbar *bar) {
	ovbar_update(bar);

	pthread_mutex_lock(&bar->parent->mutex);

	SDL_Rect rect = {bar->x, bar->y, bar->width, bar->height};
	SDL_SetRenderDrawColor(bar->parent->renderer, 0x00, 0x00, 0x00, 0xff);
	SDL_RenderFillRect(bar->parent->renderer, &rect);

	for (uint i = 0; i < sizeof(bar->buttons)/8; i++)
		ovbutton_draw(bar->buttons[i]);

	SDL_RenderPresent(bar->parent->renderer);

	pthread_mutex_unlock(&bar->parent->mutex);
}

void ovbar_process_event(ovbar *bar, SDL_Event *e) {
	for (uint i = 0; i < sizeof(bar->buttons)/8; i++)
		ovbutton_process_event(bar->buttons[i], e);

	ovbar_draw(bar);
}
