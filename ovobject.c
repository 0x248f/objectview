#include <stdio.h>

#include "objectview.h"

ovobject *ovobject_create(int x, int y, int z) {
	ovobject *obj = malloc(sizeof(ovobject));
	memset(obj, 0, sizeof(ovobject));

	obj->data = malloc(x*y*z*sizeof(double));
	obj->x = x;
	obj->y = y;
	obj->z = z;
	obj->data_size = x*y*z;
	obj->time_index = 0;

	return obj;
}

void ovobject_save(ovobject *obj, const char *path) {
	FILE *f = fopen(path, "w");
	if (f == NULL)
		return;
	
	ovobject *start;
	for (ovobject *temp = obj; temp != NULL; temp = temp->previous)
		start = temp;

	uint64_t t = 0;
	for (ovobject *temp = start; temp != NULL; temp = temp->next)
		t++;

	fprintf(f, "%lu %lu %lu %lu\n\n", t, obj->x, obj->y, obj->z);
		
	for (ovobject *temp = start; temp != NULL; temp = temp->next)
		for (uint i = 0; i < obj->data_size; i++) {
			fprintf(f, "%lf", temp->data[i]);
			if (i == obj->data_size - 1)
				fprintf(f, "\n");
			else
				fprintf(f, " ");
		}
}

ovobject *ovobject_load(const char *path) {
	FILE *f = fopen(path, "r");
	if (f == NULL)
		return NULL;

	ovobject *obj = malloc(sizeof(ovobject)), *temp = obj;
	memset(obj, 0, sizeof(ovobject));

	uint64_t t;
	fscanf(f, "%lu %lu %lu %lu\n\n", &t, &obj->x, &obj->y, &obj->z);
	obj->data_size = obj->x*obj->y*obj->z;
	obj->data = malloc(obj->data_size*sizeof(double));
	for (uint i = 0; i < t; i++) {
		if (i != t - 1) {
			temp->next = ovobject_copy(obj);
			temp->next->next = NULL;
			temp->next->previous = temp;
		}
		
		for (uint j = 0; j < obj->data_size; j++)
			if (fscanf(f, "%lf", &(temp->data[j])) == -1)
				return NULL;

		if (temp->next != NULL)
			temp = temp->next;
	}

	return obj;
}

ovobject *ovobject_copy(ovobject *obj) {
	ovobject *new_object = malloc(sizeof(ovobject));
	*new_object = *obj;
	new_object->data = malloc(obj->data_size*sizeof(double));
	for (uint i = 0; i < obj->data_size; i++)
		new_object->data[i] = obj->data[i];

	return new_object;
}

void ovobject_destroy(ovobject *obj) {
	free(obj->data);
	free(obj);
}

void ovobject_destroy_all(ovobject *obj) {
	for (ovobject *temp = obj->previous; temp != NULL; temp = temp->previous)
		ovobject_destroy(temp);
	for (ovobject *temp = obj->next; temp != NULL; temp = temp->next)
		ovobject_destroy(temp);

	ovobject_destroy(obj);
}

double ovobject_index(ovobject *obj, uint i, uint j, uint k) {
	return obj->data[i + j*obj->x + k*obj->x*obj->y];
}

double ovobject_index_time(ovobject *obj, uint i, uint j, uint k, uint t) {
	int32_t dt = t - obj->time_index;
	ovobject *temp;

	if (dt == 0)
		return ovobject_index(obj, i, j, k);

	if (dt > 0)
		for (temp = obj; temp->next != NULL; temp = temp->next)
			if (temp->time_index == t)
				return ovobject_index(temp, i, j, k);

	if (dt < 0)
		for (temp = obj; temp->next != NULL; temp = temp->next)
			if (temp->time_index == t)
				return ovobject_index(temp, i, j, k);

	return 1./0.; // This usually results in a runtime warning
}

void ovobject_set(ovobject *obj, double value, uint i, uint j, uint k) {
	obj->data[i + j*obj->x + k*obj->x*obj->y] = value;
}

void ovobject_set_time(ovobject *obj, double value, uint t, uint i, uint j, uint k) {
	int32_t dt = t - obj->time_index;
	ovobject *temp;

	if (dt == 0)
		return ovobject_set(obj, value, i, j, k);

	if (dt > 0)
		for (temp = obj; temp->next != NULL; temp = temp->next)
			if (temp->time_index == t)
				return ovobject_set(temp, value, i, j, k);

	if (dt < 0)
		for (temp = obj; temp->next != NULL; temp = temp->next)
			if (temp->time_index == t)
				return ovobject_set(temp, value, i, j, k);
}

uint64_t ovobject_max_time(ovobject *obj) {
	ovobject *temp;
	for (temp = obj; temp->previous != NULL; temp = temp->previous)
		;

	uint64_t i;
	for (i = 0; temp != NULL; temp = temp->next)
		i++;

	return i;
}
