/*
	NOTE(Douglas): To be included (once) for different OSs platform layers.
*/

#define DEAD_CELL_COLOR (0xff2b2d42)
#define ALIVE_CELL_COLOR (0xffedf2f4)

#define CELL_ALIVE (1)
#define CELL_DEAD  (0)

internal inline b32 *
get_state_cell(pixel_buffer *buffer,
               simulation_state *state,
               s32 x,
               s32 y)
{
	b32 *cell;

	cell = (b32 *) ((u8 *) state->cells_state + (y * buffer->line_stride) + (x * buffer->bytes_per_pixel));
	return cell;
}

internal inline b32 *
get_output_cell(pixel_buffer *buffer,
                simulation_state *state,
                s32 x,
                s32 y)
{
	b32 *cell;

	cell = (b32 *) ((u8 *) state->grid_output + (y * buffer->line_stride) + (x * buffer->bytes_per_pixel));
	return cell;
}


internal inline void
set_cell_state(simulation_state *state,
               u32 cell_index,
               b32 value)
{
	*((b32 *) ((u8 *) state->cells_state + cell_index)) = value;
}

internal inline b32
get_output_cell_value(pixel_buffer *buffer,
                      simulation_state *state,
                      s32 x,
                      s32 y)
{
	b32 *cell;

	cell = (b32 *) ((u8 *) state->grid_output + (y * buffer->line_stride) + (x * buffer->bytes_per_pixel));
	return *cell;
}

#if 0
internal inline void
copy_cell_to_output(pixel_buffer *buffer,
                    simulation_state *state,
                    b32 cell,
                    s32 cell_x,
                    s32 cell_y)
{
	u32 *output_cell;
	u32 cell_color;

	output_cell = (u32 *) ((u8 *) state->grid_output + (cell_y * buffer->line_stride) + (cell_x * buffer->bytes_per_pixel));

	if(cell == CELL_ALIVE)
	{
		cell_color = ALIVE_CELL_COLOR;
	}
	else
	{
		cell_color = DEAD_CELL_COLOR;
	}

	*output_cell = cell_color;
}
#endif

internal inline void
draw_cell(pixel_buffer *buffer,
          simulation_state *state,
          s32 x,
          s32 y,
          u32 color)
{
	u32 *pixel;

	pixel = (u32 *) ((u8 *) buffer->memory + (y * buffer->line_stride) + (x * buffer->bytes_per_pixel));
	*pixel = color;
}

#if 1
internal void
set_cell_pattern(pixel_buffer *buffer,
                 simulation_state *state,
                 s32 cell_x,
                 s32 cell_y,
                 char *cells_pattern,
                 size_t str_size)
{
	s32 x, y;
	s32 count;
	u32 cell_index;

	assert( (cell_x + (s32)str_size) < buffer->width);

	y = cell_y;
	count = 0;
	for(x = cell_x;
	    x < cell_x + (s32)str_size;
	    ++x, ++count)
	{
		if(cells_pattern[count] == '\0')
		{
			break;
		}

		cell_index = (y * buffer->line_stride) + (x * buffer->bytes_per_pixel);

		if(cells_pattern[count] == '#')
		{
			set_cell_state(state, cell_index, CELL_ALIVE); /* 1 */
		}
		else
		{
			set_cell_state(state, cell_index, CELL_DEAD); /* 0 */
		}
	}
}
#endif

internal void
simulation_update_and_render(pixel_buffer *buffer,
                             simulation_state *state)
{
	if(!state->initialized)
	{
		s32 x, y;
		u32 cell_index;
		char cells_pattern[100];
		size_t cells_pattern_size;

		state->initialized = TRUE;
		cells_pattern_size = sizeof(cells_pattern);

		/* default cell values (DEAD) */
		for(y = 0;
		    y < buffer->height;
		    ++y)
		{
			for(x = 0;
			    x < buffer->width;
			    ++x)
			{
				cell_index = (y * buffer->line_stride) + (x * buffer->bytes_per_pixel);
				set_cell_state(state, cell_index, CELL_DEAD); /* 0 */
			}
		}

		/* random cells */
		#if 1
		for(y = 0;
		    y < buffer->height;
		    ++y)
		{
			for(x = 0;
			    x < buffer->width;
			    ++x)
			{
				cell_index = (y * buffer->line_stride) + (x * buffer->bytes_per_pixel);

				if(random() > 0.5)
				{
					set_cell_state(state, cell_index, CELL_ALIVE); /* 1 */
					/*copy_cell_to_output(buffer, state, CELL_ALIVE, x, y);*/
				}
				else
				{
					set_cell_state(state, cell_index, CELL_DEAD); /* 0 */
					/*copy_cell_to_output(buffer, state, CELL_DEAD, x, y);*/
				}
			}
		}
		#endif

		/* TODO: Replace sprintf with something better */

		/* infinite growth pattern */
		#if 0
		sprintf_s(cells_pattern, cells_pattern_size, "########.#####...###......#######.#####");
		set_cell_pattern(buffer, state, 400, 335, cells_pattern, cells_pattern_size);
		#endif
	}

	/*
	 * NOTE: Celular automata algorithm
	*/
	{
		s32 i;
		s32 x, y;
		s32 neighbor_cells_amount;
		b32 *output_cell;
		b32 *state_cell;

		for(i = 0;
		    i < (buffer->width * buffer->height);
		    ++i)
		{
			state_cell = (b32 *)state->cells_state + i;

			if(*state_cell == CELL_ALIVE)
			{
				*((u32 *) state->grid_output + i) = CELL_ALIVE;
			}
			else
			{
				*((u32 *) state->grid_output + i) = CELL_DEAD;
			}
		}

		neighbor_cells_amount = 0;

		/* borders are ignored when updating cells state */
		for(y = 1;
		    y < buffer->height - 1;
		    ++y)
		{
			for(x = 1;
			    x < buffer->width - 1;
			    ++x)
			{
				output_cell = get_state_cell(buffer, state, x, y);
				state_cell = get_state_cell(buffer, state, x, y);

				neighbor_cells_amount =
					(get_output_cell_value(buffer, state, x-1, y-1) + get_output_cell_value(buffer, state, x-0, y-1)   + get_output_cell_value(buffer, state, x+1, y-1)) + 
					(get_output_cell_value(buffer, state, x-1, y+0) +                        0                         + get_output_cell_value(buffer, state, x+1, y+0)) +
					(get_output_cell_value(buffer, state, x-1, y+1) + get_output_cell_value(buffer, state, x-0, y+1)   + get_output_cell_value(buffer, state, x+1, y+1));

				if(*output_cell == CELL_ALIVE)
				{
					*state_cell = (neighbor_cells_amount == 2 || neighbor_cells_amount == 3);
				}
				else
				{
					*state_cell = (neighbor_cells_amount == 3);
				}

				if(*output_cell == CELL_ALIVE)
				{
					draw_cell(buffer, state, x, y, ALIVE_CELL_COLOR);
				}
				else
				{
					draw_cell(buffer, state, x, y, DEAD_CELL_COLOR);
				}
			}
		}
	}


}
