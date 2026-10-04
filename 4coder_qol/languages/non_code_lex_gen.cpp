#include <stdio.h>
#include <stdint.h>
typedef uint8_t  u8;
typedef uint64_t u64;
typedef  int64_t i64;

#define foreach(i, z, N) for(auto i=(z); i<(N); i++)

enum{
  TokenBaseKind_EOF,
  TokenBaseKind_Identifier,      // [a-zA-Z_][a-zA-Z0-9_]*
  TokenBaseKind_ScopeOpen,       // {
  TokenBaseKind_ScopeClose,      // }
  TokenBaseKind_ParenOpen,       // ([
  TokenBaseKind_ParenClose,      // ])
  TokenBaseKind_Operator,        // fallback
  TokenBaseKind_LiteralInteger,  // [0-9][x.,'0-9]*
  TokenBaseKind_LiteralString,   // '"' .* ["\n]
  TokenBaseKind_Whitespace,      // [\u000-\u032]
};

enum Q{
  Q_Root,
  Q_IdenHead,
  Q_IdenBody,
  Q_ScopeOp,
  Q_ScopeCl,
  Q_ParenOp,
  Q_ParenCl,
  Q_Punctuation,
  Q_Whitespace,
  Q_Number,
  Q_Str,
  Q_COUNT,
};

enum CClass{ CC_Iden };

struct Tuple{
  Q q0, q1;
};

struct Action{
  Q q0, q1;
  int z;
  char* name;
};

Q table[256][Q_COUNT];
i64 action_pos = 0;
i64 eof_pos = -1;
Action actions[128];
Tuple  active;

bool InClass(CClass cc, u8 ch){
  // c == CC_Iden
  return (('a' <= ch && ch <= 'z') || ('0' <= ch && ch <= '9') ||
          ('A' <= ch && ch <= 'Z') || ch == '_');
}

#define Emit(z,S) actions[action_pos++] = Action{active.q0, active.q1, z, #S}
#define EmitEOF(q,S) (eof_pos = eof_pos==-1 ? action_pos : eof_pos); actions[action_pos++] = Action{q,q,0,#S}
void Transition(Q q0, Q q1, u8 c){ active = {q0,q1}; table[c][q0] = q1; }
void Transition(Q q0, Q q1, char* s){ while(*s) Transition(q0, q1, *s++); }
void Transition(Q q0, Q q1, int a, int b){ foreach(i, a, b+1) Transition(q0, q1, i); }
void Transition(Q q0, Q q1, CClass cc){ foreach(i, 0, 256) if (InClass(cc, i)) Transition(q0, q1, i); }
void Fallback  (Q q0, Q q1){ Transition(q0, q1, 0, 255); }

int main(){
  Fallback(Q_Root,        Q_Punctuation);
  Fallback(Q_IdenHead,    Q_Root); Emit(0, TokenBaseKind_Identifier);
  Fallback(Q_IdenBody,    Q_Root); Emit(0, TokenBaseKind_Identifier);
  Fallback(Q_ScopeOp,     Q_Root); Emit(0, TokenBaseKind_ScopeOpen);
  Fallback(Q_ScopeCl,     Q_Root); Emit(0, TokenBaseKind_ScopeClose);
  Fallback(Q_ParenOp,     Q_Root); Emit(0, TokenBaseKind_ParenOpen);
  Fallback(Q_ParenCl,     Q_Root); Emit(0, TokenBaseKind_ParenClose);
  Fallback(Q_Punctuation, Q_Root); Emit(0, TokenBaseKind_Operator);
  Fallback(Q_Whitespace,  Q_Root); Emit(0, TokenBaseKind_Whitespace);
  Fallback(Q_Number,      Q_Root); Emit(0, TokenBaseKind_LiteralInteger);
  Fallback(Q_Str,         Q_Str);
  Transition(Q_Root, Q_Str, '"');  //- String
  Transition(Q_Str, Q_Root, "\"\n"); Emit(1, TokenBaseKind_LiteralString);
  Transition(Q_Root, Q_IdenHead, 'a', 'z');  //- Identifier
  Transition(Q_Root, Q_IdenHead, 'A', 'Z');
  Transition(Q_Root, Q_IdenHead, '_');
  Transition(Q_IdenHead, Q_IdenBody, CC_Iden);  // [a-zA-Z0-9_]
  Transition(Q_IdenBody, Q_IdenBody, CC_Iden);  // [a-zA-Z0-9_]
  Transition(Q_Root, Q_ScopeOp, '{');  //- Scopes/Parens
  Transition(Q_Root, Q_ScopeCl, '}');
  Transition(Q_Root, Q_ParenOp, "([");
  Transition(Q_Root, Q_ParenCl, "])");
  Transition(Q_Root, Q_Whitespace, 0, 32);  //- Whitespace
  Transition(Q_Whitespace, Q_Whitespace, 0, 32);
  Transition(Q_Root,   Q_Number, '0', '9');  //- Number
  Transition(Q_Number, Q_Number, '0', '9');
  Transition(Q_Number, Q_Number, "x.,'");
  EmitEOF(Q_Root,        TokenBaseKind_EOF);  //- EOF emits
  EmitEOF(Q_IdenHead,    TokenBaseKind_Identifier);
  EmitEOF(Q_IdenBody,    TokenBaseKind_Identifier);
  EmitEOF(Q_ScopeOp,     TokenBaseKind_ScopeOpen);
  EmitEOF(Q_ScopeCl,     TokenBaseKind_ScopeClose);
  EmitEOF(Q_ParenOp,     TokenBaseKind_ParenClose);
  EmitEOF(Q_ParenCl,     TokenBaseKind_ParenOpen);
  EmitEOF(Q_Punctuation, TokenBaseKind_Operator);
  EmitEOF(Q_Whitespace,  TokenBaseKind_Whitespace);
  EmitEOF(Q_Number,      TokenBaseKind_LiteralInteger);
  EmitEOF(Q_Str,         TokenBaseKind_LiteralString);

  printf("struct Lex_State_Non_Code{\n");
  printf("  i64 token_begin;\n");
  printf("  i64 start;\n");
  printf("  u16 q;\n");
  printf("};\n\n");

  printf("u64 g_non_code_table[256] = {\n  ");
  foreach(ch, 0, 256){
    u64 row = 0;
    foreach(q0, 0, Q_COUNT) row |= (u64(table[ch][q0]) << q0*4);
    printf("0x%.11llx,%s", row, (ch+1==256 ? "" : ch % 64 == 63 ? "\n  " : " "));
  }
  printf("\n};\n\n");

  printf("#define EMIT_CHECK(Z, I, T) EMIT(Z, I, T); if(count++ > limit){ *state = {pos,I,b}; return false; }\n");
  printf("#define EMIT(Z, I, T)                   \\\n");
  printf("  Token token = {};                     \\\n");
  printf("  token.pos = pos;                      \\\n");
  printf("  token.size = I-pos+Z;                 \\\n");
  printf("  token.kind = T;                       \\\n");
  printf("  token_list_push(arena, list, &token); \\\n");
  printf("  pos=I+Z; I = I-1+Z;\n\n");

  printf("internal b32\n");
  printf("lex_full_input_non_code_breaks(Arena *arena, Token_List *list, Lex_State_Non_Code* state, String_Const_u8 contents, i64 limit){\n");
  printf("  i64 count = 0;\n");
  printf("  u16 a = state->q;\n");
  printf("  i64 pos = state->token_begin;\n");
  printf("  for(i64 i = state->start; i < i64(contents.size); i += 1){\n");
  printf("    u64 row = g_non_code_table[contents.str[i]];\n");
  printf("    u16 b = (row >> a*4) & 0xF;\n");
  printf("    switch((b|(a<<4))){\n");
  foreach(i, 0, eof_pos){
    printf("      case 0x%x%x:{ EMIT_CHECK(%d, i, %s); } break;\n", actions[i].q0, actions[i].q1, actions[i].z, actions[i].name);
  }
  printf("    }\n");
  printf("    a = b;\n");
  printf("  }\n\n");

  printf("  switch(a){\n");
  foreach(i, eof_pos, action_pos){
    printf("    case 0x%x:{ EMIT(%d, contents.size, %s); } break;\n", actions[i].q0, actions[i].z, actions[i].name);
  }
  printf("  };\n");
  printf("  return true;\n");
  printf("}\n");
  printf("#undef EMIT_CHECK\n");
  printf("#undef EMIT\n\n");

  printf("internal Token_List\n");
  printf("lex_full_input_none(Arena *arena, String_Const_u8 input){\n");
  printf("  Token_List list = {};\n");
  printf("  Lex_State_Non_Code state = {};\n");
  printf("  lex_full_input_non_code_breaks(arena, &list, &state, input, max_i64);\n");
  printf("  return(list);\n");
  printf("}\n\n");

  printf("internal Token_List\n");
  printf("lex_full_input_async_none(Async_Context *actx, Arena *arena, String_Const_u8 input, i32 limit, b32 *canceled){\n");
  printf("  Token_List list = {};\n");
  printf("  Lex_State_Non_Code state = {};\n");
  printf("  for (;;){\n");
  printf("    if (lex_full_input_non_code_breaks(arena, &list, &state, input, limit)){\n");
  printf("      break;\n");
  printf("    }\n");
  printf("    if (async_check_canceled(actx)){\n");
  printf("      *canceled = true;\n");
  printf("      break;\n");
  printf("    }\n");
  printf("  }\n");
  printf("  return(list);\n");
  printf("}\n");
  return 0;
}