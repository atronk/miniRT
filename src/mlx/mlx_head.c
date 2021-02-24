//
// Created by Howe Robbin on 1/28/21.
//

#include "header.h"

int   key_hook(int keycode, t_info *info)
{
	if (keycode == ESC)
	{
		mlx_destroy_window(info->mlx, info->win);
		exit(0);
	}
	printf("key: %d\n", keycode);
	return (0);
}

void start_show(t_info *info)
{
	t_scene *scene;

	scene = info->scene;
	info->mlx = mlx_init();
	info->win = mlx_new_window(info->mlx, scene->width, scene->height, "miniRT");
	mlx_key_hook(info->win, key_hook, info);
	trace_ray(info->scene, info->mlx, info->win);
	mlx_loop(info->mlx);
}

void intersect_sp(t_ray *ray, t_object *obj, t_roots *roots)
{
	t_vector sp_to_ray;
	float b;
	float c;
	float discriminant;
	float a;

	sp_to_ray = vector_sub(ray->orig, obj->origin_coord);
	a = dot_prod(ray->dir, ray->dir);
	b = 2 * dot_prod(ray->dir, sp_to_ray);
	c = dot_prod(sp_to_ray, sp_to_ray) - (obj->sp_radius * obj->sp_radius);
	discriminant = (b * b) - (4 * a * c);
	if (discriminant >= 0)
	{
		roots->t1 = (-b + sqrtf(discriminant)) / 2;
		roots->t2 = (-b - sqrtf(discriminant)) / 2;
	}
	else
	{
		roots->t1 = INFINITY;
		roots->t2 = INFINITY;
	}
}

t_vector get_sp_plane(float width, float height, t_camera *cam)
{
	t_vector new;
	float vp_width;
	float vp_height;
	float aspect_ratio;

	aspect_ratio = width / height;
	vp_width = tan(M_PI * 0.5 * cam->fov / 180.) * 2;
	vp_height = vp_width / aspect_ratio;
	new.x = vp_width / width;
	new.y = vp_height / height;
	new.z = -1;
	return (new);
}

t_vector normal(t_vector hit_pos, t_object *sphere)
{
	t_vector new;

	new = vector_sub(hit_pos, sphere->origin_coord);
	normalize(&new);
	return (new);
}

//int	colour_at(t_object *sphere, t_vector pos_hit, t_vector normal, t_scene *scene)
//{
//	t_vector to_cam;
//	t_vector colour = {0, 0, 0};
//	t_ray to_light;
//
//	//t_vector half_vector;
//	to_cam = vector_sub(((t_camera *)(scene->camera->content))->origin_coord, pos_hit);
//	colour = rgb_mult_n(sphere->rgb, scene->ambl.ratio); // еще цвет амбиент
//	to_light = set_ray(pos_hit, vector_sub(((t_light *)(scene->light->content))->origin_coord, pos_hit));
//	float i = K_D * ((t_light *)(scene->light->content))->ratio * maxx(dot_prod(normal, to_light.dir), 0);
//	colour = rgb_add(colour, rgb_mult_n(((t_light *)(scene->light->content))->rgb, i));
//	//colour += ctohex(sphere->rgb) * new.diff * maxx(dot_prod(normal, to_light.dir), 0);
//	//t_vector temp0 = v_mult_scal(sphere->rgb, maxx(dot_prod(normal, to_light.dir), 0));
//	//colour = rgb_add(colour, temp0);
//	//half_vector = norm_vector(vector_add(to_light.dir, to_cam));
//	//colour += ctohex(((t_light *)(scene->light->content))->rgb) * new.spec * maxx(dot_prod(normal, half_vector), 0) * 50;
//	//t_vector temp = v_mult_scal(((t_light *)(scene->light->content))->rgb, maxx(dot_prod(normal, half_vector), 0));
//	//colour = rgb_add(colour, temp);
//	return (ctohex(colour));
//}

void	cl_inter(t_closest *cl, t_ray *ray, t_list *start, t_range *range)
{
	t_roots		roots;

	intersect_sp(ray, start->content, &roots);
	if (roots.t1 >= range->t_min && roots.t1 < range->t_max && roots.t1 < cl->closest_t)
	{
		cl->closest_t = roots.t1;
		cl->closest_obj = start->content;
	}
	if (roots.t2 >= range->t_min && roots.t2 < range->t_max && roots.t2 < cl->closest_t)
	{
		cl->closest_t = roots.t2;
		cl->closest_obj = start->content;
	}
}

float calc_lighting(t_scene *scene, t_hit *hit, t_ray *ray)
{
	float		i;
	t_list		*light;
	t_vector	to_lgt;
	t_vector	refl;
	t_closest	cl_sh;
	t_list		*obj;
	t_range		range;
	t_ray 		to_sh;
	float		n_dot_l;
	float 		r_dot_v;
	float		temp;


	light = scene->light;
	range.t_min = 0.001;
	range.t_max = 1;
	cl_sh.closest_obj = NULL;
	cl_sh.closest_t = INFINITY;
	obj = scene->object;
	i = scene->ambl.ratio;
	while (light)
	{
		to_lgt = vector_sub(((t_light *)(light->content))->origin_coord, hit->hit_pos);
		to_sh = set_ray(hit->hit_pos, to_lgt);
		cl_inter(&cl_sh, &to_sh, obj, &range);
		if (cl_sh.closest_obj == NULL)
		{
			n_dot_l = dot_prod(hit->hit_normal, to_lgt);
			if (n_dot_l > 0)
			{
				i += ((t_light *)(light->content))->ratio * n_dot_l /
					 (vector_length(to_lgt) * vector_length(hit->hit_normal));
			}
			//	R = 2N <N,L> - L
			//  R - reflection vector, N - normal, L - to_lgt
			//  Is = Il * (dot(R,V) / |R|*|V|)^s
			//  Is - i, Il - light ratio, V - negative ray.direction
			refl = v_mult_scal(hit->hit_normal, 2);
			refl = v_mult_scal(refl, n_dot_l);
			refl = vector_sub(refl, to_lgt);
			r_dot_v = dot_prod(refl, v_mult_scal(ray->dir, -1));
			if (r_dot_v > 0)
			{
				temp = vector_length(refl) * vector_length(v_mult_scal(ray->dir, -1));
				temp = r_dot_v / temp;
				i += ((t_light *)(light->content))->ratio *	powf(temp, 10);
			}
		}
		light = light->next;
	}
	return (i);
}

void get_hit_sp(t_hit *hit, t_closest *closest, t_ray *ray)
{
	hit->hit_dist = closest->closest_t;
	hit->hit_pos = vector_add(ray->orig, v_mult_scal(ray->dir, hit->hit_dist));
	hit->hit_normal = normal(hit->hit_pos, closest->closest_obj);
}
/*
void get_hit_sp(t_hit *hit, t_object *sphere, float *closest, t_ray *ray)
{
	hit->hit_dist = *closest;
	hit->hit_pos = vector_add(ray->orig, v_mult_scal(ray->dir, hit->hit_dist));
	hit->hit_normal = normal(hit->hit_pos, sphere);
}*/



int calc_colour(t_scene *scene, t_vector *plane, float vp_x, float vp_y)
{
	int			colour;
	float		light;
	t_ray		ray;
	t_list		*start;
	t_hit		new_hit;
	t_closest 	cl;
	t_range 	range;

	cl.closest_t = INFINITY;
	cl.closest_obj = NULL;
	start = scene->object;
	ray = set_ray(((t_camera *)(scene->camera->content))->origin_coord,
				  set_vector(plane->x * vp_x, plane->y * vp_y, -1));
	range.t_max = INFINITY;
	range.t_min = 1;
	while (start)
	{
		if (0 == ft_strncmp(((t_object *)(start->content))->type, "sp", 2))
		{
			cl_inter(&cl, &ray, start, &range);
		}
		start = start->next;
	}
	if (cl.closest_obj == NULL)
		colour = BGC;
	else
	{
		get_hit_sp(&new_hit, &cl, &ray);
		light = calc_lighting(scene, &new_hit, &ray);
		colour = ctohex(rgb_mult_n(cl.closest_obj->rgb, light));
	}
	return (colour);
}
/* //working
int calc_colour(t_scene *scene, t_vector *plane, float vp_x, float vp_y)
{
	int			colour;
	float		light;
	t_ray		ray;
	t_list		*start;
	t_object	*closest_obj;
	float		closest_t;
	t_roots		roots;
	t_hit		new_hit;


	start = scene->object;
	closest_obj = NULL;
	closest_t = INFINITY;
	ray = set_ray(((t_camera *)(scene->camera->content))->origin_coord,
			   set_vector(plane->x * vp_x, plane->y * vp_y, -1));
	while (start)
	{
		if (0 == ft_strncmp(((t_object *)(start->content))->type, "sp", 2))
		{
			intersect_sp(&ray, start->content, &roots);
			if (roots.t1 >= 1 && roots.t1 < INFINITY && roots.t1 < closest_t)
			{
				closest_t = roots.t1;
				closest_obj = start->content;
			}
			if (roots.t2 >= 1 && roots.t2 < INFINITY && roots.t2 < closest_t)
			{
				closest_t = roots.t2;
				closest_obj = start->content;
			}
		}
		start = start->next;
	}
	if (closest_obj == NULL)
		colour = 0xDC143C;
	else
	{
		get_hit_sp(&new_hit, closest_obj, closest_t, &ray);
		light = calc_lighting(scene, &new_hit);
		colour = ctohex(rgb_mult_n(closest_obj->rgb, light));
	}
	return (colour);
}*/

void trace_ray(t_scene *scene, void *mlx, void *win)
{
	int			canvas_x;
	int			canvas_y;
	float		vp_x;
	float		vp_y;
	int			colour;
	t_vector	plane;

	plane = get_sp_plane(scene->width, scene->height, scene->camera->content);
	canvas_y = 0;
	vp_y = scene->height / 2;
	while (vp_y >= scene->height / 2 * (-1))
	{
		vp_x = scene->width / 2 * (-1);
		canvas_x = 0;
		while (vp_x <= scene->width / 2)
		{
			colour = calc_colour(scene, &plane, vp_x, vp_y);
			mlx_pixel_put(mlx, win, canvas_x, canvas_y, colour);
			vp_x++;
			canvas_x++;
		}
		vp_y--;
		canvas_y++;
	}
}