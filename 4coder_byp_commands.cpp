
global Face_ID byp_small_italic_face;
global Face_ID byp_minimal_face;

// NOTE(BYP): Don't do this. Do as I say, not as I do
#define DECLARE_TOGGLE(name) \
global b32 byp_##name; \
CUSTOM_COMMAND_SIG(byp_toggle_##name) \
CUSTOM_DOC(stringify(glue(glue(Toggles value for `, name), `))) \
{ byp_##name ^= 1; }

DECLARE_TOGGLE(show_hex_colors);
DECLARE_TOGGLE(relative_numbers);
DECLARE_TOGGLE(show_scrollbars);
DECLARE_TOGGLE(drop_shadow);


CUSTOM_COMMAND_SIG(negate_exec_cli)
CUSTOM_DOC("runs the system command as a CLI and prints the output to the compilation buffer."){
    internal u8 my_hot_directory_space[1024];
	internal u8 my_command_space[1024];

	Scratch_Block scratch(app);
    Query_Bar_Group group(app);

	Query_Bar bar_cmd = {};
    bar_cmd.prompt = string_u8_litexpr("Command: ");
    bar_cmd.string = SCu8(my_command_space, (u64)0);
    bar_cmd.string_capacity = sizeof(command_space);
    if (!query_user_string(app, &bar_cmd)) return;
    bar_cmd.string.size = clamp_top(bar_cmd.string.size, sizeof(command_space) - 1);
    my_command_space[bar_cmd.string.size] = 0;

    String_Const_u8 hot = push_hot_directory(app, scratch);
    {
        u64 size = clamp_top(hot.size, sizeof(hot_directory_space));
        block_copy(my_hot_directory_space, hot.str, size);
        my_hot_directory_space[hot.size] = 0;
    }

	if (bar_cmd.string.size > 0 && hot.size > 0){
        View_ID view = get_active_view(app, Access_Always);
        Buffer_Identifier id = buffer_identifier(SCu8("*compilation*"));
        exec_system_command(app, view, id, hot, SCu8(my_command_space), CLI_OverlapWithConflict|CLI_CursorAtEnd|CLI_SendEndSignal);
        lock_jump_buffer(app, SCu8("*compilation*"));
    }
}

CUSTOM_COMMAND_SIG(byp_test)
CUSTOM_DOC("Just bound to the key I spam to execute whatever test code I'm working on")
{
	Scratch_Block scratch(app);
	View_ID view = get_active_view(app, Access_ReadVisible);

	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
	i64 pos = view_get_cursor_pos(app, view);
	Buffer_Cursor cursor = buffer_compute_cursor(app, buffer, seek_pos(pos));

	print_message(app, push_stringf(scratch, "Buffer[%d] = '%c'\n", pos, buffer_get_char(app, buffer, pos)));
	print_message(app, push_stringf(scratch, "Line %d Col: %d\n", cursor.line, cursor.col));

	Face_ID face = get_face_id(app, buffer);
	Face_Description desc = get_face_description(app, face);
	printf_message(app, scratch, "Face Size: %d \n", desc.parameters.pt_size);
}

CUSTOM_COMMAND_SIG(byp_reset_face_size)
CUSTOM_DOC("Resets face size to default")
{
	View_ID view = get_active_view(app, Access_Always);
	Buffer_ID buffer = view_get_buffer(app, view, Access_Always);
	Face_ID face_id = get_face_id(app, buffer);
	Face_Description description = get_face_description(app, face_id);
	description.parameters.pt_size = (i32)def_get_config_u64(app, vars_save_string_lit("default_font_size"));
	try_modify_face(app, face_id, &description);
}

// TODO: This should probably be scoped per_view and know the buffer it refers to,
// but that's to much of a fuss for something so small
global Buffer_Cursor byp_col_cursor;

CUSTOM_COMMAND_SIG(byp_toggle_set_col_ruler)
CUSTOM_DOC("Toggles the column ruler. Set to cursor column when on.")
{
	View_ID view = get_active_view(app, Access_ReadVisible);
	byp_col_cursor = (byp_col_cursor.pos != 0 ?
        Buffer_Cursor{} :
        view_compute_cursor(app, view, seek_pos(view_get_cursor_pos(app, view))));
}

CUSTOM_COMMAND_SIG(byp_space)
CUSTOM_DOC("When column ruler is set, spaces towards that, else just inserts one space")
{
	Scratch_Block scratch(app);
	if(byp_col_cursor.pos > 0){
		View_ID view = get_active_view(app, Access_ReadVisible);
		Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);

		byp_col_cursor = buffer_compute_cursor(app, buffer, seek_line_col(byp_col_cursor.line, byp_col_cursor.col));

		i64 pos = view_get_cursor_pos(app, view);
		i64 line = get_line_number_from_pos(app, buffer, pos);
		f32 col_x = view_relative_xy_of_pos(app, view, line, byp_col_cursor.pos).x;
		f32 cur_x = view_relative_xy_of_pos(app, view, line, pos).x;
		Face_ID face = get_face_id(app, buffer);
		f32 space_advance = get_face_metrics(app, face).space_advance;

		i64 N = i64((col_x - cur_x) / space_advance);
		if(N < 0){ N = 1; }
		foreach(i,N){ write_space(app); }
	}else{ write_space(app); }
}

CUSTOM_COMMAND_SIG(explorer)
CUSTOM_DOC("Opens file explorer in hot directory")
{
	Scratch_Block scratch(app);
	String_Const_u8 hot = push_hot_directory(app, scratch);
	exec_system_command(app, 0, buffer_identifier(0), hot, string_u8_litexpr("explorer ."), 0);
}

CUSTOM_COMMAND_SIG(byp_write_indent)
CUSTOM_DOC("Inserts tabs or spaces to make one indent.")
{
	Scratch_Block scratch(app);
    b32 indent_with_tabs = def_get_config_b32(vars_save_string_lit("indent_with_tabs"));
    if (indent_with_tabs) {
        write_text(app, string_u8_litexpr("\t"));
    } else {
	    i32 indent_width = (i32)def_get_config_u64(app, vars_save_string_lit("indent_width"));
	    indent_width = clamp_bot(1, indent_width);

        u8* temp = push_array(scratch, u8, indent_width);
        foreach(i,indent_width) temp[i] = ' ';

        String_Const_u8 string = {temp, (u64)indent_width};
        write_text(app, string);
    }
}

function void
byp_load_theme(String_Const_u8 theme_name){
	Color_Table *table_ptr = get_color_table_by_name(theme_name);
	if(table_ptr){ byp_copy_color_table(&target_color_table, *table_ptr); }
}

global Vim_Buffer_Peek_Entry BYP_peek_list[VIM_ADDITIONAL_PEEK] = {
	{ buffer_identifier(string_u8_litexpr("*scratch*")), 1.f, 1.f },
	{ buffer_identifier(string_u8_litexpr("todo.txt")),  1.f, 1.f },
};

CUSTOM_COMMAND_SIG(byp_list_all_locations_selection)
CUSTOM_DOC("Lists locations of selection range")
{
	vim_normal_mode(app);
	View_ID view = get_active_view(app, Access_ReadVisible);
	Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
	Range_i64 range = get_view_range(app, view);
	range.max++;

    Scratch_Block scratch(app);
	String_Const_u8 range_string = push_buffer_range(app, scratch, buffer, range);
	list_all_locations__generic(app, range_string, ListAllLocationsFlag_CaseSensitive|ListAllLocationsFlag_MatchSubstring);
}

CUSTOM_COMMAND_SIG(byp_open_current_peek)
CUSTOM_DOC("Sets the active view to the current peeked buffer")
{
	View_ID view = get_active_view(app, Access_ReadWriteVisible);
	Buffer_ID buffer = buffer_identifier_to_id(app, vim_buffer_peek_list[vim_buffer_peek_index].buffer_id);
	view_set_buffer(app, view, buffer, SetBuffer_KeepOriginalGUI);
	vim_show_buffer_peek = 1;
	vim_toggle_show_buffer_peek(app);
}

VIM_TEXT_OBJECT_SIG(byp_object_param){
	u8 c = buffer_get_char(app, buffer, cursor_pos);
	Range_i64 range = Ii64(cursor_pos + (c == ',' || c == ';'));

	for(; range.min>0; range.min--){
		c = buffer_get_char(app, buffer, range.min);
		if(c == ',' || c == ';'){ break; }
		Scan_Direction bounce = vim_bounce_direction(c);
		if(bounce == Scan_Forward){ break; }
		if(bounce == Scan_Backward){
			range.min = vim_bounce_pair(app, buffer, range.min, c)-1;
			continue;
		}
	}

	i64 buffer_size = buffer_get_size(app, buffer);
	for(; range.max < buffer_size; range.max++){
		c = buffer_get_char(app, buffer, range.max);
		if(c == ',' || c == ';'){ break; }
		Scan_Direction bounce = vim_bounce_direction(c);
		if(bounce == Scan_Backward){ break; }
		if(bounce == Scan_Forward){
			range.max = vim_bounce_pair(app, buffer, range.max, c);
			continue;
		}
	}
	range.min++; range.max--;
	if(range.min >= range.max){ range = {}; }

	return range;
}

VIM_TEXT_OBJECT_SIG(byp_object_camel){
	Range_i64 range = {};
	Scratch_Block scratch(app);
	Range_i64 line_range = get_line_range_from_pos(app, buffer, cursor_pos);
	i64 s = line_range.min;
	u8 *line_text = push_buffer_range(app, scratch, buffer, line_range).str;
	u8 c = line_text[cursor_pos-s];
	if(!character_is_alpha_numeric(c)){ return {}; }
	cursor_pos += line_text[cursor_pos-s] == '_';
	range.min = range.max = cursor_pos;

	b32 valid=false;
	for(; range.min>0; range.min--){
		c = line_text[range.min-s];
		if(!character_is_alpha_numeric(c) || c == '_'){ valid = true; break; }
	}
	if(!valid){ return {}; }

	valid=false;
	for(; range.max>0; range.max++){
		c = line_text[range.max-s];
		if(!character_is_alpha_numeric(c) || c == '_'){ valid = true; break; }
	}
	if(!valid){ return {}; }

	range.min += (vim_state.params.clusivity != VIM_Inclusive || line_text[range.min-s] != '_');
	range.max -= (vim_state.params.clusivity != VIM_Inclusive || line_text[range.max-s] != '_');
	if(range.min >= range.max){ range = {}; }

	return range;
}

function void byp_make_vim_request(Application_Links *app, BYP_Vim_Request request){
	vim_make_request(app, Vim_Request_Type(VIM_REQUEST_COUNT + request));
}

VIM_REQUEST_SIG(byp_apply_title){
	Scratch_Block scratch(app);
	String_Const_u8 text = push_buffer_range(app, scratch, buffer, range);
	u8 prev = buffer_get_char(app, buffer, range.min-1);
	for(i32 i=0; i<text.size; i++){
		text.str[i] += u8(i32('A' - 'a')*((!character_is_alpha(prev) || prev == '_') &&
                character_is_lower(text.str[i])));
		prev = text.str[i];
	}
	buffer_replace_range(app, buffer, range, text);
	buffer_post_fade(app, buffer, 0.667f, range, fcolor_resolve(fcolor_id(defcolor_paste)));
}

VIM_REQUEST_SIG(byp_apply_comment){
	i64 line0 = get_line_number_from_pos(app, buffer, range.min);
	i64 line1 = get_line_number_from_pos(app, buffer, range.max);
	line1 += (line0 == line1);
	History_Group history_group = history_group_begin(app, buffer);
	for(i64 l=line0; l<line1; l++){
		i64 line_start = get_pos_past_lead_whitespace_from_line_number(app, buffer, l);
		b32 has_comment = c_line_comment_starts_at_position(app, buffer, line_start);
		if(!has_comment){
			buffer_replace_range(app, buffer, Ii64(line_start), string_u8_litexpr("//"));
			buffer_post_fade(app, buffer, 0.667f, Ii64_size(line_start,2), fcolor_resolve(fcolor_id(defcolor_paste)));
		}
	}
	history_group_end(history_group);
}

VIM_REQUEST_SIG(byp_apply_uncomment){
	i64 line0 = get_line_number_from_pos(app, buffer, range.min);
	i64 line1 = get_line_number_from_pos(app, buffer, range.max);
	line1 += (line0 == line1);
	History_Group history_group = history_group_begin(app, buffer);
	for(i64 l=line0; l<line1; l++){
		i64 line_start = get_pos_past_lead_whitespace_from_line_number(app, buffer, l);
		b32 has_comment = c_line_comment_starts_at_position(app, buffer, line_start);
		if(has_comment){
			buffer_replace_range(app, buffer, Ii64_size(line_start,2), string_u8_empty);
		}
	}
	history_group_end(history_group);
}

VIM_COMMAND_SIG(byp_request_title){ byp_make_vim_request(app, BYP_REQUEST_Title); }
VIM_COMMAND_SIG(byp_request_comment){ byp_make_vim_request(app, BYP_REQUEST_Comment); }
VIM_COMMAND_SIG(byp_request_uncomment){ byp_make_vim_request(app, BYP_REQUEST_UnComment); }
VIM_COMMAND_SIG(byp_visual_comment){
	if(vim_state.mode == VIM_Visual){
		Vim_Edit_Type edit = vim_state.params.edit_type;
		byp_request_comment(app);
		vim_state.mode = VIM_Visual;
		vim_state.params.edit_type = edit;
	}
}
VIM_COMMAND_SIG(byp_visual_uncomment){
	if(vim_state.mode == VIM_Visual){
		Vim_Edit_Type edit = vim_state.params.edit_type;
		byp_request_uncomment(app);
		vim_state.mode = VIM_Visual;
		vim_state.params.edit_type = edit;
	}
}

