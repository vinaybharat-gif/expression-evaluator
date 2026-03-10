function calculate() {
    let exp = document.getElementById("expression").value;

    try {
        let result = eval(exp);
        document.getElementById("result").innerText = "Result: " + result;
    } 
    catch {
        document.getElementById("result").innerText = "Invalid Expression";
    }
}