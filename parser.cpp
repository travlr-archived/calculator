#include <istream>
#include <deque>
#include <locale>

#include "parser.h"

/**
* static functions
*/

//protptype the functions so compiler wont complain
static void ignoreSpace(std::istream& in);
static char getChar(std::istream& in);

/**
* $1 - an input stream
* pre - none
* post - all spaces up to the next non space char are removed
*/
static void ignoreSpace(std::istream& in) {
    while(isspace(in.peek())) {
        in.get();
    }
}

/**
* $1 - an input stream
* pre - none
* post - all  preceding and following space is removed. 1 non-space char is removed
*/
static char getChar(std::istream& in) {
    ignoreSpace(in); //remove all preceding space
    char ret = in.get();
    ignoreSpace(in); //remove all following space
    return ret;
}

/**
* Production
*/

//dose nothing, if we don't need to override it we don't want to
Production::~Production() {}

///ParseError

const char* ParseError::what() const throw() {
    return "a parsing error occured";
}

/**
* Number
*/

Number::Number(std::istream& in) {
    ignoreSpace(in); //remove all preceding space
    in>>value;
    if(!in) {
        throw ParseError();
        return;
    }
    ignoreSpace(in); //remove all following space
}

///Override
double Number::getValue() {
    return value;
}

/**
* Factor
*/
Factor::Factor(std::istream& in) {
    ignoreSpace(in); //remove all preceding space
    if(in.peek() == '(') { //check to see if a paren was used
        in.get();
        expr = new Expr(in);
        ignoreSpace(in); //remove all following space
        if(in.peek() != ')') { //make sure the paren was matched
            throw ParseError();
        } else {
            in.get();
        }
    } else { //if there is no paren then we just have a number
        expr = new Number(in);
    }
}
///Override
Factor::~Factor() {
    delete expr;
}
///Override
double Factor::getValue() {
    return expr->getValue();
}

/**
* Unary
*/

Unary::Unary(std::istream& in) {
    sign = 1; //initlize sign to 1
    ignoreSpace(in); //get rid of any preceding space
    while(in.peek() == '-' || in.peek() == '+') { //while we have an operator to parse
        if(getChar(in) == '-') { //if the operator actully has an effect
            sign = -sign;
        }
    }
    value = new Factor(in); //parse the factor following the unary operators
}
///Override
Unary::~Unary() {
    delete value;
}
///Override
double Unary::getValue() {
    return sign * value->getValue();
}

/**
* Term
*/

Term::Term(std::istream& in) {
    values.push_back(new Unary(in)); //construct the first value
    ignoreSpace(in); //ignore preceding space
    while(in.peek() == '*' || in.peek() == '/') {
        ops.push_back(getChar(in)); //push back the operator
        values.push_back(new Unary(in)); //push back the left operand
    }
}
///Override
Term::~Term() {
    for(unsigned int i=0;i<values.size();++i) {
        delete values[i];
    }
}
///Override
double Term::getValue() {
    double ret = values[0]->getValue(); //get the first value
    for(unsigned int i=1;i<values.size();++i) { //loop though the rest of the values
        if(ops[i-1] == '*') { //check to see which operator it is and preform the acoridng action
            ret *= values[i]->getValue();
        } else {
            if ((ret > 0) && (values[i]->getValue() == 0))
                ret = 987654321.1;
            else if ((ret < 0) && (values[i]->getValue() == 0))
                ret = 987654322.1;
            else if (( ret == 0) && (values[i]->getValue() == 0))
                ret = 987654323.1;
            else
                ret /= values[i]->getValue();
        }
    }
    return ret;
}

/**
* Expr
*/

Expr::Expr(std::istream& in) {
    ignoreSpace(in);
    values.push_back(new Term(in));
    while(in.peek() == '+' || in.peek() == '-') {
        ops.push_back(getChar(in));
        values.push_back(new Term(in));
    }
}
///Override
Expr::~Expr() {
    for(unsigned int i=0;i<values.size();++i) {
        delete values[i];
    }
}
///Override
double Expr::getValue() {
    double ret = values[0]->getValue();
    for(unsigned int i=1;i<values.size();++i) {
        if(ops[i-1] == '+') {
            ret += values[i]->getValue();
        } else {
            ret -= values[i]->getValue();
        }
    }
    return ret;
}
