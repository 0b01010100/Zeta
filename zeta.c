#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

#define CODE_MAX 1024 
#define ref &
#define ptr *
typedef double Num;

// Error handling
void error(const char ptr detail, ...) {
    va_list args;
    va_start(args, detail);
    vfprintf(stderr, detail, args);
    va_end(args);

    exit(EXIT_FAILURE);
}

/*
###############################################################################
#                                                                             #
#  LEXER                                                                      #
#                                                                             #
###############################################################################
*/

// Token types
typedef enum {
    INTEGER, 
    PLUS, 
    MINUS, 
    MUL, 
    DIV, 
    LPAREN, 
    RPAREN, 
    EOF_TOKEN
} TokenType;

// TokenType ToString
const char ptr TokenType_ToString(TokenType tokenType) {
    switch (tokenType) {
        case INTEGER: return "INTEGER";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case MUL: return "MUL"; 
        case DIV: return "DIV"; 
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case EOF_TOKEN: return "EOF_TOKEN";
        default: return "UNKNOWN";
    }
}

// Token structure
typedef struct {
    TokenType type;
    char value[32];
} Token;

// Lexer structure
typedef struct {
    char ptr text;
    size_t pos;
    char current_char;
} Lexer;

Lexer Lexer_Init(char ptr text);
void advance(Lexer ptr lexer);
void skip_whitespace(Lexer ptr lexer);
Num number(Lexer ptr lexer);
Token get_next_token(Lexer ptr lexer);

Lexer Lexer_Init(char ptr text){
    return (Lexer){
        .current_char = text[0],
        .pos = 0,
        .text = text
    };
}

// Advance the position in the lexer
void advance(Lexer ptr lexer) {
    lexer->pos++;
    lexer->current_char = (lexer->pos < strlen(lexer->text)) 
        ? lexer->text[lexer->pos] 
        : '\0';
}

// Skip whitespace characters
void skip_whitespace(Lexer ptr lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) {
        advance(lexer);
    }
}

// Parse number from input
Num number(Lexer ptr lexer) {
    char result[32] = {0};
    int i = 0;
    
    while (lexer->current_char != '\0' && isdigit(lexer->current_char)) {
        if (i>=32) error("Number is is too long");
        result[i++] = lexer->current_char;
        advance(lexer);
    }
    
    return atof(result);
}

// Get next token from input
Token get_next_token(Lexer ptr lexer) {
    while (lexer->current_char != '\0') {
        // Skip whitespace
        if (isspace(lexer->current_char)) {
            skip_whitespace(lexer);
            continue;
        }
        
        // Parse integers
        if (isdigit(lexer->current_char)) {
            Token token = {INTEGER, {0}};
            sprintf(token.value, "%lf", number(lexer));
            return token;
        }
        
        // Single-character tokens
        switch (lexer->current_char) {
            case '+': 
                advance(lexer);
                return (Token){PLUS, "+"};
            case '-': 
                advance(lexer);
                return (Token){MINUS, "-"};
            case '*': 
                advance(lexer);
                return (Token){MUL, "*"};
            case '/': 
                advance(lexer);
                return (Token){DIV, "/"};
            case '(': 
                advance(lexer);
                return (Token){LPAREN, "("};
            case ')': 
                advance(lexer);
                return (Token){RPAREN, ")"};
            default:
                error("Invalid character %c", lexer->current_char);
        }
    }
    
    // End of input
    return (Token){EOF_TOKEN, ""};
}

/*
###############################################################################
#                                                                             #
#  PARSER                                                                     #
#                                                                             #
###############################################################################
*/

typedef struct Ast Ast;
Ast ptr Ast_BinOp_Init(Ast ptr left, Token op, Ast ptr right);
Ast ptr Ast_Num_Init(Token num);

// parser structure
typedef struct {
    Lexer ptr lexer;
    Token current_token;
} Parser;

Parser Parser_Init(Lexer ptr lexer);
void eat(Parser ptr parser, TokenType token_type);
Ast ptr factor(Parser ptr parser);
Ast ptr term(Parser ptr parser);
Ast ptr expr(Parser ptr parser);

Parser Parser_Init(Lexer ptr lexer)
{   
    return (Parser){
        .current_token = get_next_token(lexer),
        .lexer = lexer
    };
}

// Consume expected token
void eat(Parser ptr parser, TokenType token_type) {
    if (parser->current_token.type == token_type) {
        parser->current_token = get_next_token(parser->lexer);
    } else {
        error("Invalid syntax");
    }
}

// Parse factor: INTEGER or (expr)
Ast ptr factor(Parser ptr parser) {
    Token token = parser->current_token;
    
    if (token.type == INTEGER) {
        eat(parser, INTEGER);
        return Ast_Num_Init(token);
    } else if (token.type == LPAREN) {
        eat(parser, LPAREN);
        Ast ptr node = expr(parser);
        eat(parser, RPAREN);
        return node;
    }

    error("Unexpected token %s", TokenType_ToString(token.type));
    return NULL; // <- Will never reach
}

// Parse term: factor ((MUL | DIV) factor)*
Ast ptr term(Parser ptr parser) {
    Ast ptr node = factor(parser);
    
    while (parser->current_token.type == MUL || 
           parser->current_token.type == DIV) {
        Token token = parser->current_token;
        
        if (token.type == MUL) {
            eat(parser, MUL);
        } else if (token.type == DIV) {
            eat(parser, DIV);
        }

        node = Ast_BinOp_Init(node, token, factor(parser));
    }
    
    return node;
}

// Parse expression: term ((PLUS | MINUS) term)*
Ast ptr expr(Parser ptr parser) {

    Ast ptr node = term(parser);
    
    while (parser->current_token.type == PLUS || 
           parser->current_token.type == MINUS) {
        Token token = parser->current_token;
        
        if (token.type == PLUS) {
            eat(parser, PLUS);
        } else if (token.type == MINUS) {
            eat(parser, MINUS);
        }

        node = Ast_BinOp_Init(node, token, term(parser));
    }
    
    return node;
}

Ast ptr parse(Parser ptr parser){
    return expr(parser);
}

/*
###############################################################################
#                                                                             #
#  INTERPRETER                                                                #
#                                                                             #
###############################################################################
*/

typedef enum  {
    AST_BINOP = 0,
    AST_NUM
}AstType;

struct Ast
{
    AstType type;
    union
    {
        // For Operators
        struct {Ast* left; Token op; Ast* right;};
        // For Numbers
        struct {Num value; Token num;};
    };
};

Ast ptr Ast_BinOp_Init(Ast ptr left, Token op, Ast ptr right){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_BINOP,.left = left, .op = op, .right = right};
    return ast;
}

Ast ptr Ast_Num_Init(Token num){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_NUM, .num = num, .value = atof(num.value)};
    return ast;
}

typedef struct 
{
    Parser* parser;
}Interpreter;

// Function prototypes for the visitor

double visit(Interpreter* interpreter, Ast* node);
double visit_BinOp(Interpreter* interpreter, Ast* node);
double visit_Num(Interpreter* interpreter, Ast* node);

// Generic visit function
double visit(Interpreter* interpreter, Ast* node) {
    switch (node->type) {
        case AST_BINOP:
            return visit_BinOp(interpreter, node);
        case AST_NUM:
            return visit_Num(interpreter, node);
        default:
            fprintf(stderr, "No visit function for this node type\n");
            exit(1);
    }
}

// Visit binary operation node
double visit_BinOp(Interpreter* interpreter, Ast* node) {
    double left = visit(interpreter, node->left);
    double right = visit(interpreter, node->right);
    switch (node->op.type) {
        case PLUS:
            return left + right;
        case MINUS:
            return left - right;
        case MUL:
            return left * right;
        case DIV:
            if (right == 0) {
                fprintf(stderr, "Division by zero\n");
                exit(1);
            }
            return left / right;
        default:
            fprintf(stderr, "Unknown operator\n");
            exit(1);
    }
}

// Visit number node
double visit_Num(Interpreter* interpreter, Ast* node) {
    return node->value;
}

// Interpreter initialization
Interpreter Interpreter_Init(void* parser) {
    return (Interpreter){
        .parser = parser
    };
}

// Main interpret function
double interpret(Interpreter* interpreter, Ast* tree) {
    return visit(interpreter, tree);
}

int main() {
    char input[CODE_MAX];
    
    while (true) 
    {
        printf(">>> ");
        
        // Read input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline
        size_t len = strlen(input);
        if (len > 0 && input[len-1] == '\n') {
            input[len-1] = '\0';
        }
        
        // Skip empty input
        if (strlen(input) == 0) {
            continue;
        }
        
        // Exit condition
        if (strcmp(input, "exit") == 0) {
            printf("Exiting...\n");
            break;
        }
        
        // Setup lexer and parser
        Lexer lexer = Lexer_Init(input);
        Parser parser = Parser_Init(ref lexer);
        Interpreter interpreter = Interpreter_Init(ref parser);
        Ast ptr tree = parse(ref parser);
        // Evaluate and print result
        Num result = interpret(ref interpreter, tree);
        printf("%g\n", result);
    }
    
    return 0;
}