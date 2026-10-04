struct Lex_State_Non_Code{
  i64 token_begin;
  i64 start;
  u16 q;
};

u64 g_non_code_table[256] = {
  0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0x00800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0800000008, 0xa0000000007, 0x0000000000a, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa9000000007, 0xa0000000005, 0xa0000000006, 0xa0000000007, 0xa0000000007, 0xa9000000007, 0xa0000000007, 0xa9000000007, 0xa0000000007, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa9000000229, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007,
  0xa0000000007, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000005, 0xa0000000007, 0xa0000000006, 0xa0000000007, 0xa0000000221, 0xa0000000007, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa0000000221, 0xa9000000221, 0xa0000000221, 0xa0000000221, 0xa0000000003, 0xa0000000007, 0xa0000000004, 0xa0000000007, 0xa0000000007,
  0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007,
  0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007, 0xa0000000007,
};

#define EMIT_CHECK(Z, I, T) EMIT(Z, I, T); if(count++ > limit){ *state = {pos,I,b}; return false; }
#define EMIT(Z, I, T)                   \
  Token token = {};                     \
  token.pos = pos;                      \
  token.size = I-pos+Z;                 \
  token.kind = T;                       \
  token_list_push(arena, list, &token); \
  pos=I+Z; I = I-1+Z;

internal b32
lex_full_input_non_code_breaks(Arena *arena, Token_List *list, Lex_State_Non_Code* state, String_Const_u8 contents, i64 limit){
  i64 count = 0;
  u16 a = state->q;
  i64 pos = state->token_begin;
  for(i64 i = state->start; i < i64(contents.size); i += 1){
    u64 row = g_non_code_table[contents.str[i]];
    u16 b = (row >> a*4) & 0xF;
    switch((b|(a<<4))){
      case 0x10:{ EMIT_CHECK(0, i, TokenBaseKind_Identifier); } break;
      case 0x20:{ EMIT_CHECK(0, i, TokenBaseKind_Identifier); } break;
      case 0x30:{ EMIT_CHECK(0, i, TokenBaseKind_ScopeOpen); } break;
      case 0x40:{ EMIT_CHECK(0, i, TokenBaseKind_ScopeClose); } break;
      case 0x50:{ EMIT_CHECK(0, i, TokenBaseKind_ParenOpen); } break;
      case 0x60:{ EMIT_CHECK(0, i, TokenBaseKind_ParenClose); } break;
      case 0x70:{ EMIT_CHECK(0, i, TokenBaseKind_Operator); } break;
      case 0x80:{ EMIT_CHECK(0, i, TokenBaseKind_Whitespace); } break;
      case 0x90:{ EMIT_CHECK(0, i, TokenBaseKind_LiteralInteger); } break;
      case 0xa0:{ EMIT_CHECK(1, i, TokenBaseKind_LiteralString); } break;
    }
    a = b;
  }

  switch(a){
    case 0x0:{ EMIT(0, contents.size, TokenBaseKind_EOF); } break;
    case 0x1:{ EMIT(0, contents.size, TokenBaseKind_Identifier); } break;
    case 0x2:{ EMIT(0, contents.size, TokenBaseKind_Identifier); } break;
    case 0x3:{ EMIT(0, contents.size, TokenBaseKind_ScopeOpen); } break;
    case 0x4:{ EMIT(0, contents.size, TokenBaseKind_ScopeClose); } break;
    case 0x5:{ EMIT(0, contents.size, TokenBaseKind_ParenClose); } break;
    case 0x6:{ EMIT(0, contents.size, TokenBaseKind_ParenOpen); } break;
    case 0x7:{ EMIT(0, contents.size, TokenBaseKind_Operator); } break;
    case 0x8:{ EMIT(0, contents.size, TokenBaseKind_Whitespace); } break;
    case 0x9:{ EMIT(0, contents.size, TokenBaseKind_LiteralInteger); } break;
    case 0xa:{ EMIT(0, contents.size, TokenBaseKind_LiteralString); } break;
  };
  return true;
}
#undef EMIT_CHECK
#undef EMIT

internal Token_List
lex_full_input_none(Arena *arena, String_Const_u8 input){
  Token_List list = {};
  Lex_State_Non_Code state = {};
  lex_full_input_non_code_breaks(arena, &list, &state, input, max_i64);
  return(list);
}

internal Token_List
lex_full_input_async_none(Async_Context *actx, Arena *arena, String_Const_u8 input, i32 limit, b32 *canceled){
  Token_List list = {};
  Lex_State_Non_Code state = {};
  for (;;){
    if (lex_full_input_non_code_breaks(arena, &list, &state, input, limit)){
      break;
    }
    if (async_check_canceled(actx)){
      *canceled = true;
      break;
    }
  }
  return(list);
}