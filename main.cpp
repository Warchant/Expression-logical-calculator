/**
 * @author Bogdan Vaneev
 *         Innopolis University
 */


#include <iostream>
#include <string>
#include <regex>
#include <list>
#include <algorithm>
#include <string.h>

using namespace std;

typedef long long ll;

class Expression {
protected:
	Expression* left;
	Expression* right;
	string op;
public:
	virtual ll calculate(){
		throw exception("calculate not implemented");
	};

	virtual string toJSON() {
		throw exception("toJSON not implemented");
	}
};



class Logical : public Expression {
	friend class Parser;
public:
	Logical(string op, Expression* left, Expression* right) {
		this->op = op;
		this->left = left;
		this->right = right;
	}

	static bool check(string op) {
		regex e("(and|or|xor)", regex_constants::icase);
		return regex_match(op, e);
	}

	ll calculate() {
		ll r1 = left->calculate();
		ll r2 = right->calculate();

		// case insensitive comparison
		if (_strcmpi(op.c_str(), "and") == 0) return ((r1 > 0) && (r2 > 0)) ? 1 : 0;
		if (_strcmpi(op.c_str(), "or") == 0)  return ((r1 > 0) || (r2 > 0)) ? 1 : 0;
		if (_strcmpi(op.c_str(), "xor") == 0) return ((bool)r1 ^ (bool)r2) ? 1 : 0;
		throw exception("logical NONE");
	}
};


class Relation : public Expression {
	friend class Parser;
public:
	Relation(string op, Expression* left, Expression* right) {
		this->op = op;
		this->left = left;
		this->right = right;
	}

	static bool check(string op) {
		regex e("(<=|>=|\\/=|!=)|([><=])", regex_constants::icase);
		return regex_match(op, e);
	}

	ll calculate() {
		ll r1 = left->calculate();
		ll r2 = right->calculate();

		if (op == "<")  return r1 < r2  ? 1 : 0;
		if (op == "<=") return r1 <= r2 ? 1 : 0;
		if (op == ">")  return r1 > r2  ? 1 : 0;
		if (op == ">=") return r1 >= r2 ? 1 : 0;
		if (op == "==") return r1 == r2 ? 1 : 0;
		if (op == "!=" ||
			op == "/=") return r1 != r2 ? 1 : 0;
		throw exception("relation NONE");
	}
};


class Factor : public Expression {
	friend class Parser;
public:
	Factor(string op, Expression* left, Expression* right) {
		this->op = op;
		this->left = left;
		this->right = right;
	}

	static bool check(string op) {
		return op == "*" || op == "/";
	}

	ll calculate() {
		ll r1 = left->calculate();
		ll r2 = right->calculate();

		if (op == "*") return r1 * r2;
		if (op == "/") return r1 / r2;
		throw exception("factor NONE");

	}
};


class Term : public Expression {
	friend class Parser;
public:
	Term(string op, Expression* left, Expression* right) {
		this->op = op;
		this->left = left;
		this->right = right;
	}

	static bool check(string op) {
		return op == "+" || op == "-";
	}

	ll calculate() {
		ll r1 = left->calculate();
		ll r2 = right->calculate();

		if (op == "+") return r1 + r2;
		if (op == "-") return r1 - r2;
		throw exception("term NONE");
	}
};


class Primary : public Expression {
	friend class Parser;
};


class Integer : public Primary {
	ll value;
	friend class Parser;
public:
	Integer(ll v) : value(v) {}
	ll calculate() { return value; }
};


class Parentesized : public Primary {
	Expression expression;
	friend class Parser;
public:
	ll calculate() { return expression.calculate(); }
};


class Parser {
private:
	string input;
	list<string> tokens;
	list<string>::iterator token;

	void tokenize(string s) {
		regex token("(and|or|xor)|(<=|>=|\\/=|!=)|([><=])|([\\+\\-\\*\\/])|([0-9]+)|([()])", regex_constants::icase);
		smatch m;

		while (regex_search(s, m, token)) {
			tokens.push_back(m.begin()->str());
			s = m.suffix().str();
		}

		if (tokens.size() == 0) throw exception("wrong syntax");
	}


	Expression* parseLogical() {
		Expression* left = parseRelation();
		while (true) {
			string op;
			if (token != tokens.end()) op = *token;

			if (Logical::check(op)) {
				token++;
				Expression* right = parseRelation();
				left = new Logical(op, left, right);
			}
			else {
				break;
			}
		}
		return left;
	}

	Expression* parseRelation() {
		Expression* left = parseTerm();
		while (true) {
			string op;
			if (token != tokens.end()) op = *token;

			if (Relation::check(op)) {
				token++;
				Expression* right = parseTerm();
				left = new Relation(op, left, right);
			}
			else {
				break;
			}
		}
		return left;
	}

	Expression* parseTerm() {
		Expression* left = parseFactor();
		while (true) {
			string op;
			if (token != tokens.end()) op = *token;

			if (Term::check(op)) {
				token++;
				Expression* right = parseFactor();
				left = new Term(op, left, right);
			}
			else {
				break;
			}
		}
		return left;
	}

	Expression* parseFactor() {
		Expression* left = parsePrimary();
		while (true) {
			string op;
			if (token!=tokens.end()) op = *token;

			if (Factor::check(op)) {
				token++;
				Expression* right = parsePrimary();
				left = new Factor(op, left, right);
			}
			else {
				break;
			}
		}
		return left;
	}

	Expression* parsePrimary() {
		Expression* result;
		try {
			// number
			result = new Integer(stoll(*token));
			token++;
		}
		catch (const std::invalid_argument& e) {
			// not a number
			if (*token == "(") {
				token++;
				result = parse();
				token++; // skip ")"
			}
			else {
				throw exception("syntax error (primary)");
			}
		}
		return result;
	}


public:
	Parser(string s) :input(s) {
		// get tokens
		tokenize(s);
		// start token
		token = tokens.begin();
	}

	Expression* parse() { return parseLogical(); }
};


void main() {
	try {
		string input = "555/5 + 1 -100";
		Parser *p = new Parser(input);
		Expression* tree = p->parse();
		ll result = tree->calculate();

		cout << "Result is: " << result << endl;

	}
	catch (const exception& e) {
		cout << e.what() << endl;
	}
	system("pause");
}