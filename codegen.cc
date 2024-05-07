#include <iostream>
#include <vector>
#include <string>
#include <sstream>
#include <map>
#include <memory>
#include <unordered_map>


using namespace std;

map<string, pair<string, int>> symbolTable;

int loopCount = 0;
int endwhileCount = 0;
int elseCount = 0;
int endifCount = 0;
int deleteCount = 0;


// looks up and loads the value of a variable stored on the stack into $3
void returnStatement(string var) {
    int off_set = symbolTable[var].second;
    cout << "lw $3, " << off_set << "($29)" << endl;
}

// pushes a register on the stack and decrements the stack pointer to point to the top of the stack 
void storeOnStackPush(int reg){
    cout << "sw $" << reg << ", -4($30)" << endl;
    cout << "sub $30, $30, $4" << endl;
}

// puts the value of the register on the top of the stack into the parameter register and decrements the stack pointer 
void loadFromStackPop(int reg){
    cout << "add $30, $30, $4" << endl;
    cout << "lw $" << reg << ", -4($30)" <<
    endl;
}

void loadRegisterNum(int reg, string val){
    cout << "lis $" << reg << endl << ".word " << val << endl;
}

void errorReport(const string& errorMessage, const int& index = -1) {
    cerr << "ERROR: " << errorMessage;
    if(index >= 0) {
        cerr << " Index: " << index;
    }
    cerr << endl;
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

    void printChildren() const {
        for (const auto& child : children) {
            cout << "Rule: " << child->getRule() << ", Type: " << child->getType() << endl;
            child->printChildren(); // Recursive call to print children of children
        }
    }
};

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

void arguements(Tree&, int&);
void lvalueOne(Tree&);
void lvalueTwo(Tree&);
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
            if (childRule == ".EMPTY") {
                break; // No children to process
            }
            if(childRule != ":"){
                auto child = make_unique<Tree>(childRule);
                idx++; // Move to the next line for the child to process
                buildTree(*child, lines, idx);
                // Recurse to build the child's subtree
                parent.addChild(move(child));
            }
            else if(childRule == ":"){
                string type;
                iss>>type;
                parent.setType(type);
            }
            if (idx >= lines.size()) {
                errorReport("Unexpected end of file", idx);
                return;
            }
        }
    }
    else{
        string lexeme, typeAnnotation;
        if(iss >> lexeme){
            parent.setLexeme(lexeme);
            if(iss >> typeAnnotation) { // Check for type annotation
                // Assuming type annotations are only after certain terminals
                // like IDs, NUMs, and NULLs as per the .wlp4ti format
                if(typeAnnotation == ":") {
                    string type;
                    if(iss >> type) {
                        parent.setType(type);
                    } else {
                        errorReport("Expected a type after ':'", idx);
                        return;
                    }
                }
            }

        } else {
            errorReport("Missing lexeme for terminal symbol", idx);
            return;
        }
    } 
}

void printTreeTest(const Tree& node, int depth = 0) {
    // Print the current node's rule, lexeme, and type (if it exists)
    for (int i = 0; i < depth; ++i) {
        cout << "  ";  // Indentation for each level of depth
    }

    cout << node.rule;  // Print the rule
    if (!node.lexeme.empty()) {
        cout << " " << node.lexeme;  // Print the lexeme if it exists
    }

    if (!node.type.empty()) {
        cout << " : " << node.type;  // Print the type if it exists
    }
    cout << endl;

    // Recursively print all children
    for (const auto& child : node.children) {
        printTreeTest(*child, depth + 1);  // Increase depth for child nodes
    }
}

void expr(Tree &node){
    if (node.getRule() == "expr") {
        if(node.getChildrenSize() == 1) {
            auto& child = node.children[0];
            expr(*child);
        }
        else if(node.getChildrenSize() == 3){
            //cout << "; Calculate and store expr1 in $3 and expr2 in $5" << endl;

            auto& exprChild = node.children[0];
            expr(*exprChild);
            storeOnStackPush(3);
            auto& termChild = node.children[2];
            expr(*termChild);
            loadFromStackPop(5);

            if (node.children[0]->getType() == "int*" && node.children[2]->getType() != "int*"){
                cout << "mult $3, $4" << endl;
                cout << "mflo $3" << endl;
            }else if (node.children[2]->getType() == "int*" && node.children[0]->getType() != "int*"){
                cout << "mult $5, $4" << endl;
                cout << "mflo $5" << endl;
            }

            if (node.children[1]->getRule() == "PLUS" ){
                cout << "add $3, $5, $3" << endl;
            } else if (node.children[1]->getRule() == "MINUS"){
                cout << "sub $3, $5, $3" << endl;
                if (node.children[0]->getType() == "int*" && node.children[2]->getType() == "int*"){
                    cout << "div $3, $4" << endl;
                    cout << "mflo $3" << endl;
                }
            }
        }
    }
    else if (node.getRule() == "term") {
        if(node.getChildrenSize() == 1) {
            auto& child = node.children[0];
            expr(*child);
        }
        else if(node.getChildrenSize() == 3){
            auto& exprChild = node.children[0];
            expr(*exprChild);
            storeOnStackPush(3);

            auto& termChild = node.children[2];
            expr(*termChild);
            loadFromStackPop(5);

            if (node.children[1]->getRule() == "STAR" ) {
                cout << "mult $5, $3" << endl;
                cout << "mflo $3" << endl;
            } else if (node.children[1]->getRule() == "SLASH" ) {
                cout << "div $5, $3" << endl;
                cout << "mflo $3" << endl;
            } else if (node.children[1]->getRule() == "PCT" ) {
                cout << "div $5, $3" << endl;
                cout << "mfhi $3" << endl;
            }
        }
    }
    else if (node.getRule() == "factor") {
        if (node.getChildrenSize() == 1) {
            auto& child = node.children[0];
            expr(*child);
        }
        else if(node.getChildrenSize() == 3 && node.children[0]->getRule() == "LPAREN"){
            auto& child = node.children[1];
            expr(*child);
        }
        else if (node.getChildrenSize() == 3 && node.children[0]->getRule() == "ID") {
            storeOnStackPush(29);
            storeOnStackPush(31);
            cout << "lis $5" << endl;
            string pName = node.children[0]->getLexeme();
            cout << ".word " << pName << endl;
            cout << "jalr $5" << endl;
            loadFromStackPop(31);
            loadFromStackPop(29);
        } 
        
        else if(node.getChildrenSize() == 4){
            storeOnStackPush(29);
            storeOnStackPush(31);
            int argCount = 0;
            auto& child = node.children[2];
            arguements(*child, argCount);
            cout << "lis $5" << endl;
            string pName = node.children[0]->getLexeme();
            cout << ".word " << pName << endl;
            cout << "jalr $5" << endl;
            for (int i = 0; i < argCount; i++) loadFromStackPop(31);
            loadFromStackPop(31);
            loadFromStackPop(29);
        }
        else if(node.getChildrenSize() == 5) {
            auto& child = node.children[3];
            expr(*child);
            cout << "add $1, $3, $0" << endl;
            storeOnStackPush(31);
            cout << "lis $5" << endl;
            cout << ".word new"  << endl;
            cout << "jalr $5" << endl;
            loadFromStackPop(31);
            cout << "bne $3, $0, 1" << endl;
            cout << "add $3, $11, $0" << endl;

        }
        else if(node.getChildrenSize() == 2){
            if (node.children[0]->getRule() == "STAR") {
                auto& child = node.children[1];
                expr(*child);
                cout << "lw $3, 0($3)" << endl;
            }else if (node.children[0]->getRule() == "AMP") {
                auto& child = node.children[1];
                lvalueTwo(*child);
            }
        }
    }
    else if(node.getRule() == "NUM") loadRegisterNum(3,node.getLexeme());
    else if(node.getRule() == "ID") returnStatement(node.getLexeme());
    else if(node.getRule() == "NULL") cout << "add $3, $0, $11" << endl;
}

void dcls(Tree &node){
    if (node.getChildrenSize() == 5) {
        auto& child = node.children[0];
        dcls(*child);
        symbolTable[node.children[1]->children[1]->getLexeme()] = make_pair(node.children[1]->children[1]->getType(), -4 * symbolTable.size());
        auto& child1 = node.children[3];
        expr(*child1);
        storeOnStackPush(3);
    }
}

void test(Tree &node) {
    cout << "; Calculate and store expr1 in $3 and expr2 in $5" << endl;
    auto& exprChildOne = node.children[0];
    expr(*exprChildOne);
    storeOnStackPush(3);
    auto& exprChildTwo = node.children[2];
    expr(*exprChildTwo);
    loadFromStackPop(5);

    string x = "slt";
    string cmp = node.children[0]->getType();
    if (cmp == "int*") {
        x = "sltu";
    }
    string opType = node.children[1]->getRule();
    if(opType == "EQ"){
        cout << x << " $6, $3, $5" << endl;
        cout << x << " $7, $5, $3" << endl;
        cout << "add $3, $6, $7" << endl;
        cout << "sub $3, $11, $3" << endl;
    } 
    else if(opType == "NE"){
        cout << x << " $6, $3, $5" << endl;
        cout << x << " $7, $5, $3" << endl;
        cout << "add $3, $6, $7" << endl;
    } 
    else if(opType == "LT"){
        cout << x << " $3, $5, $3" << endl;
    } 
    else if(opType == "GT"){
        cout << x << " $3, $3, $5" << endl;
    } 
    else if(opType == "LE"){
        cout << x << " $3, $3, $5" << endl;
        cout << "sub $3, $11, $3" << endl;
    } 
    else if(opType == "GE"){
        cout << x << " $3, $5, $3" << endl;
        cout << "sub $3, $11, $3" << endl;
    }
}

void lvalueOne(Tree &node){
    if(node.getChildrenSize() == 1){
        //cout << "; Assignment lhs: " << node.children[0]->getLexeme() << endl;
        if(node.children[0]->getRule() == "ID") cout << "sw $3, " << symbolTable[node.children[0]->getLexeme()].second << "($29)" << endl;
    }
    else if(node.getChildrenSize() == 3){
        auto& child = node.children[1];
        lvalueOne(*child);
    } 
    else if(node.getChildrenSize() == 2){
        if(node.children[1]->getRule() == "factor"){
            storeOnStackPush(3);
            auto& child = node.children[1];
            expr(*child);
            loadFromStackPop(5);
            cout << "sw $5, 0($3)" << endl;
        }
    }
}

void lvalueTwo(Tree &node){
    if(node.getChildrenSize() == 1){
        if(node.children[0]->getRule() == "ID"){
            int offset = symbolTable[node.children[0]->getLexeme()].second;
            cout << "lis $3" << endl;
            cout << ".word " << offset << endl;
            cout << "add $3, $3, $29" << endl;
        }
    }
    else if(node.getChildrenSize() == 3){
        auto& child = node.children[1];
        lvalueTwo(*child);
    } 
    else if(node.getChildrenSize() == 2){
        if(node.children[1]->getRule() == "factor"){
            auto& child = node.children[1];
            expr(*child);
        }
    }
}

void statements(Tree &node){
    if(node.getRule() == "statements"){
        if (node.getChildrenSize() == 2) {
            auto& statementsRecursion = node.children[0];
            auto& statementRecursion = node.children[1];
            statements(*statementsRecursion);
            statements(*statementRecursion);   
        }
    }
    else if(node.getRule() == "statement"){
        if(node.getChildrenSize() == 4) {
            auto& exprChild = node.children[2];
            expr(*exprChild);
            auto& lvalueChild = node.children[0]; 
            lvalueOne(*lvalueChild);
        }
        else if(node.getChildrenSize() == 11){
            int elseNumber = elseCount;
            int endIfNumber = endifCount;
            auto& testChild = node.children[2];
            test(*testChild);
            elseCount++;
            endifCount++;
            cout << "beq $3, $0, else" << elseNumber << endl;
            auto& statementsChildOne = node.children[5];
            statements(*statementsChildOne);
            cout << "beq $0, $0, endif" << endIfNumber << endl;
            cout << "else" << elseNumber << ":" << endl;
            auto& statementsChildTwo = node.children[9];
            statements(*statementsChildTwo);
            cout << "endif" << endIfNumber << ":" << endl;
        }
        else if(node.getChildrenSize() == 7){
            int endWhileNumber = endwhileCount;
            int loopNumber= loopCount;
            cout << "loop" << loopNumber << ":" << endl;
            auto& testChild = node.children[2];
            test(*testChild);
            endwhileCount++;
            loopCount++;
            cout << "beq $3, $0, endWhile" << endWhileNumber << endl;
            auto& statementChild = node.children[5];
            statements(*statementChild);
            cout << "beq $0, $0, loop" << loopNumber << endl;
            cout << "endWhile" << endWhileNumber << ":" << endl;
        }
        else if(node.getChildrenSize() == 5 && node.children[0]->getRule() == "DELETE"){
            int deleteNumber = deleteCount;
            deleteCount++;

            auto& exprChild = node.children[3];
            expr(*exprChild);

            cout << "beq $3, $11, checkIntStar" << deleteNumber << endl;
            cout << "add $1, $3, $0" << endl;

            storeOnStackPush(31);

            cout << "lis $5" << endl;
            cout << ".word delete" << endl;
            cout << "jalr $5" << endl;

            loadFromStackPop(31);

            cout << "checkIntStar" << deleteNumber << ":" << endl;
        }
        else if(node.getChildrenSize() == 5 && node.children[0]->getRule() != "DELETE"){
            auto& child = node.children[2];
            storeOnStackPush(1);
            expr(*child);
            
            cout << "add $1, $3, $0" << endl;
            storeOnStackPush(31);
            cout << "jalr $10" << endl;
            loadFromStackPop(31);
            loadFromStackPop(1);
        }
        
    }
}

void params(Tree& node, int &count) {
    if (node.getRule() == "params" && node.getChildrenSize() == 1) {
        auto& child = node.children[0];
        params(*child, count);
    } 
    else if (node.getRule() == "paramlist") {
        if (node.getChildrenSize() == 3) {
            count++;
            int i = count;
            auto& child = node.children[2];
            params(*child, count);
            string name = node.children[0]->children[1]->getLexeme();
            string type = node.children[0]->children[1]->getType();
            symbolTable[name] = make_pair(type, 4* (count - i + 1));
        } 
        else if (node.getChildrenSize() == 1) {
            count++;
            string name = node.children[0]->children[1]->getLexeme();
            string type = node.children[0]->children[1]->getType();
            symbolTable[name] = make_pair(type, 4);
        }
    } 
}

void dclsP(Tree& node,  int r, int &countDeclarations) {
    if (node.getChildrenSize() == 5) {
        auto& childDcl = node.children[0];
        dclsP(*childDcl, countDeclarations, r);
        countDeclarations++;
        string value = node.children[1]->children[1]->getLexeme();
        string type = node.children[1]->children[1]->getType();
        int offset = -4*r -4*(countDeclarations - 1);
        symbolTable[value] = make_pair(type, offset);
        auto& exprChild = node.children[3];
        expr(*exprChild);
        storeOnStackPush(3);
    }
}

void arguements(Tree& node, int& argCount) {
    if (node.getChildrenSize() == 1) {
        auto& child = node.children[0];
        expr(*child);
        storeOnStackPush(3);
        argCount++;
    } else {
        auto& child = node.children[0];
        expr(*child);
        storeOnStackPush(3);
        auto& argChild = node.children[2];
        argCount++;
        arguements(*argChild, argCount);
    }

}

void treeTraversal(Tree& node) {
    if(node.getRule() == "main"){
        cout << ";prologue wain" << endl;
        cout << ".import print" << endl;
        cout << ".import new" << endl;
        cout << ".import delete" << endl;
        cout << ".import init" << endl;
        cout << "lis $4" << endl;
        cout << ".word 4" << endl;
        cout << "lis $10" << endl;
        cout << ".word print" << endl;
        cout << "lis $11" << endl;
        cout << ".word 1" << endl;
        cout << "sub $29, $30, $4" << endl;

        //cout << "; storing first parameter of main on the stack" <<endl;
        symbolTable[node.children[3]->children[1]->getLexeme()] = make_pair(node.children[3]->children[1]->getType(), -4 * symbolTable.size());
        storeOnStackPush(1);

        //cout << "; storing second parameter of main on the stack" << endl;
        symbolTable[node.children[5]->children[1]->getLexeme()] = make_pair(node.children[5]->children[1]->getType(), -4 * symbolTable.size());
        storeOnStackPush(2);
        
        if (node.children[3]->children[1]->getType() != "int*") cout << "add $2, $0, $0" << endl;
        storeOnStackPush(31);
        storeOnStackPush(5);
        cout << "lis $5" << endl;
        cout << ".word init" << endl;
        cout << "jalr $5" << endl;
        loadFromStackPop(5);
        loadFromStackPop(31);

        auto& dclChild = node.children[8]; 
        dcls(*dclChild);

        // print out the symbol table
        //for (const auto& entry : symbolTable) cout << "; Variable Name: " << entry.first << ", Type: " << entry.second.first << ", Offset: " << entry.second.second << endl;

        auto& statementChild = node.children[9];
        statements(*statementChild);

        auto& child = node.children[11]; 
        expr(*child);
        for (int i = 0; i < symbolTable.size(); i++)  cout << "add $30, $30, $4" << endl;
        cout << "jr $31" << endl;
    }
    
    else if (node.getRule() == "procedures") {
        if (node.getChildrenSize() == 2) {
            auto& child = node.children[1];
            treeTraversal(*child);
        }
        auto& child = node.children[0];
        treeTraversal(*child);
    } 
    else if (node.getRule() == "procedure") {
        int count = 0;
        int countDeclarations = 0;
        symbolTable.clear();
        cout << node.children[1]->getLexeme() << ":" << endl;
        cout << "sub $29, $30, $4" << endl;
        
        auto& paramsChild = node.children[3];
        params(*paramsChild, count);
        
        storeOnStackPush(5);
        storeOnStackPush(6);
        storeOnStackPush(7);
        
        auto& dclsChild = node.children[6];
        
        dclsP(*dclsChild, 3, countDeclarations);
        auto& statementChild = node.children[7];
        statements(*statementChild);

        auto& exprChild = node.children[9];
        expr(*exprChild);

        for (int i = 0; i < countDeclarations; i++) loadFromStackPop(5);
        
        loadFromStackPop(5);
        loadFromStackPop(6);
        loadFromStackPop(7);
        cout << "jr $31" << endl;
    }
}

int main() {
    string line;
    vector <string> lines;
    // Reading lines from stdin until EOF
    while (getline(cin, line)) {
        lines.push_back(line);
    }
    if(lines.size() == 0) errorReport("invalid first expression ");
    stringstream ss(lines[0]);
    string start, p, bof;
    ss >> start >> bof >>  p;
    if(start != "start") errorReport("invalid first expression ");
    auto rootNode = make_unique<Tree>(start);
    int index = 0;
    buildTree(*rootNode, lines, index);
    //printTreeTest(*rootNode);
    treeTraversal(*rootNode->children[1]);
    return 0;
}