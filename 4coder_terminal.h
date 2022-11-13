// This is a simple single-header extension to your 4coder layer to display terminal output
// with correct escape sequences
#ifndef TERMINAL_4ED_H
#define TERMINAL_4ED_H

void terminal_render_buffer(
    Application_Links *app, View_ID view_id, Face_ID face_id,
    Buffer_ID buffer, Text_Layout_ID text_layout_id, Rect_f32 rect
);

#endif /* TERMINAL_4ED_H */

#ifdef TERMINAL_4ED_IMPL

/*function Layout_Item_List terminal__layout(Application_Links *app, Arena *arena, Buffer_ID buffer, Range_i64 range, Face_ID face, f32 width){
	Range_i64 range = list.input_index_range;
	f32 line_height = get_face_metrics(app, get_face_id(app, buffer)).line_height;
	i64 base_line_num = get_line_number_from_pos(app, buffer, range.min);

	return fold_items(app, buffer, layout_generic(app, arena, buffer, range, face, width));
}*/

void terminal_render_buffer(
    Application_Links *app, View_ID view_id, Face_ID face_id,
    Buffer_ID buffer, Text_Layout_ID text_layout_id, Rect_f32 rect
) {
	/*Layout_Function* old_layout = buffer_get_layout(app, buffer);
	if (old_layout != terminal__layout) {
		// initialize our new
		buffer_set_layout(app, buffer, terminal__layout);
		print_message(app, string_u8_litexpr("Setting new terminal layout!\n"));
	}*/

	Scratch_Block scratch(app);
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);

	i64 region_start = 0;
	ARGB_Color region_color = 0xFFFFFFFF;

	u8 *memory = push_array(scratch, u8, range_size(visible_range));
	if (buffer_read_range(app, buffer, visible_range, memory)) {
		i64 endpoint = visible_range.max - 5;

		for (i64 i = visible_range.first; i < endpoint; i++) {
			// look for ANSI sequences
			if (memory[i] == 0x1b && memory[i+1] == '[') {
				i64 j = 0;
				i64 num = 0;

				// parse number
				for (; j < 2; j++) {
					char ch = memory[i+j+2];

					if (ch == 'm') break;
					if (ch < '0' || ch > '9') break;

					num *= 10;
					num += ch;
				}

				if (memory[i+j+2] == 'm') {
					i = j + 2;

					ARGB_Color new_color;
					switch (num) {
						case 30: new_color = 0xFF000000; break;
						case 31: new_color = 0xFFFF0000; break;
						case 32: new_color = 0xFF00FF00; break;
						case 33: new_color = 0xFFFFFF00; break;
						case 34: new_color = 0xFF0000FF; break;
						default: new_color = 0xFFFF00FF; break;
					}

					// highlight last region
					paint_text_color(app, text_layout_id, Ii64(region_start, j), region_color);

					// setup next region
					region_start = i + 1;
					region_color = new_color;
				}
			}
		}

		if (region_start != visible_range.max) {
			paint_text_color(app, text_layout_id, Ii64(region_start, visible_range.max), region_color);
		}
	}
}

#endif /* TERMINAL_4ED_IMPL */
