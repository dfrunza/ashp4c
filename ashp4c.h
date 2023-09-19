#pragma once

SourceText*         read_source_text(char* filename, Arena* storage);
UnboundedArray*     tokenize_source_text(SourceText* source_text, Arena* storage);
Ast_P4Program*      parse_program(UnboundedArray* tokens, Arena* storage, Scope** root_scope);
void                pass_dry(Ast_P4Program* ast);
PassResult_NameDecl*      pass_name_decl(Ast_P4Program* ast, Scope* root_scope, Arena* storage);
PassResult_TypeDecl*      pass_type_decl(Ast_P4Program* ast, Arena* storage, PassResult_NameDecl* namedecl_result);
PassResult_PotentialType* pass_potential_type(Ast_P4Program* ast, Scope* root_scope, Arena* storage,
                              PassResult_NameDecl* namedecl, PassResult_TypeDecl* typedecl);

