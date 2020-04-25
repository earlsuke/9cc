#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

// トークンの種類
typedef enum {
  TK_RESERVED, // 記号
  TK_NUM,      // 整数トークン
  TK_EOF,      // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
  // 型
  TokenKind kind;
  // 次のトークンへのポインタ
  Token* next;
  // トークンが持つ値
  int val;
  // トークンの文字列
  char* str;
};

// 現在のtoken
Token* token;

// user 入力
char* user_input;

// エラー箇所報告付きのエラー関数

void error_at(char* loc, char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, ""); // pos個の*を出力
  fprintf(stderr, "^");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー処理のための関数
void error(char* fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 期待しているtokenを消化して次へ進める
bool consume(char op) {
  
  // 期待していないtokenであれば，falseを返す．
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    return false;
  }

  // 期待しているtokenならば，次に進める
  token = token->next;
  return true;
}


// opを期待して現在のトークンを確認，異なる場合は，エラー処理する．
void expect(char op) {
  if (token->kind != TK_RESERVED || token->str[0] != op) {
    error_at(token->str, "'%cではありません", op);
  }
  // 期待したtokenの場合は，次にすすめる
  token  = token->next;
}

int expect_number() {
  if (token->kind != TK_NUM) {
    error_at(token->str, "数ではありません．");
  }

  int val = token->val;
  token = token->next;
  return val;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

Token* new_token(TokenKind kind, Token* cur, char* str) {
  Token* tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// スペースをスキップして，token系列が作られる(Linked list)
// Token* tokenize(char* p) {
  Token* tokenize() {
  char* p = user_input;
  
  // dummy head
  Token head;
  head.next = NULL;
  Token* cur = &head;

  while (*p) {
    // 空白をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    // オペレーター
    if (*p == '+' || *p == '-') {
      cur = new_token(TK_RESERVED, cur, p++);
      continue;
    }

    if (isdigit(*p)) {
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

int main(int argc, char **argv) {

  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  // char* p = argv[1];
  // user_input = p;
  user_input = argv[1];

  // トークナイズ
  token = tokenize();

  printf(".intel_syntax noprefix\n");
  printf(".global main\n");
  printf("main:\n");

  // token系列から先頭の数値を1個持ってくる
  printf("  mov rax, %d\n", expect_number());

  while (!at_eof()) {

    // + の場合
    if (consume('+')) {
      // 足し算の命令を作る
      printf("  add rax, %d\n", expect_number());
      continue;
    }

    //　- のオペレーター
    expect('-');
    // 引き算の命令を作る
    printf("  sub rax, %d\n", expect_number());
  }
  
  printf("  ret\n");
  return 0;
}