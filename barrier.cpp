#include "barrier.hpp"

size_t Barrier::size(void)
{
  std::unique_lock<std::mutex> lock{mutex};
  return threads;
}

size_t Barrier::size(size_t size)
{
  std::unique_lock<std::mutex> lock{mutex};
  threads = size;
  return threads;
}

void Barrier::wait(void)
{
  std::unique_lock<std::mutex> lock{mutex};

  count++;

  unsigned int target = count + threads - count % threads;

  if (count % threads == 0)
    cond.notify_all();
  else
    cond.wait(lock, [this, &target] { return count >= target; });
}
