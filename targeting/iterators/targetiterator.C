#include <targeting/iterators/iterators.H>
#include <targeting/targetservice.H>

namespace targeting
{

template <typename T> void _TargetIterator<T>::advance()
{
    static TargetService& l_targetService = targetService();

    if (_current)
    {
        _current = l_targetService.getNextTarget(_current);
        if (_current)
        {
            _current = l_targetService.getNextTarget(_current);
        }
    }
}

// Explicit template instantiations

template class _TargetIterator<Target*>;
template class _TargetIterator<const Target*>;

} // namespace targeting
