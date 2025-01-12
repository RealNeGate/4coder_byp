
CUSTOM_COMMAND_SIG(byp_startup)
CUSTOM_DOC("Responding to a startup event")
{
	ProfileScope(app, "byp startup");
	User_Input input = get_current_input(app);
	if(match_core_code(&input, CoreCode_Startup)){
		String_Const_u8_Array file_names = input.event.core.file_names;
		load_themes_default_folder(app);
		default_4coder_initialize(app, file_names);

		/*
        Buffer_ID buffer = create_buffer(app, string_u8_litexpr("*peek*"),
            BufferCreate_NeverAttachToFile|BufferCreate_AlwaysNew);
            buffer_set_setting(app, buffer, BufferSetting_Unimportant, true);
				*/

		Buffer_Identifier left = buffer_identifier(string_u8_litexpr("*scratch*"));
		if(file_names.count > 0){
			left = buffer_identifier(file_names.vals[0]);
		}
		Buffer_ID left_id = buffer_identifier_to_id(app, left);

		// Bottom panel
		View_ID view = get_active_view(app, Access_Always);
		new_view_settings(app, view);

		// Left Panel
		View_ID left_view = get_active_view(app, Access_Always);
		view_set_buffer(app, view, left_id, 0);

		// Restore Active to Left
		view_set_active(app, left_view);
		vim_set_file_register(app, left_id);

		// NOTE/TODO: (BYP) This is a hack until buffer_peek has it's own View_ID (allows comp jump list)
		View_Context ctx = view_current_context(app, build_footer_panel_view_id);
		view_set_split_pixel_size(app, build_footer_panel_view_id, 0);
		ctx.render_caller = 0;
		view_alter_context(app, build_footer_panel_view_id, &ctx);

		if(def_get_config_b32(vars_save_string_lit("automatically_load_project"))){
			load_project(app);
		}

	}

	def_audio_init();

	def_enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
	clear_all_layouts(app);


	Face_Description desc = get_global_face_description(app);
	desc.parameters.pt_size -= 2;
	desc.parameters.bold = 1;
	desc.parameters.italic = 0;
	desc.parameters.hinting = 0;
	byp_small_italic_face = try_create_new_face(app, &desc);

	desc.parameters.pt_size = 0;
	desc.parameters.bold = 0;
	byp_minimal_face = try_create_new_face(app, &desc);

	system_set_fullscreen(false);
	set_window_title(app, string_u8_litexpr("4coder BYP but Cooler"));

	byp_relative_numbers = 1;
	byp_show_hex_colors = 1;
	byp_show_scrollbars = 0;

	Color_Table *table = get_color_table_by_name(string_u8_litexpr("theme-negate"));
	if(table == 0){ table = &default_color_table; }
	target_color_table = byp_init_color_table(app);
	cached_color_table = byp_init_color_table(app);
	byp_copy_color_table(&target_color_table, *table);
	byp_copy_color_table(&cached_color_table, *table);
	active_color_table = cached_color_table;
}

BUFFER_HOOK_SIG(byp_begin_buffer){
	default_begin_buffer(app, buffer_id);
	// fold_begin_buffer_hook(app, buffer_id);
    // ts_BeginBuffer(app, buffer_id);
	vim_begin_buffer_inner(app, buffer_id);
	return 0;
}

BUFFER_EDIT_RANGE_SIG(byp_buffer_edit_range){
	default_buffer_edit_range(app, buffer_id, new_range, old_cursor_range);
    vim_buffer_edit_range(app, buffer_id, new_range, old_cursor_range);
	// fold_buffer_edit_range_inner(app, buffer_id, new_range, old_cursor_range);
	return 0;
}

function void
byp_tick(Application_Links *app, Frame_Info frame_info){
    View_ID view = get_active_view(app, Access_ReadVisible);
    wb_4c_tick(app, view);

	if(tick_all_fade_ranges(app, frame_info.animation_dt)){
		animate_in_n_milliseconds(app, 0);
	}

	vim_animate_filebar(app, frame_info);
	vim_animate_cursor(app, frame_info);
	// fold_tick(app, frame_info);
	byp_tick_colors(app, frame_info);

	vim_cursor_blink++;

	b32 enable_virtual_whitespace = def_get_config_b32(vars_save_string_lit("enable_virtual_whitespace"));
	if(enable_virtual_whitespace != def_enable_virtual_whitespace){
		def_enable_virtual_whitespace = enable_virtual_whitespace;
		clear_all_layouts(app);
	}
}


function void
byp_render_caller(Application_Links *app, Frame_Info frame_info, View_ID view_id){
	ProfileScope(app, "default render caller");
	Scratch_Block scratch(app);

	Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);

	Face_ID face_id = get_face_id(app, 0);
	Face_Metrics face_metrics = get_face_metrics(app, face_id);
	f32 line_height = face_metrics.line_height;
	f32 digit_advance = face_metrics.decimal_digit_advance;

	Rect_f32 region = view_get_screen_rect(app, view_id);
	Rect_f32 prev_clip = draw_set_clip(app, region);

	Rect_f32 global_rect = global_get_screen_rectangle(app);
	f32 filebar_y = global_rect.y1 - 1.f*line_height - vim_cur_filebar_offset;
	if(region.y1 >= filebar_y){ region.y1 = filebar_y; }

	draw_rectangle_fcolor(app, region, 0.f, fcolor_id(defcolor_back));

	region = vim_draw_query_bars(app, region, view_id, face_id);

	{
		Rect_f32_Pair pair = layout_file_bar_on_bot(region, line_height);
		pair.b = rect_split_top_bottom(pair.b, line_height).a;
		vim_draw_filebar(app, view_id, buffer, frame_info, face_id, pair.b);
		region = pair.a;
	}

	Rect_f32_Pair scrollbar_pair = byp_layout_scrollbars(region, digit_advance);
	i64 show_scrollbar = false;
	view_get_setting(app, view_id, ViewSetting_ShowScrollbar, &show_scrollbar);
	show_scrollbar &= byp_show_scrollbars;
	if(show_scrollbar){
		region = scrollbar_pair.a;
	}
	draw_set_clip(app, region);

	// Draw borders
	if(region.x0 > global_rect.x0){
		Rect_f32_Pair border_pair = rect_split_left_right(region, 2.f);
		draw_rectangle_fcolor(app, border_pair.a, 0.f, fcolor_id(defcolor_margin));
		region = border_pair.b;
	}
	if(region.x1 < global_rect.x1){
		Rect_f32_Pair border_pair = rect_split_left_right_neg(region, 2.f);
		draw_rectangle_fcolor(app, border_pair.b, 0.f, fcolor_id(defcolor_margin));
		region = border_pair.a;
	}
	region.y0 += 3.f;


	if(show_fps_hud){
		Rect_f32_Pair pair = layout_fps_hud_on_bottom(region, line_height);
		draw_fps_hud(app, frame_info, face_id, pair.max);
		region = pair.min;
		animate_in_n_milliseconds(app, 1000);
	}

	// NOTE(allen): layout line numbers
	b32 show_line_number_margins = def_get_config_b32(vars_save_string_lit("show_line_number_margins"));
	Rect_f32_Pair pair = (show_line_number_margins ?
        (byp_relative_numbers ?
            vim_line_number_margin(app, buffer, region, digit_advance) :
            layout_line_number_margin(app, buffer, region, digit_advance)) :
        rect_split_left_right(region, 1.5f*digit_advance));
	Rect_f32 line_number_rect = pair.min;
	region = pair.max;

	Buffer_Scroll scroll = view_get_buffer_scroll(app, view_id);
	Buffer_Point_Delta_Result delta = delta_apply(app, view_id, frame_info.animation_dt, scroll);
	if(!block_match_struct(&scroll.position, &delta.point)){
		block_copy_struct(&scroll.position, &delta.point);
		view_set_buffer_scroll(app, view_id, scroll, SetBufferScroll_NoCursorChange);
	}
	if(delta.still_animating){ animate_in_n_milliseconds(app, 0); }
	Buffer_Point buffer_point = scroll.position;
	Text_Layout_ID text_layout_id = text_layout_create(app, buffer, region, buffer_point);

	if(show_line_number_margins){
		if(byp_relative_numbers)
			vim_draw_rel_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
		else
			vim_draw_line_number_margin(app, view_id, buffer, face_id, text_layout_id, line_number_rect);
	}else{
		draw_rectangle_fcolor(app, line_number_rect, 0.f, fcolor_id(defcolor_back));
	}

	if(show_scrollbar){
		byp_draw_scrollbars(app, view_id, buffer, text_layout_id, scrollbar_pair.b);
	}

	if(byp_drop_shadow){
		Buffer_Point shadow_point = buffer_point;
		Face_Description desc = get_face_description(app, face_id);
		shadow_point.pixel_shift -= Max((f32(desc.parameters.pt_size) / 8), 1.f)*V2f32(1.f, 1.f);
		Text_Layout_ID shadow_layout_id = text_layout_create(app, buffer, region, shadow_point);
		paint_text_color(app, shadow_layout_id, text_layout_get_visible_range(app, text_layout_id), 0xBB000000);
		draw_text_layout_default(app, shadow_layout_id);
		text_layout_free(app, shadow_layout_id);
	}

    // jpts_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);
	byp_render_buffer(app, view_id, face_id, buffer, text_layout_id, region);

	/// NOTE(BYP): If 4coder gets even smaller fonts (smaller than u32=0) this *might* be viable
	if(false){
		Buffer_Point point = {};
		Rect_f32 prev_region = rect_split_left_right_neg(region, 190.f).b;
		prev_region.x1 -= 10.f;
		Face_ID prev_face = get_face_id(app, buffer);
		buffer_set_face(app, buffer, byp_minimal_face);
		Text_Layout_ID layout_id = text_layout_create(app, buffer, prev_region, point);

		draw_rectangle(app, rect_inner(prev_region, -10.f), 5.f, 0x44000000);

		i32 prev_mode = fcoder_mode;
		fcoder_mode = 20; // Just don't render/update the cursor
		byp_render_buffer(app, view_id, byp_minimal_face, buffer, layout_id, prev_region);
		fcoder_mode = prev_mode;

		buffer_set_face(app, buffer, prev_face);
		text_layout_free(app, layout_id);
	}


	text_layout_free(app, text_layout_id);
	draw_set_clip(app, prev_clip);
}

function Rect_f32
byp_buffer_region(Application_Links *app, View_ID view_id, Rect_f32 region){

	Buffer_ID buffer = view_get_buffer(app, view_id, Access_Always);
	Face_ID face_id = get_face_id(app, 0);
	Face_Metrics metrics = get_face_metrics(app, face_id);
	f32 line_height = metrics.line_height;
	f32 digit_advance = metrics.decimal_digit_advance;

	Rect_f32 global_rect = global_get_screen_rectangle(app);
	f32 filebar_y = global_rect.y1 - 1.f*line_height - vim_cur_filebar_offset;
	if(region.y1 >= filebar_y){
		region.y1 = filebar_y;
	}

	Query_Bar *space[32];
	Query_Bar_Ptr_Array query_bars = {};
	query_bars.ptrs = space;
	if(get_active_query_bars(app, view_id, ArrayCount(space), &query_bars)){
		region = layout_query_bar_on_bot(region, line_height, query_bars.count).min;
	}

	region = layout_file_bar_on_bot(region, line_height).min;

	i64 show_scrollbar = false;
	view_get_setting(app, view_id, ViewSetting_ShowScrollbar, &show_scrollbar);
	if(byp_show_scrollbars && show_scrollbar){
		region = byp_layout_scrollbars(region, digit_advance).a;
	}

	if(region.x0 > global_rect.x0){
		region = rect_split_left_right(region, 2.f).b;
	}
	if(region.x1 < global_rect.x1){
		region = rect_split_left_right_neg(region, 2.f).a;
	}
	region.y0 += 3.f;

	if(show_fps_hud){
		region = layout_fps_hud_on_bottom(region, line_height).min;
	}

	b32 show_line_number_margins = def_get_config_b32(vars_save_string_lit("show_line_number_margins"));
	region = (show_line_number_margins ?
        (byp_relative_numbers ?
            vim_line_number_margin(app, buffer, region, digit_advance) :
            layout_line_number_margin(app, buffer, region, digit_advance)) :
        rect_split_left_right(region, 1.5f*digit_advance)).max;

	return region;
}

function void
byp_whole_screen_render_caller(Application_Links *app, Frame_Info frame_info){
	vim_draw_whole_screen(app, frame_info);
	if(byp_game_is_running){ byp_game_state.render_game_byp(app, frame_info, get_face_id(app, 0)); }
}

BUFFER_HOOK_SIG(byp_file_save){
	default_file_save(app, buffer_id);
	vim_file_save(app, buffer_id);

    if (byp_auto_indent) {
        auto_indent_buffer(app, buffer_id, buffer_range(app, buffer_id));
    }

    clean_all_lines_buffer(app, buffer_id, CleanAllLinesMode_RemoveBlankLines);

	Scratch_Block scratch(app);
	String_Const_u8 unique_name = push_buffer_unique_name(app, scratch, buffer_id);
	String_Const_u8 postfix = string_u8_litexpr(".4coder");

	if(string_match(string_postfix(unique_name, postfix.size), postfix)){
		String_Const_u8 theme_name = string_chop(unique_name, postfix.size);
		for(Color_Table_Node *node = global_theme_list.first; node; node=node->next){
			if(string_match(node->name, theme_name)){
				load_theme_current_buffer(app);
				break;
			}
		}
	}
	return 0;
}

BUFFER_HOOK_SIG(byp_new_file){
	Scratch_Block scratch(app);
	String_Const_u8 file_name = push_buffer_base_name(app, scratch, buffer_id);
	if(string_match(string_postfix(file_name, 4), string_u8_litexpr(".bat"))){
		Buffer_Insertion insert = begin_buffer_insertion_at_buffered(app, buffer_id, 0, scratch, KB(16));
		insertf(&insert, "@echo off" "\n");
		end_buffer_insertion(&insert);
		return 0;
	}

	return 0;
}
