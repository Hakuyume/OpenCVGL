#pragma once

#include <Eigen/Eigen>

struct CompVector
{
  bool operator()(const Eigen::Vector3d &a, const Eigen::Vector3d &b);
  bool operator()(const Eigen::Vector3i &a, const Eigen::Vector3i &b);
};
