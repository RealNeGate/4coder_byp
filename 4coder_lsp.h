#ifndef LSP_4ED_H
#define LSP_4ED_H

static void lsp_open_buffer(Application_Links *app, View_ID view_id, Buffer_ID buffer, String_Const_u8 full_file_name);

#endif /* LSP_4ED_H */

#ifdef LSP_4ED_IMPL

static Child_Process_ID clangd_process;

static void lsp_open_buffer(Application_Links *app, View_ID view_id, Buffer_ID buffer, String_Const_u8 file_name) {
    String_Const_u8 extension = string_file_extension(file_name);

	if (string_match(extension, string_u8_litexpr("cpp")) ||
		string_match(extension, string_u8_litexpr("c"))   ||
		string_match(extension, string_u8_litexpr("hpp")) ||
		string_match(extension, string_u8_litexpr("h"))) {
		// try to initialize the clangd server
		clangd_process = create_child_process(
			app, string_u8_litexpr("clangd"), string_u8_litexpr("")
		);
	}
}

#endif /* LSP_4ED_IMPL */
