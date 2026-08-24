#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "../include/parser.h"

/* 
 * Helper function to create a new AST node
 */
ASTNode* create_node(ASTNodeType type, int value, const char* name, ASTNode* left, ASTNode* right) {
    ASTNode* node = (ASTNode*)malloc(sizeof(ASTNode));
    node->type = type;
    node->value = value;
    if (name != NULL) {
        strncpy(node->name, name, 31);
        node->name[31] = '\0';
    } else {
        node->name[0] = '\0';
    }
    node->left = left;
    node->right = right;
    return node;
}

/* Forward declarations for recursive-descent expression parsing.
 * The grammar now has three precedence levels:
 *   expression := term (('+' | '-') term)*      (lowest precedence, left-assoc)
 *   term       := factor (('*' | '/') factor)*  (higher precedence, left-assoc)
 *   factor     := NUMBER | IDENTIFIER | '(' expression ')'
 */
ASTNode* parse_expression(TokenList* tokens, int* pos);
ASTNode* parse_term(TokenList* tokens, int* pos);
ASTNode* parse_factor(TokenList* tokens, int* pos);

/* 
 * Parses a statement (variable assignment, print, etc.)
 */
ASTNode* parse_statement(TokenList* tokens, int* pos) {
    Token current = tokens->tokens[*pos];

    if (current.type == T_LET) { // if statement starts with 'let' -> e.g. let x = 5 + 3;
        (*pos)++; // move past 'let' keyword
        Token var = tokens->tokens[*pos]; // capture the variable name ('x')
        (*pos)++; // move past variable token

        if (tokens->tokens[*pos].type != T_EQUAL) { // check for '='
            printf("Syntax error: expected '=' at pos=%d\n", *pos);
            exit(1);
        }
        (*pos)++; // skip '='

        /* 
         * parse the expression "5 + 3" 
         * returns AST_BINARY_OP(+) with left = AST_NUMBER(5), right = AST_NUMBER(3)
         */
        ASTNode* expr = parse_expression(tokens, pos);

        if (tokens->tokens[*pos].type != T_SEMICOLON) { // expect ';' at the end of the statement
            printf("Syntax error: expected ';' at pos=%d\n", *pos);
            exit(1);
        }
        (*pos)++; // skip ';'

        /*
         * Resulting AST structure for "let x = 5 + 3;"
         * AST_ASSIGN(x)
         *   └── AST_BINARY_OP(+)
         *        ├── AST_NUMBER(5)
         *        └── AST_NUMBER(3)
         *
         * - The assigned variable (x) is stored in the 'name' field of the AST_ASSIGN node.
         * - The computed expression (5 + 3) is the left child of the assignment node.
         * - The right child is NULL because it is not needed for assignments.
         */
        return create_node(AST_ASSIGN, 0, var.name, expr, NULL);

    } else if (current.type == T_PRINT) { // if statement starts with 'print'
        (*pos)++; // move past 'print' keyword

        /*
         * parse the expression inside print, e.g. print(x); -> parses 'x'
         */
        ASTNode* expr = parse_expression(tokens, pos);

        if (tokens->tokens[*pos].type != T_SEMICOLON) { // expect ';' at the end of the statement
            printf("Syntax error: expected ';' at pos=%d\n", *pos);
            exit(1);
        }
        (*pos)++; // skip ';'

        /*
         * Resulting AST structure for "print(x);"
         * AST_PRINT
         *   └── AST_VAR(x)
         *
         * - The variable to print (i.e., x) is the left child of the AST_PRINT node.
         * - The right child is NULL because it is not needed for print statements.
         */
        return create_node(AST_PRINT, 0, NULL, expr, NULL);

    } else if (current.type == T_LPAREN) { // if statement starts with '(' -> e.g. print(x);
        (*pos)++; // skip '('
        ASTNode* expr = parse_expression(tokens, pos);
        if (tokens->tokens[*pos].type != T_RPAREN) {
            printf("Syntax error: expected ')' at pos=%d\n", *pos);
            exit(1);
        }
        (*pos)++; // skip ')'
        return expr;
    } else { // handles standalone expressions that are not 'let' or 'print' statements (for example: "5 + 3;" or "x;")
        /* 
         * The parser creates an AST node representing the expression itself.
         * The left child contains the expression (e.g., AST_BINARY_OP)
         */
        ASTNode* expr = parse_expression(tokens, pos);
        if (tokens->tokens[*pos].type == T_SEMICOLON) (*pos)++;
        return expr;
    }
}

/*
 * parse_factor: parses the smallest units of an expression —
 * numbers, variables, or a parenthesized sub-expression.
 * This is the highest-precedence level of the grammar.
 *
 * BUGFIX (individual improvement): previously this logic lived directly
 * inside parse_expression(), which combined ALL operators (+,-,*,/) in a
 * single loop and built the right-hand side with a full recursive call to
 * parse_expression(). That gave every operator the same precedence and made
 * every operator right-associative (e.g. "2*3+4" was parsed as 2*(3+4)=14,
 * and "10-3-2" was parsed as 10-(3-2)=9). Splitting into factor/term/expression
 * and using iterative loops fixes both problems.
 */
ASTNode* parse_factor(TokenList* tokens, int* pos) {
    Token current = tokens->tokens[*pos];
    ASTNode* node = NULL;

    if (current.type == T_NUMBER) { // e.g. '5'
        node = create_node(AST_NUMBER, current.value, NULL, NULL, NULL);
        (*pos)++;
    } else if (current.type == T_IDENTIFIER) { // e.g. 'x'
        node = create_node(AST_VAR, 0, current.name, NULL, NULL);
        (*pos)++;
    } else if (current.type == T_LPAREN) { // '(' EXPRESSION ')'
        (*pos)++; // skip '('

        // Parentheses reset precedence: whatever is inside is parsed as a
        // full expression (lowest precedence level) again.
        node = parse_expression(tokens, pos);

        if (tokens->tokens[*pos].type != T_RPAREN) {
            printf("Syntax error: expected ')' at pos=%d\n", *pos);
            exit(1);
        }
        (*pos)++; // skip ')'
    } else {
        printf("Syntax error: unexpected token at pos=%d\n", *pos);
        exit(1);
    }

    return node;
}

/*
 * parse_term: handles '*' and '/', which bind tighter than '+' and '-'.
 * term := factor (('*' | '/') factor)*
 * The loop (instead of recursion on the right-hand side) makes '*' and '/'
 * left-associative, e.g. "10 / 2 / 5" => (10 / 2) / 5 = 1.
 */
ASTNode* parse_term(TokenList* tokens, int* pos) {
    ASTNode* left = parse_factor(tokens, pos);
    Token current = tokens->tokens[*pos];

    while (current.type == T_MULT || current.type == T_DIV) {
        char op = (current.type == T_MULT) ? '*' : '/';
        (*pos)++;
        ASTNode* right = parse_factor(tokens, pos);
        left = create_node(AST_BIN_ARY_OP, op, NULL, left, right);
        current = tokens->tokens[*pos];
    }

    return left;
}

/*
 * parse_expression: handles '+' and '-', the lowest-precedence operators.
 * expression := term (('+' | '-') term)*
 * Same left-associative loop pattern as parse_term, but operating one
 * precedence level lower (on terms instead of factors).
 */
ASTNode* parse_expression(TokenList* tokens, int* pos) {
    ASTNode* left = parse_term(tokens, pos);
    Token current = tokens->tokens[*pos];

    while (current.type == T_PLUS || current.type == T_MINUS) {
        char op = (current.type == T_PLUS) ? '+' : '-';
        (*pos)++;
        ASTNode* right = parse_term(tokens, pos);
        left = create_node(AST_BINARY_OP, op, NULL, left, right);
        current = tokens->tokens[*pos];
    }

    return left;
}

/* 
 * Entry point: parses the entire token list into an AST
 */
ASTNode* parse(TokenList* tokens) {
    int pos = 0;
    ASTNode* root = NULL;

    /*
     * e.g. let x = 5 + 3; print(x);
     * first iteration: parses "let x = 5 + 3;"
     * second iteration: parses "print(x);"
     */
    while (tokens->tokens[pos].type != T_EOF) {
        /*
         * 'stmt' points to the AST node representing the current statement.
         * For example, with the code: let x = 5 + 3;
         *      stmt -> AST_ASSIGN(x)
         *                └── AST_BINARY_OP(+)
         *                     ├── AST_NUMBER(5)
         *                     └── AST_NUMBER(3)
         *  
         * If there are multiple statements (e.g. let x = 5 + 3; print(x);), 
         * each iteration of the while loop will parse the next one, and 'stmt' 
         * will be updated to point to the root node of that new statement's AST.
         * 
         * The variable 'pos' is updated inside parse_statement() to keep track 
         * of the current position in the token list, so the loop knows where 
         * each statement starts and ends.
         */
        ASTNode* stmt = parse_statement(tokens, &pos);

        if (root == NULL) {
            root = stmt;
        } else {
            // append statements in a right-skewed list
            ASTNode* temp = root;
            // the while loop runs through the nodes on the right until it finds the last node (the one with right == NULL).
            while (temp->right != NULL) temp = temp->right;
            // add the new statement to the end of the list
            temp->right = stmt; 
        }
    }

    return root;
}

/* 
 * Recursively prints the AST
 */
void print_ast(ASTNode* node, int indent) {
    if (node == NULL) return;

    for (int i = 0; i < indent; i++) printf("  ");

    switch (node->type) {
        case AST_NUMBER:
            printf("AST_NUMBER(%d)\n", node->value);
            break;
        case AST_VAR:
            printf("AST_VAR(%s)\n", node->name);
            break;
        case AST_BINARY_OP:
            printf("AST_BINARY_OP(%c)\n", node->value);
            print_ast(node->left, indent + 1);
            print_ast(node->right, indent + 1);
            break;
        case AST_ASSIGN:
            printf("AST_ASSIGN(%s)\n", node->name);
            print_ast(node->left, indent + 1);
            break;
        case AST_PRINT:
            printf("AST_PRINT\n");
            print_ast(node->left, indent + 1);
            break;
        default:
            printf("Unknown AST node\n");
    }
}
