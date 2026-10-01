CUSTOM_ID(attachment, buffer_lang);

enum Lang_ID{
  Lang_None,
  Lang_Cpp,
  Lang_4ed,
  Lang_XSL,
  Lang_Lua,
  Lang_COUNT,
};

typedef Token_List Lex_Async_Func_Type(Async_Context*, Arena*, String_Const_u8, i32, b32*);
typedef Token_List Lex_Sync_Func_Type (Arena*, String_Const_u8);
typedef void Parse_Func_Type(Application_Links*, Code_Index_File*, Arena*, String_Const_u8, Token_Array*);
typedef FColor Token_Color_Func(Token*);

function Token_List lang_lex_async_nop(Async_Context*, Arena*, String_Const_u8, i32, b32*);
function Token_List lang_lex_sync_nop (Arena*, String_Const_u8);
function void       lang_parse_nop(Application_Links*, Code_Index_File*, Arena*, String_Const_u8, Token_Array*);
function FColor     lang_paint_nop(Token*);

struct Lang_Spec{
  Lang_ID id;
  Lex_Async_Func_Type *lex_async;
  Lex_Sync_Func_Type  *lex_full;
  Parse_Func_Type     *parse;
  Token_Color_Func    *token_color;
};

function void qol_lang_register(Lang_ID id, Lex_Async_Func_Type *lex_async, Lex_Sync_Func_Type *lex_full, Parse_Func_Type *parse, Token_Color_Func *token_color);
function Lang_Spec* qol_lang_for_buffer(Application_Links *app, Buffer_ID buffer);