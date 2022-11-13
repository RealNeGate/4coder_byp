#undef function
#include "plugin_base.cpp"
#define function static

#include <assert.h>
#include "json.h"

struct WB4CoderGlobals {
    Application_Links *app; // NOTE: this must be kept updated before any wb calls as it seems to move
    String_Const_u8    file_path;
    Arena              name_arena[1];

    Buffer_Cursor cursor;
    Buffer_Cursor marker;

    Buffer_ID  buffer;
    DirtyState dirty_state;
    int        ack_pkt;
    int        ack_pkt_counter;
};

static WB4CoderGlobals g_wb_4c = {};

internal inline Bool
json_value_to_number(json_value_s *val, double *out)
{
    json_number_s *num = (val ? json_value_as_number(val) : NULL);
    *out = (num ? atof(num->number) : 0.0);
    return num;
}

internal inline Bool
json_value_to_array(json_value_s *val, json_array_s **out)
{   return (*out = (val ? json_value_as_array(val) : NULL));   }

function void
wb_editor_message(Char const *title, Char const *message, Size message_len)
{
    String_Const_u8 t = { (u8 *)title, wb_str_len(title) };
    print_message(g_wb_4c.app, t);
    print_message(g_wb_4c.app, string_u8_litexpr(": "));
    String_Const_u8 msg = { (u8 *)message, message_len };
    print_message(g_wb_4c.app, msg);
    print_message(g_wb_4c.app, string_u8_litexpr("\n"));
}



function void
wb_4c_tick(Application_Links *app, View_ID view)
{
    // TODO: could trigger initial connection if g_wb_4c.app == NULL
    g_wb_4c.app = app; // NOTE: ensure that the tick hook has been installed, because the connect plugin can show anyway

    if (! wb_is_connected()) return;

    struct { bool buffer, cursor, marker, dirty, ack_pkt; } changed = { 0 };

    { // receive message
        animate_in_n_milliseconds(app, 16); //force tick so packets are consumed and responded to immediately

        char data[1000] = {0}; //"{\"sent_pkt\": 0, \"selection\":[{\"line\":1,\"column\":1},{\"line\":1,\"column\":1}]}"; //example data

        unsigned long iMode = 1;
        ioctlsocket(g_wb.sock, FIONBIO, &iMode);
        recv(g_wb.sock, data, sizeof(data), 0);

        bool received_a_message = data[0] != 0;
        if(received_a_message)
        {
            typedef struct { int line, column; } text_position;
            struct { int sent_pkt; text_position selection[2];} message = {0};

            // parse message
            {
                json_value_s *json = json_parse(data, strlen(data));
                json_object_s *json_obj = json_value_as_object(json);
                for (json_object_element_s *el = json_obj->start; el != NULL; el = el->next)
                {
                    if (!strcmp("sent_pkt", el->name->string))
                    {
                        double number = 0.0;
                        json_value_to_number(el->value, &number);
                        message.sent_pkt = (int)number;
                    }
                    else if (!strcmp("selection", el->name->string))
                    {
                        json_array_s *selection_array;
                        json_value_to_array(el->value, &selection_array);
                        json_array_element_s* arr_el = selection_array->start;
                        json_object_s *sel_obj = json_value_as_object(arr_el->value);
                        for (json_object_element_s *sel_el = sel_obj->start; sel_el != NULL; sel_el = sel_el->next)
                        {
                            if (!strcmp("line", sel_el->name->string))
                            {
                                double number = 0.0;
                                json_value_to_number(sel_el->value, &number);
                                message.selection[0].line = (int)number;
                            }
                            else if (!strcmp("column", sel_el->name->string))
                            {
                                double number = 0.0;
                                json_value_to_number(sel_el->value, &number);
                                message.selection[0].column = (int)number;
                            }
                        }
                    }

                }
                free(json);
            }

            int ack_pkt = message.sent_pkt + 1;

            changed.ack_pkt = ack_pkt != g_wb_4c.ack_pkt;
            g_wb_4c.ack_pkt = ack_pkt;

            // update selection
            view_set_cursor(app, view, seek_line_col(message.selection[0].line, message.selection[0].column));

            g_wb_4c.ack_pkt_counter++;
            assert(g_wb_4c.ack_pkt == g_wb_4c.ack_pkt_counter);
        }
    }


    { // update buffer
        Buffer_ID buffer = view_get_buffer(app, view, Access_ReadVisible);
        changed.buffer   = buffer && buffer != g_wb_4c.buffer;

        if (changed.buffer)
        {
            if (! g_wb_4c.name_arena->chunk_size)
            {   g_wb_4c.name_arena[0] = make_arena(get_thread_context(app)->allocator, KB(4));   }

            linalloc_clear(g_wb_4c.name_arena);
            g_wb_4c.file_path = push_buffer_file_name(app, g_wb_4c.name_arena, buffer);
            g_wb_4c.buffer    = buffer;
        }
    }

    { // update cursor
        i64           cursor_pos = view_get_cursor_pos(app, view);
        Buffer_Cursor cursor     = buffer_compute_cursor(app, g_wb_4c.buffer, seek_pos(cursor_pos));
        changed.cursor = (cursor.line != g_wb_4c.cursor.line ||
            cursor.col  != g_wb_4c.cursor.col);
        g_wb_4c.cursor = cursor;
    }

    { // update marker
        i64           marker_pos = view_get_mark_pos(app, view);
        Buffer_Cursor marker     = buffer_compute_cursor(app, g_wb_4c.buffer, seek_pos(marker_pos));
        changed.marker = (marker.line != g_wb_4c.marker.line ||
            marker.col  != g_wb_4c.marker.col);
        g_wb_4c.marker = marker;
    }

    { // update dirty_state
        Dirty_State dirty_state = buffer_get_dirty_state(app, g_wb_4c.buffer);
        changed.dirty           = dirty_state != g_wb_4c.dirty_state;
        g_wb_4c.dirty_state     = (DirtyState)dirty_state;
    }


    // TODO: could either convey dirty_state to whitebox or only transmit when dirty_state
    if (changed.buffer | changed.cursor | changed.marker | changed.dirty | changed.ack_pkt)
    {
        Selection sel = { cast(Size, g_wb_4c.cursor.line), cast(Size, g_wb_4c.cursor.col),
            cast(Size, g_wb_4c.marker.line), cast(Size, g_wb_4c.marker.col) };

        wb_send_plugin_data(sel, (char const *)g_wb_4c.file_path.str, g_wb_4c.file_path.size, "4coder", g_wb_4c.dirty_state, g_wb_4c.ack_pkt);
    }
}

function void
wb_4c_default_tick(Application_Links *app, Frame_Info frame_info)
{
    default_tick(app, frame_info);
    View_ID view = get_active_view(app, Access_ReadVisible);
    wb_4c_tick(app, view);
}

function bool
wb_4c_check_hook_is_installed(Application_Links *app)
{
    bool tick_is_used = (bool)g_wb_4c.app;
    if (! tick_is_used)
    {
        print_message(app, string_u8_litexpr(
                "WhiteBox:\n"
                "    It seems like you haven't added the hook for the WhiteBox tick into your plugin.\n"
                "    Make sure that somewhere in the function set to `HookID_Tick` `wb_4c_tick` gets called\n"
                "    alongside 4coder's `default_tick`. See `wb_4c_default_tick` for an example.\n"
                "    --\n"
                "    It's also possible that after setting the hook to include the WhiteBox tick,\n"
                "    your plugin sets the hook again later, overwriting WhiteBox's version.\n"
                "    --\n"
                "    (You may also get this message if you try to connect to WhiteBox before 4coder's first tick.\n"
                "    If this is the case, please contact plugins@whitebox.systems to let us know your use-case)\n\n"));
    }
    return tick_is_used;
}


#if 1  // COMMANDS


CUSTOM_COMMAND_SIG(whitebox_disconnect)
CUSTOM_DOC("Disconnects from a connected WhiteBox executable")
{
    if (wb_4c_check_hook_is_installed(app))
    {
        if (wb_is_connected())
        {   g_wb_4c.app = app; wb_disconnect(1);   }
        else
        {   print_message(app, string_u8_litexpr("Not connected to a WhiteBox instance\n"));   }
    }
}

CUSTOM_COMMAND_SIG(whitebox_connect)
CUSTOM_DOC("Connects to a running WhiteBox executable")
{
    if (wb_4c_check_hook_is_installed(app))
    {
        if (! wb_is_connected())
        {   g_wb_4c.app = app; wb_connect(1);   }
        else
        {   print_message(app, string_u8_litexpr("Already connected to a WhiteBox instance\n"));   }
    }
}

CUSTOM_COMMAND_SIG(whitebox_connection_check)
CUSTOM_DOC("Query if currently connected to a WhiteBox instance")
{
    if (wb_4c_check_hook_is_installed(app))
    {
        print_message(app, (wb_is_connected()
                ? string_u8_litexpr("Is connected to WhiteBox\n")
                : string_u8_litexpr("Is not connected to WhiteBox\n")));
    }
}

#endif // COMMANDS
