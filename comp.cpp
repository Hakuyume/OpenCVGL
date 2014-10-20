#include "comp.hpp"

bool CompVector::operator()(const Eigen::Vector3d &a, const Eigen::Vector3d &b)
{
  for (int i = 0; i < 3; i++)
    if (floor(a(i)) < floor(b(i)))
      return true;
    else if (floor(a(i)) > floor(b(i)))
      return false;
  return false;
}

bool CompVector::operator()(const Eigen::Vector3i &a, const Eigen::Vector3i &b)
{
  for (int i = 0; i < 3; i++)
    if (a(i) < b(i))
      return true;
    else if (a(i) > b(i))
      return false;
  return false;
}
