
// TODO(BYP): param number underlining (account for variadic ...)
function void
byp_draw_function_preview_inner(Application_Links *app, Buffer_ID buffer, Range_f32 x_range, Range_i64 range, i64 pos, i32 count){
	Scratch_Block scratch(app);
	String_Const_u8 query = push_token_or_word_under_pos(app, scratch, buffer, range.min-1);

	Buffer_ID target_buffer = -1;
	Range_i64 target_range = Ii64(-1);

	code_index_lock();
	for(Buffer_ID b = get_buffer_next(app, 0, Access_Always);
        b != 0;
        b = get_buffer_next(app, b, Access_Always))
	{
		Code_Index_File *file = code_index_get_file(b);
		if(file != 0){
			foreach(i, file->note_array.count){
				Code_Index_Note *note = file->note_array.ptrs[i];
				if((note->note_kind == CodeIndexNote_Function || note->note_kind == CodeIndexNote_Macro) &&
                    string_match(note->text, query))
				{
					target_buffer = b;
					target_range = note->pos;
					goto done;
				}
			}
		}
	}
	done:
	code_index_unlock();

	if(target_buffer < 0){ return; }

	//i64 p = target_range.max;
	target_range.max = vim_scan_bounce(app, target_buffer, target_range.max, Scan_Forward) + 1;

	/*
	i32 cur_count = 0;
	Range_i64 cur_range = Ii64(range.min+1);
	for(;;){
	   if(cur_range.max >= pos || cur_range.max >= range.max){ break; }
	   cur_count++;
	   cur_range = byp_object_param(app, buffer, cur_range.max+1);
	   if(cur_range.min == 0 && cur_range.max == 0){ break; }
	}

	i32 sig_count = 0;
	Range_i64 sig_range = Ii64(p);
	for(;;){
	   if(sig_count >= cur_count){ break; }
	   sig_count++;
	   sig_range = byp_object_param(app, target_buffer, sig_range.max+1);
	   if(sig_range.min == 0 && sig_range.max == 0){ break; }
	}
	sig_range.max++;
	Range_i64 sig_underline = sig_range;
	//*/
	Range_i64 sig_underline = target_range;
	///find_surrounding_nest;
	//token_array_from_text;

	String_Const_u8 sig = push_buffer_range(app, scratch, target_buffer, target_range);
	sig = string_condense_whitespace(scratch, sig);

	Face_Metrics metrics = get_face_metrics(app, byp_small_italic_face);
	f32 wid = 2.f;
	f32 pad = 2.f;
	Rect_f32 rect = {};
	rect.p0 = vim_cur_cursor_pos + V2f32(0, 2.f + count*(metrics.line_height + pad + 2.f*wid));
	rect.p1 = rect.p0 + V2f32(sig.size*metrics.max_advance, metrics.line_height);
	f32 x_offset = ((rect.x1 > x_range.max)*(rect.x1 - x_range.max) -
        (rect.x0 < x_range.min)*(rect.x0 - x_range.max));
	rect.x0 -= x_offset;
	rect.x1 -= x_offset;

	Rect_f32 underline = rect;
	underline.x0 += metrics.max_advance*(sig_underline.min - target_range.min);
	underline.x1 = underline.x0 + metrics.max_advance*(range_size(sig_underline));
	underline.y1 -= 1.f;
	underline.y0 = underline.y1 - 1.f;

	rect = rect_inner(rect, -pad);
	draw_rectangle_fcolor(app, rect, 3.f, fcolor_id(defcolor_back));
	rect.x0 -= pad;
	rect.x1 += pad;
	draw_rectangle_outline_fcolor(app, rect, 3.f, wid, fcolor_id(defcolor_ghost_character));
	//draw_rectangle_outline_fcolor(app, underline, 3.f, wid, fcolor_id(defcolor_ghost_character));
	draw_string(app, byp_small_italic_face, sig, rect.p0 + (pad + wid)*V2f32(1,1), fcolor_id(defcolor_ghost_character));
}

function void
byp_draw_function_preview(Application_Links *app, Buffer_ID buffer, Range_f32 x_range, i64 pos){
	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	if(token_array.tokens == 0){ return; }
	Token_Iterator_Array it = token_iterator_pos(0, &token_array, pos);
	Token *token = token_it_read(&it);

	if(token->kind == TokenBaseKind_ParentheticalOpen){ pos = token->pos + token->size; }
	else if(token_it_dec_all(&it)){
		token = token_it_read(&it);
		if(token->kind == TokenBaseKind_ParentheticalClose && pos == token->pos + token->size){
			pos = token->pos;
		}
	}

	Scratch_Block scratch(app);
	Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, FindNest_Paren);
	i32 count = 0;
	foreach(i, ranges.count){
		Token_Iterator_Array cur_it = token_iterator_pos(0, &token_array, ranges.ranges[i].min-1);
		token = token_it_read(&cur_it);
		if(token->kind == TokenBaseKind_Identifier){
			byp_draw_function_preview_inner(app, buffer, x_range, ranges.ranges[i], pos, count++);
		}
	}
}

function void
byp_hex_color_preview(Application_Links *app, Buffer_ID buffer_id, Text_Layout_ID text_layout_id){
	Scratch_Block scratch(app);
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
	u8 *text = push_buffer_range(app, scratch, buffer_id, visible_range).str;
	foreach(i, range_size(visible_range)-9){
		byp_hex_scan:
		if(text[i] != '0' || text[i+1] != 'x'){ continue; }
		foreach(j, 8){
			char c = text[i+2+j];
			if(!('0' <= c && c <= '9' || 'a' <= c && c <= 'f' || 'A' <= c && c <= 'F')){
				i+=9; goto byp_hex_scan;
			}
		}
		Range_i64 hex_range = Ii64_size(i+visible_range.min, 10);
		u32 c = u32(string_to_integer(SCu8(text+i+2, 8), 16));
		f32 avg = (((c >> 16) & 0xFF) + ((c >> 8) & 0xFF) + (c & 0xFF))/3.f;

		u32 contrast = 0xFF000000 | (i32(avg > 110.f)-1);
		paint_text_color(app, text_layout_id, hex_range, contrast);
		Rect_f32 rect_left  = text_layout_character_on_screen(app, text_layout_id, hex_range.min);
		Rect_f32 rect_right = text_layout_character_on_screen(app, text_layout_id, hex_range.max-1);
		Rect_f32 rect = rect_union(rect_left, rect_right);
		rect = rect_inner(rect, -1.f);
		draw_rectangle(app, rect, 8.f, c);
		draw_rectangle_outline(app, rect_inner(rect, -2.f), 8.f, 2.1f, contrast);
	}
}

function Rect_f32_Pair
byp_layout_scrollbars(Rect_f32 region, f32 digit_advance){ return rect_split_left_right_neg(region, 1.25f*digit_advance); }

function void
byp_draw_scrollbars(Application_Links *app, View_ID view, Buffer_ID buffer, Text_Layout_ID text_layout_id, Rect_f32 region){
	Rect_f32 prev_clip = draw_set_clip(app, region);
	f32 region_height = rect_height(region);
	u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
	f32 roundness = rect_width(region)*cursor_roundness_100*0.0075f;

	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);
	i64 line_count = buffer_get_line_count(app, buffer);
	f32 line_diff_y = clamp(5.f, region_height*(1.f/line_count), 10.f);

	i64 top_line    = get_line_number_from_pos(app, buffer, visible_range.min)-1;
	i64 bot_line    = get_line_number_from_pos(app, buffer, visible_range.max)-1;
	i64 cursor_line = get_line_number_from_pos(app, buffer, view_get_cursor_pos(app, view))-1;
	i64 mark_line   = get_line_number_from_pos(app, buffer, view_get_mark_pos(app, view))-1;

	draw_rectangle_fcolor(app, region, 0.f, fcolor_id(defcolor_highlight_cursor_line));
	{
		f32 top_y = region_height*(f32(top_line)/(line_count));
		f32 bot_y = region_height*(f32(bot_line)/(line_count));
		Rect_f32 rect = Rf32(region.x0, top_y, region.x1, bot_y);
		draw_rectangle_fcolor(app, rect, roundness, fcolor_id(defcolor_line_numbers_text));
	}

	{
		f32 top_y = region_height*(f32(cursor_line)/(line_count));
		f32 bot_y = top_y + 3.f*line_diff_y;
		Rect_f32 rect = Rf32(region.x0, top_y, region.x1, bot_y);
		draw_rectangle_fcolor(app, rect, roundness, fcolor_id(defcolor_cursor));
	}

	{
		f32 top_y = region_height*(f32(mark_line)/(line_count));
		f32 bot_y = top_y + 3.f*line_diff_y;
		Rect_f32 rect = Rf32(region.x0, top_y, region.x1, bot_y);
		draw_rectangle_fcolor(app, rect, roundness, fcolor_id(defcolor_mark));
	}
	draw_set_clip(app, prev_clip);
}

function void
draw_scope_range(Application_Links *app, View_ID view, Buffer_ID buffer, Text_Layout_ID text_layout_id, Range_i64 range, f32 x_off, f32 wid, FColor color){
	range.max -= 1;
	// exclude ifdef scopes
	u8 c0 = buffer_get_char(app, buffer, range.min);
	u8 c1 = buffer_get_char(app, buffer, range.max);
	if((c0 != '{' && c0 != '(') || (c1 != '}' && c1 != ')')){ return; }
	paint_text_color_pos(app, text_layout_id, range.min, color);
	paint_text_color_pos(app, text_layout_id, range.max, color);

	i64 line0 = get_line_number_from_pos(app, buffer, range.min);
	i64 line1 = get_line_number_from_pos(app, buffer, range.max);
	if(line0 == line1){ return; }

	Rect_f32 line_rect = vim_get_abs_block_rect(app, view, buffer, text_layout_id, range);
	line_rect.p0 -= V2f32(3.f, 4.f);
	Rect_f32 region = line_rect;
	region.x1 = region.x0 + x_off;

	Rect_f32 prev_clip = draw_set_clip(app, region);
	draw_set_clip(app, rect_intersect(region, prev_clip));

	draw_rectangle_outline_fcolor(app, line_rect, 1.5f*wid, wid, color);

	draw_set_clip(app, prev_clip);
}

function void
byp_draw_scope_brackets(Application_Links *app, View_ID view, Buffer_ID buffer, Text_Layout_ID text_layout_id, Rect_f32 region, i64 pos){

	Token_Array token_array = get_token_array_from_buffer(app, buffer);
	if(token_array.tokens == 0){ return; }
	{
		Token_Iterator_Array it = token_iterator_pos(0, &token_array, pos);
		Token *token = token_it_read(&it);

		if(token->kind == TokenBaseKind_ScopeOpen){ pos = token->pos + token->size; }
		else if(token_it_dec_all(&it)){
			token = token_it_read(&it);
			if(token->kind == TokenBaseKind_ScopeClose && pos == token->pos + token->size){
				pos = token->pos;
			}
		}
	}

	Scratch_Block scratch(app);
	Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, FindNest_Scope);
	//Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, FindNest_Scope|FindNest_Paren);
	//Range_i64_Array ranges = get_enclosure_ranges(app, scratch, buffer, pos, RangeHighlightKind_CharacterHighlight);
	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);

	region.x0 -= 3.f;
	Rect_f32 prev_clip = draw_set_clip(app, region);

	f32 wid = 2.f;
	f32 x_off = rect_width(text_layout_character_on_screen(app, text_layout_id, pos)) + 1.f;
	draw_scope_range(app, view, buffer, text_layout_id, ranges.ranges[0], x_off, wid, fcolor_id(defcolor_preproc));
	for(i32 i=1; i<ranges.count; i++){
		Range_i64 range = ranges.ranges[i];
		draw_scope_range(app, view, buffer, text_layout_id, range, x_off, wid, fcolor_id(defcolor_ghost_character));
	}
	draw_set_clip(app, prev_clip);


	/// Basically just fleury end brace annotations
	for(i32 i=ranges.count-1; i >= 0; --i){
		Range_i64 range = ranges.ranges[i];

		i64 start_line_num = get_line_number_from_pos(app, buffer, range.start);
		i64 end_line_num   = get_line_number_from_pos(app, buffer, range.end);
		if(start_line_num == end_line_num || range.end >= visible_range.end){ continue; }

		Token *start_token = 0;
		Token_Iterator_Array it = token_iterator_pos(0, &token_array, range.start-1);
		i32 token_count=0, paren_level=0;

		for(;;){
			Token *token = token_it_read(&it);
			if(token == 0){ break; }
			++token_count;

			if(0){}
			else if(token->kind == TokenBaseKind_ParentheticalClose){ ++paren_level; }
			else if(token->kind == TokenBaseKind_ParentheticalOpen){  --paren_level; }

			if(paren_level == 0){
				if(token->kind == TokenBaseKind_ScopeClose ||
                    (token->kind == TokenBaseKind_StatementClose && token->sub_kind != TokenCppKind_Colon))
				{
					break;
				}
				else if(token->kind == TokenBaseKind_Identifier ||
                    token->kind == TokenBaseKind_Keyword ||
                    token->kind == TokenBaseKind_Comment ||
                    token->kind == byp_TokenKind_ControlFlow)
				{
					start_token = token;
					break;
				}
			}

			if(!token_it_dec_non_whitespace(&it)){ break; }
		}

		if(start_token){
			start_line_num = get_line_number_from_pos(app, buffer, start_token->pos);
			String_Const_u8 start_line_text = push_buffer_line(app, scratch, buffer, start_line_num);

			u64 whitespace_count = 0;
			foreach(j,start_line_text.size){
				if(start_line_text.str[j] > 32){ break; }
				++whitespace_count;
			}
			start_line_text.str  += whitespace_count;
			start_line_text.size -= whitespace_count;
			start_line_text.size -= (start_line_text.str[start_line_text.size - 1] == 13);

			i64 last_char = get_line_end_pos(app, buffer, end_line_num)-1;
			Rect_f32 last_char_rect = text_layout_character_on_screen(app, text_layout_id, last_char);
			Vec2_f32 string_pos = V2f32(last_char_rect.x1, last_char_rect.y0);
			string_pos += V2f32(2, 4);

			draw_string(app, byp_small_italic_face, start_line_text, string_pos, fcolor_id(defcolor_ghost_character));
		}
	}
}


#if 1
function void
byp_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id,
    Buffer_ID buffer, Text_Layout_ID text_layout_id,
    Rect_f32 rect)
{
    ProfileScope(app, "render buffer");

    View_ID active_view = get_active_view(app, Access_Always);
    b32 is_active_view = (active_view == view_id);
    Rect_f32 prev_clip = draw_set_clip(app, rect);

    Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);

    // NOTE(allen): Cursor shape
    Face_Metrics metrics = get_face_metrics(app, face_id);
    u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
    f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
    f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));

    // NOTE(allen): Token colorizing
    Token_Array token_array = get_token_array_from_buffer(app, buffer);

    Managed_Scope scope = buffer_get_managed_scope(app, buffer);
    JPTS_Data*tree_data = scope_attachment(app, scope, ts_data, JPTS_Data);
    TSTree *tree = jpts_buffer_get_tree(tree_data);

    // Paint the default color for the entire ranage, specific colors will be overwritten by the tree query
    paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
    if (tree && token_array.tokens != 0){
        if (use_ts_highlighting) {
            jpts_draw_node_colors(app, text_layout_id, buffer);
        } else {
            draw_cpp_token_colors(app, text_layout_id, &token_array);
        }

        // NOTE(allen): Scan for TODOs and NOTEs
        b32 use_comment_keyword = def_get_config_b32(vars_save_string_lit("use_comment_keywords"));
        if (use_comment_keyword){
            ProfileScope(app, "draw_comment_highlights");
            Comment_Highlight_Pair pairs[] = {
                {string_u8_litexpr("NOTE"), finalize_color(defcolor_comment_pop, 0)},
                {string_u8_litexpr("TODO"), finalize_color(defcolor_comment_pop, 1)},
            };
            draw_comment_highlights(app, buffer, text_layout_id, &token_array, pairs, ArrayCount(pairs));
        }
    }

    i64 cursor_pos = view_correct_cursor(app, view_id);
    view_correct_mark(app, view_id);

    // NOTE(allen): Scope highlight
    b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
    if (use_scope_highlight){
        Color_Array colors = finalize_color_array(defcolor_back_cycle);
        draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }

    b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
    b32 use_jump_highlight = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
    if (use_error_highlight || use_jump_highlight){
        // NOTE(allen): Error highlight
        String_Const_u8 name = string_u8_litexpr("*compilation*");
        Buffer_ID compilation_buffer = get_buffer_by_name(app, name, Access_Always);
        if (use_error_highlight){
            draw_jump_highlights(app, buffer, text_layout_id, compilation_buffer,
                fcolor_id(defcolor_highlight_junk));
        }

        // NOTE(allen): Search highlight
        if (use_jump_highlight){
            Buffer_ID jump_buffer = get_locked_jump_buffer(app);
            if (jump_buffer != compilation_buffer){
                draw_jump_highlights(app, buffer, text_layout_id, jump_buffer,
                    fcolor_id(defcolor_highlight_white));
            }
        }
    }

    // NOTE(allen): Color parens
    b32 use_paren_helper = def_get_config_b32(vars_save_string_lit("use_paren_helper"));
    if (use_paren_helper){
        Color_Array colors = finalize_color_array(defcolor_text_cycle);
        draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
    }

    // NOTE(allen): Line highlight
    b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
    if (highlight_line_at_cursor && is_active_view){
        i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
        draw_line_highlight(app, text_layout_id, line_number, fcolor_id(defcolor_highlight_cursor_line));
    }

	byp_draw_scope_brackets(app, view_id, buffer, text_layout_id, rect, cursor_pos);

    // NOTE(allen): Whitespace highlight
    b64 show_whitespace = false;
    view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
    if (show_whitespace){
        if (token_array.tokens == 0){
            draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
        }
        else{
            draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
        }
    }

	if(is_active_view && vim_state.mode == VIM_Visual){
		vim_draw_visual_mode(app, view_id, buffer, face_id, text_layout_id);
	}

	// fold_draw(app, buffer, text_layout_id);

	vim_draw_search_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);

    // NOTE(allen): Cursor
    switch (fcoder_mode){
        case FCoderMode_Original:
        {
		    vim_draw_cursor(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
            // draw_original_4coder_style_cursor_mark_highlight(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
        }break;
        case FCoderMode_NotepadLike:
        {
            draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);
        }break;
    }

    // NOTE(allen): Fade ranges
    paint_fade_ranges(app, text_layout_id, buffer);

    // NOTE(allen): put the actual text on the actual screen
    {
        ProfileScope(app, "draw_text_layout_default");
        draw_text_layout_default(app, text_layout_id);
    }

	vim_draw_after_text(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
	if(is_active_view){
		byp_draw_function_preview(app, buffer, If32(rect.x0, rect.x1), cursor_pos);
	}

    ts_tree_delete(tree);
    draw_set_clip(app, prev_clip);
}
#else
function void
byp_render_buffer(Application_Links *app, View_ID view_id, Face_ID face_id, Buffer_ID buffer, Text_Layout_ID text_layout_id, Rect_f32 rect){
	ProfileScope(app, "render buffer");
	b32 is_active_view = view_id == get_active_view(app, Access_Always);
	Rect_f32 prev_clip = draw_set_clip(app, rect);

	Range_i64 visible_range = text_layout_get_visible_range(app, text_layout_id);

	Face_Metrics metrics = get_face_metrics(app, face_id);
	u64 cursor_roundness_100 = def_get_config_u64(app, vars_save_string_lit("cursor_roundness"));
	f32 cursor_roundness = metrics.normal_advance*cursor_roundness_100*0.01f;
	f32 mark_thickness = (f32)def_get_config_u64(app, vars_save_string_lit("mark_thickness"));

	i64 cursor_pos = view_correct_cursor(app, view_id);
	view_correct_mark(app, view_id);

	b32 highlight_line_at_cursor = def_get_config_b32(vars_save_string_lit("highlight_line_at_cursor"));
	if(highlight_line_at_cursor && is_active_view){
		i64 line_number = get_line_number_from_pos(app, buffer, cursor_pos);
		draw_line_highlight(app, text_layout_id, line_number, fcolor_id(defcolor_highlight_cursor_line));
	}

	if(is_active_view && byp_col_cursor.pos > 0){
		byp_col_cursor = buffer_compute_cursor(app, buffer, seek_line_col(byp_col_cursor.line, byp_col_cursor.col));

		i64 rel_pos = cursor_pos;
		Rect_f32 rel_rect = text_layout_character_on_screen(app, text_layout_id, rel_pos);

		i64 line = get_line_number_from_pos(app, buffer, rel_pos);
		Vec2_f32 col_rel = view_relative_xy_of_pos(app, view_id, line, byp_col_cursor.pos);
		Vec2_f32 rel_point = view_relative_xy_of_pos(app, view_id, line, rel_pos);
		Rect_f32 col_highlight_rect = {};
		col_highlight_rect.p0 = (rel_rect.p0 - rel_point) + col_rel;
		col_highlight_rect.p1 = (rel_rect.p1 - rel_point) + col_rel;
		col_highlight_rect.y0 = rect.y0;
		col_highlight_rect.y1 = rect.y1;

		draw_rectangle_fcolor(app, col_highlight_rect, 0.f, fcolor_id(defcolor_highlight_cursor_line));
	}

	Token_Array token_array = {};
	token_array = get_token_array_from_buffer(app, buffer);
	if(token_array.tokens == 0){
		paint_text_color_fcolor(app, text_layout_id, visible_range, fcolor_id(defcolor_text_default));
	}else{
		byp_draw_token_colors(app, view_id, buffer, text_layout_id);
	}

	/*if (buffer == comp_buffer) {
		terminal_render_buffer(app, view_id, face_id, buffer, text_layout_id, rect);
	}*/

	b32 use_scope_highlight = def_get_config_b32(vars_save_string_lit("use_scope_highlight"));
	if(use_scope_highlight){
		Color_Array colors = finalize_color_array(defcolor_back_cycle);
		draw_scope_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
	}

	b32 use_error_highlight = def_get_config_b32(vars_save_string_lit("use_error_highlight"));
	b32 use_jump_highlight  = def_get_config_b32(vars_save_string_lit("use_jump_highlight"));
	if(use_error_highlight || use_jump_highlight){
		Buffer_ID comp_buffer = get_buffer_by_name(app, string_u8_litexpr("*compilation*"), Access_Always);

        if(use_error_highlight){
			draw_jump_highlights(app, buffer, text_layout_id, comp_buffer, fcolor_id(defcolor_highlight_junk));
			// TODO(BYP): Draw error messsage annotations
		}

		if(use_jump_highlight){
			Buffer_ID jump_buffer = get_locked_jump_buffer(app);
			if(jump_buffer != comp_buffer){
				draw_jump_highlights(app, buffer, text_layout_id, jump_buffer, fcolor_id(defcolor_highlight_white));
			}
		}
	}

	b32 use_paren_helper = def_get_config_b32(vars_save_string_lit("use_paren_helper"));
	if(use_paren_helper){
		Color_Array colors = finalize_color_array(defcolor_text_cycle);
		draw_paren_highlight(app, buffer, text_layout_id, cursor_pos, colors.vals, colors.count);
	}

	byp_draw_scope_brackets(app, view_id, buffer, text_layout_id, rect, cursor_pos);

	b64 show_whitespace = false;
	view_get_setting(app, view_id, ViewSetting_ShowWhitespace, &show_whitespace);
	if(show_whitespace){
		if(token_array.tokens == 0){
			draw_whitespace_highlight(app, buffer, text_layout_id, cursor_roundness);
		}else{
			draw_whitespace_highlight(app, text_layout_id, &token_array, cursor_roundness);
		}
	}

	/*if(byp_show_hex_colors){
		byp_hex_color_preview(app, buffer, text_layout_id);
	}*/

	if(is_active_view && vim_state.mode == VIM_Visual){
		vim_draw_visual_mode(app, view_id, buffer, face_id, text_layout_id);
	}

	// fold_draw(app, buffer, text_layout_id);

	vim_draw_search_highlight(app, view_id, buffer, text_layout_id, cursor_roundness);

	switch(fcoder_mode){
		case FCoderMode_Original:
		vim_draw_cursor(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness); break;
		case FCoderMode_NotepadLike:
		draw_notepad_style_cursor_highlight(app, view_id, buffer, text_layout_id, cursor_roundness); break;
	}


	paint_fade_ranges(app, text_layout_id, buffer);
	draw_text_layout_default(app, text_layout_id);

	vim_draw_after_text(app, view_id, is_active_view, buffer, text_layout_id, cursor_roundness, mark_thickness);
	if(is_active_view){
		byp_draw_function_preview(app, buffer, If32(rect.x0, rect.x1), cursor_pos);
	}

	draw_set_clip(app, prev_clip);
}
#endif
