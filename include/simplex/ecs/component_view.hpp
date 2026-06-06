#pragma once
#include <simplex/ecs/sparse_set.hpp>

/// @brief A lazy, filtered range over entities that have ALL of Components...
///        Drives iteration from the smallest pool and skips entities missing any component.
/// @tparam capacity   Must match the capacity the component_sparse_sets were created with.
/// @tparam Components The required component types.
template <size_t capacity, typename... Components>
class component_view
{

    public:
        // Built by the entity_system, which resolves each pool pointer (asserting non-null).
        inline component_view(component_sparse_set<Components, capacity>*... pool_ptrs)
            : pools(pool_ptrs...)
        {

            // Pick the lead (smallest) pool once. dense_to_sparse is entity_t for every
            // pool, so we can iterate any pool's entity span uniformly regardless of which
            // component type ends up being the lead.
            size_t best = static_cast<size_t>(-1);
            ([&]
            {
                const size_t c = pool_ptrs->count();
                if (c < best)
                {
                    best       = c;
                    lead_begin = pool_ptrs->begin();
                    lead_end   = pool_ptrs->end();
                }
            }(), ...);


        }

        class iterator
        {

            public:
                inline iterator(const component_view* parent, const entity_t* cursor)
                    : view(parent), current(cursor)
                {
                    this->advance_to_match();
                }

                inline iterator& operator++()
                {
                    ++this->current;
                    this->advance_to_match();
                    return *this;
                }

                inline bool operator!=(const iterator& other) const { return this->current != other.current; }
                inline bool operator==(const iterator& other) const { return this->current == other.current; }

                // Yields (entity, Components&...). References stay valid until the next
                // structural change to any participating pool.
                inline std::tuple<entity_t, Components&...>
                operator*() const { return this->view->dereference(*this->current); }

            private:
                inline void
                advance_to_match()
                {
                    while (this->current != this->view->lead_end && !this->view->matches(*this->current))
                        ++this->current;
                }

            private:
                const component_view*   view;
                const entity_t*         current;

        };

        inline iterator begin() const { return iterator(this, lead_begin); }
        inline iterator end()   const { return iterator(this, lead_end);   }

    private:
        // True iff e is present in every pool. The lead pool always passes (one redundant,
        // cheap lookup per matched entity); kept for uniformity since the lead is a runtime choice.
        inline bool
        matches(const entity_t& e) const
        {
            return std::apply([&](auto*... ps) { return (ps->contains(e) && ...); }, pools);
        }

        inline std::tuple<entity_t, Components&...>
        dereference(const entity_t& e) const
        {
            return std::apply(
                [&](auto*... ps) -> std::tuple<entity_t, Components&...>
                { return std::tuple<entity_t, Components&...>(e, ps->get(e)...); },
                pools);
        }

    private:
        std::tuple<component_sparse_set<Components, capacity>*...>   pools;
        const entity_t*                                             lead_begin = nullptr;
        const entity_t*                                             lead_end   = nullptr;

};
