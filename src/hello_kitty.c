#include <pebble.h>

#if defined(PBL_ROUND)
	#define WINDOW_WIDTH 180
	#define WINDOW_HEIGHT 180
#else
	#define WINDOW_WIDTH 144
	#define WINDOW_HEIGHT 168
#endif

/**/


static Window *window;

static Layer *kitty_head_layer;

static TextLayer *text_time_layer;

static GBitmap *bitmap_kitty_head;


void update_display_time(struct tm *tick_time) {
  // Need to be static because they're used by the system later.
  static char time_text[] = "00:00";
  char *time_format;

  if (clock_is_24h_style()) {
    time_format = "%R";
  } else {
    time_format = "%I:%M";
  }

  strftime(time_text, sizeof(time_text), time_format, tick_time);

  // Kludge to handle lack of non-padded hour format string
  // for twelve hour clock.
  if (!clock_is_24h_style() && (time_text[0] == '0')) {
    memmove(time_text, &time_text[1], sizeof(time_text) - 1);
  }

  text_layer_set_text(text_time_layer, time_text);
}


void handle_minute_tick(struct tm *tick_time, TimeUnits units_changed) {
	update_display_time(tick_time);
}


void kitty_head_layer_update_callback(Layer *layer, GContext* ctx) {
	GSize bitmap_size = gbitmap_get_bounds(bitmap_kitty_head).size;
	graphics_context_set_compositing_mode(ctx, GCompOpSet);
	graphics_draw_bitmap_in_rect(
		ctx,
		bitmap_kitty_head,
		GRect(0, 0, bitmap_size.w, bitmap_size.h)
	);
}


static void window_load(Window *window) {
  Layer *window_layer = window_get_root_layer(window);
  GRect bounds = layer_get_bounds(window_layer);

	text_time_layer = text_layer_create(GRect(0, 15, WINDOW_WIDTH, 60));
	text_layer_set_font(text_time_layer, fonts_get_system_font(FONT_KEY_ROBOTO_BOLD_SUBSET_49));
	text_layer_set_text_color(text_time_layer, GColorBlack);
	text_layer_set_text_alignment(text_time_layer, GTextAlignmentCenter);
	text_layer_set_background_color(text_time_layer, GColorClear);
	layer_set_bounds(text_layer_get_layer(text_time_layer), bounds);
	layer_add_child(window_layer, text_layer_get_layer(text_time_layer));

	bitmap_kitty_head = gbitmap_create_with_resource(RESOURCE_ID_IMAGE_KITTY_HEAD);
	GRect kitty_head_bounds = gbitmap_get_bounds(bitmap_kitty_head);
  kitty_head_layer = layer_create(kitty_head_bounds);
  layer_set_frame(kitty_head_layer, GRect(0, 65, kitty_head_bounds.size.w, kitty_head_bounds.size.h));
  layer_set_update_proc(kitty_head_layer, kitty_head_layer_update_callback);
  layer_add_child(window_layer, kitty_head_layer);
}


static void window_unload(Window *window) {
	text_layer_destroy(text_time_layer);
	layer_destroy(kitty_head_layer);
	gbitmap_destroy(bitmap_kitty_head);
}


static void init(void) {
  window = window_create();
  window_set_window_handlers(window, (WindowHandlers) {
    .load = window_load,
    .unload = window_unload,
  });
  const bool animated = true;
  window_stack_push(window, animated);
	window_set_background_color(window, GColorWhite);

	tick_timer_service_subscribe(MINUTE_UNIT, handle_minute_tick);
}


static void deinit(void) {
	tick_timer_service_unsubscribe();

  window_destroy(window);
}


int main(void) {
  init();

  app_event_loop();

  deinit();
}

