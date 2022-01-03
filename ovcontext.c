#include <sys/param.h>

#include "objectview.h"

ovcontext *ovcontext_create(const char *name) {
	ovcontext *ctx = malloc(sizeof(ovcontext));
	memset(ctx, 0, sizeof(ovcontext));

	strncpy(ctx->name, name, MIN(strlen(name), sizeof(ctx->name)));

	return ctx;
}

void ovcontext_destroy(ovcontext *ctx) {
	free(ctx);
}

void ovcontext_toggle_running(ovcontext *ctx) {
	ctx->running = !ctx->running;
}

void ovcontext_step(ovcontext *ctx) {
	objectview *ov = ctx->ov;
	ovobject *obj = ov->object;
	if (obj->next == NULL) {
		if (ctx->step_function != NULL) {
			obj->next = ovobject_copy(obj);
			obj->next->previous = obj;
			obj->next->time_index = obj->time_index + 1;
			ctx->step_function(obj);
			ctx->ov->t_max++;
		}
	} else {
		ov->object = ov->object->next;
	}
}

void ovcontext_step_back(ovcontext *ctx) {
	objectview *ov = ctx->ov;
	if (ov->object->previous != NULL)
		ov->object = ov->object->previous;
}
