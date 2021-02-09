//
// Created by Howe Robbin on 1/28/21.
//

#ifndef MLX_HEAD_H
# define MLX_HEAD_H

typedef struct s_data
{
	void	*img;
	char 	*addr;
	int     bits_per_pixel;
	int     line_length;
	int     endian;
}				t_data;

typedef struct s_vplane
{
	float width;
	float height;
	float x_pixel;
	float y_pixel;
}				t_vplane;

void 	my_mlx_pixel_put(t_data *data, int x, int y, int colour);
void	show_red_win();
t_vplane	*get_new_plane(float width, float height, float fov);
void	trace_ray(void *mlx, void *window, t_config *config);


#endif //MINIRT_MLX_H
