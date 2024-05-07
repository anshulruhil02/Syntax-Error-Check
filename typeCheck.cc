#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <memory>
#include <unordered_map>

using namespace std;

bool hasErrorOccurred = false; // Global flag to track error state

void errorReport(const string& errorMessage, const int& index = -1) {
    cerr << "ERROR: " << errorMessage;
    if(index >= 0) {
        cerr << " Index: " << index;
    }
    cerr << endl;
    hasErrorOccurred = true;
}

class Tree {
public:
    string rule;
    vector<unique_ptr<Tree>> children;
    string type;
    string lexeme;
    Tree(const string& r) : rule(r), type(""), lexeme(""){}
    void addChild(unique_ptr<Tree> child){
        if(child->getRule() == "NUM") child->setType("int");
        else if(child->getRule() == "NULL") child->setType("int*");
        children.push_back(move(child));
    }
    void setType(const string& t){
        type = t;
    }
    void setLexeme(const string& value){
        lexeme = value;
    }
    string const getType(){
        return type;
    }
    string const getLexeme(){
        return lexeme;
    }
    string const getRule(){
        return rule;
    }

    size_t const getChildrenSize() const {
        return children.size();
    }
};

class SymbolTable {
public:
    unordered_map<string, string> table;

    // Add a variable along with its type to the symbol table
    void declareVariable(const string& name, const string& type) {
        if (table.find(name) != table.end()) {
            errorReport("Variable " + name + " is already declared");
        } else {
            table[name] = type;
        }
    }

    // Retrieve the type of a variable
    string const getType(const string& name) {
        auto it = table.find(name);
        if (it != table.end()) {
            return it->second;
        } else {
            errorReport("Variable " + name + " was not declared");
            return ""; // Return an empty string if not found (error case)
        }
    }

    // Check if a variable has been declared
    bool isDeclared(const string& name) {
        return table.find(name) != table.end();
    }

    // Check for a type mismatch
    bool checkType(const string& name, const string& expectedType) {
        auto it = table.find(name);
        if (it != table.end()) {
            return it->second == expectedType;
        } else {
            errorReport("Variable " + name + " was not declared");
            return false;
        }
    }

    void printContents() const {
        for (const auto& pair : table) {
            cout << "Variable: " << pair.first << ", Type: " << pair.second << endl;
        }
    }
};

class ProcedureTable {
public:
    // Map from procedure name to a vector of strings representing argument types
    unordered_map<string, vector<string>> table;

    // Method to add a procedure's signature to the table
    void addProcedure(const string& name, const vector<string>& argTypes = {}) {
        if (procedureExists(name)) errorReport("Procedure already exists!");
        table[name] = argTypes;
    }

    // Method to check if a procedure with a given name and signature exists
    bool procedureExists(const string& name) const {
        auto it = table.find(name);
        if (it != table.end()) return true; 
        return false; // Procedure name not found in the table
    }


    void printTable() const {
        cout << "Procedure Table Contents:\n";
        for (const auto& entry : table) {
            const string& name = entry.first;
            const vector<string>& argTypes = entry.second;
        
            cout << "Procedure Name: " << name << " | Argument Types: ";
            if (argTypes.empty()) {
             cout << "None";
            } else {
                for (size_t i = 0; i < argTypes.size(); ++i) {
                  cout << argTypes[i];
                  if (i < argTypes.size() - 1) cout << ", ";
                }
            }
            cout << "\n";
        }
    }
};

void treeTraversal(Tree& node, vector<SymbolTable>& symbolTableStack);

bool isNonTerminal(const string& word) {
    for (char ch : word) {
        if (!islower(ch)) {
            // If any character is not lowercase, return false
            return false;
        }
    }
    // If we've gone through all characters without returning false, it's all lowercase
    return true;
}

// Assume parseTree is correctly implemented to build the tree based on the lines
void buildTree(Tree& parent, vector<string>& lines, int& idx) {
    
    if (idx >= lines.size()) {
        errorReport("Reached end of file without completing parse tree", idx);
    }

    stringstream iss(lines[idx]);
    string rule;
    iss >> rule;

    if(isNonTerminal(rule)){
        //if(hasErrorOccurred) errorReport("Unexpected end of file", idx);
        string childRule;
    
        while (iss >> childRule) {
            if(hasErrorOccurred) break;
            if (childRule == ".EMPTY") {
                break; // No children to process
            }
            auto child = make_unique<Tree>(childRule);
            idx++; // Move to the next line for the child to process
            if (idx >= lines.size()) {
                errorReport("Unexpected end of file", idx);
                return;
            }
            buildTree(*child, lines, idx); // Recurse to build the child's subtree
            parent.addChild(move(child));
        }
    }else{
        string lexeme;
        if(iss >> lexeme){
            parent.setLexeme(lexeme);
        } else {
            errorReport("Missing lexeme for terminal symbol", idx);
            return;
        }
    } 
}

void printTreeTest(const Tree& node, int depth = 0) {
    // Print the current node's rule and lexeme
    for (int i = 0; i < depth; ++i) {
        cout << "  ";  // Indentation for each level of depth
    }
    cout << node.rule;  // Print the rule
    if (!node.lexeme.empty()) {
        cout << " " << node.lexeme;  // Print the lexeme if it exists
    }
    cout << endl;

    // Recursively print all children
    for (const auto& child : node.children) {
        printTreeTest(*child, depth + 1);  // Increase depth for child nodes
    }
}

void lvaluerRecursionBaseCase(Tree& node,vector<SymbolTable>& symbolTableStack){
    //string factorType = node.children[0]->getRule();
    //cout<<factorType<<endl;
    string returnType = node.children[0]->getLexeme();
    //cout<<"Return type: "<<returnType<<endl;
                
    if(symbolTableStack.back().getType(returnType) == "int"){
        node.setType("int");
        node.children[0]->setType("int");
    }
    else if(symbolTableStack.back().getType(returnType) == "int*"){
        node.setType("int*");
        node.children[0]->setType("int*");
    }
    else errorReport("Variable not declared! ");  
}

void factorRecursionBaseCase(Tree& node, vector<SymbolTable>& symbolTableStack){
    string factorType = node.children[0]->getRule();
    //cout<<factorType<<endl;
    string returnType = node.children[0]->getLexeme();
    //cout<<"Return type: "<<returnType<<endl;
    if(factorType == "NUM"){
        node.setType("int");
        node.children[0]->setType("int");
    }
    else if(factorType == "NULL"){
        node.setType("int*");
        node.children[0]->setType("int*");
    }
    else if(symbolTableStack.back().getType(returnType) == "int"){
        node.setType("int");
        node.children[0]->setType("int");
    }
    else if(symbolTableStack.back().getType(returnType) == "int*"){
        node.setType("int*");
        node.children[0]->setType("int*");
    }
    else if(symbolTableStack.back().getType(returnType) == "int" || "int*") errorReport("Variable not declared! ");
    else errorReport("Invalid return type ");  
}

int exprCount = 0;
void treeTraversal(Tree& node, vector<SymbolTable>& symbolTableStack, ProcedureTable& procedureTable, string& procedureName , vector<string>& parameterTypeList, int& procedureNumber) {
    
    if(node.rule == "procedures"){
        if(procedureNumber != 0 && !symbolTableStack.empty()){
            symbolTableStack.pop_back(); 
        } 
        symbolTableStack.push_back(SymbolTable());
        procedureNumber++;
    }

    if(node.rule == "procedure"){
        bool hasParameters;
        node.children[3]->getChildrenSize() == 0 ? hasParameters = false : hasParameters = true;
        procedureName = node.children[1]->getLexeme(); 
        
        if(!hasParameters) procedureTable.addProcedure(procedureName);
    }
    //cout<<procedureName;
    if(node.rule == "paramlist"){
        bool lastParameter;
        int typeSize = node.children[0]->children[0]->getChildrenSize();
        string type = (typeSize == 1) ? "int" : "int*";
        parameterTypeList.push_back(type);
        node.getChildrenSize() == 1 ? lastParameter = true : lastParameter = false;
        //cout<<procedureName<<endl;
        if(lastParameter){
            procedureTable.addProcedure(procedureName,parameterTypeList);
        }
    }
    if(node.rule == "main"){
        int typeSize = node.children[5]->children[0]->getChildrenSize();
        string type = (typeSize == 1) ? "int" : "int*";
        if(type != "int"){
            errorReport("The second parameter of wain is not int type");
        }
    }
    if(node.rule == "dcl"){
        string name = node.children[1]->getLexeme();
        int typeSize = node.children[0]->getChildrenSize();
        string type = (typeSize == 1) ? "int" : "int*";
        symbolTableStack.back().declareVariable(name, type);
        node.children[1]->setType(type);
    }
    if(node.rule == "dcls"){
        bool isEmpty;
        node.getChildrenSize() == 5 ? isEmpty = false: isEmpty = true;
        if(!isEmpty){
            int typeSize = node.children[1]->children[0]->getChildrenSize();
            string type = (typeSize == 1) ? "int" : "int*";
            node.children[1]->setType(type);
            string assignType = node.children[3]->getType();
            //cout<<"Declaring type: "<<type<<" assigning type: "<<assignType<<endl;
            if(type != assignType) errorReport("Type casting error");
        } 
    }


    
    int nodeSize = node.getChildrenSize();
    if(node.getRule() == "factor" && nodeSize == 1) factorRecursionBaseCase(node, symbolTableStack);
    if(node.getRule() == "lvalue" && nodeSize == 1) lvaluerRecursionBaseCase(node, symbolTableStack);

    // Recursively traverse children
    for (const auto& child : node.children){
        treeTraversal(*child, symbolTableStack, procedureTable,procedureName,parameterTypeList, procedureNumber);
        //cout<<node.getRule()<<" Child's type: "<<node.children[0]->getType()<<endl;
        if(node.getRule() != "arglist")node.setType(node.children[0]->getType());
        if(node.getRule() == "statement") node.setType("");
        if(node.getRule() == "test") node.setType("");

        // syntax error checks and assigning types for return expressions 
        

        if(node.children[0]->getRule() == "LPAREN" && node.getRule() == "factor")  node.setType(node.children[1]->getType());
        if(node.children[0]->getRule() == "LPAREN" && node.getRule() == "lvalue")  node.setType(node.children[1]->getType());

        if(node.children[0]->getRule() == "STAR" && node.getRule() == "factor" && node.children[1]->getType() != ""){
            if(node.children[1]->getType() != "int*") errorReport("In rule [factor STAR factor]: Expected child of type int*");
            node.setType("int");
        }
        if(node.children[0]->getRule() == "STAR" && node.getRule() == "lvalue" && node.children[1]->getType() != ""){
            if(node.children[1]->getType() != "int*") errorReport("In rule [factor STAR factor]: Expected child of type int*");
            node.setType("int");
        }  
        if(node.children[0]->getRule() == "NEW" && node.getRule() == "factor")  node.setType("int*");
        if(node.children[0]->getRule() == "AMP" && node.getRule() == "factor"  && node.children[1]->getType() != ""){
            if(node.children[1]->getType() != "int") errorReport("In rule [factor AMP lvalue]: Expected child of type int");
            node.setType("int*");
        }

        // ARITHMETIC ERROR CHECKS
        if(node.getRule() == "expr" && node.getChildrenSize() == 3){
            if(node.children[1]->getRule() == "PLUS"){
                if(node.children[0]->getType() == "int*" || node.children[2]->getType() == "int*"){
                    node.setType("int*");
                }
            }
            if(node.children[1]->getRule() == "MINUS"){
                if(node.children[0]->getType() == "int" && node.children[2]->getType() == "int*"){
                    errorReport("In rule [expr expr MINUS term]: cannot subtract int minus int*");
                }
                if(node.children[0]->getType() == "int*"){
                    node.setType("int*");
                }
            }
        }

        if(node.getRule() == "term" && node.getChildrenSize() == 3){
            if(node.children[1]->getRule() == "STAR"){
                if(node.children[0]->getType() == "int*" || node.children[2]->getType() == "int*"){
                    errorReport("In rule [term term STAR factor]: factor child is not type int");
                }
            }
            if(node.children[1]->getRule() == "SLASH"){
                if(node.children[0]->getType() == "int*" || node.children[2]->getType() == "int*"){
                    errorReport("In rule [term term SLASH factor]: factor child is not type int");
                }
            }
            if(node.children[1]->getRule() == "PCT"){
                if(node.children[0]->getType() == "int*" || node.children[2]->getType() == "int*"){
                    errorReport("In rule [term term PCT factor]: factor child is not type int");
                }
            }
        }  

        if(node.getRule() == "factor" && node.getChildrenSize() >= 2){
            if(node.children[0]->getRule() == "ID" && node.children[1]->getRule() == "LPAREN" )  node.setType("int");
        }

        if(node.getRule() == "statement"){
            if(node.getChildrenSize() == 4 && node.children[0]->getType() != "" && node.children[2]->getType() != "")
                if(node.children[0]->getType() != node.children[2]->getType()) errorReport("Type Mismatch");

 
            if(node.getChildrenSize() == 5){
                if(node.children[0]->getRule() == "PRINTLN" && node.children[2]->getType() != "")
                    if(node.children[2]->getType() != "int") errorReport("In rule [statement PRINTLN LPAREN expr RPAREN SEMI]: expr does not have type int");
                
                if(node.children[0]->getRule() == "DELETE" && node.children[3]->getType() != ""){
                    if(node.children[3]->getType() != "int*") errorReport("In rule [statement DELETE LBRACK RBRACK expr SEMI]: expr does not have type int*");
                }
            }
        }

        if(node.getRule() == "test" && node.getChildrenSize() == 3){
            if(node.children[0]->getType() != "" && node.children[2]->getType()!="") 
                if(node.children[0]->getType() !=  node.children[2]->getType()) errorReport("Type mismatch during comparision");
        }
        

    }
}

void returnTypeCheck(Tree& node){
    if(node.getRule() == "expr" && exprCount == 0){
        //cout<<"First expr encountered" << node.children[0]->getRule() <<endl;
        if(node.getType() != "int") errorReport("Return type is not int");
        exprCount++;
    }
    for (const auto& child : node.children) returnTypeCheck(*child);
}

void printTree(const Tree& node) {
    int i;
    bool isTerminal;
    bool hasType;
    isNonTerminal(node.rule) ? isTerminal = false : isTerminal = true;
    node.type == "" ? hasType = false: hasType = true;
    int numberOfChildren = node.getChildrenSize();
    cout << node.rule;
    if(isTerminal) cout<<" "<<node.lexeme; 
    else{
        if(numberOfChildren == 0) cout << " .EMPTY";
        for(i = 0; i < numberOfChildren; i++) cout << " " << node.children[i]->getRule();
    } 
    if(hasType) cout<< " : " << node.type;
    cout << endl;
    for (const auto& child : node.children) {
        printTree(*child); 
    }
}

int main() {
    string line;
    vector <string> lines;
    SymbolTable globalSymbolTable;
    vector<SymbolTable> symbolTableStack;
    ProcedureTable procedureTable;
    int procedureNumber = 0;
    string procedureName; // Initially empty
    vector<string> parameterTypeList; // Initially empty
    // Reading lines from stdin until EOF
    while (getline(cin, line)) {
        lines.push_back(line);
    }
    if(lines.size() == 0) errorReport("invalid first expression ");
    stringstream ss(lines[0]);
    string start;
    ss >> start;
    if(start != "start") errorReport("invalid first expression ");
    auto rootNode = make_unique<Tree>(start);
    int index = 0;
    buildTree(*rootNode, lines, index);
    //printTreeTest(*rootNode);
    treeTraversal(*rootNode,symbolTableStack, procedureTable,procedureName,parameterTypeList, procedureNumber);
    //procedureTable.printTable();
    //returnTypeCheck(*rootNode);
    printTree(*rootNode);
    return 0;
}
