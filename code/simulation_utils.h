/*
 * NOTE: Psudo-Random number between 0.0 ... 1.0 | max: 32767
*/
global_variable u32 global_random_number_seed;

internal void
set_random_number_seed(u32 value)
{
  global_random_number_seed = value;
}

internal inline f32
random()
{
  f32 result = 0.0f;

  global_random_number_seed = (214013 * global_random_number_seed + 2531011);
  result = (f32)((global_random_number_seed >> 16) & 0x7FFF);
  result /= 32767.0f;

  return result;
}
