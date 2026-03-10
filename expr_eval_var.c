/* expr_eval_var.c
   Infix -> Postfix -> Evaluate (with variable support)
   Supports:
   - Multi-digit integers
   - Variables (a-z, A-Z)
   - Operators: + - * / ^
   - Parentheses: ( )
   Compile: gcc expr_eval_var.c -o expr_eval_var -lm
*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define MAXN 1000
#define MAXTOK 500

// ---------- Helpers ----------
int isOperator(char c) {
    return c=='+'||c=='-'||c=='*'||c=='/'||c=='^';
}
int precedence(char op) {
    if (op=='+'||op=='-') return 1;
    if (op=='*'||op=='/') return 2;
    if (op=='^') return 3;
    return 0;
}

// ---------- Tokenization ----------
typedef enum {TOK_NUM, TOK_OP, TOK_LP, TOK_RP, TOK_VAR, TOK_INVALID} TokType;
typedef struct {
    TokType type;
    char str[64]; // number, operator, or variable name
} Token;

int tokenize(const char *s, Token toks[], int *tcount) {
    int i=0, n=strlen(s), ti=0;
    while (i<n) {
        if (isspace((unsigned char)s[i])) { i++; continue; }

        if (isdigit((unsigned char)s[i])) {
            int j=0;
            while (i<n && isdigit((unsigned char)s[i])) {
                toks[ti].str[j++]=s[i++];
            }
            toks[ti].str[j]=0;
            toks[ti].type = TOK_NUM;
            ti++;
            continue;
        }

        if (isalpha((unsigned char)s[i])) {
            int j=0;
            while (i<n && isalpha((unsigned char)s[i])) {
                toks[ti].str[j++]=s[i++];
            }
            toks[ti].str[j]=0;
            toks[ti].type = TOK_VAR;
            ti++;
            continue;
        }

        if (isOperator(s[i])) {
            toks[ti].type = TOK_OP;
            toks[ti].str[0]=s[i];
            toks[ti].str[1]=0;
            ti++; i++; continue;
        }

        if (s[i]=='(') { toks[ti].type = TOK_LP; toks[ti].str[0]='('; toks[ti].str[1]=0; ti++; i++; continue; }
        if (s[i]==')') { toks[ti].type = TOK_RP; toks[ti].str[0]=')'; toks[ti].str[1]=0; ti++; i++; continue; }

        toks[ti].type = TOK_INVALID;
        snprintf(toks[ti].str, sizeof(toks[ti].str), "ch:%c", s[i]);
        return -1;
    }
    *tcount = ti;
    return 0;
}

// ---------- Infix -> Postfix ----------
int infix_to_postfix(Token in[], int n, Token out[], int *out_n, char *err) {
    char opstack[MAXN];
    int top = -1, oi=0;
    for (int i=0;i<n;i++) {
        Token t = in[i];
        if (t.type==TOK_NUM || t.type==TOK_VAR) {
            out[oi++] = t;
        } else if (t.type==TOK_LP) {
            opstack[++top]='(';
        } else if (t.type==TOK_RP) {
            int found=0;
            while (top>=0) {
                char c = opstack[top--];
                if (c=='(') { found=1; break; }
                Token tt; tt.type = TOK_OP; tt.str[0]=c; tt.str[1]=0;
                out[oi++] = tt;
            }
            if (!found) { strcpy(err, "Error: Unmatched ')' found."); return -1; }
        } else if (t.type==TOK_OP) {
            char cur = t.str[0];
            while (top>=0 && opstack[top]!='(') {
                char peek = opstack[top];
                int ppeek = precedence(peek);
                int pcur = precedence(cur);
                if ((ppeek > pcur) || (ppeek==pcur && cur!='^')) {
                    top--;
                    Token tt; tt.type=TOK_OP; tt.str[0]=peek; tt.str[1]=0;
                    out[oi++]=tt;
                } else break;
            }
            opstack[++top]=cur;
        } else {
            strcpy(err, "Error: Invalid token found.");
            return -1;
        }
    }
    while (top>=0) {
        char c = opstack[top--];
        if (c=='(' || c==')') {
            strcpy(err, "Error: Unmatched '(' found.");
            return -1;
        }
        Token tt; tt.type=TOK_OP; tt.str[0]=c; tt.str[1]=0;
        out[oi++]=tt;
    }
    *out_n = oi;
    return 0;
}

// ---------- Evaluate postfix ----------
int eval_postfix(Token post[], int n, long long *result, char *err, long long varVals[256]) {
    long long stack[MAXN]; int top=-1;
    for (int i=0;i<n;i++) {
        if (post[i].type==TOK_NUM) {
            long long val = atoll(post[i].str);
            stack[++top]=val;
        } else if (post[i].type==TOK_VAR) {
            unsigned char v = post[i].str[0];
            stack[++top]=varVals[v];
        } else if (post[i].type==TOK_OP) {
            if (top<1) { strcpy(err, "Error: Not enough operands."); return -1; }
            long long b = stack[top--];
            long long a = stack[top--];
            char op = post[i].str[0];
            long long res=0;
            if (op=='+') res = a + b;
            else if (op=='-') res = a - b;
            else if (op=='*') res = a * b;
            else if (op=='/') {
                if (b==0) { strcpy(err, "Error: Division by zero."); return -1; }
                res = a / b;
            } else if (op=='^') {
                if (b < 0) { strcpy(err, "Error: Negative exponent not supported."); return -1; }
                res = 1;
                for (long long k=0;k<b;k++) res *= a;
            } else { strcpy(err, "Error: Unknown operator."); return -1; }
            stack[++top]=res;
        } else {
            strcpy(err, "Error: Invalid token in postfix.");
            return -1;
        }
    }
    if (top!=0) { strcpy(err, "Error: Invalid expression (extra operands)."); return -1; }
    *result = stack[top];
    return 0;
}

// ---------- Main ----------
int main() {
    char line[1024];
    long long varVals[256] = {0};
    while (1) {
        printf("Enter infix expression (or 'quit'):\n");
        if (!fgets(line, sizeof(line), stdin)) break;
        line[strcspn(line, "\n")] = 0;
        if (strcmp(line, "quit")==0) break;
        if (strlen(line)==0) { printf("Enter expression:\n"); continue; }

        Token toks[MAXTOK]; int tcount=0;
        char err[256]={0};
        if (tokenize(line, toks, &tcount) != 0) {
            printf("Tokenization error: invalid character.\n"); continue;
        }

        // collect variables
        int varUsed[256]={0};
        for (int i=0;i<tcount;i++) {
            if (toks[i].type==TOK_VAR)
                varUsed[(unsigned char)toks[i].str[0]] = 1;
        }

        // ask for variable values
        for (int i=0;i<256;i++) {
            if (varUsed[i]) {
                printf("Enter value for %c: ", i);
                scanf("%lld", &varVals[i]);
            }
        }
        getchar(); // consume leftover newline

        Token postfix[MAXTOK]; int pn=0;
        if (infix_to_postfix(toks, tcount, postfix, &pn, err)!=0) {
            printf("%s\n", err); continue;
        }

        printf("Postfix: ");
        for (int i=0;i<pn;i++) printf("%s ", postfix[i].str);
        printf("\n");

        long long res;
        if (eval_postfix(postfix, pn, &res, err, varVals)!=0) {
            printf("%s\n", err); continue;
        }
        printf("Result: %lld\n", res);
        printf("-----------------\n");
    }
    return 0;
}