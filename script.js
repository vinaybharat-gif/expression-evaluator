function calculate() {
    try {
        let exp = document.getElementById("expression").value;
        
        if (!exp || exp.trim() === "") {
            document.getElementById("result").innerText = "Please enter an expression";
            return;
        }
        
        let result = eval(exp);
        document.getElementById("result").innerText = "Result: " + result;
    } 
    catch (error) {
        document.getElementById("result").innerText = "Invalid Expression: " + error.message;
    }
}