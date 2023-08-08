#pragma once

UnboundedArray*     tokenize_text(char* text, int text_size, Arena* lexeme_storage, Arena* tokens_storage);
ParsedProgram*      parse_program(UnboundedArray* tokens, Arena* storage);
void                pass_dry(ParsedProgram* p4program);
Pass_NameDecl*      pass_name_decl(ParsedProgram* p4program, Arena* storage);
Pass_TypeDecl*      pass_type_decl(ParsedProgram* p4program, Arena* storage, Pass_NameDecl* namedecl);
Pass_PotentialType* pass_potential_type(ParsedProgram* p4program, Arena* storage, Pass_NameDecl* namedecl, Pass_TypeDecl* typedecl);

