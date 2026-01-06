#include "frontend/frontend.h"
#include "frontend/lexer.h"
#include "frontend/parser.h"

void Frontend::do_analysis(Arena* storage, Arena* scratch, SourceText* source_text)
{
  Lexer lexer = {};
  lexer.storage = storage;
  lexer.tokenize(source_text);

  Parser parser = {};
  parser.storage = storage;
  parser.source_file = source_text->filename;
  parser.tokens = lexer.tokens;
  p4program = parser.parse();
  root_scope = parser.root_scope;

  scratch->free();
}