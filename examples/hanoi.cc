#include <iostream>
#include <fstream>

#include "sdd/sdd.hh"
#include "sdd/tools/dot/sdd.hh"
#include "sdd/tools/nodes.hh"

using conf   = sdd::conf2;
using SDD    = sdd::SDD<conf>;

int
main(int argc, char** argv)
{
  auto manager = sdd::manager<conf>::init();

  {
    const auto x0 = SDD(0, {0}, sdd::one<conf>());
    const auto y0 = SDD(2, {0}, SDD(1, {0}, x0));
    const auto z0 = SDD(2, {1}, SDD(1, {1}, x0));

    std::ofstream filex("/Users/hal/Desktop/x0.dot");
    filex << sdd::tools::dot(x0);
    std::ofstream filey("/Users/hal/Desktop/y0.dot");
    filey << sdd::tools::dot(y0);
    std::ofstream filez("/Users/hal/Desktop/z0.dot");
    filez << sdd::tools::dot(z0);

    std::ofstream file("/Users/hal/Desktop/x0_y0_z0.dot");
    file << sdd::tools::dot(z0 + y0);
  }
  {
    const auto y1 = SDD(2, {0}, SDD(1, {0}, SDD(0, {0}, sdd::one<conf>())));
    const auto z1 = SDD(2, {1}, SDD(1, {1}, SDD(0, {0}, sdd::one<conf>())));

    std::ofstream filey("/Users/hal/Desktop/y1.dot");
    filey << sdd::tools::dot(y1);
    std::ofstream filez("/Users/hal/Desktop/z1.dot");
    filez << sdd::tools::dot(z1);

    std::ofstream filea("/Users/hal/Desktop/y1_z1.dot");
    filea << sdd::tools::dot(y1 + z1);
  }

  return 0;
}
