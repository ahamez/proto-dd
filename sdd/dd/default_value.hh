#ifndef _SDD_DD_DEFAULT_VALUE_HH_
#define _SDD_DD_DEFAULT_VALUE_HH_

namespace sdd { namespace dd {

/*------------------------------------------------------------------------------------------------*/

template <typename T>
struct default_value
{
  static T value() noexcept(noexcept(T())) {return T();}
};

/*------------------------------------------------------------------------------------------------*/

}} // namespace sdd::dd

#endif // _SDD_DD_DEFAULT_VALUE_HH_
