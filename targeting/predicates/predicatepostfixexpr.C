#include <predicatepostfixexpr.H>

#include <cassert>

namespace TARGETING
{

PredicatePostfixExpr&
    PredicatePostfixExpr::push(std::shared_ptr<PredicateBase> predicate)
{
    _ops.emplace_back(std::move(predicate));
    return *this;
}

PredicatePostfixExpr& PredicatePostfixExpr::And()
{
    _ops.emplace_back(LogicalOp::And);
    return *this;
}

PredicatePostfixExpr& PredicatePostfixExpr::Or()
{
    _ops.emplace_back(LogicalOp::Or);
    return *this;
}

PredicatePostfixExpr& PredicatePostfixExpr::Not()
{
    _ops.emplace_back(LogicalOp::Not);
    return *this;
}

bool PredicatePostfixExpr::evalStackItem(uintptr_t& item,
                                         const TargetPtr& target) const
{
    if (item <= 1)
        return item; // already evaluated
    auto* pred = reinterpret_cast<PredicateBase*>(item);
    bool result = (*pred)(target);
    item = result ? 1 : 0;
    return result;
}

bool PredicatePostfixExpr::operator()(const TargetPtr& target) const
{
    assert(target != nullptr);
    std::vector<uintptr_t> stack;

    for (const auto& op : _ops)
    {
        if (std::holds_alternative<std::shared_ptr<PredicateBase>>(op))
        {
            auto& pred = std::get<std::shared_ptr<PredicateBase>>(op);
            stack.push_back(reinterpret_cast<uintptr_t>(pred.get()));
        }
        else
        {
            LogicalOp logic = std::get<LogicalOp>(op);
            switch (logic)
            {
                case LogicalOp::And:
                {
                    assert(stack.size() >= 2);
                    auto rhs = stack.back();
                    stack.pop_back();
                    auto& lhs = stack.back();
                    if (!evalStackItem(lhs, target))
                    {
                        lhs = 0;
                    }
                    else
                    {
                        bool r = evalStackItem(rhs, target);
                        lhs = (lhs && r);
                    }
                    break;
                }
                case LogicalOp::Or:
                {
                    assert(stack.size() >= 2);
                    auto rhs = stack.back();
                    stack.pop_back();
                    auto& lhs = stack.back();
                    if (evalStackItem(lhs, target))
                    {
                        lhs = 1;
                    }
                    else
                    {
                        bool r = evalStackItem(rhs, target);
                        lhs = (lhs || r);
                    }
                    break;
                }
                case LogicalOp::Not:
                {
                    assert(stack.size() >= 1);
                    auto& top = stack.back();
                    bool r = evalStackItem(top, target);
                    top = !r;
                    break;
                }
                default:
                    assert(false);
            }
        }
    }

    if (stack.empty())
        return true;

    assert(stack.size() == 1);
    return evalStackItem(stack.back(), target);
}

} // namespace TARGETING
