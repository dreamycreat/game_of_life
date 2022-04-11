internal void
simulation_update_and_render(pixel_buffer *buffer)
{
	/* NOTE: (@CLEANUP) Teste code | Clear screen */
	{
		s32 x, y;
		u32 *pixel;

		pixel = (u32 *) buffer->memory;

		for(y = buffer->height - 1;
		    y >= 0;
		    --y)
		{
			for(x = 0;
			    x < buffer->width;
			    ++x)
			{
				*pixel++ = 0xffffffff;
			}
		}
	}
}
