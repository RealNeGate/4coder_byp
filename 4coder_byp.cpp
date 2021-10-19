#pragma once

#include "4coder_default_include.cpp"

CUSTOM_ID(colors, defcolor_function);
CUSTOM_ID(colors, defcolor_type);
CUSTOM_ID(colors, defcolor_macro);
CUSTOM_ID(colors, defcolor_control);
CUSTOM_ID(colors, defcolor_struct);

#include "4coder_vimrc.h"
#include "4coder_vim\\4coder_vim_include.h"

#include "4coder_byp_helper.h"

#include "4coder_byp_token.h"
#include "4coder_byp_token.cpp"

#include "4coder_byp_build.cpp"
#include "4coder_byp_colors.cpp"
#include "4coder_byp_commands.cpp"

#include "4coder_byp_draw.cpp"
#include "4coder_byp_game.cpp"

#include "4coder_byp_bindings.cpp"
#include "4coder_byp_hooks.cpp"


#if !defined(META_PASS)
#include "generated/managed_id_metadata.cpp"
#endif


void
custom_layer_init(Application_Links *app){
	default_framework_init(app);
	set_all_default_hooks(app);

	vim_buffer_peek_list[ArrayCount(vim_default_peek_list) + 0] = BYP_peek_list[0];
	vim_buffer_peek_list[ArrayCount(vim_default_peek_list) + 1] = BYP_peek_list[1];
	vim_request_vtable[VIM_REQUEST_COUNT + BYP_REQUEST_Title]   = byp_apply_title;
	vim_request_vtable[VIM_REQUEST_COUNT + BYP_REQUEST_Comment] = byp_apply_comment;
	vim_request_vtable[VIM_REQUEST_COUNT + BYP_REQUEST_UnComment] = byp_apply_uncomment;

	vim_text_object_vtable[VIM_TEXT_OBJECT_COUNT + BYP_OBJECT_param] = {',', (Vim_Text_Object_Func *)byp_object_param};
	vim_text_object_vtable[VIM_TEXT_OBJECT_COUNT + BYP_OBJECT_camel] = {'_', (Vim_Text_Object_Func *)byp_object_camel};
	vim_init(app);

	set_custom_hook(app, HookID_ViewEventHandler, vim_view_input_handler);

	set_custom_hook(app, HookID_SaveFile, byp_file_save);
	set_custom_hook(app, HookID_BufferRegion, byp_buffer_region);
	set_custom_hook(app, HookID_RenderCaller, byp_render_caller);
	set_custom_hook(app, HookID_WholeScreenRenderCaller, byp_whole_screen_render_caller);

	set_custom_hook(app, HookID_Tick, byp_tick);
	set_custom_hook(app, HookID_BeginBuffer, vim_begin_buffer);
	set_custom_hook(app, HookID_ViewChangeBuffer, vim_view_change_buffer);

	Thread_Context *tctx = get_thread_context(app);
	mapping_init(tctx, &framework_mapping);
	String_ID global_map_id = vars_save_string_lit("keys_global");
	String_ID file_map_id = vars_save_string_lit("keys_file");
	String_ID code_map_id = vars_save_string_lit("keys_code");
	byp_essential_mapping(&framework_mapping, global_map_id, file_map_id, code_map_id);
	byp_default_bindings(&framework_mapping, global_map_id, file_map_id, code_map_id);

	vim_default_bindings(app, KeyCode_BackwardSlash);
	byp_vim_bindings(app);
}
