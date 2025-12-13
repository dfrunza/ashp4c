#include <basic.h>
#include <ast.h>

char* AstEnum_to_string(enum AstEnum ast)
{
  switch (ast) {
    case AstEnum::none: return "none";

      /** PROGRAM **/

    case AstEnum::p4program: return "p4program";
    case AstEnum::declarationList: return "declarationList";
    case AstEnum::declaration: return "declaration";
    case AstEnum::name: return "name";
    case AstEnum::parameterList: return "parameterList";
    case AstEnum::parameter: return "parameter";
    case AstEnum::paramDirection: return "paramDirection";
    case AstEnum::packageTypeDeclaration: return "packageTypeDeclaration";
    case AstEnum::instantiation: return "instantiation";

      /** PARSER **/

    case AstEnum::parserDeclaration: return "parserDeclaration";
    case AstEnum::parserTypeDeclaration: return "parserTypeDeclaration";
    case AstEnum::parserLocalElements: return "parserLocalElements";
    case AstEnum::parserLocalElement: return "parserLocalElement";
    case AstEnum::parserStates: return "parserStates";
    case AstEnum::parserState: return "parserState";
    case AstEnum::parserStatements: return "parserStatements";
    case AstEnum::parserStatement: return "parserStatement";
    case AstEnum::parserBlockStatement: return "parserBlockStatement";
    case AstEnum::transitionStatement: return "transitionStatement";
    case AstEnum::stateExpression: return "stateExpression";
    case AstEnum::selectExpression: return "selectExpression";
    case AstEnum::selectCaseList: return "selectCaseList";
    case AstEnum::selectCase: return "selectCase";
    case AstEnum::keysetExpression: return "keysetExpression";
    case AstEnum::tupleKeysetExpression: return "tupleKeysetExpression";
    case AstEnum::simpleKeysetExpression: return "simpleKeysetExpression";
    case AstEnum::simpleExpressionList: return "simpleExpressionList";

      /** CONTROL **/

    case AstEnum::controlDeclaration: return "controlDeclaration";
    case AstEnum::controlTypeDeclaration: return "controlTypeDeclaration";
    case AstEnum::controlLocalDeclarations: return "controlLocalDeclarations";
    case AstEnum::controlLocalDeclaration: return "controlLocalDeclaration";

      /** TYPES **/

    case AstEnum::typeRef: return "typeRef";
    case AstEnum::tupleType: return "tupleType";
    case AstEnum::headerStackType: return "headerStackType";
    case AstEnum::baseTypeBoolean: return "baseTypeBoolean";
    case AstEnum::baseTypeInteger: return "baseTypeInteger";
    case AstEnum::baseTypeBit: return "baseTypeBit";
    case AstEnum::baseTypeVarbit: return "baseTypeVarbit";
    case AstEnum::baseTypeString: return "baseTypeString";
    case AstEnum::baseTypeVoid: return "baseTypeVoid";
    case AstEnum::baseTypeError: return "baseTypeError";
    case AstEnum::integerTypeSize: return "integerTypeSize";
    case AstEnum::realTypeArg: return "realTypeArg";
    case AstEnum::typeArg: return "typeArg";
    case AstEnum::typeArgumentList: return "typeArgumentList";
    case AstEnum::typeDeclaration: return "typeDeclaration";
    case AstEnum::derivedTypeDeclaration: return "derivedTypeDeclaration";
    case AstEnum::headerTypeDeclaration: return "headerTypeDeclaration";
    case AstEnum::headerUnionDeclaration: return "headerUnionDeclaration";
    case AstEnum::structTypeDeclaration: return "structTypeDeclaration";
    case AstEnum::structFieldList: return "structFieldList";
    case AstEnum::structField: return "structField";
    case AstEnum::enumDeclaration: return "enumDeclaration";
    case AstEnum::errorDeclaration: return "errorDeclaration";
    case AstEnum::matchKindDeclaration: return "matchKindDeclaration";
    case AstEnum::identifierList: return "identifierList";
    case AstEnum::specifiedIdentifierList: return "specifiedIdentifierList";
    case AstEnum::specifiedIdentifier: return "specifiedIdentifier";
    case AstEnum::typedefDeclaration: return "typedefDeclaration";

      /** STATEMENTS **/

    case AstEnum::assignmentStatement: return "assignmentStatement";
    case AstEnum::emptyStatement: return "emptyStatement";
    case AstEnum::returnStatement: return "returnStatement";
    case AstEnum::exitStatement: return "exitStatement";
    case AstEnum::conditionalStatement: return "conditionalStatement";
    case AstEnum::directApplication: return "directApplication";
    case AstEnum::statement: return "statement";
    case AstEnum::blockStatement: return "blockStatement";
    case AstEnum::statementOrDeclaration: return "statementOrDeclaration";
    case AstEnum::statementOrDeclList: return "statementOrDeclList";
    case AstEnum::switchStatement: return "switchStatement";
    case AstEnum::switchCases: return "switchCases";
    case AstEnum::switchCase: return "switchCase";
    case AstEnum::switchLabel: return "switchLabel";

      /** TABLES **/

    case AstEnum::tableDeclaration: return "tableDeclaration";
    case AstEnum::tablePropertyList: return "tablePropertyList";
    case AstEnum::tableProperty: return "tableProperty";
    case AstEnum::keyProperty: return "keyProperty";
    case AstEnum::keyElementList: return "keyElementList";
    case AstEnum::keyElement: return "keyElement";
    case AstEnum::actionsProperty: return "actionsProperty";
    case AstEnum::actionList: return "actionList";
    case AstEnum::actionRef: return "actionRef";
#if 0
      case AstEnum::entriesProperty: return "entriesProperty";
    case AstEnum::entriesList: return "entriesList";
    case AstEnum::entry: return "entry";
    case AstEnum::simpleProperty: return "simpleProperty";
#endif
    case AstEnum::actionDeclaration: return "actionDeclaration";

      /** VARIABLES **/

    case AstEnum::variableDeclaration: return "variableDeclaration";

      /** EXPRESSIONS **/

    case AstEnum::functionDeclaration: return "functionDeclaration";
    case AstEnum::argumentList: return "argumentList";
    case AstEnum::argument: return "argument";
    case AstEnum::expressionList: return "expressionList";
    case AstEnum::expression: return "expression";
    case AstEnum::lvalueExpression: return "lvalueExpression";
    case AstEnum::binaryExpression: return "binaryExpression";
    case AstEnum::unaryExpression: return "unaryExpression";
    case AstEnum::functionCall: return "functionCall";
    case AstEnum::memberSelector: return "memberSelector";
    case AstEnum::castExpression: return "castExpression";
    case AstEnum::arraySubscript: return "arraySubscript";
    case AstEnum::indexExpression: return "indexExpression";
    case AstEnum::integerLiteral: return "integerLiteral";
    case AstEnum::stringLiteral: return "stringLiteral";
    case AstEnum::dontcare: return "dontcare";
    case AstEnum::default_: return "default";

    default: return "?";
  }
  assert(0);
  return 0;
}
