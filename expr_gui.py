import tkinter as tk
from tkinter import messagebox, simpledialog, scrolledtext

# ---------- Expression Logic ----------

def is_op(ch):
    return ch in '+-*/^'

def precedence(op):
    if op in '+-': return 1
    if op in '*/': return 2
    if op == '^': return 3
    return 0

class ExprError(Exception):
    pass

def tokenize(expr):
    toks = []
    i = 0
    while i < len(expr):
        ch = expr[i]
        if ch.isspace():
            i += 1
            continue
        if ch.isdigit():
            num = ch
            i += 1
            while i < len(expr) and expr[i].isdigit():
                num += expr[i]
                i += 1
            toks.append(('NUM', num))
        elif ch.isalpha():
            var = ch
            i += 1
            while i < len(expr) and expr[i].isalpha():
                var += expr[i]
                i += 1
            toks.append(('VAR', var))
        elif ch in '+-*/^':
            toks.append(('OP', ch))
            i += 1
        elif ch == '(':
            toks.append(('LP', ch))
            i += 1
        elif ch == ')':
            toks.append(('RP', ch))
            i += 1
        else:
            raise ExprError(f"Invalid character '{ch}'")
    return toks

def infix_to_postfix(toks):
    stack = []
    output = []
    for ttype, val in toks:
        if ttype in ('NUM', 'VAR'):
            output.append((ttype, val))
        elif ttype == 'LP':
            stack.append(val)
        elif ttype == 'RP':
            while stack and stack[-1] != '(':
                output.append(('OP', stack.pop()))
            if not stack:
                raise ExprError("Unmatched ')'")
            stack.pop()  # remove '('
        elif ttype == 'OP':
            while stack and stack[-1] != '(':
                if (precedence(stack[-1]) > precedence(val)) or (
                    precedence(stack[-1]) == precedence(val) and val != '^'
                ):
                    output.append(('OP', stack.pop()))
                else:
                    break
            stack.append(val)
    while stack:
        top = stack.pop()
        if top in '()':
            raise ExprError("Unmatched '('")
        output.append(('OP', top))
    return output

def eval_postfix(post, var_values):
    stack = []
    for ttype, val in post:
        if ttype == 'NUM':
            stack.append(int(val))
        elif ttype == 'VAR':
            if val not in var_values:
                raise ExprError(f"Value for variable '{val}' not provided")
            stack.append(var_values[val])
        elif ttype == 'OP':
            if len(stack) < 2:
                raise ExprError("Not enough operands")
            b = stack.pop()
            a = stack.pop()
            if val == '+': stack.append(a + b)
            elif val == '-': stack.append(a - b)
            elif val == '*': stack.append(a * b)
            elif val == '/':
                if b == 0:
                    raise ExprError("Division by zero")
                stack.append(a // b)
            elif val == '^':
                stack.append(a ** b)
    if len(stack) != 1:
        raise ExprError("Invalid expression")
    return stack[0]

# ---------- GUI Part ----------

def evaluate_expression():
    expr = entry.get().strip()
    if not expr:
        messagebox.showinfo("Info", "Enter an infix expression.")
        return

    try:
        toks = tokenize(expr)
        post = infix_to_postfix(toks)
    except ExprError as e:
        messagebox.showerror("Error", str(e))
        return

    # Collect variables
    vars_used = sorted({val for ttype, val in toks if ttype == 'VAR'})
    var_values = {}
    for v in vars_used:
        val = simpledialog.askstring("Variable Input", f"Enter value for {v}:")
        if val is None:
            return
        try:
            var_values[v] = int(val)
        except ValueError:
            messagebox.showerror("Error", f"Invalid number for variable '{v}'.")
            return

    try:
        res = eval_postfix(post, var_values)
    except ExprError as e:
        messagebox.showerror("Error", str(e))
        return

    post_str = " ".join(val for _, val in post)
    output_text.delete(1.0, tk.END)
    output_text.insert(tk.END, f"Postfix: {post_str}\n")
    output_text.insert(tk.END, f"Result: {res}\n")

# ---------- Main Window ----------

root = tk.Tk()
root.title("Expression Evaluator with Variables")
root.geometry("480x400")
root.config(bg="#f7f7ff")

title = tk.Label(root, text="Infix Expression Evaluator", font=("Arial", 16, "bold"), bg="#f7f7ff", fg="#4b0082")
title.pack(pady=10)

entry = tk.Entry(root, font=("Arial", 14), width=35, justify='center', bd=3, relief="ridge")
entry.pack(pady=10)

btn_eval = tk.Button(root, text="Evaluate", font=("Arial", 12, "bold"), bg="#9370db", fg="white", command=evaluate_expression)
btn_eval.pack(pady=5)

output_text = scrolledtext.ScrolledText(root, width=50, height=10, font=("Consolas", 12), wrap=tk.WORD, bd=3, relief="sunken")
output_text.pack(pady=10)

root.mainloop()