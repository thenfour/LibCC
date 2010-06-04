


#include "stdafx.h"
#define LIBCC_PARSE_TRACE_ENABLED 1
#include "../libcc/parse.hpp"
#include <memory>

#include <iostream>

using namespace LibCC::Parse;

struct Node;
typedef std::tr1::shared_ptr<Node> NodePtr;

struct Node
{
	enum Type
	{
		NodeNumber,
		NodeNAryExpression,
		NodeUnaryExpression,
		NodeFunctionCall
	};

	enum Operator
	{
		OpAdd,// n-ary
		OpSubtract,// n-ary
		OpMultiply,// n-ary
		OpDivide,// n-ary
		OpMod,// n-ary but that would be weird
		OpAssign,
		OpBitNot,
		OpBitAnd,
		OpBitOr,
		OpBitXor,
		OpBoolAnd,
		OpBoolOr,
		OpBoolNot,// unary
		OpBoolEq,
		OpBoolNe,
		OpBoolLt,// binary only
		OpBoolLte,// binary only
		OpBoolGt,// binary only
		OpBoolGte// binary only
	};

	std::string ToString(Operator op)
	{
		switch(op)
		{
		case OpAdd: return "OpAdd";
		case OpSubtract: return "OpSubtract";
		case OpMultiply: return "OpMultiply";
		case OpDivide: return "OpDivide";
		case OpMod: return "OpMod";
		case OpAssign: return "OpAssign";
		case OpBitNot: return "OpBitNot";
		case OpBitAnd: return "OpBitAnd";
		case OpBitOr: return "OpBitOr";
		case OpBitXor: return "OpBitXor";
		case OpBoolAnd: return "OpBoolAnd";
		case OpBoolOr: return "OpBoolOr";
		case OpBoolNot: return "OpBoolNot";
		case OpBoolEq: return "OpBoolEq";
		case OpBoolNe: return "OpBoolNe";
		case OpBoolLt: return "OpBoolLt";
		case OpBoolLte: return "OpBoolLte";
		case OpBoolGt: return "OpBoolGt";
		case OpBoolGte: return "OpBoolGte";
		}
		return "Unknown operator";
	}

	Type type;
	Operator op;
	double val;
	std::vector<NodePtr> children;

	void Dump(int indentLevel = 0)
	{
		std::string indent(indentLevel, ' ');

		switch(type)
		{
		case NodeNumber:
			std::cout << indent << "Number: " << val << std::endl;
			break;
		case NodeNAryExpression:
			std::cout << indent << "NAryExpression(" << ToString(op) << ")" << std::endl;
			std::cout << indent << "{" << std::endl;
			for (size_t i = 0; i < children.size(); i++)
			{
				children[i]->Dump(indentLevel + 1);
			}
			std::cout << indent << "}" << std::endl;
			break;
		case NodeUnaryExpression:
			std::cout << indent << "Unary expression: " << val << std::endl;
			std::cout << indent << "->op: " << static_cast<int>(op) << std::endl;
			break;
		case NodeFunctionCall:
			std::cout << indent << "function call: " << std::endl;
			break;
		}
	}
};

// UNARY EXPRESSION
//   !operator + numeric literal
// OPERAND
//   unary | ("(" + expression + ")")
// BINARY EXPRESSION is
//   operand + operator + operand
// EXPRESSION is
//   binary | unary
template<typename Toutput>
struct UnaryExpressionT : public ParserWithOutput<Toutput, UnaryExpressionT<Toutput> >
{
public:
	UnaryExpressionT(const Toutput& output_) : ParserWithOutput(output_) { }

	std::wstring GetParserName() const { return L"UnaryExpressionT"; }

	bool Parse(ParseResult& result, ScriptReader& input)
	{
		wchar_t oper = 0;
		bool operExists = false;
		double val;

		Parser p =
			(
				!CharOf(L"!", RefExistsOutput(operExists, oper))
				+ Rational<double>(RefOutput(val))
			);

		if (!p.ParseRetainingStateOnError(result, input))
			return false;

		BackupOutput(input);

		NodePtr number(new Node());
		number->type = Node::NodeNumber;
		number->val = val;

		NodePtr outp(number);

		if(operExists)
		{
			switch(oper)
			{
			case '!':
				{
					NodePtr parent(new Node());
					parent->type = Node::NodeUnaryExpression;
					parent->op = Node::OpBoolNot;
					parent->children.push_back(number);
					outp = parent;
					break;
				}
			default:
				// ERROR
				break;
			}
		}

		output.Save(input, outp);
		return true;
	}
};

template<typename Toutput>
UnaryExpressionT<Toutput> UnaryExpression(Toutput& output)
{
	return UnaryExpressionT<Toutput>(output);
}


// OPERAND
//   unary | ("(" + expression + ")")
// this takes care of order-of-operations regarding parenthesis automatically
template<typename TExpression, typename Toutput>// struct Expression is not yet known, so make it a template var as a workaround.
struct OperandT : public ParserWithOutput<Toutput, OperandT<TExpression, Toutput> >
{
public:
	OperandT(const Toutput& output_) : ParserWithOutput(output_) { }

	std::wstring GetParserName() const { return L"OperandT"; }

	bool Parse(ParseResult& result, ScriptReader& input)
	{
		NodePtr outp;

		Parser p =
			(
				UnaryExpression(RefOutput(outp))
				|
				(Char('(') + TExpression(RefOutput(outp)) + Char(')'))
			);

		if (!p.ParseRetainingStateOnError(result, input))
			return false;

		BackupOutput(input);
		output.Save(input, outp);
		return true;
	}
};

template<typename TExpression, typename Toutput>
OperandT<TExpression, Toutput> Operand(Toutput& output)
{
	return OperandT<TExpression, Toutput>(output);
}




// N-ARY EXPRESSION:
//   operand + *( operator + operand )
// here we will develop a list of operands and operators, and we can place them in order of operations and group them as we please.
template<typename Toutput>
struct NAryExpressionT : public ParserWithOutput<Toutput, NAryExpressionT<Toutput> >
{
public:
	NAryExpressionT(const Toutput& output_) : ParserWithOutput(output_) { }

	std::wstring GetParserName() const { return L"NAryExpressionT"; }

	Node::Operator ParseOperator(wchar_t ch)
	{
		switch(ch)
		{
		case '+':
			return Node::OpAdd;
		case '-':
			return Node::OpSubtract;
		case '/':
			return Node::OpDivide;
		case '*':
			return Node::OpMultiply;
		case '%':
			return Node::OpMod;
		}
	return Node::OpAdd;
	}

	int GetOperatorPrecedence(Node::Operator op)
	{
		switch(op)
		{
		case Node::OpAdd:
		case Node::OpSubtract:
			return 6;
		case Node::OpDivide:
		case Node::OpMultiply:
		case Node::OpMod:
			return 5;
		}
		return 0;// ERROR;
	}

	// inserts the current operands into the tree. a single node is always returned.
	// technically speaking, since this is a recursive approach, we're looking at potential stack overflow.
	NodePtr GenerateTree(bool consumeFirstOperand, const std::vector<NodePtr>& operands, std::vector<NodePtr>::const_iterator& itoperand, const std::vector<wchar_t>& opers, std::vector<wchar_t>::const_iterator& itoper)
	{
		if(itoper == opers.end())
		{
			// no operators remaining; we're at the end of the list. this also hits if there is only a single unary expression in the entire expression.
			// assert(operands.size() == 1);// will always be true on a successful parse.
			return operands.front();
		}

		NodePtr thisOpNode(new Node());
		thisOpNode->type = Node::NodeNAryExpression;
		thisOpNode->op = ParseOperator(*itoper);
		if(consumeFirstOperand)
		{
			thisOpNode->children.push_back(*itoperand);
			++ itoperand;
		}
		int thisOpPrecedence = GetOperatorPrecedence(thisOpNode->op);

		++ itoper;

		// group like items in sequence
		// the moment we hit a different operator, push a new node & recurse. parent/child relationship is then determined by oper precedence.
		// so 2+3 will turn into add(1,2)
		// 2+3*4 = add(2,mul(3,4))
		// 2+3*4+5 = add(2,add(mul(3,4),5)) - not 100% optimally organized but it's simple and always works
		while(true)
		{
			if(itoperand == operands.end())
				return thisOpNode;

			if(itoper == opers.end())
			{
				// no more operators. that means *itoperand must be the rhs to this
				thisOpNode->children.push_back(*itoperand);
				return thisOpNode;
			}

			// compare operators
			Node::Operator nextOp = ParseOperator(*itoper);
			if(nextOp == thisOpNode->op)
			{
				// chain it up
				thisOpNode->children.push_back(*itoperand);
			}
			else
			{
				int nextOpPrecedence = GetOperatorPrecedence(nextOp);
				if(thisOpPrecedence >= nextOpPrecedence)
				{
					// e.g. we are add and they are mul. we are the parent, and the next operand is THEIRS
					// 2 + 3 * 4
					//       ^    - itoper
					//     ^      - itoperand
					NodePtr next = GenerateTree(true, operands, itoperand, opers, itoper);
					thisOpNode->children.push_back(next);
					return thisOpNode;
				}

				// e.g. we are mul and next is add. we are the child
				// and the next operand is OURS
				// 2 * 3 + 4
				//       ^    - itoper
				//     ^      - itoperand

				// so first consume that operand that's ours.
				thisOpNode->children.push_back(*itoperand);
				++ itoperand;
				// 2 * 3 + 4
				//       ^    - itoper
				//         ^  - itoperand
				NodePtr next = GenerateTree(false, operands, itoperand, opers, itoper);
				next->children.push_back(thisOpNode);
				return next;
			}

			++ itoperand;
			++ itoper;
		}
	}

	bool Parse(ParseResult& result, ScriptReader& input)
	{
		std::vector<NodePtr> operands;
		std::vector<wchar_t> opers;

		Parser p =
			(
				Operand<NAryExpressionT<Toutput> >(InserterOutput<NodePtr>(operands, std::back_inserter(operands))) +
				*
				(
					CharOf(L"+-/*%", InserterOutput<wchar_t>(opers, std::back_inserter(opers)))
					+ Operand<NAryExpressionT<Toutput> >(InserterOutput<NodePtr>(operands, std::back_inserter(operands)))
				)
			);

		if (!p.ParseRetainingStateOnError(result, input))
			return false;
		
		// assert(opers.size() == (operands.size() - 1))
		// assert(operands.size() != 0);// this would be a failed parse.

		NodePtr root = GenerateTree(true, operands, operands.begin(), opers, opers.begin());

		BackupOutput(input);
		output.Save(input, root);
		return true;
	}
};

template<typename Toutput>
NAryExpressionT<Toutput> NAryExpression(Toutput& output)
{
	return NAryExpressionT<Toutput>(output);
}


NodePtr ParseExpression(const std::wstring& exp)
{
	NodePtr node;
	ParseResultMem result;
	result.SetTraceEnabled(true);
	NAryExpression(RefOutput(node)).Parse(result, BasicStringReader(exp));

	//for(std::vector<std::wstring>::const_iterator it = result.parseMessages.begin(); it != result.parseMessages.end(); ++ it)
	//{
	//	std::wcout << *it << std::endl;
	//	OutputDebugString(it->c_str());
	//	OutputDebugString(L"\r\n");
	//}

	//for(std::vector<std::wstring>::const_iterator it = result.trace.begin(); it != result.trace.end(); ++ it)
	//{
	//	std::wcout << *it << std::endl;
	//	OutputDebugString(it->c_str());
	//	OutputDebugString(L"\r\n");
	//}

	return node;
}

int _tmain(int argc, _TCHAR* argv[])
{
	while (true)
	{
		wchar_t buf[512];
		std::wcin.getline(buf, 512);

		NodePtr result = ParseExpression(buf);
		result->Dump();
	}

	return 0;
}
