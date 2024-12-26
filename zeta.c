#include <stdio.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>
#include <stdarg.h>

/*
###############################################################################
#                                                                             #
#  UTILS                                                                       #
#                                                                             #
###############################################################################
*/

// Maximum amount of Zeta code to interpret
#define CODE_MAX 1024 * 4 

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

typedef struct {
    size_t elCount;  // Number of elements currently in the array
    size_t capacity; // Allocated capacity of the array
    size_t elementSize; // Size of each element in bytes
    char ptr data;      // Pointer to the array data
} darray;

// Function prototypes
darray ptr darray_create(size_t elementSize);
void darray_destroy(darray ptr array);
void darray_add(darray ptr array, void ptr element);
void ptr darray_get(darray ptr array, size_t index);
void darray_resize(darray ptr array, size_t newCapacity);

// Create a new dynamic array
darray ptr _darray_create(size_t elementSize) {
    darray ptr array = (darray ptr)malloc(sizeof(darray));
    array->elCount = 0;
    array->capacity = 4; // Initial capacity
    array->elementSize = elementSize;
    array->data = malloc(array->capacity * elementSize);
    return array;
}

#define darray_create(type) _darray_create(sizeof(type))

// Free the memory used by the dynamic array
void darray_destroy(darray ptr array) {
    free(array->data);
    free(array);
}

// Add an element to the dynamic array
void darray_add(darray ptr array, void ptr element) {
    // Resize if capacity is reached
    if (array->elCount >= array->capacity) {
        darray_resize(array, array->capacity * 2);
    }

    // Copy the element into the array
    void ptr target = (char ptr)array->data + (array->elCount * array->elementSize);
    memcpy(target, element, array->elementSize);
    array->elCount++;
}

// Get an element by index
void ptr darray_get(darray ptr array, size_t index) {
    if (index >= array->elCount) {
        error("Index out of bounds");
        return NULL;
    }
    return (char ptr)array->data + (index * array->elementSize);
}

// Resize the dynamic array to a new capacity
void darray_resize(darray ptr array, size_t newCapacity) {
    array->data = realloc(array->data, newCapacity * array->elementSize);
    array->capacity = newCapacity;
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
    SEMI,
    PLUS, 
    MINUS, 
    MUL, 
    DIV, 
    LPAREN, 
    RPAREN,
    NUMBER,
    ASSIGN,
    ID, 
    EOL_TOKEN,
    EOF_TOKEN
} TokenType;

// Maping TokenType to Strings
const char ptr TokenType_ToString(TokenType tokenType) {
    switch (tokenType) {
        case ID: return "ID";
        case ASSIGN: return "ASSIGN";
        case NUMBER: return "NUMBER";
        case LPAREN: return "LPAREN";
        case RPAREN: return "RPAREN";
        case MUL: return "MUL"; 
        case DIV: return "DIV"; 
        case PLUS: return "PLUS";
        case MINUS: return "MINUS";
        case SEMI: return "SEMI";
        case EOL_TOKEN: return "EOL_TOKEN";
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
    FILE ptr file;
    char ptr line;
    size_t line_len;
    size_t pos; // col
    size_t row;
    char current_char;
} Lexer;

Lexer Lexer_Init(FILE ptr line);
void advance(Lexer ptr lexer);
void skip_whitespace(Lexer ptr lexer);
Token number(Lexer ptr lexer);
Token get_next_token(Lexer ptr lexer);

// Create Lexer Type
Lexer Lexer_Init(FILE ptr file){
    char *line = (char *)malloc(CODE_MAX);
    if (!line) {
        fprintf(stderr, "Failed to allocate memory for buffer\n");
        exit(EXIT_FAILURE);
    }
    fgets(line, CODE_MAX, file);
    return (Lexer){
        .file = file,
        .line = line,
        .current_char = line[0],
        .line_len = strlen(line),
        .pos = 0,
        .row = 0,
    };
}

// Advance the position in the lexer
void advance(Lexer ptr lexer) {
    lexer->pos++;
    lexer->current_char = (lexer->pos < lexer->line_len) 
        ? lexer->line[lexer->pos] 
        : '\0';
}

// Checking for the next char without advancing
char peek(Lexer ptr lexer, size_t offset) {
    size_t next_pos = lexer->pos + offset;
    return (next_pos < lexer->line_len) ? lexer->line[next_pos] : '\0';
}

// Skip whitespace characters
void skip_whitespace(Lexer ptr lexer) {
    while (lexer->current_char != '\0' && isspace(lexer->current_char)) {
        advance(lexer);
    }
}

// Parse identifier from input
Token identifier(Lexer ptr lexer){
    Token token = {.type = ID};
    int i = 0;
    do
    {
        if (i >= 31) error("Too many characters in variable name");
        token.value[i++] = lexer->current_char;
        advance(lexer);
    }
    while (lexer->current_char != NULL && isalnum(lexer->current_char));
    return token;
}

// Parse number from input
Token number(Lexer ptr lexer) {
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
    Token token = (Token){.type = NUMBER};
    strcpy(token.value, result);
    return token;
}

// Get next token from input
Token get_next_token(Lexer ptr lexer) {
    while (lexer->current_char != '\n' && lexer->current_char != '\0') {

        // Skip whitespace
        if (isspace(lexer->current_char)) {
            skip_whitespace(lexer);
            continue;
        }
        
        if (isalpha(lexer->current_char)){
            return identifier(lexer);
        }

        // Number tokens
        if (isdigit(lexer->current_char) || lexer->current_char == '.') {
            return number(lexer);
        }
        
        // Single-character tokens
        switch (lexer->current_char) {
            case ';': 
                advance(lexer);
                return (Token){SEMI, ";"};
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
            case '=': 
                advance(lexer);
                return (Token){ASSIGN, "="};
            default:
                error("Invalid character %c at [%zu:%zu]", lexer->current_char, lexer->row, lexer->pos);
        }
    }
        
    // Read input
    if (fgets(lexer->line, CODE_MAX, lexer->file) == NULL) {
        return (Token){EOF_TOKEN, "EOF"};
    }

    lexer->row++;
    lexer->pos=0;
    lexer->current_char = lexer->line[0];
    // End of input
    return (Token){EOL_TOKEN, "EOL"};
}

/*
###############################################################################
#                                                                             #
#  PARSER                                                                     #
#                                                                             #
###############################################################################
*/

// parser structure
typedef struct {
    Lexer ptr lexer;
    Token current_token;
} Parser;

// Consume expected token
void eat(Parser ptr parser, unsigned int count, TokenType types[]) {
    int matched = 0;

    // Check if the current token matches any of the expected types
    for (unsigned int i = 0; i < count; i++) {
        if (parser->current_token.type == types[i]) {
            matched = 1;
            break;
        }
    }

    if (matched) {
        parser->current_token = get_next_token(parser->lexer); // Consume token
    } else {
        error("Invalid syntax"); // Handle unexpected token
    }
}

typedef struct Ast Ast;
Ast ptr Ast_Var_Init(Token num);
Ast ptr Ast_Assign_Init(Ast ptr left, Token op, Ast ptr right);
Ast ptr Ast_BinOp_Init(Ast ptr left, Token op, Ast ptr right);
Ast ptr Ast_Num_Init(Token num);
Ast ptr Ast_Unary_Init(Token num, Ast ptr expr);
Ast ptr Ast_Compound_Init(darray ptr list);
Ast ptr Ast_NoOp_Init();


// Ast Types
typedef enum  {
    AST_VAR,
    AST_ASSIGN,
    AST_UNARY,
    AST_BINOP,
    AST_NUM,
    AST_COMPOUND,
    AST_NoOp
}AstType;

// Generic Ast class
struct Ast
{
    AstType type;
    union
    {   
        // For Compound
        struct {darray ptr childrend;};
        // For Unary Operartor
        struct {Ast ptr expr; /*Token op;*/};
        // For Binary and Assign Operators
        struct {Ast ptr left; Token op; Ast ptr right;};
        // For Numbers and Var
        struct {Num value; Token token;};
    };
};


// For creating Ast for Unary Operators
Ast ptr Ast_Unary_Init(Token num, Ast ptr expr){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_UNARY, .expr = expr};
    ast->op = num;
    return ast;
}

// For creating Ast for Assign Operators
Ast ptr Ast_Assign_Init(Ast ptr left, Token op, Ast ptr right){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_ASSIGN,.left = left, .op = op, .right = right};
    return ast;
}

// For creating Ast for Variables
Ast ptr Ast_Var_Init(Token token){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_VAR, .token = token};
    //memcpy(ref ast->token, ref token, sizeof(Token));
    return ast;
}

// For creating Ast for Binary Operators
Ast ptr Ast_BinOp_Init(Ast ptr left, Token op, Ast ptr right){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_BINOP,.left = left, .op = op, .right = right};
    return ast;
}

// For creating Ast for Numbers
Ast ptr Ast_Num_Init(Token num){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_NUM, .token = num, .value = atof(num.value)};
    return ast;
}

// For creating Ast for No Operations
Ast ptr Ast_NoOp_Init(){
    Ast ptr ast = malloc(sizeof(Ast));
    *ast = (Ast){.type = AST_NoOp};
    return ast;
}

Ast ptr Ast_Compound_Init(darray ptr list){
    Ast ptr root = malloc(sizeof(Ast));
    root->type = AST_COMPOUND;
    root->childrend = list;    
    return root;
}


Parser Parser_Init(Lexer ptr lexer);
void eat(Parser ptr parser, unsigned int count, TokenType types[]);
Ast ptr factor(Parser ptr parser);
Ast ptr term(Parser ptr parser);
Ast ptr expr(Parser ptr parser);

Ast ptr program(Parser ptr parser);
Ast ptr compound_statment(Parser ptr parser);
darray ptr statement_list(Parser ptr parser);
Ast ptr statment(Parser ptr parser);

// Initalize the Parser class
Parser Parser_Init(Lexer ptr lexer)
{   
    return (Parser){
        .current_token = get_next_token(lexer),
        .lexer = lexer,
    };
}

Ast ptr variable(Parser ptr parser);
// Parse factor: NUMBER or (expr)
Ast ptr factor(Parser ptr parser) {
    Token token = parser->current_token;
    if (token.type == MINUS) {
        eat(parser, 1, (TokenType[]){MINUS});
        return Ast_Unary_Init(token, factor(parser));
    }else if(token.type == PLUS) {
        eat(parser, 1, (TokenType[]){PLUS});
        return Ast_Unary_Init(token, factor(parser));
    }else if (token.type == NUMBER) {
        eat(parser, 1, (TokenType[]){NUMBER});
        return Ast_Num_Init(token);
    }else if (token.type == LPAREN) {
        eat(parser, 1, (TokenType[]){LPAREN});
        Ast ptr node = expr(parser);
        eat(parser, 1, (TokenType[]){RPAREN});
        return node;
    }else{
        Ast ptr node = variable(parser);
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
            eat(parser, 1, (TokenType[]){MUL});
        } else if (token.type == DIV) {
            eat(parser, 1, (TokenType[]){DIV});
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
            eat(parser, 1, (TokenType[]){PLUS});
        } else if (token.type == MINUS) {
            eat(parser, 1, (TokenType[]){MINUS});
        }

        node = Ast_BinOp_Init(node, token, term(parser));
    }
    
    return node;
}

// Parse empty
Ast ptr empty(Parser ptr parser){
    if(parser->current_token.type != SEMI){
        eat(parser, 2, (TokenType[]){NUMBER, EOL_TOKEN});
    }
    return Ast_NoOp_Init();
}

// Parse varaible
Ast ptr variable(Parser ptr parser){
    Ast ptr node = Ast_Var_Init(parser->current_token);
    eat(parser, 1, (TokenType[]){ID});
    return node;
}

// Parse assignment statement
Ast ptr assignment_statement(Parser ptr parser){
    Ast ptr left = variable(parser);
    Token token = parser->current_token;
    eat(parser, 1, (TokenType[]){ASSIGN});
    Ast ptr right = expr(parser);
    Ast ptr node = Ast_Assign_Init(left, token, right);
    return node;
}

Ast ptr compound_statment(Parser ptr parser);

// Parse statement
Ast ptr statment(Parser ptr parser){
    Ast ptr node;
    if (parser->current_token.type == ID){
        node = assignment_statement(parser);
    }
    else{
        node = empty(parser);
    }
    return node;
}

// Parse statement list
darray ptr statement_list(Parser ptr parser){
    Ast ptr node = statment(parser);
    size_t col = parser->lexer->row;
    darray ptr results = darray_create(Ast ptr);
    darray_add(results, ref node);

    while (parser->current_token.type == SEMI)
    {
        eat(parser, 1, (TokenType[]){SEMI});
        node = statment(parser);
        darray_add(results, ref node);
    }

    //Ast ptr n = *(Ast ptr*)darray_get(results, 0); -> debug
    if (parser->current_token.type == ID && col == parser->lexer->row){
        error("Worng place for an ID");
    }
    return results;
}

//Parse statement
Ast ptr compound_statment(Parser ptr parser){
    Ast ptr root = Ast_Compound_Init(statement_list(parser));
    if(parser->current_token.type != ID){
        eat(parser, 2, (TokenType[]){EOL_TOKEN, EOF_TOKEN});
    }
    return root;
}

Ast ptr program(Parser ptr parser){
    Ast ptr node = compound_statment(parser);
    return node;
}


// Parse expression
Ast ptr parse(Parser ptr parser){
    Ast ptr node = program(parser);
    return node;
}

/*
###############################################################################
#                                                                             #
#  INTERPRETER                                                                #
#                                                                             #
###############################################################################
*/

// Variable: Pair<String, Num>
typedef struct {
    char ptr name;
    Num value;
} Variable;

// Varaible List: Vector<Variable>
typedef struct {
    Variable ptr vars;
    int count;
    int capacity;
} VariableTable;

// The Interpretert
typedef struct 
{
    Parser ptr parser;
    VariableTable vtable;
}Interpreter;

// Chnage the value of a variable in the varaible table
Num set_variable(Interpreter ptr interpreter, const char ptr name, Num value) {

    for (int i = 0; i < interpreter->vtable.count; i++) {
        if (strcmp(interpreter->vtable.vars[i].name, name) == 0) {
            interpreter->vtable.vars[i].value = value;
            return value;
        }
    }

    if (interpreter->vtable.count == interpreter->vtable.capacity) {
        interpreter->vtable.capacity = interpreter->vtable.capacity ? interpreter->vtable.capacity * 2 : 4;
        interpreter->vtable.vars = realloc(interpreter->vtable.vars, interpreter->vtable.capacity * sizeof(Variable));
        if (!interpreter->vtable.vars) {
            fprintf(stderr, "Memory allocation failed\n");
            exit(1);
        }
    }

    interpreter->vtable.vars[interpreter->vtable.count++] = (Variable){ strdup(name), value };
    return value;
}

// Get access to a variable in the variable in the variable table
Num get_variable(Interpreter ptr interpreter, const char ptr name) {
    for (int i = 0; i < interpreter->vtable.count; i++) {
        if (strcmp(interpreter->vtable.vars[i].name, name) == 0) {
            return interpreter->vtable.vars[i].value;
        }
    }
    fprintf(stderr, "Undefined variable: %s\n", name);
    exit(1);
    return 0;
}

// Destroy variable table
void free_variables(Interpreter ptr interpreter) {
    for (int i = 0; i < interpreter->vtable.count; i++) {
        free(interpreter->vtable.vars[i].name);
    }
    free(interpreter->vtable.vars);
}

// Function prototypes for the visitor

// Generic vist function
Num visit(Interpreter ptr interpreter, Ast ptr node);

// Visit assign operation node
Num visit_AssignOp(Interpreter ptr interpreter, Ast ptr node) {
    Num result = set_variable(interpreter, node->left->token.value, visit(interpreter, node->right));
    free(node);
    return result;
}

// Visit unary operation node
Num visit_UnaryOp(Interpreter ptr self, Ast ptr node) {
    TokenType op = node->op.type;
    Num result;
    if (op == PLUS){
        result = +visit(self, node->expr);
    }else if(op == MINUS){
        result = -visit(self, node->expr);
    }
    free(node);
    return result;
}

// Visit binary operation node
Num visit_BinOp(Interpreter ptr interpreter, Ast ptr node) {
    Num left = visit(interpreter, node->left);
    Num right = visit(interpreter, node->right);
    Num result;
    switch (node->op.type) {
        case PLUS:
            result = left + right;
            break;
        case MINUS:
            result = left - right;
            break;
        case MUL:
            result = left * right;
            break;
        case DIV:
            if (right == 0) {
                error("Division by zero");
            }
            result = left / right;
            break;
        default:
            free(node);
            error("Unknown operator");
            break;
    }
    free(node);
    return result;
}

// Visit number node
Num visit_Num(Interpreter ptr interpreter, Ast ptr node) {
    Num num =  node->value;
    free(node);
    return num;
}

// Visit variable node
Num visit_Var(Interpreter ptr interpreter, Ast ptr node) {
    Num num = get_variable(interpreter, node->token.value);
    free(node);
    return num;
}

// Vist compount node
void visit_Compound(Interpreter ptr interpreter, Ast ptr node){
    for (size_t i = 0; i < node->childrend->elCount; i++)
    {
        Ast ptr statement =  *(Ast ptr ptr)darray_get(node->childrend,  i);
        Num result = visit(interpreter, statement);
        if(statement->type != AST_NoOp){
            printf("%g ", result);
        }
    }
    darray_destroy(node->childrend);
    free(node);
}

// Vist no operation node
void visit_NoOp(Interpreter ptr interpreter, Ast ptr node){
    // nothing
    free(node);
}

// Generic visit function
Num visit(Interpreter ptr interpreter, Ast ptr node) {
    switch (node->type) {
        case AST_ASSIGN:
            return visit_AssignOp(interpreter, node);
        case AST_UNARY:
            return visit_UnaryOp(interpreter, node);
        case AST_BINOP:
            return visit_BinOp(interpreter, node);
        case AST_NUM:
            return visit_Num(interpreter, node);
        case AST_VAR:
            return visit_Var(interpreter, node);
        case AST_COMPOUND:
            visit_Compound(interpreter, node);
            break;
        case AST_NoOp:
            visit_NoOp(interpreter, node);
            break;
        default:
            error("No visit function for this node type");
    }
}

// Interpreter initialization
Interpreter Interpreter_Init(Parser ptr parser) {
    return (Interpreter){
        .parser = parser
    };
}

// Main interpret function
void interpret(Interpreter ptr interpreter) {
    while (interpreter->parser->current_token.type != EOF_TOKEN) 
    {
        Ast ptr tree = parse(interpreter->parser);
        visit(interpreter, tree);
    }
}

// Gets the fule/absolute path
char ptr get_full_path(const char* relative_path) {
    #ifdef _WIN32
        static char full_path[PATH_MAX];
        //_fullpath
        if (_fullpath(full_path, relative_path, sizeof(full_path)) == NULL) {
            perror("Error getting full path");
            return NULL;
        }
    #else
        static char full_path[MAX_PATH];
        //realpath
        if (realpath(relative_path, full_path) == NULL) {
            perror("Error getting full path");
            return NULL;
        }
    #endif

    return full_path;
}

// Check args for the file to interpret
FILE ptr parse_args(int argc, char ptr argv[]) {
    if (argc < 2) {
        error("zeta.exe: fatal error: no input files.\ncompilation terminated.\n");
        return NULL;
    }

    FILE ptr f = fopen(get_full_path(argv[1]), "r");
    if (!f) {
        error("Cannot find '%s': No such file or directory .\n", argv[1]);
        return NULL;
    }

    return f;
}

int main(int argc, char ptr argv[]) 
{
    FILE ptr file = parse_args(argc, argv);
    
    // Setup lexer and parser
    Lexer lexer = Lexer_Init(file);
    Parser parser = Parser_Init(ref lexer);
    Interpreter interpreter = Interpreter_Init(ref parser);

    // Evaluate and print result
    interpret(ref interpreter); 

    // Release resources
    free(lexer.line);
    fclose(lexer.file);
    free(interpreter.vtable.vars);

    return 0;
}