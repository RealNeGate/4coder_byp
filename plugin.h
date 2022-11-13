#ifndef PLUGIN_H
#include <stddef.h>
#include "cee.h"

#define PLUGIN_PIPE_NAME L"\\\\.\\pipe\\whitebox_plugin"

// TODO: U4
typedef struct { Size line, clmn; } SrcLoc;

//  TODO: SrcLoc & comparisons
// NOTE: all 4 should be valid at any time.
// if the cursor is just being used normally, current == initial
// current may be >, ==, or < initial
typedef union Selection {
    struct { SrcLoc current, initial; };
    SrcLoc locs[2];
} Selection;

static inline Bool
is_selection(Selection const *selection)
{
    Bool result = (selection->current.line != selection->initial.line ||
        selection->current.clmn != selection->initial.clmn);
    return result;
}

static inline Bool
selection_eq(Selection const *a, Selection const *b)
{
    Bool result = (a && b &&
        a->current.line == b->current.line &&
        a->current.clmn == b->current.clmn &&
        a->initial.line == b->initial.line &&
        a->initial.clmn == b->initial.clmn);
    return result;
}

static int
rng_contains_rng(unsigned parent_begin_line, unsigned short parent_begin_clmn,
    unsigned parent_end_line,   unsigned short parent_end_clmn,
    unsigned child_begin_line,  unsigned short child_begin_clmn,
    unsigned child_end_line,    unsigned short child_end_clmn)

{
    int starts_after_parent = (child_begin_line  >  parent_begin_line ||
        (child_begin_line == parent_begin_line &&
            child_begin_clmn >= parent_begin_clmn));
    int ends_before_parent  = (child_end_line    <  parent_end_line ||
        (child_end_line   == parent_end_line &&
            child_end_clmn   <= parent_end_clmn));
    return starts_after_parent && ends_before_parent;
}


// TODO: should this be a string?
// if not, there should be a valid connection possible to EDITOR_unknown
typedef enum PluginEditor {
    EDITOR_none,
    EDITOR_file,  // using a given file to parse location etc
    EDITOR_4coder,
    EDITOR_notepad_pp,
    EDITOR_vim,
    EDITOR_COUNT,
} PluginEditor;

typedef enum DirtyState {
    DIRTY_current              = 0x0,
    DIRTY_unsaved              = 0x1,
    DIRTY_unloaded             = 0x2,
    DIRTY_unsaved_and_unloaded = DIRTY_unsaved | DIRTY_unloaded,
} DirtyState;

typedef struct PluginSupports
{
    Bool receiving_data;
    Bool setting_cursor;
} PluginSupports;

typedef struct PluginDataOut { // from plugin to WhiteBox proper
    Selection      sel;
    U1             dirty_state;
    char          *path;
    int            ack_pkt; //most recently acknowledged packet (if plugin supports receiving data)
    int            sent_pkt; //most recent packet sent FROM whitebox (if plugin supports receiving data)(note: this field modified by whitebox itself, not plugin actions, so this assumes that plugin-data is not being cleared per-frame)
    PluginSupports plugin_supports;
} PluginDataOut;
enum {
    PluginDataOut_Fixed_Size = offsetof(PluginDataOut, path),
    Plugin_Path_Len = 260,
};
// typedef struct PluginIn { } PluginIn;
//
#define PLUGIN_H
#endif//PLUGIN_H
