module;
export module AtlasFunctionModule;


import AtlasCore;
import BaseNodeModule;

namespace Atlas
{


namespace AST
{


//============================================================================
export class PrototypeAST {
    String Name;
    String ret_type;
    Vector<std::pair<String, String>> args;

public:
    PrototypeAST(
        const String& Name,
        Vector<std::pair<String, String>> Args,
        String RetType
    ) noexcept:
        Name(Name),
        args(std::move(Args)),
        ret_type(RetType)
    {}

    auto const& getName() const { return Name; }
    auto const& getArgs() const { return args; }
    auto const& getRetType() const { return ret_type; }
};


//============================================================================
export class FunctionAST {
    UniquePtr<PrototypeAST> Proto;
    Vector<UniquePtr<ASTNode>> Body;

public:
    FunctionAST(
        UniquePtr<PrototypeAST> Proto,
        UniquePtr<ASTNode> B
    ) noexcept:
        Proto(std::move(Proto))
    {
        Body.push_back(std::move(B));
    }

    ~FunctionAST() noexcept = default;

    void addSubExpr(UniquePtr<ASTNode> B) { Body.push_back(std::move(B)); }
    auto const& getProto() const { return Proto; }
    auto const& getBody() const { return Body; }
};

}

}
