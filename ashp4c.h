#pragma once

SourceText*         read_source_text(char* filename, Arena* storage);
TokenizedSource*    tokenize_text(SourceText* source_text, Arena* storage);
ParsedProgram*      parse_program(TokenizedSource* lex_result, Arena* storage);
void                pass_dry(ParsedProgram* p4program);
PassResult_NameDecl*      pass_name_decl(ParsedProgram* p4program, Arena* storage);
PassResult_TypeDecl*      pass_type_decl(ParsedProgram* p4program, Arena* storage, PassResult_NameDecl* namedecl_result);
PassResult_PotentialType* pass_potential_type(ParsedProgram* p4program, Arena* storage,
                              PassResult_NameDecl* namedecl, PassResult_TypeDecl* typedecl);

