#ifndef _SDD_INTERNAL_MANAGER_HH_
#define _SDD_INTERNAL_MANAGER_HH_

#include <cassert>

#include <boost/container/flat_set.hpp>

#include "sdd/internal_manager_fwd.hh"
#include "sdd/dd/context.hh"
#include "sdd/dd/definition.hh"
#include "sdd/hom/context.hh"
#include "sdd/hom/definition.hh"
#include "sdd/hom/identity.hh"
#include "sdd/mem/cache.hh"
#include "sdd/mem/unique_table.hh"

namespace sdd {

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Contains global unique tables and caches for SDD and homomorphisms.
template <typename C>
struct internal_manager
{
  // Can't copy an internal_manager.
  internal_manager(const internal_manager&) = delete;
  internal_manager& operator=(const internal_manager&) = delete;

  // Can't move an internal_manager.
  internal_manager(const internal_manager&&) = delete;
  internal_manager& operator=(const internal_manager&&) = delete;

  /// @brief The type of a unified proto environment.
  using proto_env_unique_type = typename dd::proto_env<C, sdd_ptr_type<C>>::unique_type;

  /// @brief The type of a smart pointer to a unified proto environment.
  using proto_env_ptr_type = typename dd::proto_env<C, sdd_ptr_type<C>>::ptr_type;

  /// @brief The type of a unified SDD.
  using sdd_unique_type = typename SDD<C>::unique_type;

  /// @brief The type of a unified homomorphism.
  using hom_unique_type = typename homomorphism<C>::unique_type;

  /// @brief The type of a smart pointer to a unified homomorphism.
  using hom_ptr_type = typename homomorphism<C>::ptr_type;

  /// @brief Manage the handlers needed by ptr when a unified data is no longer referenced.
  struct ptr_handlers
  {
    ptr_handlers( mem::unique_table<proto_env_unique_type>& proto_env_ut
                , mem::unique_table<sdd_unique_type>& sdd_ut
                , mem::unique_table<hom_unique_type>& hom_ut)
    {
      mem::set_deletion_handler<proto_env_unique_type>([&](const proto_env_unique_type& u)
                                                          {proto_env_ut.erase(u);});
      mem::set_deletion_handler<sdd_unique_type>([&](const sdd_unique_type& u){sdd_ut.erase(u);});
      mem::set_deletion_handler<hom_unique_type>([&](const hom_unique_type& u){hom_ut.erase(u);});
    }

    ~ptr_handlers()
    {
      mem::reset_deletion_handler<proto_env_unique_type>();
      mem::reset_deletion_handler<sdd_unique_type>();
      mem::reset_deletion_handler<hom_unique_type>();
    }
  } handlers;

  /// @brief The set of unified proto environments.
  mem::unique_table<proto_env_unique_type> proto_env_unique_table;

  /// @brief The set of unified SDD.
  mem::unique_table<sdd_unique_type> sdd_unique_table;

  /// @brief The SDD operations evaluation context.
  dd::context<C> sdd_context;

  /// @brief The set of unified homomorphisms.
  mem::unique_table<hom_unique_type> hom_unique_table;

  /// @brief The homomorphisms evaluation context.
  hom::context<C> hom_context;

  /// @brief The cached empty proto environment.
  const proto_env_ptr_type empty_proto_env;

  /// @brief The cached |0| terminal.
  const sdd_ptr_type<C> zero;

  /// @brief The cached |1| terminal.
  const sdd_ptr_type<C> one;

  /// @brief The cached Id homomorphism.
  const hom_ptr_type id;

  /// @brief Used to avoid frequent useless reallocations in saturation_fixpoint().
  boost::container::flat_set<homomorphism<C>> saturation_fixpoint_data;

  /// @brief Used by proto_arcs_cache.
  dummy_context dummy_cxt;

  /// @brief Cache the construction of arcs from a proto_node.
  mem::cache<dummy_context, mk_arcs_op<C, sdd_ptr_type<C>>, dummy_error> proto_arcs_cache;

  /// @brief Constructor with a given configuration.
  internal_manager(const C& configuration)
    : handlers(proto_env_unique_table, sdd_unique_table, hom_unique_table)
    , proto_env_unique_table(configuration.sdd_unique_table_size)
    , sdd_unique_table(configuration.sdd_unique_table_size)
    , sdd_context( configuration.sdd_difference_cache_size
                 , configuration.sdd_intersection_cache_size
                 , configuration.sdd_sum_cache_size)
    , hom_unique_table(configuration.hom_unique_table_size)
    , hom_context(configuration.hom_cache_size, sdd_context)
    , empty_proto_env(mk_empty_proto_env())
    , zero(mk_terminal<zero_terminal<C>>())
    , one(mk_terminal<one_terminal<C>>())
    , id(mk_id())
    , saturation_fixpoint_data()
    , dummy_cxt()
    , proto_arcs_cache(dummy_cxt, "mk_arcs_cache", 10000)
  {}

private:

  /// @brief Helper to construct an empty proto environment.
  proto_env_ptr_type
  mk_empty_proto_env()
  {
    char* addr = proto_env_unique_table.allocate(0 /*extra bytes*/);
    proto_env_unique_type* u = new (addr) proto_env_unique_type();
    return proto_env_ptr_type(proto_env_unique_table(u));
  }

  /// @brief Helper to construct terminals.
  template <typename T>
  sdd_ptr_type<C>
  mk_terminal()
  {
    char* addr = sdd_unique_table.allocate(0 /*extra bytes*/);
    sdd_unique_type* u = new (addr) sdd_unique_type(mem::construct<T>());
    return sdd_ptr_type<C>(sdd_unique_table(u));
  }

  /// @brief Helper to construct Id.
  hom_ptr_type
  mk_id()
  {
    char* addr = hom_unique_table.allocate(0 /*extra bytes*/);
    hom_unique_type* u = new (addr) hom_unique_type(mem::construct<hom::_identity<C>>());
    return hom_ptr_type(hom_unique_table(u));
  }
};

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Contains the global (static) internal manager.
/// @related internal_manager
template <typename C>
inline
internal_manager<C>**
global_ptr()
noexcept
{
  static internal_manager<C>* m = nullptr;
  return &m;
}

/*------------------------------------------------------------------------------------------------*/

/// @internal
/// @brief Get the global context as a reference.
/// @related internal_manager
///
/// Make internal calls to the global internal_manager easier.
template <typename C>
inline
internal_manager<C>&
global()
noexcept
{
  assert(*global_ptr<C>() != nullptr && "Uninitialized global internal_manager");
  return **global_ptr<C>();
}

/*------------------------------------------------------------------------------------------------*/

} // namespace sdd

#endif // _SDD_INTERNAL_MANAGER_HH_
