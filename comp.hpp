#pragma once

#include <Eigen/Eigen>

struct CompVector
{
  bool operator()(const Eigen::Vector3d &a, const Eigen::Vector3d &b) const;
  bool operator()(const Eigen::Vector3i &a, const Eigen::Vector3i &b) const;
};
