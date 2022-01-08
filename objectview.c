#include "objectview.h"

objectview *ov_create(ovobject *obj) {
	objectview *ov = malloc(sizeof(objectview));
	memset(ov, 0, sizeof(objectview));

	ov->object = obj;
	ov->t_max = ovobject_max_time(obj);
	ov_update(ov);
	return ov;
}

void ov_destroy(objectview *ov) {
	free(ov);
}

void ov_slice_up(objectview *ov) {
	if (ov->current_slice != ov->depth - 1)
		ov->current_slice += 1;
}

void ov_slice_down(objectview *ov) {
	if (ov->current_slice != 0)
		ov->current_slice -= 1;
}

void ov_lut_colour(objectview *ov, double val, SDL_Colour *colour) {
	uint8_t r, g, b;
	uint32_t rgb;
	double ratio = (val-ov->min_value)/(ov->max_value-ov->min_value);

	switch (ov->lut_type) {
	default:
	case OV_LUT_GREY_SCALE:
		r = g = b = 0xff*ratio;
		break;
	case OV_LUT_RGB_SCALE:
		rgb = 0xffffff*ratio;
		r = (rgb >> 16)&0xff;
		g = (rgb >> 8)&0xff;
		b = rgb&0xff;
		break;
	}

	colour->r = r;
	colour->g = g;
	colour->b = b;
}

double ov_get_value(objectview *ov, int i, int j, int k) {
	double val;
	ovobject *obj = ov->object;

	switch (ov->view_type) {
	default:
	case OV_VIEW_XY:
		val = ovobject_index(obj, i, j, k);
		break;
	case OV_VIEW_XZ:
		val = ovobject_index(obj, i, k, j);
		break;
	case OV_VIEW_YZ:
		val = ovobject_index(obj, k, i, j);
		break;
	}

	return val;
}

void ov_update(objectview *ov) {
	ovobject *obj = ov->object;
	double max = 0, min = 0, val;
	int height, width, depth, k = ov->current_slice;

	if (obj == NULL)
		return;

	switch (ov->view_type) {
	default:
	case OV_VIEW_XY:
		width = obj->x;
		height = obj->y;
		depth = obj->z;
		break;
	case OV_VIEW_XZ:
		width = obj->x;
		height = obj->z;
		depth = obj->y;
		break;
	case OV_VIEW_YZ:
		width = obj->y;
		height = obj->z;
		depth = obj->x;
		break;
	}

	ov->width = width;
	ov->height = height;
	ov->depth = depth;

	for (int i = 0; i < width; i++) {
		for (int j = 0; j < height; j++) {
		        val = ov_get_value(ov, i, j, k);

			if (val > max)
				max = val;
			if ((i == 0 && j == 0) || val < min)
				min = val;
		}
	}

	ov->max_value = max;
	ov->min_value = min;
}
