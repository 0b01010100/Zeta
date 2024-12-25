#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

// Maximum amount of Zeta code to interpret
#define CODE_MAX 1024 

// reference (just for code clarity)
#define ref &

// pointer (just for code clarity)
#define ptr *

// Represents Any Number
typedef double Num;

// Error handling
void error(const char ptr detail, ...) {
    va_list args;
    va_start(args, detail);
    vfprintf(stderr, detail, args);
    va_end(args);
    printf("\n");
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

// Maping TokenType to Strings
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
    size_t text_len;
    size_t pos;
    char current_char;
} Lexer;

Lexer Lexer_Init(char ptr text);
void advance(Lexer ptr lexer);
void skip_whitespace(Lexer ptr lexer);
Num number(Lexer ptr lexer);
Token get_next_token(Lexer ptr lexer);

// Create Lexer Type
Lexer Lexer_Init(char ptr text){
    return (Lexer){
        .current_char = text[0],
        .text_len = strlen(text),
        .pos = 0,
        .text = text
    };
}

// Advance the position in the lexer
void advance(Lexer ptr lexer) {
    lexer->pos++;
    lexer->current_char = (lexer->pos < lexer->text_len) 
        ? lexer->text[lexer->pos] 
        : '\0';
}

// Checking for the next char without advancing
char peek(Lexer ptr lexer, size_t offset) {
    size_t next_pos = lexer->pos + offset;
    return (next_pos < lexer->text_len) ? lexer->text[next_pos] : '\0';
}

// Skip whitespace characters
void skip_whitespace(Lexer ptr lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) {
        advance(lexer);
    }
}

// Parse number from input
Num number(Lexer ptr lexer) {
    char result[32];
    int i = 0;
    bool hasDot = false;
    bool hasE = false;

    do {
        // Check if we exceed the result buffer
        if (i >= 31) error("Too many digits in the number");
        
        if (lexer->current_char == '.') {
            // Handle decimal point
            if (hasDot) {
                error("Too many '.' in this number");
            }
            hasDot = true;
        } 
        else if (lexer->current_char == 'E' || lexer->current_char == 'e') {
            
            // Handle scientific notation
            if (hasE) {
                error("Too many 'E' or 'e' in this number");
            }
            hasE = true;
            
            result[i++] = lexer->current_char;
            advance(lexer); // Move past 'E' or 'e'

            // Check for optional '+' or '-'
            if (lexer->current_char == '+' || lexer->current_char == '-') {
                result[i++] = lexer->current_char; // Append sign to the result
                advance(lexer); // Move past the sign
            }

            // Ensure there is at least one digit in the exponent
            if (!isdigit(lexer->current_char)) {
                error("'E' or 'e' must be followed by a number");
            }
        }

        // Append current character to the result
        result[i++] = lexer->current_char;
        advance(lexer);
    } while (lexer->current_char != '\0' &&
             (isdigit(lexer->current_char) ||
              lexer->current_char == '.' ||
              lexer->current_char == 'E' || lexer->current_char == 'e'));

    result[i] = '\0'; // Null-terminate the string
    Num n = atof(result);
    return n;
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
        if (isdigit(lexer->current_char) || lexer->current_char == '.') {
            Token token = {INTEGER, {0}};
            sprintf(token.value, "%g", number(lexer));
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
Ast ptr Ast_Unary_Init(Token num, Ast ptr expr);

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

// Initalize the Parser class
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
    if (token.type == MINUS) {
        eat(parser, MINUS);
        return Ast_Unary_Init(token, factor(parser));
    }else if(token.type == PLUS) {
        eat(parser, PLUS);
        return Ast_Unary_Init(token, factor(parser));
    }
    else if (token.type == INTEGER) {
        eat(parser, INTEGER);
        return Ast_Num_Init(token);
    } else if (token.type == LPAREN) {
        eat(parser, LPAREN);
        Ast ptr node = expr(parser);
        eat(parser, RPAREN);
        return node;
    }

    error("Unexpected token %s", TokenType_ToString(token.type));
    return NULL; // <- May never reach (just to get rid of that warning though)
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

// Parse expression
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

// Ast Types
typedef enum  {
    AST_UNARY = 0,
    AST_BINOP,
    AST_NUM
}AstType;

// Generic Ast class
struct Ast
{
    AstType type;
    union
    {   
        // For Unary Operartor
        struct {Ast ptr expr; /*Token op;*/};
        // For Binary Operators
        struct {Ast ptr left; Token op; Ast ptr right;};
        // For Numbers
        struct {Num value; Token num;};
    };
};

// For creating Ast for Binary Operators
Ast ptr Ast_BinOp_Init(Ast ptr left, Token op, Ast ptr right){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_BINOP,.left = left, .op = op, .right = right};
    return ast;
}

// For creating Ast for Numbers
Ast ptr Ast_Num_Init(Token num){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_NUM, .num = num, .value = atof(num.value)};
    return ast;
}

// For creating Ast for Unary Operators
Ast ptr Ast_Unary_Init(Token num, Ast ptr expr){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_UNARY, .expr = expr};
    ast->op = num;
    return ast;
}

// The Interpretert
typedef struct 
{
    Parser ptr parser;
}Interpreter;

// Function prototypes for the visitor

Num visit(Interpreter ptr interpreter, Ast ptr node);
Num visit_BinOp(Interpreter ptr interpreter, Ast ptr node);
Num visit_Num(Interpreter ptr interpreter, Ast ptr node);
Num visit_UnaryOp(Interpreter ptr interpreter, Ast ptr node);

// Generic visit function
Num visit(Interpreter ptr interpreter, Ast ptr node) {
    switch (node->type) {
        case AST_UNARY:
            return visit_UnaryOp(interpreter, node);
        case AST_BINOP:
            return visit_BinOp(interpreter, node);
        case AST_NUM:
            return visit_Num(interpreter, node);
        default:
            error("No visit function for this node type");
    }
}

// Visit unary operation node
Num visit_UnaryOp(Interpreter ptr self, Ast ptr node) {
    TokenType op = node->op.type;
    if (op == PLUS){
        return +visit(self, node->expr);
    }else if(op == MINUS){
        return -visit(self, node->expr);
    }
    return 0; // <- May never reach (just to get rid of that warning though)
}

// Visit binary operation node
Num visit_BinOp(Interpreter ptr interpreter, Ast ptr node) {
    Num left = visit(interpreter, node->left);
    Num right = visit(interpreter, node->right);
    switch (node->op.type) {
        case PLUS:
            return left + right;
        case MINUS:
            return left - right;
        case MUL:
            return left * right;
        case DIV:
            if (right == 0) {
                error("Division by zero");
            }
            return left / right;
        default:
            error("Unknown operator");
    }
}

// Visit number node
Num visit_Num(Interpreter ptr interpreter, Ast ptr node) {
    return node->value;
}

// Interpreter initialization
Interpreter Interpreter_Init(Parser ptr parser) {
    return (Interpreter){
        .parser = parser
    };
}

// Main interpret function
Num interpret(Interpreter ptr interpreter, Ast ptr tree) {
    return visit(interpreter, tree);
}

// Entry Point
int main() 
{
    char input[CODE_MAX];
    
    while (true) 
    {
        printf(">>> ");
        
        // Read input
        if (fgets(input, sizeof(input), stdin) == NULL) {
            break;
        }
        
        // Remove newline and calculate length once
        size_t len = strlen(input);
        if (len > 0 && input[len - 1] == '\n') {
            input[--len] = '\0'; // Adjust length after removing newline
        }

        // Skip empty input
        if (len == 0) {
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