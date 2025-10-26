#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <stdbool.h>

// ==================== LEXICAL ANALYZER ====================

typedef enum {
    TOK_INT, TOK_FLOAT, TOK_CHAR, TOK_VOID, TOK_IF, TOK_ELSE,
    TOK_WHILE, TOK_FOR, TOK_RETURN, TOK_IDENTIFIER, TOK_INTEGER_LITERAL,
    TOK_FLOAT_LITERAL, TOK_CHAR_LITERAL, TOK_STRING_LITERAL,
    TOK_PLUS, TOK_MINUS, TOK_MULTIPLY, TOK_DIVIDE, TOK_MODULO,
    TOK_ASSIGN, TOK_EQ, TOK_NE, TOK_LT, TOK_LE, TOK_GT, TOK_GE,
    TOK_AND, TOK_OR, TOK_NOT, TOK_LPAREN, TOK_RPAREN,
    TOK_LBRACE, TOK_RBRACE, TOK_SEMICOLON, TOK_COMMA, TOK_EOF, TOK_ERROR
} TokenType;

typedef struct {
    TokenType type;
    char value[256];
    int line, column;
} Token;

typedef struct {
    const char* source;
    int pos, line, column, length;
} Lexer;

const char* keywords[] = {"int", "float", "char", "void", "if", "else", "while", "for", "return"};
TokenType keywordTypes[] = {TOK_INT, TOK_FLOAT, TOK_CHAR, TOK_VOID, TOK_IF, TOK_ELSE, TOK_WHILE, TOK_FOR, TOK_RETURN};

void initLexer(Lexer* lex, const char* src) {
    lex->source = src;
    lex->pos = 0;
    lex->line = 1;
    lex->column = 1;
    lex->length = strlen(src);
}

void skipWhitespace(Lexer* lex) {
    while (lex->pos < lex->length) {
        char c = lex->source[lex->pos];
        if (c == ' ' || c == '\t' || c == '\r') {
            lex->pos++;
            lex->column++;
        } else if (c == '\n') {
            lex->pos++;
            lex->line++;
            lex->column = 1;
        } else break;
    }
}

Token getNextToken(Lexer* lex) {
    Token tok;
    skipWhitespace(lex);
    
    if (lex->pos >= lex->length) {
        tok.type = TOK_EOF;
        strcpy(tok.value, "EOF");
        tok.line = lex->line;
        tok.column = lex->column;
        return tok;
    }
    
    char c = lex->source[lex->pos];
    tok.line = lex->line;
    tok.column = lex->column;
    
    // Identifier or keyword
    if (isalpha(c) || c == '_') {
        int start = lex->pos, len = 0;
        while (lex->pos < lex->length && (isalnum(lex->source[lex->pos]) || lex->source[lex->pos] == '_')) {
            lex->pos++;
            lex->column++;
            len++;
        }
        strncpy(tok.value, &lex->source[start], len);
        tok.value[len] = '\0';
        
        tok.type = TOK_IDENTIFIER;
        for (int i = 0; i < 9; i++) {
            if (strcmp(tok.value, keywords[i]) == 0) {
                tok.type = keywordTypes[i];
                break;
            }
        }
        return tok;
    }
    
    // Number
    if (isdigit(c)) {
        int start = lex->pos, len = 0;
        bool isFloat = false;
        while (lex->pos < lex->length && (isdigit(lex->source[lex->pos]) || lex->source[lex->pos] == '.')) {
            if (lex->source[lex->pos] == '.') isFloat = true;
            lex->pos++;
            lex->column++;
            len++;
        }
        strncpy(tok.value, &lex->source[start], len);
        tok.value[len] = '\0';
        tok.type = isFloat ? TOK_FLOAT_LITERAL : TOK_INTEGER_LITERAL;
        return tok;
    }
    
    // String literal
    if (c == '"') {
        int start = lex->pos, len = 0;
        lex->pos++;
        lex->column++;
        len++;
        while (lex->pos < lex->length && lex->source[lex->pos] != '"') {
            if (lex->source[lex->pos] == '\\') {
                lex->pos += 2;
                lex->column += 2;
                len += 2;
            } else {
                lex->pos++;
                lex->column++;
                len++;
            }
        }
        if (lex->pos < lex->length) {
            lex->pos++;
            lex->column++;
            len++;
        }
        strncpy(tok.value, &lex->source[start], len);
        tok.value[len] = '\0';
        tok.type = TOK_STRING_LITERAL;
        return tok;
    }
    
    // Character literal
    if (c == '\'') {
        int start = lex->pos, len = 0;
        lex->pos++;
        lex->column++;
        len++;
        while (lex->pos < lex->length && lex->source[lex->pos] != '\'') {
            lex->pos += (lex->source[lex->pos] == '\\') ? 2 : 1;
            lex->column += (lex->source[lex->pos - 1] == '\\') ? 2 : 1;
            len += (lex->source[lex->pos - 1] == '\\') ? 2 : 1;
        }
        if (lex->pos < lex->length) {
            lex->pos++;
            lex->column++;
            len++;
        }
        strncpy(tok.value, &lex->source[start], len);
        tok.value[len] = '\0';
        tok.type = TOK_CHAR_LITERAL;
        return tok;
    }
    
    // Two-character operators
    if (lex->pos + 1 < lex->length) {
        char next = lex->source[lex->pos + 1];
        if (c == '=' && next == '=') {
            tok.type = TOK_EQ;
            strcpy(tok.value, "==");
            lex->pos += 2;
            lex->column += 2;
            return tok;
        }
        if (c == '!' && next == '=') {
            tok.type = TOK_NE;
            strcpy(tok.value, "!=");
            lex->pos += 2;
            lex->column += 2;
            return tok;
        }
        if (c == '<' && next == '=') {
            tok.type = TOK_LE;
            strcpy(tok.value, "<=");
            lex->pos += 2;
            lex->column += 2;
            return tok;
        }
        if (c == '>' && next == '=') {
            tok.type = TOK_GE;
            strcpy(tok.value, ">=");
            lex->pos += 2;
            lex->column += 2;
            return tok;
        }
        if (c == '&' && next == '&') {
            tok.type = TOK_AND;
            strcpy(tok.value, "&&");
            lex->pos += 2;
            lex->column += 2;
            return tok;
        }
        if (c == '|' && next == '|') {
            tok.type = TOK_OR;
            strcpy(tok.value, "||");
            lex->pos += 2;
            lex->column += 2;
            return tok;
        }
    }
    
    // Single-character tokens
    tok.value[0] = c;
    tok.value[1] = '\0';
    lex->pos++;
    lex->column++;
    
    switch (c) {
        case '+': tok.type = TOK_PLUS; break;
        case '-': tok.type = TOK_MINUS; break;
        case '*': tok.type = TOK_MULTIPLY; break;
        case '/': tok.type = TOK_DIVIDE; break;
        case '%': tok.type = TOK_MODULO; break;
        case '=': tok.type = TOK_ASSIGN; break;
        case '<': tok.type = TOK_LT; break;
        case '>': tok.type = TOK_GT; break;
        case '!': tok.type = TOK_NOT; break;
        case '(': tok.type = TOK_LPAREN; break;
        case ')': tok.type = TOK_RPAREN; break;
        case '{': tok.type = TOK_LBRACE; break;
        case '}': tok.type = TOK_RBRACE; break;
        case ';': tok.type = TOK_SEMICOLON; break;
        case ',': tok.type = TOK_COMMA; break;
        default: tok.type = TOK_ERROR; break;
    }
    return tok;
}

// ==================== ABSTRACT SYNTAX TREE ====================

typedef enum {
    NODE_PROGRAM, NODE_FUNCTION, NODE_VAR_DECL, NODE_BLOCK,
    NODE_IF, NODE_WHILE, NODE_FOR, NODE_RETURN, NODE_ASSIGN,
    NODE_BINARY_OP, NODE_UNARY_OP, NODE_CALL, NODE_IDENTIFIER,
    NODE_INTEGER, NODE_FLOAT, NODE_CHAR, NODE_STRING
} NodeType;

typedef struct ASTNode {
    NodeType type;
    char value[256];
    TokenType dataType;
    struct ASTNode* left;
    struct ASTNode* right;
    struct ASTNode* extra;
    struct ASTNode** children;
    int childCount;
} ASTNode;

ASTNode* createNode(NodeType type) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value[0] = '\0';
    node->dataType = TOK_VOID;
    node->left = node->right = node->extra = NULL;
    node->children = NULL;
    node->childCount = 0;
    return node;
}

void addChild(ASTNode* parent, ASTNode* child) {
    parent->children = realloc(parent->children, sizeof(ASTNode*) * (parent->childCount + 1));
    parent->children[parent->childCount++] = child;
}

// ==================== PARSER ====================

typedef struct {
    Lexer* lexer;
    Token currentToken;
} Parser;

void advance(Parser* p) {
    p->currentToken = getNextToken(p->lexer);
}

bool match(Parser* p, TokenType type) {
    return p->currentToken.type == type;
}

bool expect(Parser* p, TokenType type) {
    if (!match(p, type)) {
        printf("Parse error at line %d: expected token type %d, got %d\n", 
               p->currentToken.line, type, p->currentToken.type);
        return false;
    }
    advance(p);
    return true;
}

// Forward declarations
ASTNode* parseExpression(Parser* p);
ASTNode* parseStatement(Parser* p);

ASTNode* parsePrimary(Parser* p) {
    ASTNode* node = NULL;
    
    if (match(p, TOK_INTEGER_LITERAL)) {
        node = createNode(NODE_INTEGER);
        strcpy(node->value, p->currentToken.value);
        advance(p);
    } else if (match(p, TOK_FLOAT_LITERAL)) {
        node = createNode(NODE_FLOAT);
        strcpy(node->value, p->currentToken.value);
        advance(p);
    } else if (match(p, TOK_CHAR_LITERAL)) {
        node = createNode(NODE_CHAR);
        strcpy(node->value, p->currentToken.value);
        advance(p);
    } else if (match(p, TOK_STRING_LITERAL)) {
        node = createNode(NODE_STRING);
        strcpy(node->value, p->currentToken.value);
        advance(p);
    } else if (match(p, TOK_IDENTIFIER)) {
        node = createNode(NODE_IDENTIFIER);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        
        if (match(p, TOK_LPAREN)) {
            ASTNode* callNode = createNode(NODE_CALL);
            strcpy(callNode->value, node->value);
            free(node);
            advance(p);
            
            while (!match(p, TOK_RPAREN) && !match(p, TOK_EOF)) {
                addChild(callNode, parseExpression(p));
                if (match(p, TOK_COMMA)) advance(p);
            }
            expect(p, TOK_RPAREN);
            return callNode;
        }
    } else if (match(p, TOK_LPAREN)) {
        advance(p);
        node = parseExpression(p);
        expect(p, TOK_RPAREN);
    }
    
    return node;
}

ASTNode* parseUnary(Parser* p) {
    if (match(p, TOK_MINUS) || match(p, TOK_NOT)) {
        ASTNode* node = createNode(NODE_UNARY_OP);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        node->left = parseUnary(p);
        return node;
    }
    return parsePrimary(p);
}

ASTNode* parseMultiplicative(Parser* p) {
    ASTNode* left = parseUnary(p);
    
    while (match(p, TOK_MULTIPLY) || match(p, TOK_DIVIDE) || match(p, TOK_MODULO)) {
        ASTNode* node = createNode(NODE_BINARY_OP);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        node->left = left;
        node->right = parseUnary(p);
        left = node;
    }
    return left;
}

ASTNode* parseAdditive(Parser* p) {
    ASTNode* left = parseMultiplicative(p);
    
    while (match(p, TOK_PLUS) || match(p, TOK_MINUS)) {
        ASTNode* node = createNode(NODE_BINARY_OP);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        node->left = left;
        node->right = parseMultiplicative(p);
        left = node;
    }
    return left;
}

ASTNode* parseRelational(Parser* p) {
    ASTNode* left = parseAdditive(p);
    
    while (match(p, TOK_LT) || match(p, TOK_LE) || match(p, TOK_GT) || match(p, TOK_GE)) {
        ASTNode* node = createNode(NODE_BINARY_OP);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        node->left = left;
        node->right = parseAdditive(p);
        left = node;
    }
    return left;
}

ASTNode* parseEquality(Parser* p) {
    ASTNode* left = parseRelational(p);
    
    while (match(p, TOK_EQ) || match(p, TOK_NE)) {
        ASTNode* node = createNode(NODE_BINARY_OP);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        node->left = left;
        node->right = parseRelational(p);
        left = node;
    }
    return left;
}

ASTNode* parseLogicalAnd(Parser* p) {
    ASTNode* left = parseEquality(p);
    
    while (match(p, TOK_AND)) {
        ASTNode* node = createNode(NODE_BINARY_OP);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        node->left = left;
        node->right = parseEquality(p);
        left = node;
    }
    return left;
}

ASTNode* parseLogicalOr(Parser* p) {
    ASTNode* left = parseLogicalAnd(p);
    
    while (match(p, TOK_OR)) {
        ASTNode* node = createNode(NODE_BINARY_OP);
        strcpy(node->value, p->currentToken.value);
        advance(p);
        node->left = left;
        node->right = parseLogicalAnd(p);
        left = node;
    }
    return left;
}

ASTNode* parseExpression(Parser* p) {
    return parseLogicalOr(p);
}

ASTNode* parseVarDecl(Parser* p) {
    ASTNode* node = createNode(NODE_VAR_DECL);
    node->dataType = p->currentToken.type;
    advance(p);
    
    strcpy(node->value, p->currentToken.value);
    expect(p, TOK_IDENTIFIER);
    
    if (match(p, TOK_ASSIGN)) {
        advance(p);
        node->right = parseExpression(p);
    }
    
    expect(p, TOK_SEMICOLON);
    return node;
}

ASTNode* parseBlock(Parser* p) {
    ASTNode* node = createNode(NODE_BLOCK);
    expect(p, TOK_LBRACE);
    
    while (!match(p, TOK_RBRACE) && !match(p, TOK_EOF)) {
        addChild(node, parseStatement(p));
    }
    
    expect(p, TOK_RBRACE);
    return node;
}

ASTNode* parseIf(Parser* p) {
    ASTNode* node = createNode(NODE_IF);
    advance(p);
    expect(p, TOK_LPAREN);
    node->left = parseExpression(p);
    expect(p, TOK_RPAREN);
    node->right = parseStatement(p);
    
    if (match(p, TOK_ELSE)) {
        advance(p);
        node->extra = parseStatement(p);
    }
    return node;
}

ASTNode* parseWhile(Parser* p) {
    ASTNode* node = createNode(NODE_WHILE);
    advance(p);
    expect(p, TOK_LPAREN);
    node->left = parseExpression(p);
    expect(p, TOK_RPAREN);
    node->right = parseStatement(p);
    return node;
}

ASTNode* parseFor(Parser* p) {
    ASTNode* node = createNode(NODE_FOR);
    advance(p);
    expect(p, TOK_LPAREN);
    
    if (!match(p, TOK_SEMICOLON)) {
        node->left = parseStatement(p);
    } else {
        advance(p);
    }
    
    if (!match(p, TOK_SEMICOLON)) {
        node->right = parseExpression(p);
    }
    expect(p, TOK_SEMICOLON);
    
    if (!match(p, TOK_RPAREN)) {
        node->extra = parseExpression(p);
    }
    expect(p, TOK_RPAREN);
    
    ASTNode* body = parseStatement(p);
    addChild(node, body);
    return node;
}

ASTNode* parseReturn(Parser* p) {
    ASTNode* node = createNode(NODE_RETURN);
    advance(p);
    
    if (!match(p, TOK_SEMICOLON)) {
        node->left = parseExpression(p);
    }
    expect(p, TOK_SEMICOLON);
    return node;
}

ASTNode* parseStatement(Parser* p) {
    if (match(p, TOK_INT) || match(p, TOK_FLOAT) || match(p, TOK_CHAR)) {
        return parseVarDecl(p);
    }
    if (match(p, TOK_LBRACE)) {
        return parseBlock(p);
    }
    if (match(p, TOK_IF)) {
        return parseIf(p);
    }
    if (match(p, TOK_WHILE)) {
        return parseWhile(p);
    }
    if (match(p, TOK_FOR)) {
        return parseFor(p);
    }
    if (match(p, TOK_RETURN)) {
        return parseReturn(p);
    }
    
    // Assignment or expression statement
    ASTNode* expr = parseExpression(p);
    
    if (match(p, TOK_ASSIGN)) {
        ASTNode* node = createNode(NODE_ASSIGN);
        node->left = expr;
        advance(p);
        node->right = parseExpression(p);
        expect(p, TOK_SEMICOLON);
        return node;
    }
    
    expect(p, TOK_SEMICOLON);
    return expr;
}

ASTNode* parseFunction(Parser* p) {
    ASTNode* node = createNode(NODE_FUNCTION);
    node->dataType = p->currentToken.type;
    advance(p);
    
    strcpy(node->value, p->currentToken.value);
    expect(p, TOK_IDENTIFIER);
    expect(p, TOK_LPAREN);
    
    while (!match(p, TOK_RPAREN) && !match(p, TOK_EOF)) {
        ASTNode* param = createNode(NODE_VAR_DECL);
        param->dataType = p->currentToken.type;
        advance(p);
        strcpy(param->value, p->currentToken.value);
        expect(p, TOK_IDENTIFIER);
        addChild(node, param);
        
        if (match(p, TOK_COMMA)) advance(p);
    }
    
    expect(p, TOK_RPAREN);
    node->left = parseBlock(p);
    return node;
}

ASTNode* parseProgram(Parser* p) {
    ASTNode* root = createNode(NODE_PROGRAM);
    
    while (!match(p, TOK_EOF)) {
        addChild(root, parseFunction(p));
    }
    
    return root;
}

// ==================== SEMANTIC ANALYZER ====================

typedef struct {
    char name[256];
    TokenType type;
} Symbol;

typedef struct {
    Symbol symbols[100];
    int count;
} SymbolTable;

SymbolTable symbolTable;

void initSymbolTable() {
    symbolTable.count = 0;
}

void addSymbol(const char* name, TokenType type) {
    strcpy(symbolTable.symbols[symbolTable.count].name, name);
    symbolTable.symbols[symbolTable.count].type = type;
    symbolTable.count++;
}

Symbol* findSymbol(const char* name) {
    for (int i = 0; i < symbolTable.count; i++) {
        if (strcmp(symbolTable.symbols[i].name, name) == 0) {
            return &symbolTable.symbols[i];
        }
    }
    return NULL;
}

bool semanticAnalysis(ASTNode* node) {
    if (!node) return true;
    
    switch (node->type) {
        case NODE_VAR_DECL:
            if (findSymbol(node->value)) {
                printf("Semantic error: Variable '%s' already declared\n", node->value);
                return false;
            }
            addSymbol(node->value, node->dataType);
            if (node->right) semanticAnalysis(node->right);
            break;
            
        case NODE_IDENTIFIER:
            if (!findSymbol(node->value)) {
                printf("Semantic error: Undefined variable '%s'\n", node->value);
                return false;
            }
            break;
            
        case NODE_ASSIGN:
            if (node->left->type == NODE_IDENTIFIER) {
                if (!findSymbol(node->left->value)) {
                    printf("Semantic error: Undefined variable '%s'\n", node->left->value);
                    return false;
                }
            }
            semanticAnalysis(node->left);
            semanticAnalysis(node->right);
            break;
            
        case NODE_BINARY_OP:
        case NODE_UNARY_OP:
            semanticAnalysis(node->left);
            semanticAnalysis(node->right);
            break;
            
        case NODE_IF:
        case NODE_WHILE:
            semanticAnalysis(node->left);
            semanticAnalysis(node->right);
            semanticAnalysis(node->extra);
            break;
            
        case NODE_FOR:
            semanticAnalysis(node->left);
            semanticAnalysis(node->right);
            semanticAnalysis(node->extra);
            for (int i = 0; i < node->childCount; i++) {
                semanticAnalysis(node->children[i]);
            }
            break;
            
        case NODE_RETURN:
        case NODE_BLOCK:
        case NODE_FUNCTION:
        case NODE_PROGRAM:
            for (int i = 0; i < node->childCount; i++) {
                semanticAnalysis(node->children[i]);
            }
            semanticAnalysis(node->left);
            semanticAnalysis(node->right);
            break;
            
        default:
            break;
    }
    
    return true;
}

// ==================== IR CODE GENERATOR ====================

int tempCount = 0;
int labelCount = 0;

char* newTemp() {
    char* temp = (char*)malloc(20);
    sprintf(temp, "t%d", tempCount++);
    return temp;
}

char* newLabel() {
    char* label = (char*)malloc(20);
    sprintf(label, "L%d", labelCount++);
    return label;
}

void generateIR(ASTNode* node, FILE *fptr) {
    if (!node) return;
    
    switch (node->type) {
        case NODE_PROGRAM:
            for (int i = 0; i < node->childCount; i++) {
                generateIR(node->children[i], fptr);
            }
            break;
            
        case NODE_FUNCTION:
            fprintf(fptr, "FUNCTION %s:\n", node->value);
            for (int i = 0; i < node->childCount; i++) {
                fprintf(fptr, "  PARAM %s\n", node->children[i]->value);
            }
            generateIR(node->left, fptr);
            fprintf(fptr, "END FUNCTION %s\n\n", node->value);
            break;
            
        case NODE_BLOCK:
            for (int i = 0; i < node->childCount; i++) {
                generateIR(node->children[i], fptr);
            }
            break;
            
        case NODE_VAR_DECL:
            fprintf(fptr, "  DECLARE %s\n", node->value);
            if (node->right) {
                generateIR(node->right, fptr);
                char* temp = newTemp();
                fprintf(fptr, "  %s = %s\n", node->value, temp);
            }
            break;
            
        case NODE_ASSIGN:
            generateIR(node->right, fptr);
            char* assignTemp = newTemp();
            fprintf(fptr, "  %s = %s\n", node->left->value, assignTemp);
            break;
            
        case NODE_BINARY_OP: {
            generateIR(node->left, fptr);
            char* leftTemp = newTemp();
            generateIR(node->right, fptr);
            char* rightTemp = newTemp();
            char* resultTemp = newTemp();
            fprintf(fptr, "  %s = %s %s %s\n", resultTemp, leftTemp, node->value, rightTemp);
            break;
        }
            
        case NODE_UNARY_OP: {
            generateIR(node->left, fptr);
            char* operandTemp = newTemp();
            char* resultTemp = newTemp();
            fprintf(fptr, "  %s = %s%s\n", resultTemp, node->value, operandTemp);
            break;
        }
            
        case NODE_IF: {
            generateIR(node->left, fptr);
            char* condTemp = newTemp();
            char* elseLabel = newLabel();
            char* endLabel = newLabel();
            
            fprintf(fptr, "  IF_FALSE %s GOTO %s\n", condTemp, elseLabel);
            generateIR(node->right, fptr);
            fprintf(fptr, "  GOTO %s\n", endLabel);
            fprintf(fptr, "%s:\n", elseLabel);
            if (node->extra) generateIR(node->extra, fptr);
            fprintf(fptr, "%s:\n", endLabel);
            break;
        }
            
        case NODE_WHILE: {
            char* startLabel = newLabel();
            char* endLabel = newLabel();
            
            fprintf(fptr, "%s:\n", startLabel);
            generateIR(node->left, fptr);
            char* condTemp = newTemp();
            fprintf(fptr, "  IF_FALSE %s GOTO %s\n", condTemp, endLabel);
            generateIR(node->right, fptr);
            fprintf(fptr, "  GOTO %s\n", startLabel);
            fprintf(fptr, "%s:\n", endLabel);
            break;
        }
            
        case NODE_FOR: {
            char* startLabel = newLabel();
            char* endLabel = newLabel();
            
            generateIR(node->left, fptr);
            fprintf(fptr, "%s:\n", startLabel);
            if (node->right) {
                generateIR(node->right, fptr);
                char* condTemp = newTemp();
                fprintf(fptr, "  IF_FALSE %s GOTO %s\n", condTemp, endLabel);
            }
            for (int i = 0; i < node->childCount; i++) {
                generateIR(node->children[i], fptr);
            }
            if (node->extra) generateIR(node->extra, fptr);
            fprintf(fptr, "  GOTO %s\n", startLabel);
            fprintf(fptr, "%s:\n", endLabel);
            break;
        }
            
        case NODE_RETURN:
            if (node->left) {
                generateIR(node->left, fptr);
                char* retTemp = newTemp();
                fprintf(fptr, "  RETURN %s\n", retTemp);
            } else {
                fprintf(fptr, "  RETURN\n");
            }
            break;
            
        case NODE_CALL: {
            for (int i = 0; i < node->childCount; i++) {
                generateIR(node->children[i], fptr);
                char* argTemp = newTemp();
                fprintf(fptr, "  PUSH_PARAM %s\n", argTemp);
            }
            char* callTemp = newTemp();
            fprintf(fptr, "  %s = CALL %s, %d\n", callTemp, node->value, node->childCount);
            break;
        }
            
        case NODE_IDENTIFIER:
            fprintf(fptr, "  %s = %s\n", newTemp(), node->value);
            break;
            
        case NODE_INTEGER:
        case NODE_FLOAT:
        case NODE_CHAR:
        case NODE_STRING:
            fprintf(fptr, "  %s = %s\n", newTemp(), node->value);
            break;
            
        default:
            break;
    }
}


// ==================== MAIN ====================

int main() {
    FILE *fptr = fopen("source.c", "r");
    if (fptr == NULL){
        printf("Error opening source\n");
        return 1;
    }

    fseek(fptr, 0, SEEK_END);
    long filesize = ftell(fptr); 
    rewind(fptr);  

    char *source = (char *)malloc(sizeof(char) * (filesize + 1));
    if (source == NULL) {
        printf("Memory allocation failed!\n");
        fclose(fptr);
        return 1;
    }

    // Read entire file into buffer
    fread(source, 1, filesize, fptr);
    source[filesize] = '\0';  // Null-terminate the string

    fclose(fptr);
    
    printf("SOURCE CODE:\n");
    printf("============\n%s\n", source);
    
    // Phase 1: Lexical Analysis
    printf("\n=== PHASE 1: LEXICAL ANALYSIS ===\n\n");
    Lexer lexer;
    initLexer(&lexer, source);
    
    Token tok;
    printf("%-15s %-20s Line:Col\n", "Token Type", "Value");
    printf("------------------------------------------------\n");
    do {
        tok = getNextToken(&lexer);
        if (tok.type != TOK_EOF) {
            printf("%-15d %-20s %d:%d\n", tok.type, tok.value, tok.line, tok.column);
        }
    } while (tok.type != TOK_EOF);
    
    // Phase 2: Parsing
    printf("\n=== PHASE 2: SYNTAX ANALYSIS (PARSING) ===\n\n");
    initLexer(&lexer, source);
    Parser parser;
    parser.lexer = &lexer;
    advance(&parser);
    
    ASTNode* ast = parseProgram(&parser);
    printf("Abstract Syntax Tree created successfully!\n");
    printf("Root node type: PROGRAM\n");
    printf("Number of functions: %d\n", ast->childCount);
    
    // Phase 3: Semantic Analysis
    printf("\n=== PHASE 3: SEMANTIC ANALYSIS ===\n\n");
    initSymbolTable();
    
    if (semanticAnalysis(ast)) {
        printf("Semantic analysis completed successfully!\n");
        printf("\nSymbol Table:\n");
        printf("%-20s %-15s\n", "Symbol", "Type");
        printf("-------------------------------------\n");
        for (int i = 0; i < symbolTable.count; i++) {
            const char* typeStr = "unknown";
            switch (symbolTable.symbols[i].type) {
                case TOK_INT: typeStr = "int"; break;
                case TOK_FLOAT: typeStr = "float"; break;
                case TOK_CHAR: typeStr = "char"; break;
                case TOK_VOID: typeStr = "void"; break;
                default: break;
            }
            printf("%-20s %-15s\n", symbolTable.symbols[i].name, typeStr);
        }
    } else {
        printf("Semantic analysis failed!\n");
        return 1;
    }
    
    // Phase 4: Intermediate Code Generation
    printf("\n=== PHASE 4: INTERMEDIATE CODE GENERATION ===\n");
    tempCount = 0;
    labelCount = 0;
    FILE *ofptr = fopen("IR.txt", "w");
    if (ofptr == NULL){
        printf("Error opening file\n");
        return 1;
    }
    generateIR(ast, ofptr);
    
    printf("\n=== COMPILATION COMPLETED SUCCESSFULLY ===\n");
    printf("\nStatistics:\n");
    printf("  Temporary variables used: %d\n", tempCount);
    printf("  Labels generated: %d\n", labelCount);
    printf("  Symbols in table: %d\n", symbolTable.count);
    
    return 0;
}